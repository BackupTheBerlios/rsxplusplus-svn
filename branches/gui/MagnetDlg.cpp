/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
 * Copyright (C) 2010 Oyashiro-sama, oyashirosama dot hnnkni at gmail dot com
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
#include "../client/DCPlusPlus.h"
#include "WinUtil.h"

#include "MagnetDlg.h"

BEGIN_EVENT_TABLE(MagnetDlg, wxDialog)
	EVT_RADIOBUTTON(IDC_MAGNET_QUEUE, MagnetDlg::onRadioButton)
	EVT_RADIOBUTTON(IDC_MAGNET_SEARCH, MagnetDlg::onRadioButton)
	EVT_RADIOBUTTON(IDC_MAGNET_NOTHING, MagnetDlg::onRadioButton)
	EVT_BUTTON(wxID_OK, MagnetDlg::onOK)
END_EVENT_TABLE()

MagnetDlg::MagnetDlg(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	magnetBitmap = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(32, 32), 0);
	magnetBitmap->SetIcon(wxIcon(_T("IDI_MAGNET"), wxBITMAP_TYPE_ICO_RESOURCE, 32, 32));
	bSizer2->Add(magnetBitmap, 0, wxALL, 5);
	
	detectedLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	detectedLabel->Wrap(260);
	bSizer2->Add(detectedLabel, 0, wxALL, 5);
	
	bSizer1->Add(bSizer2, 0, wxEXPAND | wxALL, 5);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(3, 2, 0, 0);
	fgSizer1->AddGrowableCol(1);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	hashLabel = new wxStaticText(this, IDC_MAGNET_HASH, CTSTRING(MAGNET_DLG_HASH), wxDefaultPosition, wxDefaultSize, 0);
	hashLabel->Wrap(-1);
	fgSizer1->Add(hashLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	hashEdit = new wxTextCtrl(this, IDC_MAGNET_DISP_HASH, wxEmptyString, wxDefaultPosition, wxSize(-1,-1), wxTE_READONLY);
	hashEdit->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	hashEdit->SetMinSize(wxSize(250,-1));
	
	fgSizer1->Add(hashEdit, 0, wxALL | wxEXPAND, 5);
	
	nameLabel = new wxStaticText(this, IDC_MAGNET_NAME, CTSTRING(MAGNET_DLG_FILE), wxDefaultPosition, wxDefaultSize, 0);
	nameLabel->Wrap(-1);
	fgSizer1->Add(nameLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	nameEdit = new wxTextCtrl(this, IDC_MAGNET_DISP_NAME, wxEmptyString, wxDefaultPosition, wxSize(-1,-1), wxTE_READONLY);
	nameEdit->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	nameEdit->SetMinSize(wxSize(250,-1));
	
	fgSizer1->Add(nameEdit, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, 5);
	
	sizeLabel = new wxStaticText(this, IDC_MAGNET_SIZE, CTSTRING(MAGNET_DLG_SIZE), wxDefaultPosition, wxDefaultSize, 0);
	sizeLabel->Wrap(-1);
	fgSizer1->Add(sizeLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	sizeEdit = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1,-1), wxTE_READONLY);
	sizeEdit->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	sizeEdit->SetMinSize(wxSize(250,-1));
	
	fgSizer1->Add(sizeEdit, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, 5);
	
	bSizer1->Add(fgSizer1, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxVERTICAL);
	
	queueRadio = new wxRadioButton(this, IDC_MAGNET_QUEUE, CTSTRING(MAGNET_DLG_QUEUE), wxDefaultPosition, wxDefaultSize, 0);
	queueRadio->SetValue(true); 
	bSizer4->Add(queueRadio, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	searchRadio = new wxRadioButton(this, IDC_MAGNET_SEARCH, CTSTRING(MAGNET_DLG_SEARCH), wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(searchRadio, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	nothingRadio = new wxRadioButton(this, IDC_MAGNET_NOTHING, CTSTRING(MAGNET_DLG_NOTHING), wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(nothingRadio, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	rememberCheck = new wxCheckBox(this, IDC_MAGNET_REMEMBER, CTSTRING(MAGNET_DLG_REMEMBER), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER);
	
	bSizer4->Add(rememberCheck, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	bSizer3->Add(bSizer4, 1, wxEXPAND, 5);
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxVERTICAL);
	
	OK = new wxButton(this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer5->Add(OK, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
	
	Cancel = new wxButton(this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer5->Add(Cancel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
	
	bSizer3->Add(bSizer5, 0, wxEXPAND, 5);
	
	bSizer1->Add(bSizer3, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	Center();
}

MagnetDlg::~MagnetDlg()
{
}

void MagnetDlg::onRadioButton(wxCommandEvent& event)
{
	switch(event.GetId())
	{
		case IDC_MAGNET_QUEUE:
		case IDC_MAGNET_SEARCH:
			if (mSize > 0 && mFileName.length() > 0)
			{
				rememberCheck->Enable();
			}
			break;
		case IDC_MAGNET_NOTHING:
			if (rememberCheck->GetValue())
			{
				rememberCheck->SetValue(false);
			}
			rememberCheck->Enable(false);
			break;
	}
}

void MagnetDlg::onOK(wxCommandEvent &WXUNUSED(event))
{
	if(rememberCheck->GetValue())
	{
		SettingsManager::getInstance()->set(SettingsManager::MAGNET_ASK,  false);
		if (queueRadio->GetValue())
			SettingsManager::getInstance()->set(SettingsManager::MAGNET_ACTION, SettingsManager::MAGNET_AUTO_DOWNLOAD);
		else if (searchRadio->GetValue())
			SettingsManager::getInstance()->set(SettingsManager::MAGNET_ACTION, SettingsManager::MAGNET_AUTO_SEARCH);
	}

	if (searchRadio->GetValue())
	{
		TTHValue tmphash(Text::fromT(mHash));
		WinUtil::searchHash(tmphash); 
	}
	else if (queueRadio->GetValue())
	{
		try
		{
			string target = SETTING(DOWNLOAD_DIRECTORY) + Text::fromT(mFileName);
			QueueManager::getInstance()->add(target, mSize, TTHValue(Text::fromT(mHash)), HintedUser(UserPtr(), Util::emptyString));
		}
		catch(const Exception& e)
		{
			LogManager::getInstance()->message(e.getError(), LogManager::LOG_ERROR);
		}
	}
	EndModal(wxID_OK);
}

void MagnetDlg::setValues(const tstring& aHash, const tstring& aFileName, const int64_t aSize)
{
	mHash = aHash;
	mFileName = aFileName;
	mSize = aSize;

	if (mSize <= 0 || mFileName.length() <= 0)
	{
		queueRadio->Show(false);
		rememberCheck->Show(false);
	}
	detectedLabel->SetLabel(CTSTRING(MAGNET_DLG_TEXT_GOOD));
	detectedLabel->Wrap(260);

	hashEdit->SetValue(mHash.c_str());

	string strFileName = Text::wideToAcp(mFileName);
	if (Text::validateUtf8(strFileName))
		mFileName = Text::toT(strFileName);

	nameEdit->SetValue(mFileName.length() > 0 ? mFileName.c_str() : _T("N/A"));
	char buf[32];
	sizeEdit->SetValue(mSize > 0 ? Text::toT(_i64toa(mSize, buf, 10)).c_str() : _T("N/A"));
		//search->minFileSize > 0 ? _i64toa(search->minFileSize, buf, 10) : ""

	queueRadio->SetValue(true);

	searchRadio->SetFocus();
	this->Layout();
	bSizer1->Fit(this);
}