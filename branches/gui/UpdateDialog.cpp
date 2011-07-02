/* 
* Copyright (C) 2010 adrian_007 adrian-007 on o2 point pl
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"
#include "UpdateDialog.h"
#include "WinUtil.h"

#include "../client/version.h"
#include "../client/SimpleXML.h"

BEGIN_EVENT_TABLE(UpdateDialog, wxDialog)
	EVT_BUTTON(wxID_CANCEL, UpdateDialog::onClose)
	EVT_BUTTON(IDC_UPDATE_DOWNLOAD, UpdateDialog::onDownload)
END_EVENT_TABLE()

UpdateDialog::UpdateDialog(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, _("Update"))
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	{
		wxFlexGridSizer* sizer = new wxFlexGridSizer(2, 2, 10, 10);
		sizer->AddGrowableCol(1);
		sizer->SetFlexibleDirection(wxBOTH);
		sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

		ctrlCurrentVersion = new wxStaticText(this, wxID_ANY, VERSIONSTRING, wxDefaultPosition, wxDefaultSize);
		ctrlCurrentVersionLabel = new wxStaticText(this, wxID_ANY, wxT("Current version:"), wxDefaultPosition, wxDefaultSize);
		ctrlLatestVersion = new wxStaticText(this, wxID_ANY, wxT("N/A"), wxDefaultPosition, wxDefaultSize);
		ctrlLatestVersionLabel = new wxStaticText(this, wxID_ANY, wxT("Latest version:"), wxDefaultPosition, wxDefaultSize);

		sizer->Add(ctrlCurrentVersionLabel, 0, wxALIGN_RIGHT, 5);
		sizer->Add(ctrlCurrentVersion, 0, wxALIGN_CENTER_HORIZONTAL, 5);
		sizer->Add(ctrlLatestVersionLabel, 0, wxALIGN_RIGHT, 5);
		sizer->Add(ctrlLatestVersion, 0, wxALIGN_CENTER_HORIZONTAL, 5);

		wxStaticBoxSizer* sBox = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Version")), wxVERTICAL);

		sBox->Add(sizer, 1, wxEXPAND, 5);
		mainSizer->Add(sBox, 0, wxALL | wxEXPAND, 10);
	}

	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

		ctrlStatus = new wxTextCtrl(this, wxID_ANY, wxT("N/A"), wxDefaultPosition, wxSize(-1, -1), wxTE_READONLY);
		ctrlChangelog = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(350, 150), wxTE_READONLY | wxTE_MULTILINE | wxTE_AUTO_URL | wxVSCROLL | wxHSCROLL);

		sizer->Add(ctrlStatus, 0, wxALL | wxEXPAND, 5);
		sizer->Add(ctrlChangelog, 0, wxALL | wxEXPAND, 5);

		wxStaticBoxSizer* sBox = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("History")), wxVERTICAL);

		sBox->Add(sizer, 1, wxEXPAND, 5);
		mainSizer->Add(sBox, 0, wxALL | wxEXPAND, 10);
	}

	{
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		btnClose = new wxButton(this, wxID_CANCEL, CTSTRING(CLOSE));
		btnDownload = new wxButton(this, IDC_UPDATE_DOWNLOAD, CTSTRING(DOWNLOAD));
		btnDownload->Enable(false);

		sizer->Add(btnDownload, 0, 0, 5);
		sizer->AddSpacer(20);
		sizer->Add(btnClose, 0, 0, 5);

		mainSizer->Add(sizer, 1, wxALL | wxCENTER, 10);
	}

	mainSizer->CalcMin();

	SetSizer(mainSizer);
	Layout();
	mainSizer->Fit(this);

	Center();

	hc = new HttpConnection;
	hc->addListener(this);
	hc->downloadFile(VERSION_URL);
}

void UpdateDialog::onClose(wxCommandEvent& /*event*/) {
	if(hc) {
		hc->removeListeners();
		delete hc;
		hc = 0;
	}
	EndModal(wxID_OK);
}

void UpdateDialog::onDownload(wxCommandEvent& /*event*/) {
	if(!downloadURL.empty())
		WinUtil::openLink(Text::toT(downloadURL));
}

void UpdateDialog::on(HttpConnectionListener::Failed, HttpConnection* /*conn*/, const string& aLine) throw() {
	callAsync([this, aLine] { ctrlStatus->SetLabelText(TSTRING(CONNECTION_ERROR) + wxT(": ") + Text::toT(aLine) + wxT("!")); });
}

void UpdateDialog::on(HttpConnectionListener::Complete, HttpConnection* /*conn*/, string const& /*aLine*/, bool /*fromCoral*/) throw() {
	callAsync([this] { ctrlStatus->SetLabelText(TSTRING(DATA_RETRIEVED) + wxT("!")); });
	string sText;
	try {
		double latestVersion;

		SimpleXML xml;
		xml.fromXML(xmldata);
		xml.stepIn();

		if (xml.findChild("Version")) {
			string ver = xml.getChildData();

			callAsync([this, ver] { ctrlLatestVersion->SetLabelText(Text::toT(ver)); });

			latestVersion = Util::toDouble(ver);
			xml.resetCurrentChild();
		} else
			throw Exception();

		if (xml.findChild("URL")) {
			downloadURL = xml.getChildData();
			xml.resetCurrentChild();
			if (latestVersion > VERSIONFLOAT)
				btnDownload->Enable(true);
		} else
			throw Exception();

		while(xml.findChild("Message")) {
			const string& sData = xml.getChildData();
			sText += sData + "\n";
		}
		callAsync([this, sText] { ctrlChangelog->SetLabelText(Text::toT(sText)); });
	} catch (const Exception&) {
		callAsync([this] { ctrlChangelog->SetLabelText(_("Couldn't parse xml-data")); });
	}
}

void UpdateDialog::on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
	if(xmldata.empty()) {
		callAsync([this] { ctrlStatus->SetLabelText(TSTRING(RETRIEVING_DATA) + wxT("...")); });
	}
	xmldata.append(reinterpret_cast<const char*>(buf), len);
}
