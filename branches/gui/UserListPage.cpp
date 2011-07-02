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
#include "WinUtil.h"

#include "UserListPage.h"

BEGIN_EVENT_TABLE(UserListPage, SettingsPage)
	EVT_BUTTON(IDC_CHANGE_COLOR, UserListPage::onChange)
	EVT_BUTTON(IDC_IMAGEBROWSE, UserListPage::onBrowse)
END_EVENT_TABLE()

SettingsPage::TextItem UserListPage::texts[] = {
	{ IDC_CHANGE_COLOR, ResourceManager::SETTINGS_CHANGE },
	{ IDC_USERLIST, ResourceManager::USERLIST_ICONS },
	{ IDC_IMAGEBROWSE, ResourceManager::BROWSE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item UserListPage::items[] = {
	{ IDC_USERLIST_IMAGE, SettingsManager::USERLIST_IMAGE, SettingsPage::T_STR },
	{ 0, 0, SettingsPage::T_END }
};

UserListPage::UserListPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("User list")), wxVERTICAL);
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	stylesList = new wxListBox(this, IDC_USERLIST_COLORS, wxDefaultPosition, wxSize(-1,200), 0, NULL, 0); 
	bSizer2->Add(stylesList, 1, wxALL, 5);
	
	previewRich = new wxRichTextCtrl(this, IDC_PREVIEW, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxHSCROLL | wxVSCROLL | wxWANTS_CHARS);
	bSizer2->Add(previewRich, 1, wxEXPAND  |  wxALL, 5);
	
	sbSizer1->Add(bSizer2, 0, wxEXPAND, 5);
	
	changeButton = new wxButton(this, IDC_CHANGE_COLOR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer1->Add(changeButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	iconsLabel = new wxStaticText(this, IDC_USERLIST, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	iconsLabel->Wrap(-1);
	sbSizer1->Add(iconsLabel, 0, wxALL, 5);
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);
	
	iconsEdit = new wxTextCtrl(this, IDC_USERLIST_IMAGE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer3->Add(iconsEdit, 1, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	iconsButton = new wxButton(this, IDC_IMAGEBROWSE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer3->Add(iconsButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	sbSizer1->Add(bSizer3, 0, wxEXPAND, 5);
	
	bSizer1->Add(sbSizer1, 0, wxEXPAND | wxALL, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	SettingsPage::translate(texts);
	SettingsPage::read(items);

	colours[0] = SETTING(NORMAL_COLOUR);
	colours[1] = SETTING(FAVORITE_COLOR);
	colours[2] = SETTING(RESERVED_SLOT_COLOR);
	colours[3] = SETTING(IGNORED_COLOR);
	colours[4] = SETTING(FIREBALL_COLOR);
	colours[5] = SETTING(SERVER_COLOR);
	colours[6] = SETTING(PASIVE_COLOR);
	colours[7] = SETTING(OP_COLOR);
	colours[8] = SETTING(FULL_CHECKED_COLOUR);
	colours[9] = SETTING(BAD_CLIENT_COLOUR);
	colours[10] = SETTING(BAD_FILELIST_COLOUR);

	previewRich->SetBackgroundColour(wxColor(WinUtil::bgColor));
	stylesList->AppendString(CTSTRING(SETTINGS_COLOR_NORMAL));
	stylesList->AppendString(CTSTRING(SETTINGS_COLOR_FAVORITE));	
	stylesList->AppendString(CTSTRING(SETTINGS_COLOR_RESERVED));
	stylesList->AppendString(CTSTRING(SETTINGS_COLOR_IGNORED));
	stylesList->AppendString(CTSTRING(COLOR_FAST));
	stylesList->AppendString(CTSTRING(COLOR_SERVER));
	stylesList->AppendString(CTSTRING(COLOR_PASIVE));
	stylesList->AppendString(CTSTRING(COLOR_OP));
	stylesList->AppendString(CTSTRING(SETTINGS_COLOR_FULL_CHECKED));
	stylesList->AppendString(CTSTRING(SETTINGS_COLOR_BAD_CLIENT));
	stylesList->AppendString(CTSTRING(SETTINGS_COLOR_BAD_FILELIST));	
	stylesList->SetSelection(0);

	refreshPreview();
}

UserListPage::~UserListPage()
{
}

void UserListPage::onChange(wxCommandEvent &WXUNUSED(event))
{
	int index = stylesList->GetSelection();
	if (index == -1)
		return;
	
	wxColourData *data = new wxColourData();
	data->SetColour(colours[index]);
	wxColourDialog dlg(this, data);
	if (dlg.ShowModal() == wxID_OK)
	{
		colours[index] = dlg.GetColourData().GetColour().GetRGB();
		refreshPreview();
	}
	wxDELETE(data);
}

void UserListPage::onBrowse(wxCommandEvent &WXUNUSED(event))
{
	wxFileDialog dlg(this);
	if (dlg.ShowModal() == wxID_OK)
	{
		iconsEdit->SetValue(dlg.GetPath());
	}
}

void UserListPage::refreshPreview()
{
	previewRich->Clear();

	previewRich->BeginStyle(wxTextAttr(wxColor(colours[0])));
	previewRich->WriteText(TSTRING(SETTINGS_COLOR_NORMAL) + _T("\n"));
	previewRich->EndStyle();
	
	previewRich->BeginStyle(wxTextAttr(wxColor(colours[1])));
	previewRich->WriteText(TSTRING(SETTINGS_COLOR_FAVORITE) + _T("\n"));
	previewRich->EndStyle();
	
	previewRich->BeginStyle(wxTextAttr(wxColor(colours[2])));
	previewRich->WriteText(TSTRING(SETTINGS_COLOR_RESERVED) + _T("\n"));
	previewRich->EndStyle();
	
	previewRich->BeginStyle(wxTextAttr(wxColor(colours[3])));
	previewRich->WriteText(TSTRING(SETTINGS_COLOR_IGNORED) + _T("\n"));
	previewRich->EndStyle();
	
	previewRich->BeginStyle(wxTextAttr(wxColor(colours[4])));
	previewRich->WriteText(TSTRING(COLOR_FAST) + _T("\n"));
	previewRich->EndStyle();
	
	previewRich->BeginStyle(wxTextAttr(wxColor(colours[5])));
	previewRich->WriteText(TSTRING(COLOR_SERVER) + _T("\n"));
	previewRich->EndStyle();
	
	previewRich->BeginStyle(wxTextAttr(wxColor(colours[6])));
	previewRich->WriteText(TSTRING(COLOR_PASIVE) + _T("\n"));
	previewRich->EndStyle();
	
	previewRich->BeginStyle(wxTextAttr(wxColor(colours[7])));
	previewRich->WriteText(TSTRING(COLOR_OP) + _T("\n"));
	previewRich->EndStyle();
	
	previewRich->BeginStyle(wxTextAttr(wxColor(colours[8])));
	previewRich->WriteText(TSTRING(SETTINGS_COLOR_FULL_CHECKED) + _T("\n"));
	previewRich->EndStyle();
	
	previewRich->BeginStyle(wxTextAttr(wxColor(colours[9])));
	previewRich->WriteText(TSTRING(SETTINGS_COLOR_BAD_CLIENT) + _T("\n"));
	previewRich->EndStyle();
	
	previewRich->BeginStyle(wxTextAttr(wxColor(colours[10])));
	previewRich->WriteText(TSTRING(SETTINGS_COLOR_BAD_FILELIST));
	previewRich->EndStyle();
}

void UserListPage::write()
{
	SettingsPage::write(items);
	SettingsManager::getInstance()->set(SettingsManager::NORMAL_COLOUR, colours[0]);
	SettingsManager::getInstance()->set(SettingsManager::FAVORITE_COLOR, colours[1]);
	SettingsManager::getInstance()->set(SettingsManager::RESERVED_SLOT_COLOR, colours[2]);
	SettingsManager::getInstance()->set(SettingsManager::IGNORED_COLOR, colours[3]);
	SettingsManager::getInstance()->set(SettingsManager::FIREBALL_COLOR, colours[4]);
	SettingsManager::getInstance()->set(SettingsManager::SERVER_COLOR, colours[5]);
	SettingsManager::getInstance()->set(SettingsManager::PASIVE_COLOR, colours[6]);
	SettingsManager::getInstance()->set(SettingsManager::OP_COLOR, colours[7]);
	SettingsManager::getInstance()->set(SettingsManager::FULL_CHECKED_COLOUR, colours[8]);
	SettingsManager::getInstance()->set(SettingsManager::BAD_CLIENT_COLOUR, colours[9]);
	SettingsManager::getInstance()->set(SettingsManager::BAD_FILELIST_COLOUR, colours[10]);

	//WinUtil::reLoadImages();
}
