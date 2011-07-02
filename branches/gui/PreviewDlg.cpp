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

#include "PreviewDlg.h"

BEGIN_EVENT_TABLE(PreviewDlg, wxDialog)
	EVT_BUTTON(IDC_PREVIEW_BROWSE, PreviewDlg::onBrowse)
END_EVENT_TABLE()

PreviewDlg::PreviewDlg(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	nameLabel = new wxStaticText(this, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0);
	nameLabel->Wrap(-1);
	bSizer1->Add(nameLabel, 0, wxALL, 5);
	
	nameEdit = new wxTextCtrl(this, IDC_PREVIEW_NAME, wxEmptyString, wxDefaultPosition, wxSize(300,-1), 0);
	bSizer1->Add(nameEdit, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	appLabel = new wxStaticText(this, wxID_ANY, _("Application:"), wxDefaultPosition, wxDefaultSize, 0);
	appLabel->Wrap(-1);
	bSizer1->Add(appLabel, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	appEdit = new wxTextCtrl(this, IDC_PREVIEW_APPLICATION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(appEdit, 1, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
	
	appButton = new wxButton(this, IDC_PREVIEW_BROWSE, _("Browse"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(appButton, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	bSizer1->Add(bSizer2, 1, wxEXPAND, 5);
	
	argLabel = new wxStaticText(this, wxID_ANY, _("Arguments (%[file] %[dir] available)"), wxDefaultPosition, wxDefaultSize, 0);
	argLabel->Wrap(-1);
	bSizer1->Add(argLabel, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	argEdit = new wxTextCtrl(this, IDC_PREVIEW_ARGUMENTS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(argEdit, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	extLabel = new wxStaticText(this, wxID_ANY, _("Apply to extensions (avi;mov;divx)"), wxDefaultPosition, wxDefaultSize, 0);
	extLabel->Wrap(-1);
	bSizer1->Add(extLabel, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	extEdit = new wxTextCtrl(this, IDC_PREVIEW_EXTENSION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(extEdit, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton(this, wxID_OK);
	m_sdbSizer1->AddButton(m_sdbSizer1OK);
	m_sdbSizer1Cancel = new wxButton(this, wxID_CANCEL);
	m_sdbSizer1->AddButton(m_sdbSizer1Cancel);
	m_sdbSizer1->Realize();
	bSizer1->Add(m_sdbSizer1, 1, wxEXPAND, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);
}

PreviewDlg::~PreviewDlg()
{
}

void PreviewDlg::onBrowse(wxCommandEvent &WXUNUSED(event))
{
	wxFileDialog openDialog(this);
	if (openDialog.ShowModal() == wxID_OK)
	{
		appEdit->SetValue(openDialog.GetPath());
	}
}

void PreviewDlg::getValues(wxString *name, wxString *app, wxString *arg, wxString *ext)
{
	*name = nameEdit->GetValue();
	*app = appEdit->GetValue();
	*arg = argEdit->GetValue();
	*ext = extEdit->GetValue();
}

void PreviewDlg::setValues(const wxString name, const wxString app, const wxString arg, const wxString ext)
{
	nameEdit->SetValue(name);
	appEdit->SetValue(app);
	argEdit->SetValue(arg);
	extEdit->SetValue(ext);
}