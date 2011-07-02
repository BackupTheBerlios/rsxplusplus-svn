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

#include "ExpertsPage.h"

SettingsPage::TextItem ExpertsPage::texts[] = {
	{ IDC_SETTINGS_WRITE_BUFFER, ResourceManager::SETTINGS_WRITE_BUFFER },
	{ IDC_SETTINGS_KB, ResourceManager::KB },
	{ IDC_SETTINGS_SEARCH_HISTORY, ResourceManager::SETTINGS_SEARCH_HISTORY },
	{ IDC_SETSTRONGDC_PM_LINES, ResourceManager::SETTINGS_PM_HISTORY },
	{ IDC_SETTINGS_AUTO_SEARCH_LIMIT, ResourceManager::SETTINGS_AUTO_SEARCH_LIMIT },
	{ IDC_STATIC1, ResourceManager::PORT },
	{ IDC_STATIC2, ResourceManager::USER },
	{ IDC_STATIC3, ResourceManager::PASSWORD },
	{ IDC_SETTINGS_ODC_SHUTDOWNTIMEOUT, ResourceManager::SETTINGS_ODC_SHUTDOWNTIMEOUT },
	{ IDC_MAXCOMPRESS, ResourceManager::SETTINGS_MAX_COMPRESS },
	{ IDC_INTERVAL_TEXT, ResourceManager::MINIMUM_SEARCH_INTERVAL },
	{ IDC_MATCH_QUEUE_TEXT, ResourceManager::SETTINGS_SB_MAX_SOURCES },
	{ IDC_USERLISTDBLCLICKACTION, ResourceManager::USERLISTDBLCLICKACTION },
	{ IDC_TRANSFERLISTDBLCLICKACTION, ResourceManager::TRANSFERLISTDBLCLICKACTION },
	{ IDC_CHATDBLCLICKACTION, ResourceManager::CHATDBLCLICKACTION },
	{ IDC_SHUTDOWNACTION, ResourceManager::SHUTDOWN_ACTION },
	{ IDC_SETTINGS_DOWNCONN, ResourceManager::SETTINGS_DOWNCONN },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item ExpertsPage::items[] = {
	{ IDC_BUFFERSIZE, SettingsManager::BUFFER_SIZE, SettingsPage::T_INT },
	{ IDC_SOCKET_IN_BUFFER, SettingsManager::SOCKET_IN_BUFFER, SettingsPage::T_INT },
	{ IDC_SOCKET_OUT_BUFFER, SettingsManager::SOCKET_OUT_BUFFER, SettingsPage::T_INT },
	{ IDC_PM_LINES, SettingsManager::PM_LAST_LINES_LOG, SettingsPage::T_INT },
	{ IDC_SEARCH_HISTORY, SettingsManager::SEARCH_HISTORY, SettingsPage::T_INT },
	{ IDC_EDIT1, SettingsManager::WEBSERVER_PORT, SettingsPage::T_INT }, 
	{ IDC_EDIT2, SettingsManager::WEBSERVER_USER, SettingsPage::T_STR }, 
	{ IDC_EDIT3, SettingsManager::WEBSERVER_PASS, SettingsPage::T_STR }, 
	{ IDC_SHUTDOWNTIMEOUT, SettingsManager::SHUTDOWN_TIMEOUT, SettingsPage::T_INT },
	{ IDC_MAX_COMPRESSION, SettingsManager::MAX_COMPRESSION, SettingsPage::T_INT },
	{ IDC_INTERVAL, SettingsManager::MINIMUM_SEARCH_INTERVAL, SettingsPage::T_INT },
	{ IDC_MATCH, SettingsManager::MAX_AUTO_MATCH_SOURCES, SettingsPage::T_INT },
	{ IDC_AUTO_SEARCH_LIMIT, SettingsManager::AUTO_SEARCH_LIMIT, SettingsPage::T_INT },
	{ IDC_DOWNCONN, SettingsManager::DOWNCONN_PER_SEC, SettingsPage::T_INT },
	{ 0, 0, SettingsPage::T_END }
};

ExpertsPage::ExpertsPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	webserverLabel = new wxStaticText(this, wxID_ANY, _("Webserver"), wxDefaultPosition, wxDefaultSize, 0);
	webserverLabel->Wrap(-1);
	bSizer2->Add(webserverLabel, 0, wxALL, 5);
	
	line1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer2->Add(line1, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	
	bSizer1->Add(bSizer2, 0, wxEXPAND, 5);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(1, 6, 0, 0);
	fgSizer1->AddGrowableCol(2);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	portLabel = new wxStaticText(this, IDC_STATIC1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	portLabel->Wrap(-1);
	fgSizer1->Add(portLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	portSpin = new wxSpinCtrl(this, IDC_EDIT1, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 65535, 0);
	fgSizer1->Add(portSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	userLabel = new wxStaticText(this, IDC_STATIC2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	userLabel->Wrap(-1);
	fgSizer1->Add(userLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	userEdit = new wxTextCtrl(this, IDC_EDIT2, wxEmptyString, wxDefaultPosition, wxSize(75,-1), 0);
	fgSizer1->Add(userEdit, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	passwordLabel = new wxStaticText(this, IDC_STATIC3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	passwordLabel->Wrap(-1);
	fgSizer1->Add(passwordLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	passwordEdit = new wxTextCtrl(this, IDC_EDIT3, wxEmptyString, wxDefaultPosition, wxSize(75,-1), wxTE_PASSWORD);
	fgSizer1->Add(passwordEdit, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	bSizer1->Add(fgSizer1, 0, wxEXPAND, 5);
	
	line2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer1->Add(line2, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer(1, 5, 0, 0);
	fgSizer2->AddGrowableCol(2);
	fgSizer2->SetFlexibleDirection(wxBOTH);
	fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	shutdownLabel = new wxStaticText(this, IDC_SHUTDOWNACTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	shutdownLabel->Wrap(-1);
	fgSizer2->Add(shutdownLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	shutdownCombo = new wxComboBox(this, IDC_COMBO1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer2->Add(shutdownCombo, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	timeoutLabel = new wxStaticText(this, IDC_SETTINGS_ODC_SHUTDOWNTIMEOUT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	timeoutLabel->Wrap(-1);
	fgSizer2->Add(timeoutLabel, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	timeoutSpin = new wxSpinCtrl(this, IDC_SHUTDOWNTIMEOUT, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 1, 3600, 150);
	fgSizer2->Add(timeoutSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	sLabel = new wxStaticText(this, IDC_S, _("s"), wxDefaultPosition, wxDefaultSize, 0);
	sLabel->Wrap(-1);
	fgSizer2->Add(sLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	bSizer1->Add(fgSizer2, 0, wxEXPAND, 5);
	
	line3 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer1->Add(line3, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer(2, 6, 0, 0);
	fgSizer3->AddGrowableCol(2);
	fgSizer3->SetFlexibleDirection(wxBOTH);
	fgSizer3->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	readBufLabel = new wxStaticText(this, IDC_SETTINGS_SOCKET_IN_BUFFER, _("Socket read buffer"), wxDefaultPosition, wxDefaultSize, 0);
	readBufLabel->Wrap(-1);
	fgSizer3->Add(readBufLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	readBufSpin = new wxSpinCtrl(this, IDC_SOCKET_IN_BUFFER, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 1024, 1048576, 1024);
	fgSizer3->Add(readBufSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	bLabel = new wxStaticText(this, wxID_ANY, _("B"), wxDefaultPosition, wxDefaultSize, 0);
	bLabel->Wrap(-1);
	fgSizer3->Add(bLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	bufSizeLabel = new wxStaticText(this, IDC_SETTINGS_WRITE_BUFFER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bufSizeLabel->Wrap(-1);
	fgSizer3->Add(bufSizeLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	bufSizeSpin = new wxSpinCtrl(this, IDC_BUFFERSIZE, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 4096, 0);
	fgSizer3->Add(bufSizeSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	kibLabel = new wxStaticText(this, IDC_SETTINGS_KB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	kibLabel->Wrap(-1);
	fgSizer3->Add(kibLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	writeBufLabel = new wxStaticText(this, IDC_SETTINGS_SOCKET_OUT_BUFFER, _("Socket write buffer"), wxDefaultPosition, wxDefaultSize, 0);
	writeBufLabel->Wrap(-1);
	fgSizer3->Add(writeBufLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	writeBufSpin = new wxSpinCtrl(this, IDC_SOCKET_OUT_BUFFER, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 1024, 1048576, 1024);
	fgSizer3->Add(writeBufSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	b2Label = new wxStaticText(this, wxID_ANY, _("B"), wxDefaultPosition, wxDefaultSize, 0);
	b2Label->Wrap(-1);
	fgSizer3->Add(b2Label, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	bSizer1->Add(fgSizer3, 0, wxEXPAND, 5);
	
	line4 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer1->Add(line4, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer(4, 2, 0, 0);
	fgSizer4->SetFlexibleDirection(wxBOTH);
	fgSizer4->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	searchHistLabel = new wxStaticText(this, IDC_SETTINGS_SEARCH_HISTORY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	searchHistLabel->Wrap(-1);
	fgSizer4->Add(searchHistLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	searchHistSpin = new wxSpinCtrl(this, IDC_SEARCH_HISTORY, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 1, 100, 1);
	fgSizer4->Add(searchHistSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	pmHistLabel = new wxStaticText(this, IDC_SETSTRONGDC_PM_LINES, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	pmHistLabel->Wrap(-1);
	fgSizer4->Add(pmHistLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	pmHistSpin = new wxSpinCtrl(this, IDC_PM_LINES, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 999, 0);
	fgSizer4->Add(pmHistSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	compressionLabel = new wxStaticText(this, IDC_MAXCOMPRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	compressionLabel->Wrap(-1);
	fgSizer4->Add(compressionLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	compressionSpin = new wxSpinCtrl(this, IDC_MAX_COMPRESSION, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 9, 0);
	fgSizer4->Add(compressionSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	attemptsLabel = new wxStaticText(this, IDC_SETTINGS_DOWNCONN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	attemptsLabel->Wrap(-1);
	fgSizer4->Add(attemptsLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	attemptsSpin = new wxSpinCtrl(this, IDC_DOWNCONN, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 100, 0);
	fgSizer4->Add(attemptsSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	bSizer3->Add(fgSizer4, 0, wxEXPAND, 5);
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer(3, 3, 0, 0);
	fgSizer5->AddGrowableCol(0);
	fgSizer5->SetFlexibleDirection(wxBOTH);
	fgSizer5->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	intervalLabel = new wxStaticText(this, IDC_INTERVAL_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	intervalLabel->Wrap(-1);
	fgSizer5->Add(intervalLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	intervalSpin = new wxSpinCtrl(this, IDC_INTERVAL, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 10, 9999, 30);
	fgSizer5->Add(intervalSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	s2Label = new wxStaticText(this, IDC_S2, _("s"), wxDefaultPosition, wxDefaultSize, 0);
	s2Label->Wrap(-1);
	fgSizer5->Add(s2Label, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	matchLabel = new wxStaticText(this, IDC_MATCH_QUEUE_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	matchLabel->Wrap(-1);
	fgSizer5->Add(matchLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	matchSpin = new wxSpinCtrl(this, IDC_MATCH, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 1, 999, 1);
	fgSizer5->Add(matchSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	emptyLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	emptyLabel->Wrap(-1);
	fgSizer5->Add(emptyLabel, 0, wxALL, 5);
	
	autosearchLabel = new wxStaticText(this, IDC_SETTINGS_AUTO_SEARCH_LIMIT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	autosearchLabel->Wrap(-1);
	fgSizer5->Add(autosearchLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	autosearchSpin = new wxSpinCtrl(this, IDC_AUTO_SEARCH_LIMIT, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 1, 999, 1);
	fgSizer5->Add(autosearchSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	bSizer3->Add(fgSizer5, 1, wxALIGN_BOTTOM, 5);
	
	bSizer1->Add(bSizer3, 0, wxEXPAND, 5);
	
	line5 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer1->Add(line5, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer(3, 2, 0, 0);
	fgSizer6->AddGrowableCol(0);
	fgSizer6->SetFlexibleDirection(wxBOTH);
	fgSizer6->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	transActionLabel = new wxStaticText(this, IDC_TRANSFERLISTDBLCLICKACTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	transActionLabel->Wrap(-1);
	fgSizer6->Add(transActionLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	transActionCombo = new wxComboBox(this, IDC_TRANSFERLIST_DBLCLICK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer6->Add(transActionCombo, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	chatActionLabel = new wxStaticText(this, IDC_CHATDBLCLICKACTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	chatActionLabel->Wrap(-1);
	fgSizer6->Add(chatActionLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	chatActionCombo = new wxComboBox(this, IDC_CHAT_DBLCLICK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer6->Add(chatActionCombo, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	listActionLabel = new wxStaticText(this, IDC_USERLISTDBLCLICKACTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	listActionLabel->Wrap(-1);
	fgSizer6->Add(listActionLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	listActionCombo = new wxComboBox(this, IDC_USERLIST_DBLCLICK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer6->Add(listActionCombo, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	bSizer1->Add(fgSizer6, 0, wxALIGN_CENTER_HORIZONTAL, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	SettingsPage::translate(texts);
	SettingsPage::read(items);

	shutdownCombo->AppendString(CTSTRING(POWER_OFF));
	shutdownCombo->AppendString(CTSTRING(LOG_OFF));
	shutdownCombo->AppendString(CTSTRING(REBOOT));
	shutdownCombo->AppendString(CTSTRING(SUSPEND));
	shutdownCombo->AppendString(CTSTRING(HIBERNATE));
	shutdownCombo->SetSelection(SETTING(SHUTDOWN_ACTION));

	listActionCombo->AppendString(CTSTRING(GET_FILE_LIST));
  listActionCombo->AppendString(CTSTRING(SEND_PUBLIC_MESSAGE));
  listActionCombo->AppendString(CTSTRING(SEND_PRIVATE_MESSAGE));
  listActionCombo->AppendString(CTSTRING(MATCH_QUEUE));
  listActionCombo->AppendString(CTSTRING(GRANT_EXTRA_SLOT));
	listActionCombo->AppendString(CTSTRING(ADD_TO_FAVORITES));
	listActionCombo->AppendString(CTSTRING(BROWSE_FILE_LIST));
	listActionCombo->SetSelection(SETTING(USERLIST_DBLCLICK));

	transActionCombo->AppendString(CTSTRING(SEND_PRIVATE_MESSAGE));
	transActionCombo->AppendString(CTSTRING(GET_FILE_LIST));
	transActionCombo->AppendString(CTSTRING(MATCH_QUEUE));
	transActionCombo->AppendString(CTSTRING(GRANT_EXTRA_SLOT));
	transActionCombo->AppendString(CTSTRING(ADD_TO_FAVORITES));
	transActionCombo->AppendString(CTSTRING(BROWSE_FILE_LIST));
	transActionCombo->SetSelection(SETTING(TRANSFERLIST_DBLCLICK));

	chatActionCombo->AppendString(CTSTRING(SELECT_USER_LIST));
  chatActionCombo->AppendString(CTSTRING(SEND_PUBLIC_MESSAGE));
  chatActionCombo->AppendString(CTSTRING(SEND_PRIVATE_MESSAGE));
  chatActionCombo->AppendString(CTSTRING(GET_FILE_LIST));
  chatActionCombo->AppendString(CTSTRING(MATCH_QUEUE));
  chatActionCombo->AppendString(CTSTRING(GRANT_EXTRA_SLOT));
	chatActionCombo->AppendString(CTSTRING(ADD_TO_FAVORITES));
	chatActionCombo->SetSelection(SETTING(CHAT_DBLCLICK));
}

ExpertsPage::~ExpertsPage()
{
}

void ExpertsPage::write()
{
	SettingsPage::write(items);
	SettingsManager::getInstance()->set(SettingsManager::SHUTDOWN_ACTION, shutdownCombo->GetSelection());

	settings->set(SettingsManager::USERLIST_DBLCLICK, listActionCombo->GetSelection());
	settings->set(SettingsManager::TRANSFERLIST_DBLCLICK, transActionCombo->GetSelection());
	settings->set(SettingsManager::CHAT_DBLCLICK, chatActionCombo->GetSelection());

	if(SETTING(AUTO_SEARCH_LIMIT) < 1)
		settings->set(SettingsManager::AUTO_SEARCH_LIMIT, 1);	
}