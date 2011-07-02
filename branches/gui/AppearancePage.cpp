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
#include "../client/SettingsManager.h"
#include "WinUtil.h"

#include "AppearancePage.h"

BEGIN_EVENT_TABLE(AppearancePage, SettingsPage)
	EVT_BUTTON(IDC_BROWSE, AppearancePage::onBrowse)
	EVT_BUTTON(IDC_TIMESTAMP_HELP, AppearancePage::onTimeStampHelp)
END_EVENT_TABLE()

SettingsPage::TextItem AppearancePage::texts[] = {
	{ IDC_SETTINGS_APPEARANCE_OPTIONS, ResourceManager::SETTINGS_OPTIONS },
	{ IDC_SETTINGS_BOLD_CONTENTS, ResourceManager::SETTINGS_BOLD_OPTIONS },
	{ IDC_SETTINGS_DEFAULT_AWAY_MSG, ResourceManager::SETTINGS_DEFAULT_AWAY_MSG },
	{ IDC_SETTINGS_TIME_STAMPS_FORMAT, ResourceManager::SETTINGS_TIME_STAMPS_FORMAT },
	{ IDC_SETTINGS_LANGUAGE_FILE, ResourceManager::SETTINGS_LANGUAGE_FILE },
	{ IDC_BROWSE, ResourceManager::BROWSE_ACCEL },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item AppearancePage::items[] = {
	{ IDC_DEFAULT_AWAY_MESSAGE, SettingsManager::DEFAULT_AWAY_MESSAGE, SettingsPage::T_STR },
	{ IDC_TIME_STAMPS_FORMAT, SettingsManager::TIME_STAMPS_FORMAT, SettingsPage::T_STR },
	{ IDC_LANGUAGE, SettingsManager::LANGUAGE_FILE, SettingsPage::T_STR },
	{ 0, 0, SettingsPage::T_END }
};

SettingsPage::ListItem AppearancePage::listItems[] = {
	{ SettingsManager::SHOW_PROGRESS_BARS, ResourceManager::SETTINGS_SHOW_PROGRESS_BARS },
	{ SettingsManager::SHOW_INFOTIPS, ResourceManager::SETTINGS_SHOW_INFO_TIPS },
	{ SettingsManager::FILTER_MESSAGES, ResourceManager::SETTINGS_FILTER_MESSAGES },
	{ SettingsManager::MINIMIZE_TRAY, ResourceManager::SETTINGS_MINIMIZE_TRAY },
	{ SettingsManager::TIME_STAMPS, ResourceManager::SETTINGS_TIME_STAMPS },
	{ SettingsManager::STATUS_IN_CHAT, ResourceManager::SETTINGS_STATUS_IN_CHAT },
	{ SettingsManager::SHOW_JOINS, ResourceManager::SETTINGS_SHOW_JOINS },
	{ SettingsManager::FAV_SHOW_JOINS, ResourceManager::SETTINGS_FAV_SHOW_JOINS },
	{ SettingsManager::EXPAND_QUEUE , ResourceManager::SETTINGS_EXPAND_QUEUE },
	{ SettingsManager::SORT_FAVUSERS_FIRST, ResourceManager::SETTINGS_SORT_FAVUSERS_FIRST },
	{ SettingsManager::USE_SYSTEM_ICONS, ResourceManager::SETTINGS_USE_SYSTEM_ICONS },
	{ SettingsManager::GET_USER_COUNTRY, ResourceManager::SETTINGS_GET_USER_COUNTRY },
	{ SettingsManager::CZCHARS_DISABLE, ResourceManager::SETSTRONGDC_CZCHARS_DISABLE },
	{ SettingsManager::USE_OLD_SHARING_UI, ResourceManager::SETTINGS_USE_OLD_SHARING_UI },
	{ SettingsManager::SUPPRESS_MAIN_CHAT, ResourceManager::SETTINGS_ADVANCED_SUPPRESS_MAIN_CHAT },
	{ SettingsManager::UC_SUBMENU, ResourceManager::UC_SUBMENU },
	{ SettingsManager::USE_EXPLORER_THEME, ResourceManager::USE_EXPLORER_THEME },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::ListItem AppearancePage::boldItems[] = {
	{ SettingsManager::BOLD_FINISHED_DOWNLOADS, ResourceManager::FINISHED_DOWNLOADS },
	{ SettingsManager::BOLD_FINISHED_UPLOADS, ResourceManager::FINISHED_UPLOADS },
	{ SettingsManager::BOLD_QUEUE, ResourceManager::DOWNLOAD_QUEUE },
	{ SettingsManager::BOLD_HUB, ResourceManager::HUB },
	{ SettingsManager::BOLD_PM, ResourceManager::PRIVATE_MESSAGE },
	{ SettingsManager::BOLD_SEARCH, ResourceManager::SEARCH },
	{ SettingsManager::BOLD_UPLOAD_QUEUE, ResourceManager::UPLOAD_QUEUE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

AppearancePage::AppearancePage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_APPEARANCE_OPTIONS, wxEmptyString), wxVERTICAL);
	
	wxArrayString optionsListChoices;
	optionsList = new wxCheckListBox(this, IDC_APPEARANCE_BOOLEANS, wxDefaultPosition, wxSize(-1,100), optionsListChoices, 0);
	sbSizer1->Add(optionsList, 1, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);
	
	bSizer1->Add(sbSizer1, 1, wxALL | wxEXPAND, 5);
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_BOLD_CONTENTS, wxEmptyString), wxVERTICAL);
	
	wxArrayString highlightListChoices;
	highlightList = new wxCheckListBox(this, IDC_BOLD_BOOLEANS, wxDefaultPosition, wxSize(-1,100), highlightListChoices, 0);
	sbSizer2->Add(highlightList, 1, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);
	
	bSizer1->Add(sbSizer2, 1, wxALL | wxEXPAND, 5);
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_DEFAULT_AWAY_MSG, wxEmptyString), wxVERTICAL);
	
	awayMsgEdit = new wxTextCtrl(this, IDC_DEFAULT_AWAY_MESSAGE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer3->Add(awayMsgEdit, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);
	
	bSizer2->Add(sbSizer3, 1, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_TIME_STAMPS_FORMAT, wxEmptyString), wxHORIZONTAL);
	
	timestampEdit = new wxTextCtrl(this, IDC_TIME_STAMPS_FORMAT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer4->Add(timestampEdit, 1, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	timestampButton = new wxButton(this, IDC_TIMESTAMP_HELP, _T("?"), wxDefaultPosition, wxSize(20,-1), 0);
	sbSizer4->Add(timestampButton, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	
	bSizer2->Add(sbSizer4, 1, wxBOTTOM | wxEXPAND | wxRIGHT | wxTOP, 5);
	
	bSizer1->Add(bSizer2, 0, wxEXPAND, 5);
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_LANGUAGE_FILE, wxEmptyString), wxHORIZONTAL);
	
	languageEdit = new wxTextCtrl(this, IDC_LANGUAGE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer5->Add(languageEdit, 1, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	languageButton = new wxButton(this, IDC_BROWSE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer5->Add(languageButton, 0, wxBOTTOM | wxRIGHT, 5);
	
	bSizer1->Add(sbSizer5, 0, wxALL | wxEXPAND, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);
	
	SettingsPage::translate(texts);
	SettingsPage::read(items, listItems, optionsList);
	SettingsPage::read(items, boldItems, highlightList);
}

AppearancePage::~AppearancePage()
{
}

void AppearancePage::onBrowse(wxCommandEvent &WXUNUSED(event))
{
	wxFileDialog dlg(this);
	dlg.SetWildcard(_T("Language Files|*.xml|All Files|*.*"));
	if (dlg.ShowModal() == wxID_OK)
		languageEdit->SetValue(dlg.GetPath());
}

void AppearancePage::onTimeStampHelp(wxCommandEvent &WXUNUSED(event))
{
	wxMessageBox(CTSTRING(TIMESTAMP_HELP), CTSTRING(TIMESTAMP_HELP_DESC), wxOK | wxICON_INFORMATION);
}

void AppearancePage::write()
{
	SettingsPage::write(items, listItems, optionsList);
	SettingsPage::write(items, boldItems, highlightList);
}