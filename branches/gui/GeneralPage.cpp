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

#include "GeneralPage.h"

SettingsPage::TextItem GeneralPage::texts[] = {
	{ IDC_SETTINGS_PERSONAL_INFORMATION, ResourceManager::SETTINGS_PERSONAL_INFORMATION },
	{ IDC_SETTINGS_NICK, ResourceManager::NICK },
	{ IDC_SETTINGS_EMAIL, ResourceManager::EMAIL },
	{ IDC_SETTINGS_DESCRIPTION, ResourceManager::DESCRIPTION },
	{ IDC_SETTINGS_UPLOAD_LINE_SPEED, ResourceManager::SETTINGS_UPLOAD_LINE_SPEED },
	{ IDC_SETTINGS_MEBIBITS, ResourceManager::MBITSPS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item GeneralPage::items[] = {
	{ IDC_NICK,			SettingsManager::NICK,			SettingsPage::T_STR },
	{ IDC_EMAIL,		SettingsManager::EMAIL,			SettingsPage::T_STR },
	{ IDC_DESCRIPTION,	SettingsManager::DESCRIPTION,	SettingsPage::T_STR },
	{ IDC_CONNECTION,	SettingsManager::UPLOAD_SPEED,	SettingsPage::T_STR },
	{ 0, 0, SettingsPage::T_END }
};

BEGIN_EVENT_TABLE(GeneralPage, SettingsPage)
	EVT_TEXT(IDC_NICK, GeneralPage::onTextUpdated)
	EVT_TEXT(IDC_EMAIL, GeneralPage::onTextUpdated)
	EVT_TEXT(IDC_DESCRIPTION, GeneralPage::onTextUpdated)
END_EVENT_TABLE()

GeneralPage::GeneralPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxTextValidator textValidator(wxFILTER_EXCLUDE_CHAR_LIST);
	wxArrayString _excludeList;
	_excludeList.Add(_T("$"));
	_excludeList.Add(_T("|"));
	_excludeList.Add(_T("<"));
	_excludeList.Add(_T(">"));
	_excludeList.Add(_T(" "));
	_excludeList.Shrink();
	textValidator.SetExcludes(_excludeList);

	wxTextValidator descrValidator(wxFILTER_EXCLUDE_CHAR_LIST);
	wxArrayString _descrExcludeList;
	_excludeList.Add(_T("$"));
	_excludeList.Add(_T("|"));
	_excludeList.Add(_T("<"));
	_excludeList.Add(_T(">"));
	_excludeList.Shrink();
	descrValidator.SetExcludes(_descrExcludeList);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_PERSONAL_INFORMATION, wxEmptyString), wxVERTICAL);

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer1->AddGrowableCol(1);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	nickLabel = new wxStaticText(this, IDC_SETTINGS_NICK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	nickLabel->Wrap(-1);
	fgSizer1->Add(nickLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 5);

	nickEdit = new wxTextCtrl(this, IDC_NICK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, textValidator);
	nickEdit->SetMaxLength(35);
	fgSizer1->Add(nickEdit, 0, wxALL | wxEXPAND, 5);

	emailLabel = new wxStaticText(this, IDC_SETTINGS_EMAIL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	emailLabel->Wrap(-1);
	fgSizer1->Add(emailLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 5);

	emailEdit = new wxTextCtrl(this, IDC_EMAIL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, textValidator);
	emailEdit->SetMaxLength(35);
	fgSizer1->Add(emailEdit, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);

	descrLabel = new wxStaticText(this, IDC_SETTINGS_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	descrLabel->Wrap(-1);
	fgSizer1->Add(descrLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 5);

	descrEdit = new wxTextCtrl(this, IDC_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, descrValidator);
	descrEdit->SetMaxLength(35);
	fgSizer1->Add(descrEdit, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);

	speedLabel = new wxStaticText(this, IDC_SETTINGS_UPLOAD_LINE_SPEED, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	speedLabel->Wrap(-1);
	fgSizer1->Add(speedLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 5);

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	speedCombo = new wxComboBox(this, IDC_CONNECTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	for(StringIter i = SettingsManager::connectionSpeeds.begin(); i != SettingsManager::connectionSpeeds.end(); ++i)
		speedCombo->AppendString(Text::toT(*i));
	speedCombo->SetSelection(speedCombo->FindString(
		settings->get((SettingsManager::StrSetting)SettingsManager::UPLOAD_SPEED, true)));
	bSizer2->Add(speedCombo, 2, wxBOTTOM | wxLEFT | wxRIGHT, 5);

	mbitLabel = new wxStaticText(this, IDC_SETTINGS_MEBIBITS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mbitLabel->Wrap(-1);
	bSizer2->Add(mbitLabel, 3, wxALIGN_CENTER_VERTICAL, 5);

	fgSizer1->Add(bSizer2, 1, wxEXPAND, 5);

	sbSizer1->Add(fgSizer1, 1, wxEXPAND, 5);

	bSizer1->Add(sbSizer1, 0, wxALL | wxEXPAND, 5);

	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	SettingsPage::translate(texts);
	SettingsPage::read(items);
}

void GeneralPage::onTextUpdated(wxCommandEvent& event)
{
	wxTextCtrl *ctrl = (wxTextCtrl*)event.GetEventObject();
	wxString str = ctrl->GetValue();
	long curPos = ctrl->GetInsertionPoint();
	if ((event.GetId() == IDC_NICK) || (event.GetId() == IDC_EMAIL))
	{
		str.Replace(_T(" "), wxEmptyString);
	}
	str.Replace(_T("$"), wxEmptyString);
	str.Replace(_T("|"), wxEmptyString);
	str.Replace(_T("<"), wxEmptyString);
	str.Replace(_T(">"), wxEmptyString);

	if (!ctrl->GetValue().IsSameAs(str))
	{
		ctrl->SetValue(str);
		ctrl->SetInsertionPoint(curPos);
	}
}

GeneralPage::~GeneralPage()
{
}

void GeneralPage::write()
{
	SettingsPage::write(items);
}
