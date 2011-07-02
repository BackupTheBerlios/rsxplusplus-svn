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
#include "ADLSProperties.h"

#include "../client/ADLSearch.h"

BEGIN_EVENT_TABLE(ADLSProperties, wxDialog)
	EVT_BUTTON(wxID_OK, ADLSProperties::onOk)
END_EVENT_TABLE()

ADLSProperties::ADLSProperties(ADLSearch* search, wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxDialog(parent, wxID_ANY, title, pos, size, style)
{
	this->search = search;

	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	// Search String & Type
	{
		wxFlexGridSizer* sizer = new wxFlexGridSizer(2, 2, 0, 0);
		sizer->AddGrowableCol(1);
		sizer->SetFlexibleDirection(wxBOTH);
		sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

		ctrlSearchString = new wxTextCtrl(this, IDC_SOURCE_TYPE, wxEmptyString, wxDefaultPosition, wxSize(200,-1), 0);
		ctrlSearchStringLabel = new wxStaticText(this, wxID_ANY, _T("Search string"));

		ctrlSearchType = new wxComboBox(this, IDC_ADLSP_TYPE, wxEmptyString, wxDefaultPosition, wxSize(80, -1), 0, 0, wxCB_DROPDOWN | wxCB_READONLY); 
		ctrlSearchTypeLabel = new wxStaticText(this, wxID_ANY, _T("Search type"));

		sizer->Add(ctrlSearchStringLabel, 0, wxALL, 5);
		sizer->Add(ctrlSearchTypeLabel, 0, wxALL, 5);
		sizer->Add(ctrlSearchString, 0, wxALL, 5);
		sizer->Add(ctrlSearchType, 0, wxALL, 5);

		mainSizer->Add(sizer);
	}

	// Size stuff
	{
		wxFlexGridSizer* sizer = new wxFlexGridSizer(2, 3, 0, 0);
		sizer->AddGrowableCol(1);
		sizer->SetFlexibleDirection(wxBOTH);
		sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

		ctrlMinSize = new wxTextCtrl(this, IDC_ADLSP_SIZE_MIN, wxEmptyString, wxDefaultPosition, wxSize(95,-1), 0);
		ctrlMinSizeLabel = new wxStaticText(this, wxID_ANY, _T("Min size"));

		ctrlMaxSize = new wxTextCtrl(this, IDC_ADLSP_SIZE_MAX, wxEmptyString, wxDefaultPosition, wxSize(95,-1), 0);
		ctrlMaxSizeLabel = new wxStaticText(this, wxID_ANY, _T("Max size"));

		ctrlSizeType = new wxComboBox(this, IDC_SIZE_TYPE, wxEmptyString, wxDefaultPosition, wxSize(80, -1), 0, 0, wxCB_DROPDOWN | wxCB_READONLY); 
		ctrlSizeTypeLabel = new wxStaticText(this, wxID_ANY, _T("Size type"));

		sizer->Add(ctrlMinSizeLabel, 0, wxALL, 5);
		sizer->Add(ctrlMaxSizeLabel, 0, wxALL, 5);
		sizer->Add(ctrlSizeTypeLabel, 0, wxALL, 5);

		sizer->Add(ctrlMinSize, 0, wxALL, 5);
		sizer->Add(ctrlMaxSize, 0, wxALL, 5);
		sizer->Add(ctrlSizeType, 0, wxALL, 5);

		mainSizer->Add(sizer);
	}

	// Destination dir & checkboxes
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* v1Sizer = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer* v2Sizer = new wxBoxSizer(wxVERTICAL);

		ctrlDestinationDir = new wxTextCtrl(this, IDC_ADLSP_DESTINATION, wxEmptyString, wxDefaultPosition, wxSize(200,-1), 0);
		ctrlDestinationDirLabel = new wxStaticText(this, wxID_ANY, _T("Destination Directory"));

		ctrlEnabled = new wxCheckBox(this, IDC_IS_ACTIVE, _T("Enabled"));
		ctrlDownloadMatches = new wxCheckBox(this, IDC_AUTOQUEUE, _T("Download Matches"));
		ctrlForbidden = new wxCheckBox(this, IDC_IS_FORBIDDEN, _T("Forbidden"));

		v1Sizer->Add(ctrlDestinationDirLabel, 0, wxALL, 5);
		v1Sizer->Add(ctrlDestinationDir, 0, wxALL, 5);

		v2Sizer->Add(ctrlEnabled, 0, wxALL, 5);
		v2Sizer->Add(ctrlDownloadMatches, 0, wxALL, 5);
		v2Sizer->Add(ctrlForbidden, 0, wxALL, 5);

		sizer->Add(v1Sizer, 0, wxLEFT, 0);
		sizer->Add(v2Sizer, 0, wxRIGHT, 0);

		mainSizer->Add(sizer);
	}

	// Raw action
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

		ctrlRaw = new wxComboBox(this, IDC_ADLSEARCH_RAW_ACTION, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0, 0, wxCB_DROPDOWN | wxCB_READONLY); 
		ctrlRawLabel = new wxStaticText(this, wxID_ANY, _T("Raw"));
	
		sizer->Add(ctrlRawLabel, 0, wxALL, 5);
		sizer->Add(ctrlRaw, 0, wxALL, 5);

		mainSizer->Add(sizer, 0, wxEXPAND, 0);
	}

	// OK/Cancel buttons
	{
		wxStdDialogButtonSizer* sizer = new wxStdDialogButtonSizer();
		btnOK = new wxButton(this, wxID_OK, CTSTRING(OK));
		btnCancel = new wxButton(this, wxID_CANCEL, CTSTRING(CANCEL));

		sizer->AddButton(btnOK);
		sizer->AddButton(btnCancel);
		sizer->Realize();

		mainSizer->Add(sizer, 5, wxBOTTOM | wxCENTER, 5);
	}

	SetSizer(mainSizer);
	Layout();
	mainSizer->Fit(this);

	Center();

	wxArrayString lst;
	lst.Add(CTSTRING(FILENAME));
	lst.Add(CTSTRING(DIRECTORY));
	lst.Add(CTSTRING(ADLS_FULL_PATH));

	ctrlSearchType->Append(lst);

	lst.Clear();
	lst.Add(CTSTRING(B));
	lst.Add(CTSTRING(KB));
	lst.Add(CTSTRING(MB));
	lst.Add(CTSTRING(GB));
	
	ctrlSizeType->Append(lst);

	lst.Clear();
	lst.Add("No Action");
	lst.Add("Raw 1");
	lst.Add("Raw 2");
	lst.Add("Raw 3");
	lst.Add("Raw 4");
	lst.Add("Raw 5");

	ctrlRaw->Append(lst);

	if(search) {
		ctrlSearchString->WriteText(search->searchString.c_str());
		ctrlDestinationDir->WriteText(search->destDir.c_str());
		if(search->maxFileSize > 0)
			ctrlMaxSize->WriteText(Util::toString(search->maxFileSize).c_str());
		if(search->minFileSize > 0)
			ctrlMinSize->WriteText(Util::toString(search->minFileSize).c_str());

		ctrlEnabled->SetValue(search->isActive);
		ctrlDownloadMatches->SetValue(search->isAutoQueue);
		ctrlForbidden->SetValue(search->isForbidden);

		ctrlSearchType->SetSelection(search->sourceType);
		ctrlSizeType->SetSelection(search->typeFileSize);
		ctrlRaw->SetSelection(search->raw);
	}


}

void ADLSProperties::onOk(wxCommandEvent& /*event*/) {
	search->searchString = ctrlSearchString->GetValue().char_str();
	search->destDir = ctrlDestinationDir->GetValue().char_str();

	wxString value;

	value = ctrlMaxSize->GetValue();
	if(!value.IsEmpty()) {
		value.ToLongLong(&search->maxFileSize);
	} else {
		search->maxFileSize = -1;
	}

	value = ctrlMinSize->GetValue();
	if(!value.IsEmpty()) {
		value.ToLongLong(&search->minFileSize);
	} else {
		search->minFileSize = -1;
	}

	search->sourceType = (ADLSearch::SourceType)ctrlSearchType->GetSelection();
	search->typeFileSize = (ADLSearch::SizeType)ctrlSizeType->GetSelection();
	search->raw = ctrlRaw->GetSelection();

	search->isActive = ctrlEnabled->GetValue();
	search->isAutoQueue = ctrlDownloadMatches->GetValue();
	search->isForbidden = ctrlForbidden->GetValue();

	EndModal(wxID_OK);
}
