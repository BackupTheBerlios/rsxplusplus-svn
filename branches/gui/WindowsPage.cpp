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

#include "WindowsPage.h"

SettingsPage::Item WindowsPage::items[] = { 
	{ IDC_MAX_TAB_ROWS, SettingsManager::MAX_TAB_ROWS, SettingsPage::T_INT },
	{ 0, 0, SettingsPage::T_END } 
};

SettingsPage::TextItem WindowsPage::textItem[] = {
	{ IDC_SETTINGS_AUTO_OPEN, ResourceManager::SETTINGS_AUTO_OPEN },
	{ IDC_SETTINGS_WINDOWS_OPTIONS, ResourceManager::SETTINGS_WINDOWS_OPTIONS },
	{ IDC_TABSTEXT, ResourceManager::TABS_POSITION },
	{ IDC_SETTINGS_MAX_TAB_ROWS, ResourceManager::SETTINGS_MAX_TAB_ROWS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

WindowsPage::ListItem WindowsPage::listItems[] = {
	{ SettingsManager::OPEN_PUBLIC, ResourceManager::PUBLIC_HUBS },
	{ SettingsManager::OPEN_FAVORITE_HUBS, ResourceManager::FAVORITE_HUBS },
	{ SettingsManager::OPEN_FAVORITE_USERS, ResourceManager::FAVORITE_USERS },
	{ SettingsManager::OPEN_RECENT_HUBS, ResourceManager::RECENT_HUBS },	
	{ SettingsManager::OPEN_QUEUE, ResourceManager::DOWNLOAD_QUEUE },
	{ SettingsManager::OPEN_FINISHED_DOWNLOADS, ResourceManager::FINISHED_DOWNLOADS },
	{ SettingsManager::OPEN_UPLOAD_QUEUE, ResourceManager::UPLOAD_QUEUE },
	{ SettingsManager::OPEN_FINISHED_UPLOADS, ResourceManager::FINISHED_UPLOADS },
	{ SettingsManager::OPEN_SEARCH_SPY, ResourceManager::SEARCH_SPY },
	{ SettingsManager::OPEN_NETWORK_STATISTICS, ResourceManager::NETWORK_STATISTICS },
	{ SettingsManager::OPEN_NOTEPAD, ResourceManager::NOTEPAD },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

WindowsPage::ListItem WindowsPage::optionItems[] = {
	{ SettingsManager::POPUP_PMS, ResourceManager::SETTINGS_POPUP_PMS },
	{ SettingsManager::POPUP_HUB_PMS, ResourceManager::SETTINGS_POPUP_HUB_PMS },
	{ SettingsManager::POPUP_BOT_PMS, ResourceManager::SETTINGS_POPUP_BOT_PMS },
	{ SettingsManager::POPUNDER_FILELIST, ResourceManager::SETTINGS_POPUNDER_FILELIST },
	{ SettingsManager::POPUNDER_PM, ResourceManager::SETTINGS_POPUNDER_PM },
	{ SettingsManager::JOIN_OPEN_NEW_WINDOW, ResourceManager::SETTINGS_OPEN_NEW_WINDOW },
	{ SettingsManager::IGNORE_HUB_PMS, ResourceManager::SETTINGS_IGNORE_HUB_PMS },
	{ SettingsManager::IGNORE_BOT_PMS, ResourceManager::SETTINGS_IGNORE_BOT_PMS },
	{ SettingsManager::TOGGLE_ACTIVE_WINDOW, ResourceManager::SETTINGS_TOGGLE_ACTIVE_WINDOW },
	{ SettingsManager::PROMPT_PASSWORD, ResourceManager::SETTINGS_PROMPT_PASSWORD },
	{ SettingsManager::CONFIRM_EXIT, ResourceManager::SETTINGS_CONFIRM_EXIT },
	{ SettingsManager::CONFIRM_HUB_REMOVAL, ResourceManager::SETTINGS_CONFIRM_HUB_REMOVAL },
	{ SettingsManager::CONFIRM_DELETE, ResourceManager::SETTINGS_CONFIRM_ITEM_REMOVAL },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

WindowsPage::WindowsPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_AUTO_OPEN, wxEmptyString), wxVERTICAL);
	
	wxArrayString autoOpenListChoices;
	autoOpenList = new wxCheckListBox(this, IDC_WINDOWS_STARTUP, wxDefaultPosition, wxSize(-1,125), autoOpenListChoices, 0);
	sbSizer1->Add(autoOpenList, 0, wxALL | wxEXPAND, 5);
	
	bSizer1->Add(sbSizer1, 0, wxEXPAND | wxALL, 5);
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_WINDOWS_OPTIONS, wxEmptyString), wxVERTICAL);
	
	wxArrayString optionsListChoices;
	optionsList = new wxCheckListBox(this, IDC_WINDOWS_OPTIONS, wxDefaultPosition, wxSize(-1,125), optionsListChoices, 0);
	sbSizer2->Add(optionsList, 0, wxALL | wxEXPAND, 5);
	
	bSizer1->Add(sbSizer2, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxEmptyString), wxVERTICAL);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(2, 2, 0, 0);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	positionLabel = new wxStaticText(this, IDC_TABSTEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	positionLabel->Wrap(-1);
	fgSizer1->Add(positionLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	positionCombo = new wxComboBox(this, IDC_TABSCOMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer1->Add(positionCombo, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	
	rowsLabel = new wxStaticText(this, IDC_SETTINGS_MAX_TAB_ROWS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	rowsLabel->Wrap(-1);
	fgSizer1->Add(rowsLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	rowsSpin = new wxSpinCtrl(this, IDC_MAX_TAB_ROWS, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 1, 10, 1);
	fgSizer1->Add(rowsSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	sbSizer3->Add(fgSizer1, 0, wxALIGN_CENTER_HORIZONTAL, 5);
	
	bSizer1->Add(sbSizer3, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	SettingsPage::translate(textItem);
	SettingsPage::read(items, listItems, autoOpenList);
	SettingsPage::read(items, optionItems, optionsList);

	positionCombo->AppendString(CTSTRING(TABS_TOP));
	positionCombo->AppendString(CTSTRING(TABS_BOTTOM));
	positionCombo->AppendString(CTSTRING(TABS_LEFT));
	positionCombo->AppendString(CTSTRING(TABS_RIGHT));
	positionCombo->SetSelection(SETTING(TABS_POS));
}

WindowsPage::~WindowsPage()
{
}

void WindowsPage::write()
{
	SettingsPage::write(items, listItems, autoOpenList);
	SettingsPage::write(items, optionItems, optionsList);

	settings->set(SettingsManager::TABS_POS, positionCombo->GetSelection());
}