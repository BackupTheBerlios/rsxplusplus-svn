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

#include "FakePage.h"

SettingsPage::TextItem FakePage::texts[] = {
	{ DAA, ResourceManager::TEXT_FAKEPERCENT },
	{ IDC_TIMEOUTS, ResourceManager::ACCEPTED_TIMEOUTS },
	{ IDC_DISCONNECTS, ResourceManager::ACCEPTED_DISCONNECTS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
}; 

SettingsPage::Item FakePage::items[] = {
	{ IDC_PERCENT_FAKE_SHARE_TOLERATED, SettingsManager::PERCENT_FAKE_SHARE_TOLERATED, SettingsPage::T_INT }, 
	{ IDC_TIMEOUTS_NO, SettingsManager::ACCEPTED_TIMEOUTS, SettingsPage::T_INT }, 
	{ IDC_DISCONNECTS_NO, SettingsManager::ACCEPTED_DISCONNECTS, SettingsPage::T_INT }, 
	{ 0, 0, SettingsPage::T_END }
};

FakePage::ListItem FakePage::listItems[] = {
	{ SettingsManager::CHECK_NEW_USERS, ResourceManager::CHECK_ON_CONNECT },
	{ SettingsManager::DISPLAY_CHEATS_IN_MAIN_CHAT, ResourceManager::SETTINGS_DISPLAY_CHEATS_IN_MAIN_CHAT },
	{ SettingsManager::SHOW_SHARE_CHECKED_USERS, ResourceManager::SETTINGS_ADVANCED_SHOW_SHARE_CHECKED_USERS },
		{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

FakePage::FakePage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, FAKE_SET, _("Fake share detector")), wxVERTICAL);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(2, 4, 0, 0);
	fgSizer1->AddGrowableCol(2);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	percentLabel = new wxStaticText(this, DAA, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	percentLabel->Wrap(-1);
	fgSizer1->Add(percentLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	percentSpin = new wxSpinCtrl(this, IDC_PERCENT_FAKE_SHARE_TOLERATED, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 99999, 0);
	fgSizer1->Add(percentSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	timeoutsLabel = new wxStaticText(this, IDC_TIMEOUTS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	timeoutsLabel->Wrap(-1);
	fgSizer1->Add(timeoutsLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	timeoutsSpin = new wxSpinCtrl(this, IDC_TIMEOUTS_NO, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 99999, 0);
	fgSizer1->Add(timeoutsSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	disconnectLabel = new wxStaticText(this, IDC_DISCONNECTS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	disconnectLabel->Wrap(-1);
	fgSizer1->Add(disconnectLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	disconnectSpin = new wxSpinCtrl(this, IDC_DISCONNECTS_NO, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 99999, 0);
	fgSizer1->Add(disconnectSpin, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	sbSizer1->Add(fgSizer1, 1, wxEXPAND, 5);
	
	bSizer1->Add(sbSizer1, 0, wxEXPAND | wxALL, 5);
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Raw commands")), wxVERTICAL);
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer(3, 4, 0, 0);
	fgSizer2->AddGrowableCol(2);
	fgSizer2->SetFlexibleDirection(wxBOTH);
	fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	discRawLabel = new wxStaticText(this, wxID_ANY, _("Disconnect raw"), wxDefaultPosition, wxDefaultSize, 0);
	discRawLabel->Wrap(-1);
	fgSizer2->Add(discRawLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	discRawCombo = new wxComboBox(this, IDC_DISCONNECT_RAW, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer2->Add(discRawCombo, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	listlenLabel = new wxStaticText(this, wxID_ANY, _("Listlen mismatch"), wxDefaultPosition, wxDefaultSize, 0);
	listlenLabel->Wrap(-1);
	fgSizer2->Add(listlenLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	listlenCombo = new wxComboBox(this, IDC_LISTLEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer2->Add(listlenCombo, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	timeoutRawLabel = new wxStaticText(this, wxID_ANY, _("Timeout raw"), wxDefaultPosition, wxDefaultSize, 0);
	timeoutRawLabel->Wrap(-1);
	fgSizer2->Add(timeoutRawLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	timeoutRawCombo = new wxComboBox(this, IDC_TIMEOUT_RAW, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer2->Add(timeoutRawCombo, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	smallLabel = new wxStaticText(this, wxID_ANY, _("Filelist too small"), wxDefaultPosition, wxDefaultSize, 0);
	smallLabel->Wrap(-1);
	fgSizer2->Add(smallLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	smallCombo = new wxComboBox(this, IDC_FILELIST_TOO_SMALL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer2->Add(smallCombo, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	fakeRawLabel = new wxStaticText(this, wxID_ANY, _("Fakeshare raw"), wxDefaultPosition, wxDefaultSize, 0);
	fakeRawLabel->Wrap(-1);
	fgSizer2->Add(fakeRawLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	fakeRawCombo = new wxComboBox(this, IDC_FAKE_RAW, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer2->Add(fakeRawCombo, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	unavailLabel = new wxStaticText(this, wxID_ANY, _("Filelist unavailable"), wxDefaultPosition, wxDefaultSize, 0);
	unavailLabel->Wrap(-1);
	fgSizer2->Add(unavailLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	unavailCombo = new wxComboBox(this, IDC_FILELIST_UNAVAILABLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer2->Add(unavailCombo, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	sbSizer2->Add(fgSizer2, 1, wxEXPAND, 5);
	
	bSizer1->Add(sbSizer2, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxArrayString optionsListChoices;
	optionsList = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxSize(-1,150), optionsListChoices, 0);
	bSizer1->Add(optionsList, 0, wxALL | wxEXPAND, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	SettingsPage::translate(texts);
	SettingsPage::read(items, listItems, optionsList);

#define ADDSTRINGS(x) \
	x->AppendString(_("No action")); \
	x->AppendString(_T("Raw 1")); \
	x->AppendString(_T("Raw 2")); \
	x->AppendString(_T("Raw 3")); \
	x->AppendString(_T("Raw 4")); \
	x->AppendString(_T("Raw 5"));

	ADDSTRINGS(discRawCombo);
	ADDSTRINGS(listlenCombo);
	ADDSTRINGS(timeoutRawCombo);
	ADDSTRINGS(smallCombo);
	ADDSTRINGS(fakeRawCombo);
	ADDSTRINGS(unavailCombo);

	discRawCombo->SetSelection(settings->get(SettingsManager::DISCONNECT_RAW));
	listlenCombo->SetSelection(settings->get(SettingsManager::LISTLEN_MISMATCH));
	timeoutRawCombo->SetSelection(settings->get(SettingsManager::TIMEOUT_RAW));
	smallCombo->SetSelection(settings->get(SettingsManager::FILELIST_TOO_SMALL));
	fakeRawCombo->SetSelection(settings->get(SettingsManager::FAKESHARE_RAW));
	unavailCombo->SetSelection(settings->get(SettingsManager::FILELIST_UNAVAILABLE));

}

FakePage::~FakePage()
{
}

void FakePage::write()
{
	SettingsPage::write(items, listItems, optionsList);
	
	SettingsManager::getInstance()->set(SettingsManager::DISCONNECT_RAW, discRawCombo->GetSelection());
	SettingsManager::getInstance()->set(SettingsManager::TIMEOUT_RAW, timeoutRawCombo->GetSelection());
	SettingsManager::getInstance()->set(SettingsManager::FAKESHARE_RAW, fakeRawCombo->GetSelection());
	SettingsManager::getInstance()->set(SettingsManager::LISTLEN_MISMATCH, listlenCombo->GetSelection());
	SettingsManager::getInstance()->set(SettingsManager::FILELIST_TOO_SMALL, smallCombo->GetSelection());
	SettingsManager::getInstance()->set(SettingsManager::FILELIST_UNAVAILABLE, unavailCombo->GetSelection());
}