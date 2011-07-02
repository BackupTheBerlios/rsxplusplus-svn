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
#include "GetTTHDialog.h"

#include "WinUtil.h"

BEGIN_EVENT_TABLE(GetTTHDialog, wxDialog)
	EVT_HYPERLINK(IDC_GETTTH_TTH_LINK, GetTTHDialog::onTTHLink)
	EVT_HYPERLINK(IDC_GETTTH_MAGNET_LINK, GetTTHDialog::onMagnetLink)
END_EVENT_TABLE()

GetTTHDialog::GetTTHDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	fileNameLabel = new wxStaticText(this, wxID_ANY, _("File name:"), wxDefaultPosition, wxDefaultSize, 0);
	fileNameLabel->Wrap(-1);
	bSizer1->Add(fileNameLabel, 0, wxALL, 5);
	
	fileNameEdit = new wxTextCtrl(this, IDC_GETTTH_FILE_EDIT, wxEmptyString, wxDefaultPosition, wxSize(350,-1), wxTE_READONLY);
	bSizer1->Add(fileNameEdit, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	tthLabel = new wxStaticText(this, wxID_ANY, _("TTH:"), wxDefaultPosition, wxDefaultSize, 0);
	tthLabel->Wrap(-1);
	bSizer2->Add(tthLabel, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	tthLink = new wxHyperlinkCtrl(this, IDC_GETTTH_TTH_LINK, _("Copy TTH to clipboard"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_ALIGN_RIGHT|wxNO_BORDER);
	bSizer2->Add(tthLink, 1, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	bSizer1->Add(bSizer2, 0, wxEXPAND, 5);
	
	tthEdit = new wxTextCtrl(this, IDC_GETTTH_TTH_EDIT, wxEmptyString, wxDefaultPosition, wxSize(350,-1), wxTE_READONLY);
	bSizer1->Add(tthEdit, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);
	
	magnetLabel = new wxStaticText(this, wxID_ANY, _("Magnet link:"), wxDefaultPosition, wxDefaultSize, 0);
	magnetLabel->Wrap(-1);
	bSizer3->Add(magnetLabel, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	magnetLink = new wxHyperlinkCtrl(this, IDC_GETTTH_MAGNET_LINK, _("Copy magnet link to clipboard"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_ALIGN_RIGHT|wxNO_BORDER);
	bSizer3->Add(magnetLink, 1, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	bSizer1->Add(bSizer3, 0, wxEXPAND, 5);
	
	magnetEdit = new wxTextCtrl(this, IDC_GETTTH_MAGNET_EDIT, wxEmptyString, wxDefaultPosition, wxSize(350,50), wxTE_MULTILINE|wxTE_NO_VSCROLL|wxTE_READONLY);
	bSizer1->Add(magnetEdit, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton(this, wxID_OK);
	m_sdbSizer1->AddButton(m_sdbSizer1OK);
	m_sdbSizer1->Realize();
	bSizer1->Add(m_sdbSizer1, 1, wxEXPAND|wxBOTTOM|wxLEFT, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);
}

GetTTHDialog::~GetTTHDialog()
{
}

void GetTTHDialog::fillValues(const tstring& filename, const tstring& tth, const tstring& magnet)
{
	fileNameEdit->SetValue(filename);
	tthEdit->SetValue(tth);
	magnetEdit->SetValue(magnet);
}

void GetTTHDialog::onTTHLink(wxHyperlinkEvent &WXUNUSED(event))
{
	WinUtil::setClipboard(tthEdit->GetValue());
}

void GetTTHDialog::onMagnetLink(wxHyperlinkEvent &WXUNUSED(event))
{
	WinUtil::setClipboard(magnetEdit->GetValue());
}