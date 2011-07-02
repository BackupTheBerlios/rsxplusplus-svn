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
#include "../client/CryptoManager.h"

#include "SecurityPage.h"

BEGIN_EVENT_TABLE(SecurityPage, wxPanel)
	EVT_BUTTON(IDC_BROWSE_PRIVATE_KEY, SecurityPage::onPrivate)
	EVT_BUTTON(IDC_BROWSE_CERTIFICATE, SecurityPage::onOwn)
	EVT_BUTTON(IDC_BROWSE_TRUSTED_PATH, SecurityPage::onTrusted)
	EVT_BUTTON(IDC_GENERATE_CERTS, SecurityPage::onGenerate)
END_EVENT_TABLE()

SettingsPage::TextItem SecurityPage::texts[] = {
	{ IDC_STATIC1, ResourceManager::PRIVATE_KEY_FILE },
	{ IDC_STATIC2, ResourceManager::OWN_CERTIFICATE_FILE },
	{ IDC_STATIC3, ResourceManager::TRUSTED_CERTIFICATES_PATH },
	{ IDC_GENERATE_CERTS, ResourceManager::GENERATE_CERTIFICATES },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item SecurityPage::items[] = {
	{ IDC_TLS_CERTIFICATE_FILE, SettingsManager::TLS_CERTIFICATE_FILE, SettingsPage::T_STR },
	{ IDC_TLS_PRIVATE_KEY_FILE, SettingsManager::TLS_PRIVATE_KEY_FILE, SettingsPage::T_STR },
	{ IDC_TLS_TRUSTED_CERTIFICATES_PATH, SettingsManager::TLS_TRUSTED_CERTIFICATES_PATH, SettingsPage::T_STR },
	{ 0, 0, SettingsPage::T_END }
};

SettingsPage::ListItem SecurityPage::listItems[] = {
	{ SettingsManager::USE_TLS, ResourceManager::SETTINGS_USE_TLS },
	{ SettingsManager::ALLOW_UNTRUSTED_HUBS, ResourceManager::SETTINGS_ALLOW_UNTRUSTED_HUBS	},
	{ SettingsManager::ALLOW_UNTRUSTED_CLIENTS, ResourceManager::SETTINGS_ALLOW_UNTRUSTED_CLIENTS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SecurityPage::SecurityPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(3, 3, 0, 0);
	fgSizer1->AddGrowableCol(1);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	privateLabel = new wxStaticText(this, IDC_STATIC1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	privateLabel->Wrap(-1);
	fgSizer1->Add(privateLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
	
	privateEdit = new wxTextCtrl(this, IDC_TLS_PRIVATE_KEY_FILE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(privateEdit, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5);
	
	privateButton = new wxButton(this, IDC_BROWSE_PRIVATE_KEY, _T("..."), wxDefaultPosition, wxSize(30,-1), 0);
	fgSizer1->Add(privateButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	
	ownLabel = new wxStaticText(this, IDC_STATIC2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	ownLabel->Wrap(-1);
	fgSizer1->Add(ownLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	ownEdit = new wxTextCtrl(this, IDC_TLS_CERTIFICATE_FILE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(ownEdit, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	ownButton = new wxButton(this, IDC_BROWSE_CERTIFICATE, _T("..."), wxDefaultPosition, wxSize(30,-1), 0);
	fgSizer1->Add(ownButton, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	trustedLabel = new wxStaticText(this, IDC_STATIC3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	trustedLabel->Wrap(-1);
	fgSizer1->Add(trustedLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	trustedEdit = new wxTextCtrl(this, IDC_TLS_TRUSTED_CERTIFICATES_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(trustedEdit, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	trustedButton = new wxButton(this, IDC_BROWSE_TRUSTED_PATH, _T("..."), wxDefaultPosition, wxSize(30,-1), 0);
	fgSizer1->Add(trustedButton, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	bSizer1->Add(fgSizer1, 0, wxEXPAND, 5);
	
	generateButton = new wxButton(this, IDC_GENERATE_CERTS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(generateButton, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxArrayString optionsListChoices;
	optionsList = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxSize(-1,150), optionsListChoices, 0);
	bSizer1->Add(optionsList, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);
	
	SettingsPage::translate(texts);
	SettingsPage::read(items, listItems, optionsList);
}

SecurityPage::~SecurityPage()
{
}

void SecurityPage::onPrivate(wxCommandEvent &WXUNUSED(event))
{
	wxFileDialog dlg(this);
	dlg.SetPath(privateEdit->GetValue());
	if (dlg.ShowModal() == wxID_OK)
	{
		privateEdit->SetValue(dlg.GetPath());
	}
}

void SecurityPage::onOwn(wxCommandEvent &WXUNUSED(event))
{
	wxFileDialog dlg(this);
	dlg.SetPath(ownEdit->GetValue());
	if (dlg.ShowModal() == wxID_OK)
	{
		ownEdit->SetValue(dlg.GetPath());
	}
}

void SecurityPage::onTrusted(wxCommandEvent &WXUNUSED(event))
{
	wxDirDialog dlg(this);
	dlg.SetPath(trustedEdit->GetValue());
	if (dlg.ShowModal() == wxID_OK)
	{
		trustedEdit->SetValue(dlg.GetPath());
	}
}

void SecurityPage::onGenerate(wxCommandEvent &WXUNUSED(event))
{
	try
	{
		CryptoManager::getInstance()->generateCertificate();
	}
	catch(const CryptoException& e)
	{
		wxMessageBox(Text::toT(e.getError()).c_str(), _("Error generating certificate"));
	}
}

void SecurityPage::write()
{
	SettingsPage::write(items, listItems, optionsList);
}