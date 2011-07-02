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

#include "PopupsPage.h"
#include "WinUtil.h"

BEGIN_EVENT_TABLE(PopupsPage, SettingsPage)
	EVT_BUTTON(IDC_PREVIEW, PopupsPage::onPreview)
END_EVENT_TABLE()

SettingsPage::TextItem PopupsPage::texts[] = {
	{ IDC_POPUPGROUP, ResourceManager::BALLOON_POPUPS },
	{ IDC_PREVIEW, ResourceManager::SETSTRONGDC_PREVIEW },
	//{ IDC_POPUPTYPE, ResourceManager::POPUP_TYPE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item PopupsPage::items[] = {
	{ 0, 0, SettingsPage::T_END }
};

SettingsPage::ListItem PopupsPage::listItems[] = {
	{ SettingsManager::POPUP_HUB_CONNECTED, ResourceManager::POPUP_HUB_CONNECTED },
	{ SettingsManager::POPUP_HUB_DISCONNECTED, ResourceManager::POPUP_HUB_DISCONNECTED },
	{ SettingsManager::POPUP_FAVORITE_CONNECTED, ResourceManager::POPUP_FAVORITE_CONNECTED },
	{ SettingsManager::POPUP_CHEATING_USER, ResourceManager::POPUP_CHEATING_USER },
	{ SettingsManager::POPUP_DOWNLOAD_START, ResourceManager::POPUP_DOWNLOAD_START },
	{ SettingsManager::POPUP_DOWNLOAD_FAILED, ResourceManager::POPUP_DOWNLOAD_FAILED },
	{ SettingsManager::POPUP_DOWNLOAD_FINISHED, ResourceManager::POPUP_DOWNLOAD_FINISHED },
	{ SettingsManager::POPUP_UPLOAD_FINISHED, ResourceManager::POPUP_UPLOAD_FINISHED },
	{ SettingsManager::POPUP_PM, ResourceManager::POPUP_PM },
	{ SettingsManager::POPUP_NEW_PM, ResourceManager::POPUP_NEW_PM },
	{ SettingsManager::POPUP_AWAY, ResourceManager::SHOW_POPUP_AWAY },
	{ SettingsManager::POPUP_MINIMIZED, ResourceManager::SHOW_POPUP_MINIMIZED },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PopupsPage::PopupsPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_POPUPGROUP, wxEmptyString), wxVERTICAL);
	
	wxArrayString popupListChoices;
	popupList = new wxCheckListBox(this, IDC_POPUPLIST, wxDefaultPosition, wxSize(-1,300), popupListChoices, 0);
	sbSizer1->Add(popupList, 0, wxALL | wxEXPAND, 5);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(1, 3, 0, 0);
	fgSizer1->AddGrowableCol(2);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
/* FIXME? Currently we support only OS native notifications - is it worth implementing some other generic way?		
	typeLabel = new wxStaticText(this, IDC_POPUPTYPE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	typeLabel->Wrap(-1);
	fgSizer1->Add(typeLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	typeCombo = new wxComboBox(this, IDC_COMBO1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	typeCombo->AppendString(_T("Balloon popup"));
	typeCombo->AppendString(_T("Window popup"));
	typeCombo->SetSelection(SETTING(POPUP_TYPE));
	fgSizer1->Add(typeCombo, 0, wxALL, 5);
*/	
	previewButton = new wxButton(this, IDC_PREVIEW, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(previewButton, 0, wxALL | wxALIGN_RIGHT, 5);
	
	sbSizer1->Add(fgSizer1, 0, wxEXPAND, 5);
	
	bSizer1->Add(sbSizer1, 1, wxEXPAND | wxTOP | wxBOTTOM, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	SettingsPage::translate(texts);
	SettingsPage::read(items, listItems, popupList);
}

PopupsPage::~PopupsPage()
{
}

void PopupsPage::onPreview(wxCommandEvent &WXUNUSED(event))
{
	WinUtil::notifyUser(TSTRING(DOWNLOAD_FINISHED_IDLE), TSTRING(FILE) + _T(": sdc30-64.7z\n") +
		TSTRING(USER) + _T(": ") + Text::toT(SETTING(NICK)));
}

void PopupsPage::write()
{
	SettingsPage::write(items, listItems, popupList);

	// Not supported: SettingsManager::getInstance()->set(SettingsManager::POPUP_TYPE, typeCombo->GetSelection());
}