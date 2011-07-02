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
#include "../client/ShareManager.h"

#include "UploadPage.h"

SettingsPage::TextItem UploadPage::texts[] = {
	{ IDC_SLOTS_GROUP, ResourceManager::SLOTS },
	{ IDC_SETTINGS_UPLOADS_MIN_SPEED, ResourceManager::SETTINGS_UPLOADS_MIN_SPEED },
	{ IDC_SETTINGS_KBPS, ResourceManager::KBPS }, 
	{ IDC_SETTINGS_UPLOADS_SLOTS, ResourceManager::SETTINGS_UPLOADS_SLOTS },
	{ IDC_STRONGDC_SMALL_SLOTS, ResourceManager::SETSTRONGDC_SMALL_UP_SLOTS },
	{ IDC_STRONGDC_SMALL_SIZE, ResourceManager::SETSTRONGDC_SMALL_FILES },
	{ IDC_STRONGDC_NOTE_SMALL, ResourceManager::SETSTRONGDC_NOTE_SMALL_UP },
	{ IDC_SETTINGS_AUTO_SLOTS, ResourceManager::SETTINGS_AUTO_SLOTS	},	
	{ IDC_SETTINGS_PARTIAL_SLOTS, ResourceManager::SETSTRONGDC_PARTIAL_SLOTS },		
	{ IDC_STATICb, ResourceManager::EXTRA_HUB_SLOTS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY },
};
SettingsPage::Item UploadPage::items[] = {
	{ IDC_SLOTS, SettingsManager::SLOTS, SettingsPage::T_INT }, 
	{ IDC_MIN_UPLOAD_SPEED, SettingsManager::MIN_UPLOAD_SPEED, SettingsPage::T_INT },
	{ IDC_EXTRA_SLOTS, SettingsManager::EXTRA_SLOTS, SettingsPage::T_INT },
	{ IDC_SMALL_FILE_SIZE, SettingsManager::SET_MINISLOT_SIZE, SettingsPage::T_INT },
	{ IDC_EXTRA_SLOTS2, SettingsManager::HUB_SLOTS, SettingsPage::T_INT },
	{ IDC_AUTO_SLOTS, SettingsManager::AUTO_SLOTS, SettingsPage::T_INT  },
	{ IDC_PARTIAL_SLOTS, SettingsManager::EXTRA_PARTIAL_SLOTS, SettingsPage::T_INT  },
	{ 0, 0, SettingsPage::T_END }
};

UploadPage::UploadPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SLOTS_GROUP, wxEmptyString), wxVERTICAL);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(2, 5, 0, 0);
	fgSizer1->AddGrowableCol(2);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	upSlotLabel = new wxStaticText(this, IDC_SETTINGS_UPLOADS_SLOTS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	upSlotLabel->Wrap(-1);
	fgSizer1->Add(upSlotLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 5);
	
	upSlotSpin = new wxSpinCtrl(this, IDC_SLOTS, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 1, UD_MAXVAL, 1);
	fgSizer1->Add(upSlotSpin, 0, wxALL, 5);
	
	slotRatioLabel = new wxStaticText(this, IDC_STATICb, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	slotRatioLabel->Wrap(-1);
	fgSizer1->Add(slotRatioLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 5);
	
	slotRatioSpin = new wxSpinCtrl(this, IDC_EXTRA_SLOTS2, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, UD_MAXVAL, 0);
	fgSizer1->Add(slotRatioSpin, 0, wxALL, 5);
	
	slotsEmptyLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	slotsEmptyLabel->Wrap(-1);
	fgSizer1->Add(slotsEmptyLabel, 0, wxALL, 5);
	
	smallSlotLabel = new wxStaticText(this, IDC_STRONGDC_SMALL_SLOTS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	smallSlotLabel->Wrap(-1);
	fgSizer1->Add(smallSlotLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	smallSlotSpin = new wxSpinCtrl(this, IDC_EXTRA_SLOTS, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 3, 100, 3);
	fgSizer1->Add(smallSlotSpin, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	smallFileLabel = new wxStaticText(this, IDC_STRONGDC_SMALL_SIZE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	smallFileLabel->Wrap(-1);
	fgSizer1->Add(smallFileLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	smallFileSpin = new wxSpinCtrl(this, IDC_SMALL_FILE_SIZE, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 64, 30000, 64);
	fgSizer1->Add(smallFileSpin, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	smallKibLabel = new wxStaticText(this, wxID_ANY, _("KiB"), wxDefaultPosition, wxDefaultSize, 0);
	smallKibLabel->Wrap(-1);
	fgSizer1->Add(smallKibLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	sbSizer1->Add(fgSizer1, 1, wxEXPAND, 5);
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer(3, 3, 0, 0);
	fgSizer2->AddGrowableCol(0);
	fgSizer2->SetFlexibleDirection(wxBOTH);
	fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	openExtraLabel = new wxStaticText(this, IDC_SETTINGS_UPLOADS_MIN_SPEED, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	openExtraLabel->Wrap(-1);
	fgSizer2->Add(openExtraLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	
	openExtraSpin = new wxSpinCtrl(this, IDC_MIN_UPLOAD_SPEED, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 10, 0);
	fgSizer2->Add(openExtraSpin, 0, wxALL, 5);
	
	openKibLabel = new wxStaticText(this, IDC_SETTINGS_KBPS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	openKibLabel->Wrap(-1);
	fgSizer2->Add(openKibLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	
	maxAutoLabel = new wxStaticText(this, IDC_SETTINGS_AUTO_SLOTS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	maxAutoLabel->Wrap(-1);
	fgSizer2->Add(maxAutoLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	maxAutoSpin = new wxSpinCtrl(this, IDC_AUTO_SLOTS, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 100, 0);
	fgSizer2->Add(maxAutoSpin, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	slotsEmpty2Label = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	slotsEmpty2Label->Wrap(-1);
	fgSizer2->Add(slotsEmpty2Label, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	extraPartLabel = new wxStaticText(this, IDC_SETTINGS_PARTIAL_SLOTS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	extraPartLabel->Wrap(-1);
	fgSizer2->Add(extraPartLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	extraPartSpin = new wxSpinCtrl(this, IDC_PARTIAL_SLOTS, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 10, 0);
	fgSizer2->Add(extraPartSpin, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	sbSizer1->Add(fgSizer2, 1, wxEXPAND, 5);
	
	slotsNoteLabel = new wxStaticText(this, IDC_STRONGDC_NOTE_SMALL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	slotsNoteLabel->Wrap(-1);
	sbSizer1->Add(slotsNoteLabel, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL, 5);
	
	bSizer1->Add(sbSizer1, 0, wxALL | wxEXPAND, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);
	
	SettingsPage::translate(texts);
	SettingsPage::read(items);
}

UploadPage::~UploadPage()
{
}

void UploadPage::write()
{
	SettingsPage::write(items);
	
	if(SETTING(SLOTS) < 1)
		settings->set(SettingsManager::SLOTS, 1);

	// Do specialized writing here
	if(SETTING(EXTRA_SLOTS) < 3)
		settings->set(SettingsManager::EXTRA_SLOTS, 3);

	if(SETTING(SET_MINISLOT_SIZE) < 64)
		settings->set(SettingsManager::SET_MINISLOT_SIZE, 64);

	if(SETTING(HUB_SLOTS) < 0)
		settings->set(SettingsManager::HUB_SLOTS, 0);
		
	if(SETTING(AUTO_SLOTS) < 0)
		settings->set(SettingsManager::AUTO_SLOTS, 0);		
}