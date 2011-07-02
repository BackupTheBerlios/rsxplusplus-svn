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

#include "WinUtil.h"
#include "../client/version.h"

#include "AboutDlg.h"

AboutDlg::AboutDlg(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, wxID_ANY, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxEmptyString), wxHORIZONTAL);
	
	logoBitmap = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0);
	logoBitmap->SetIcon(wxIcon(_T("IDI_APPICON")));
	sbSizer1->Add(logoBitmap, 0, wxALL, 5);
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxVERTICAL);
	
	versionLabel = new wxStaticText(this, IDC_VERSION, _T("StrongDC++ v") + wxString(VERSIONSTRING) + _T(" (c) Copyright 2004-2010 Big Muscle\nBased on: DC++ ") +
		wxString(DCVERSIONSTRING) + _T(" (c) Copyright 2001-2010 Jacek Sieka"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	versionLabel->Wrap(-1);
	bSizer3->Add(versionLabel, 0, wxALL | wxEXPAND, 5);
	
	strongLink = new wxHyperlinkCtrl(this, wxID_ANY, _("http://strongdc.sf.net"), wxT("http://strongdc.sf.net"), wxDefaultPosition, wxDefaultSize, wxHL_ALIGN_CENTRE | wxHL_DEFAULT_STYLE);
	bSizer3->Add(strongLink, 0, wxALL | wxEXPAND, 5);

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxHORIZONTAL);
	
	tthLabel = new wxStaticText(this, IDC_TTH, _("TTH:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	tthLabel->Wrap(-1);
	bSizer4->Add(tthLabel, 1, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	tthEdit = new wxTextCtrl(this, wxID_ANY, wxString(WinUtil::tth.c_str()), wxDefaultPosition, wxSize(330,-1), wxTE_READONLY | wxNO_BORDER);
	tthEdit->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	
	bSizer4->Add( tthEdit, 0, 0, 5 );
	
	bSizer3->Add( bSizer4, 1, wxEXPAND, 5 );

	sbSizer1->Add(bSizer3, 1, wxEXPAND, 5);
	
	bSizer1->Add(sbSizer1, 0, wxALL | wxEXPAND, 5);
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Thanks")), wxVERTICAL);
	
	thanksEdit = new wxTextCtrl(this, wxID_ANY, thanks, wxDefaultPosition, wxSize(450,60), wxTE_MULTILINE | wxTE_READONLY | wxNO_BORDER);
	thanksEdit->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	
	sbSizer2->Add(thanksEdit, 0, wxALL, 5);
	
	bSizer1->Add(sbSizer2, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Totals")), wxVERTICAL);
	
	upDownLabel = new wxStaticText(this, IDC_TOTALS, _T("Upload: ") + wxString(Util::formatBytesW(SETTING(TOTAL_UPLOAD))) + _T(", Download: ") + 
		wxString(Util::formatBytesW(SETTING(TOTAL_DOWNLOAD))), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	upDownLabel->Wrap(-1);
	sbSizer3->Add(upDownLabel, 0, wxALL | wxEXPAND, 5);
	
	wxString ratio;
	
	if(SETTING(TOTAL_DOWNLOAD) > 0)
	{
		ratioLabel = new wxStaticText(this, IDC_RATIO, wxString::Format(_T("Ratio (up/down): %.2f"),
			((double)SETTING(TOTAL_UPLOAD)) / ((double)SETTING(TOTAL_DOWNLOAD))), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	}
	else
	{
		ratioLabel = new wxStaticText(this, IDC_RATIO, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	}
	ratioLabel->Wrap(-1);
	sbSizer3->Add(ratioLabel, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	bSizer1->Add(sbSizer3, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton(this, wxID_OK);
	m_sdbSizer1->AddButton(m_sdbSizer1OK);
	m_sdbSizer1->Realize();
	bSizer1->Add(m_sdbSizer1, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	Center();
}

AboutDlg::~AboutDlg()
{
}