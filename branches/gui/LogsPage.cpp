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
#include "../client/LogManager.h"
#include "../client/File.h"

#include "LogsPage.h"

BEGIN_EVENT_TABLE(LogsPage, SettingsPage)
	EVT_BUTTON(IDC_BROWSE_LOG, LogsPage::onBrowse)
	EVT_LISTBOX(IDC_LOG_OPTIONS, LogsPage::onItemSelected)
END_EVENT_TABLE()

SettingsPage::TextItem LogsPage::texts[] = {
	{ IDC_SETTINGS_LOGGING,		ResourceManager::SETTINGS_LOGGING },
	{ IDC_SETTINGS_LOG_DIR,		ResourceManager::DIRECTORY },
	{ IDC_BROWSE_LOG,			ResourceManager::BROWSE_ACCEL },
	{ IDC_SETTINGS_FORMAT,		ResourceManager::SETTINGS_FORMAT },
	{ IDC_SETTINGS_FILE_NAME,	ResourceManager::SETTINGS_FILE_NAME },
	{ 0,						ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item LogsPage::items[] = {
	{ IDC_LOG_DIRECTORY, SettingsManager::LOG_DIRECTORY, SettingsPage::T_STR },
	{ 0, 0, SettingsPage::T_END }
};

SettingsPage::ListItem LogsPage::listItems[] = {
	{ SettingsManager::LOG_MAIN_CHAT,			ResourceManager::SETTINGS_LOG_MAIN_CHAT },
	{ SettingsManager::LOG_PRIVATE_CHAT,		ResourceManager::SETTINGS_LOG_PRIVATE_CHAT },
	{ SettingsManager::LOG_DOWNLOADS,			ResourceManager::SETTINGS_LOG_DOWNLOADS }, 
	{ SettingsManager::LOG_UPLOADS,				ResourceManager::SETTINGS_LOG_UPLOADS },
	{ SettingsManager::LOG_SYSTEM,				ResourceManager::SETTINGS_LOG_SYSTEM_MESSAGES },
	{ SettingsManager::LOG_STATUS_MESSAGES,		ResourceManager::SETTINGS_LOG_STATUS_MESSAGES },
	{ SettingsManager::LOG_WEBSERVER,			ResourceManager::SETTINGS_LOG_WEBSERVER },
	{ SettingsManager::LOG_FILELIST_TRANSFERS, ResourceManager::SETTINGS_LOG_FILELIST_TRANSFERS },
	{ 0,										ResourceManager::SETTINGS_AUTO_AWAY }
};


LogsPage::LogsPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_LOGGING, wxEmptyString), wxVERTICAL);
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	dirLabel = new wxStaticText(this, IDC_SETTINGS_LOG_DIR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	dirLabel->Wrap(-1);
	bSizer2->Add(dirLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	dirEdit = new wxTextCtrl(this, IDC_LOG_DIRECTORY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(dirEdit, 1, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	dirButton = new wxButton(this, IDC_BROWSE_LOG, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(dirButton, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	sbSizer1->Add(bSizer2, 0, wxEXPAND, 5);
	
	wxArrayString optionsListChoices;
	optionsList = new wxCheckListBox(this, IDC_LOG_OPTIONS, wxDefaultPosition, wxSize(-1,150), optionsListChoices, 0);
	sbSizer1->Add(optionsList, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(2, 2, 0, 0);
	fgSizer1->AddGrowableCol(1);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	formatLabel = new wxStaticText(this, IDC_SETTINGS_FORMAT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	formatLabel->Wrap(-1);
	fgSizer1->Add(formatLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	formatEdit = new wxTextCtrl(this, IDC_LOG_FORMAT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(formatEdit, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, 5);
	
	filenameLabel = new wxStaticText(this, IDC_SETTINGS_FILE_NAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	filenameLabel->Wrap(-1);
	fgSizer1->Add(filenameLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	filenameEdit = new wxTextCtrl(this, IDC_LOG_FILE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(filenameEdit, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, 5);
	
	sbSizer1->Add(fgSizer1, 0, wxEXPAND, 5);
	
	bSizer1->Add(sbSizer1, 1, wxEXPAND | wxALL, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	SettingsPage::translate(texts);
	SettingsPage::read(items, listItems, optionsList);

	for(int i = 0; i < LogManager::LAST; ++i)
	{
		TStringPair pair;
		pair.first = Text::toT(LogManager::getInstance()->getSetting(i, LogManager::FILE));
		pair.second = Text::toT(LogManager::getInstance()->getSetting(i, LogManager::FORMAT));
		options.push_back(pair);
	}

	formatEdit->Enable(false);
	filenameEdit->Enable(false);

	oldSelection = -1;
}

LogsPage::~LogsPage()
{
}

void LogsPage::onBrowse(wxCommandEvent &WXUNUSED(event))
{
	wxDirDialog dlg(this);
	dlg.SetPath(SETTING(LOG_DIRECTORY));
	if (dlg.ShowModal() == wxID_OK)
	{
		wxString dir = dlg.GetPath();
		if ((dir.Len() > 0) && (dir.Last() != _T(PATH_SEPARATOR)))
			dir += _T(PATH_SEPARATOR);
		dirEdit->SetValue(dir);
	}
}

void LogsPage::onItemSelected(wxCommandEvent &WXUNUSED(event))
{
	getValues();

	int sel = optionsList->GetSelection();
		
	if(sel >= 0 && sel < LogManager::LAST)
	{
		bool checkState = optionsList->IsChecked(sel);
		
		formatEdit->Enable(checkState);
		filenameEdit->Enable(checkState);
				
		filenameEdit->SetValue(options[sel].first);
		formatEdit->SetValue(options[sel].second);
	
		//save the old selection so we know where to save the values
		oldSelection = sel;
	}
	else
	{
		formatEdit->Enable(false);
		filenameEdit->Enable(false);

		filenameEdit->SetValue(wxEmptyString);
		formatEdit->SetValue(wxEmptyString);
	}
}

void LogsPage::write()
{
	SettingsPage::write(items, listItems, optionsList);

	const string& s = SETTING(LOG_DIRECTORY);
	if(s.length() > 0 && s[s.length() - 1] != '\\') {
		SettingsManager::getInstance()->set(SettingsManager::LOG_DIRECTORY, s + '\\');
	}
	File::ensureDirectory(SETTING(LOG_DIRECTORY));

	//make sure we save the last edit too, the user
	//might not have changed the selection
	getValues();

	for(int i = 0; i < LogManager::LAST; ++i) {
		string tmp = Text::fromT(options[i].first);
		if(stricmp(Util::getFileExt(tmp), ".log") != 0)
			tmp += ".log";

		LogManager::getInstance()->saveSetting(i, LogManager::FILE, tmp);
		LogManager::getInstance()->saveSetting(i, LogManager::FORMAT, Text::fromT(options[i].second));
	}
}

void LogsPage::getValues()
{
	if(oldSelection >= 0)
	{
		wxString str;

		str = filenameEdit->GetValue();
		if(str.Len() > 0)
			options[oldSelection].first = str;
		str = formatEdit->GetValue();
		if(str.Len() > 0)
			options[oldSelection].second = str;
	}
}