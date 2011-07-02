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

#include "AdvancedPage.h"

BEGIN_EVENT_TABLE(AdvancedPage, SettingsPage)
	EVT_BUTTON(IDC_WINAMP_HELP, AdvancedPage::onHelp)
END_EVENT_TABLE()

SettingsPage::TextItem AdvancedPage::texts[] = {
	{ IDC_STRONGDC_WINAMP, ResourceManager::SETSTRONGDC_WINAMP },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item AdvancedPage::items[] = {
	{ IDC_WINAMP, SettingsManager::WINAMP_FORMAT, SettingsPage::T_STR },
	{ 0, 0, SettingsPage::T_END }
};

AdvancedPage::ListItem AdvancedPage::listItems[] = {
	{ SettingsManager::AUTO_AWAY, ResourceManager::SETTINGS_AUTO_AWAY },
	{ SettingsManager::AUTO_FOLLOW, ResourceManager::SETTINGS_AUTO_FOLLOW },
	{ SettingsManager::CLEAR_SEARCH, ResourceManager::SETTINGS_CLEAR_SEARCH },
	{ SettingsManager::MINIMIZE_ON_STARTUP, ResourceManager::SETTINGS_MINIMIZE_ON_STARTUP },
	{ SettingsManager::REMOVE_FORBIDDEN, ResourceManager::SETSTRONGDC_REMOVE_FORBIDDEN },
	{ SettingsManager::URL_HANDLER, ResourceManager::SETTINGS_URL_HANDLER },
	{ SettingsManager::MAGNET_REGISTER, ResourceManager::SETSTRONGDC_MAGNET_URI_HANDLER },
	{ SettingsManager::KEEP_LISTS, ResourceManager::SETTINGS_KEEP_LISTS },
	{ SettingsManager::AUTO_KICK, ResourceManager::SETTINGS_AUTO_KICK },
	{ SettingsManager::AUTO_KICK_NO_FAVS, ResourceManager::SETTINGS_AUTO_KICK_NO_FAVS },
	{ SettingsManager::NO_AWAYMSG_TO_BOTS, ResourceManager::SETTINGS_NO_AWAYMSG_TO_BOTS },
	{ SettingsManager::ADLS_BREAK_ON_FIRST, ResourceManager::SETTINGS_ADLS_BREAK_ON_FIRST },
	{ SettingsManager::COMPRESS_TRANSFERS, ResourceManager::SETTINGS_COMPRESS_TRANSFERS },
	{ SettingsManager::HUB_USER_COMMANDS, ResourceManager::SETTINGS_HUB_USER_COMMANDS },
	{ SettingsManager::SEARCH_PASSIVE, ResourceManager::SETSTRONGDC_PASSIVE_SEARCH },
	{ SettingsManager::SEND_UNKNOWN_COMMANDS, ResourceManager::SETTINGS_SEND_UNKNOWN_COMMANDS },
	{ SettingsManager::ADD_FINISHED_INSTANTLY, ResourceManager::ADD_FINISHED_INSTANTLY },
	{ SettingsManager::USE_CTRL_FOR_LINE_HISTORY, ResourceManager::SETTINGS_USE_CTRL_FOR_LINE_HISTORY },
	{ SettingsManager::DEBUG_COMMANDS, ResourceManager::SETTINGS_DEBUG_COMMANDS },
	{ SettingsManager::WEBSERVER, ResourceManager::SETTINGS_WEBSERVER }, 
	{ SettingsManager::DONT_ANNOUNCE_NEW_VERSIONS, ResourceManager::SETTINGS_DISPLAY_DC_UPDATE },
	{ SettingsManager::FILTER_ENTER, ResourceManager::SETTINGS_FILTER_ENTER },
	{ SettingsManager::SHOW_SHELL_MENU, ResourceManager::SETTINGS_SHOW_SHELL_MENU },
	{ SettingsManager::MAGNET_ASK, ResourceManager::MAGNET_ASK },
	{ SettingsManager::CORAL, ResourceManager::CORAL },
	{ SettingsManager::FAST_HASH, ResourceManager::FAST_HASH },
	{ SettingsManager::USE_DHT, ResourceManager::USE_DHT },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

AdvancedPage::AdvancedPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxArrayString optionsListChoices;
	optionsList = new wxCheckListBox(this, IDC_ADVANCED_BOOLEANS, wxDefaultPosition, wxSize(-1,350), optionsListChoices, 0);
	bSizer1->Add(optionsList, 0, wxALL | wxEXPAND, 5);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_STRONGDC_WINAMP, wxEmptyString), wxHORIZONTAL);
	
	winampEdit = new wxTextCtrl(this, IDC_WINAMP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer1->Add(winampEdit, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	
	helpButton = new wxButton(this, IDC_WINAMP_HELP, _T("?"), wxDefaultPosition, wxSize(20,-1), 0);
	sbSizer1->Add(helpButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	
	bSizer1->Add(sbSizer1, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);
	
	SettingsPage::translate(texts);
	SettingsPage::read(items, listItems, optionsList);
}

AdvancedPage::~AdvancedPage()
{
}

void AdvancedPage::onHelp(wxCommandEvent &WXUNUSED(event))
{
	wxMessageBox(CTSTRING(WINAMP_HELP), CTSTRING(WINAMP_HELP_DESC), wxOK | wxICON_INFORMATION);
}

void AdvancedPage::write()
{
	SettingsPage::write(items, listItems, optionsList);
}
