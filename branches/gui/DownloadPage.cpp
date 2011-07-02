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

#include "DownloadPage.h"
#include "PublicHubListDlg.h"

BEGIN_EVENT_TABLE(DownloadPage, SettingsPage)
	EVT_BUTTON(IDC_BROWSEDIR, DownloadPage::onDirBrowse)
	EVT_BUTTON(IDC_BROWSETEMPDIR, DownloadPage::onTempDirBrowse)
	EVT_BUTTON(IDC_SETTINGS_LIST_CONFIG, DownloadPage::onConfigureLists)
END_EVENT_TABLE()

SettingsPage::TextItem DownloadPage::texts[] = {
	{ IDC_SETTINGS_DIRECTORIES, ResourceManager::SETTINGS_DIRECTORIES }, 
	{ IDC_SETTINGS_DOWNLOAD_DIRECTORY, ResourceManager::SETTINGS_DOWNLOAD_DIRECTORY },
	{ IDC_BROWSEDIR, ResourceManager::BROWSE_ACCEL },
	{ IDC_SETTINGS_UNFINISHED_DOWNLOAD_DIRECTORY, ResourceManager::SETTINGS_UNFINISHED_DOWNLOAD_DIRECTORY }, 
	{ IDC_BROWSETEMPDIR, ResourceManager::BROWSE }, 
	{ IDC_SETTINGS_DOWNLOAD_LIMITS, ResourceManager::SETTINGS_DOWNLOAD_LIMITS },
	{ IDC_SETTINGS_DOWNLOADS_MAX, ResourceManager::SETTINGS_DOWNLOADS_MAX },
	{ IDC_SETTINGS_FILES_MAX, ResourceManager::SETTINGS_FILES_MAX },
	{ IDC_EXTRA_DOWNLOADS_MAX, ResourceManager::SETTINGS_STRONGDC_EXTRA_DOWNLOADS },
	{ IDC_SETTINGS_DOWNLOADS_SPEED_PAUSE, ResourceManager::SETTINGS_DOWNLOADS_SPEED_PAUSE },
	{ IDC_SETTINGS_SPEEDS_NOT_ACCURATE, ResourceManager::SETTINGS_SPEEDS_NOT_ACCURATE },
	{ IDC_SETTINGS_PUBLIC_HUB_LIST, ResourceManager::SETTINGS_PUBLIC_HUB_LIST },
	{ IDC_SETTINGS_PUBLIC_HUB_LIST_URL, ResourceManager::SETTINGS_PUBLIC_HUB_LIST_URL },
	{ IDC_SETTINGS_LIST_CONFIG, ResourceManager::SETTINGS_CONFIGURE_HUB_LISTS },
	{ IDC_SETTINGS_PUBLIC_HUB_LIST_HTTP_PROXY, ResourceManager::SETTINGS_PUBLIC_HUB_LIST_HTTP_PROXY },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item DownloadPage::items[] = {
	{ IDC_TEMP_DOWNLOAD_DIRECTORY, SettingsManager::TEMP_DOWNLOAD_DIRECTORY, SettingsPage::T_STR },
	{ IDC_DOWNLOADDIR,	SettingsManager::DOWNLOAD_DIRECTORY, SettingsPage::T_STR }, 
	{ IDC_DOWNLOADS, SettingsManager::DOWNLOAD_SLOTS, SettingsPage::T_INT },
	{ IDC_FILES, SettingsManager::FILE_SLOTS, SettingsPage::T_INT },
	{ IDC_MAXSPEED, SettingsManager::MAX_DOWNLOAD_SPEED, SettingsPage::T_INT },
	{ IDC_EXTRA_DOWN_SLOT, SettingsManager::EXTRA_DOWNLOAD_SLOTS, SettingsPage::T_INT },
	{ IDC_PROXY, SettingsManager::HTTP_PROXY, SettingsPage::T_STR },
	{ 0, 0, SettingsPage::T_END }
};

DownloadPage::DownloadPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_DIRECTORIES, wxEmptyString), wxVERTICAL);

	defDirLabel = new wxStaticText(this, IDC_SETTINGS_DOWNLOAD_DIRECTORY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	defDirLabel->Wrap(-1);
	sbSizer1->Add(defDirLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	defDirEdit = new wxTextCtrl(this, IDC_DOWNLOADDIR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(defDirEdit, 1, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	defDirButton = new wxButton(this, IDC_BROWSEDIR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(defDirButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	sbSizer1->Add(bSizer2, 0, wxEXPAND, 5);

	tempDirLabel = new wxStaticText(this, IDC_SETTINGS_UNFINISHED_DOWNLOAD_DIRECTORY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	tempDirLabel->Wrap(-1);
	sbSizer1->Add(tempDirLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);

	tempDirEdit = new wxTextCtrl(this, IDC_TEMP_DOWNLOAD_DIRECTORY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer3->Add(tempDirEdit, 1, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	tempDirButton = new wxButton(this, IDC_BROWSETEMPDIR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer3->Add(tempDirButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	sbSizer1->Add(bSizer3, 0, wxEXPAND, 5);

	bSizer1->Add(sbSizer1, 0, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_DOWNLOAD_LIMITS, wxEmptyString), wxVERTICAL);

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(4, 2, 0, 0);
	fgSizer1->AddGrowableCol(1);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	maxSimFilesSpin = new wxSpinCtrl(this, IDC_FILES, wxEmptyString, wxDefaultPosition, wxSize(60,-1),
		wxSP_ARROW_KEYS, 0, 100);
	fgSizer1->Add(maxSimFilesSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	maxSimFilesLabel = new wxStaticText(this, IDC_SETTINGS_FILES_MAX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	maxSimFilesLabel->Wrap(-1);
	fgSizer1->Add(maxSimFilesLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	maxSimDownSpin = new wxSpinCtrl(this, IDC_DOWNLOADS, wxEmptyString, wxDefaultPosition, wxSize(60,-1),
		wxSP_ARROW_KEYS, 0, 100);
	fgSizer1->Add(maxSimDownSpin, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);

	maxSimDownLabel = new wxStaticText(this, IDC_SETTINGS_DOWNLOADS_MAX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	maxSimDownLabel->Wrap(-1);
	fgSizer1->Add(maxSimDownLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	highExtraSlotSpin = new wxSpinCtrl(this, IDC_EXTRA_DOWN_SLOT, wxEmptyString, wxDefaultPosition, wxSize(60,-1),
		wxSP_ARROW_KEYS, 0, 100);
	fgSizer1->Add(highExtraSlotSpin, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);

	highExtraSlotLabel = new wxStaticText(this, IDC_EXTRA_DOWNLOADS_MAX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	highExtraSlotLabel->Wrap(-1);
	fgSizer1->Add(highExtraSlotLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	noNewIfExceedSpin = new wxSpinCtrl(this, IDC_MAXSPEED, wxEmptyString, wxDefaultPosition, wxSize(60,-1),
		wxSP_ARROW_KEYS, 0, 10000);
	noNewIfExceedSpin->SetLabel(_T("5"));
	fgSizer1->Add(noNewIfExceedSpin, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);

	noNewIfExceedLabel = new wxStaticText(this, IDC_SETTINGS_DOWNLOADS_SPEED_PAUSE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	noNewIfExceedLabel->Wrap(-1);
	fgSizer1->Add(noNewIfExceedLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	sbSizer2->Add(fgSizer1, 1, wxEXPAND, 5);

	limitNoteLabel = new wxStaticText(this, IDC_SETTINGS_SPEEDS_NOT_ACCURATE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	limitNoteLabel->Wrap(-1);
	sbSizer2->Add(limitNoteLabel, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);

	bSizer1->Add(sbSizer2, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_PUBLIC_HUB_LIST, wxEmptyString), wxVERTICAL);

	pubHubUrlLabel = new wxStaticText(this, IDC_SETTINGS_PUBLIC_HUB_LIST_URL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	pubHubUrlLabel->Wrap(-1);
	sbSizer3->Add(pubHubUrlLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	confPubHubButton = new wxButton(this, IDC_SETTINGS_LIST_CONFIG, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer3->Add(confPubHubButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	hublistProxyLabel = new wxStaticText(this, IDC_SETTINGS_PUBLIC_HUB_LIST_HTTP_PROXY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	hublistProxyLabel->Wrap(-1);
	sbSizer3->Add(hublistProxyLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	hublistProxyEdit = new wxTextCtrl(this, IDC_PROXY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer3->Add(hublistProxyEdit, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	bSizer1->Add(sbSizer3, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	this->SetSizer(bSizer1);
	this->Layout();

	SettingsPage::translate(texts);
	SettingsPage::read(items);
}

DownloadPage::~DownloadPage()
{
}

void DownloadPage::onDirBrowse(wxCommandEvent &WXUNUSED(event))
{
	wxDirDialog dlg(this);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxString path = dlg.GetPath();
		if (path.Last() != PATH_SEPARATOR)
			defDirEdit->SetValue(path + PATH_SEPARATOR);
		else
			defDirEdit->SetValue(path);
	}
}

void DownloadPage::onTempDirBrowse(wxCommandEvent &WXUNUSED(event))
{
	wxDirDialog dlg(this);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxString path = dlg.GetPath();
		if (path.Last() != PATH_SEPARATOR)
			tempDirEdit->SetValue(path + PATH_SEPARATOR);
		else
			tempDirEdit->SetValue(path);
	}
}

void DownloadPage::onConfigureLists(wxCommandEvent &WXUNUSED(event))
{
	PublicHubListDlg dlg(this);
	dlg.ShowModal();
}

void DownloadPage::write()
{
	SettingsPage::write(items);
}
