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

#include "SoundsPage.h"

BEGIN_EVENT_TABLE(SoundsPage, SettingsPage)
	EVT_BUTTON(IDC_PLAY, SoundsPage::onPlay)
	EVT_BUTTON(IDC_NONE, SoundsPage::onNone)
	EVT_BUTTON(IDC_BROWSE, SoundsPage::onBrowse)
END_EVENT_TABLE()

SettingsPage::TextItem SoundsPage::texts[] = {
	{ IDC_BROWSE, ResourceManager::BROWSE },	
	{ IDC_PLAY, ResourceManager::PLAY },
	{ IDC_NONE, ResourceManager::NONE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SoundsPage::snds SoundsPage::sounds[] = {
	{ ResourceManager::SOUND_DOWNLOAD_BEGINS,	SettingsManager::BEGINFILE, ""},
	{ ResourceManager::SOUND_DOWNLOAD_FINISHED,	SettingsManager::FINISHFILE, ""},
	{ ResourceManager::SOUND_SOURCE_ADDED,	SettingsManager::SOURCEFILE, ""},
	{ ResourceManager::SOUND_UPLOAD_FINISHED,	SettingsManager::UPLOADFILE, ""},
	{ ResourceManager::SOUND_FAKER_FOUND,	SettingsManager::FAKERFILE, ""},
	{ ResourceManager::SETSTRONGDC_PRIVATE_SOUND,	SettingsManager::BEEPFILE, ""},
	{ ResourceManager::MYNICK_IN_CHAT,	SettingsManager::CHATNAMEFILE, ""},
	{ ResourceManager::SOUND_TTH_INVALID,	SettingsManager::SOUND_TTH, ""},
	{ ResourceManager::SOUND_EXCEPTION,	SettingsManager::SOUND_EXC, ""},
	{ ResourceManager::HUB_CONNECTED,	SettingsManager::SOUND_HUBCON, ""},
	{ ResourceManager::HUB_DISCONNECTED,	SettingsManager::SOUND_HUBDISCON, ""},
	{ ResourceManager::FAVUSER_ONLINE,	SettingsManager::SOUND_FAVUSER, ""},
	{ ResourceManager::SOUND_TYPING_NOTIFY,	SettingsManager::SOUND_TYPING_NOTIFY, ""}
};

SoundsPage::SoundsPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	soundList = new wxListCtrl(this, IDC_SOUNDLIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
	bSizer1->Add(soundList, 1, wxALL | wxEXPAND, 5);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(1, 3, 0, 0);
	fgSizer1->AddGrowableCol(1);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	playButton = new wxButton(this, IDC_PLAY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(playButton, 0, wxALL, 5);
	
	noneButton = new wxButton(this, IDC_NONE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(noneButton, 0, wxALL | wxALIGN_RIGHT, 5);
	
	browseButton = new wxButton(this, IDC_BROWSE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(browseButton, 0, wxALL, 5);
	
	bSizer1->Add(fgSizer1, 0, wxEXPAND, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	SettingsPage::translate(texts);

	soundList->InsertColumn(0, CTSTRING(SETTINGS_SOUNDS), wxLIST_FORMAT_LEFT, 172);
	soundList->InsertColumn(1, CTSTRING(FILENAME), wxLIST_FORMAT_LEFT, 210);

	long j;
	for(int i = 0; i < sizeof(sounds) / sizeof(snds); i++)
	{
		j = soundList->InsertItem(i, Text::toT(ResourceManager::getInstance()->getString(sounds[i].name)));
		sounds[i].value = SettingsManager::getInstance()->get((SettingsManager::StrSetting)sounds[i].setting, true);
		soundList->SetItem(j, 1, Text::toT(sounds[i].value));
	}
}

SoundsPage::~SoundsPage()
{
}

void SoundsPage::onPlay(wxCommandEvent &WXUNUSED(event))
{
	long item = soundList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxSound snd(soundList->GetItemText(item, 1));
		if (snd.IsOk())
			snd.Play(wxSOUND_ASYNC);
	}
}

void SoundsPage::onNone(wxCommandEvent &WXUNUSED(event))
{
	long item = soundList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		soundList->SetItem(item, 1, wxEmptyString);
	}
}

void SoundsPage::onBrowse(wxCommandEvent &WXUNUSED(event))
{
	long item = soundList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxFileDialog dlg(this);
		if (dlg.ShowModal() == wxID_OK)
		{
			soundList->SetItem(item, 1, dlg.GetPath());
		}
	}
}

void SoundsPage::write()
{
	wxString str;

	for(int i = 0; i < sizeof(sounds) / sizeof(snds); i++)
	{
		str = soundList->GetItemText(i, 1);
		settings->set((SettingsManager::StrSetting)sounds[i].setting, string(str.mb_str(wxConvUTF8)));
	}
}