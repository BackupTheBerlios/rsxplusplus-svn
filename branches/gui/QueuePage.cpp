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

#include "QueuePage.h"

SettingsPage::TextItem QueuePage::texts[] = {
	{ IDC_SETTINGS_AUTOPRIO, ResourceManager::SETTINGS_PRIO_AUTOPRIO },
	{ IDC_SETTINGS_PRIO_HIGHEST, ResourceManager::SETTINGS_PRIO_HIGHEST },
	{ IDC_SETTINGS_KB3, ResourceManager::KB },
	{ IDC_SETTINGS_PRIO_HIGH, ResourceManager::SETTINGS_PRIO_HIGH },
	{ IDC_SETTINGS_KB4, ResourceManager::KB },
	{ IDC_SETTINGS_PRIO_NORMAL, ResourceManager::SETTINGS_PRIO_NORMAL },
	{ IDC_SETTINGS_KB5, ResourceManager::KB },
	{ IDC_SETTINGS_PRIO_LOW, ResourceManager::SETTINGS_PRIO_LOW },
	{ IDC_SETTINGS_KB6, ResourceManager::KB },
	{ IDC_SETTINGS_SEGMENT, ResourceManager::SETTINGS_SEGMENT },
	{ IDC_AUTOSEGMENT, ResourceManager::SETTINGS_AUTO_SEARCH },
	{ IDC_DONTBEGIN, ResourceManager::DONT_ADD_SEGMENT_TEXT },
	{ IDC_MINUTES, ResourceManager::MINUTES },
	{ IDC_KBPS, ResourceManager::KBPS },
	{ IDC_CHUNKCOUNT, ResourceManager::TEXT_MANUAL },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item QueuePage::items[] = {
	{ IDC_PRIO_HIGHEST_SIZE, SettingsManager::PRIO_HIGHEST_SIZE, SettingsPage::T_INT },
	{ IDC_PRIO_HIGH_SIZE, SettingsManager::PRIO_HIGH_SIZE, SettingsPage::T_INT },
	{ IDC_PRIO_NORMAL_SIZE, SettingsManager::PRIO_NORMAL_SIZE, SettingsPage::T_INT },
	{ IDC_PRIO_LOW_SIZE, SettingsManager::PRIO_LOW_SIZE, SettingsPage::T_INT },
	{ IDC_AUTOSEGMENT, SettingsManager::AUTO_SEARCH, SettingsPage::T_BOOL },
	{ IDC_DONTBEGIN, SettingsManager::DONT_BEGIN_SEGMENT, SettingsPage::T_BOOL },
	{ IDC_BEGIN_EDIT, SettingsManager::DONT_BEGIN_SEGMENT_SPEED, SettingsPage::T_INT },
	{ IDC_SEARCH_EDIT, SettingsManager::SEARCH_TIME, SettingsPage::T_INT },
	{ IDC_CHUNKCOUNT, SettingsManager::SEGMENTS_MANUAL, SettingsPage::T_BOOL },
	{ IDC_SEG_NUMBER, SettingsManager::NUMBER_OF_SEGMENTS, SettingsPage::T_INT },
	{ 0, 0, SettingsPage::T_END }
};

SettingsPage::ListItem QueuePage::optionItems[] = {
	{ SettingsManager::MULTI_CHUNK, ResourceManager::ENABLE_MULTI_SOURCE },
	{ SettingsManager::PRIO_LOWEST, ResourceManager::SETTINGS_PRIO_LOWEST },
	{ SettingsManager::AUTO_SEARCH_AUTO_MATCH, ResourceManager::SETTINGS_AUTO_SEARCH_AUTO_MATCH },
	{ SettingsManager::SKIP_ZERO_BYTE, ResourceManager::SETTINGS_SKIP_ZERO_BYTE },
	{ SettingsManager::DONT_DL_ALREADY_SHARED, ResourceManager::SETTINGS_DONT_DL_ALREADY_SHARED },
	{ SettingsManager::ANTI_FRAG, ResourceManager::SETTINGS_ANTI_FRAG },
	{ SettingsManager::AUTO_PRIORITY_DEFAULT, ResourceManager::SETTINGS_AUTO_PRIORITY_DEFAULT },
	{ SettingsManager::OVERLAP_CHUNKS, ResourceManager::OVERLAP_CHUNKS },
	{ SettingsManager::KEEP_FINISHED_FILES, ResourceManager::KEEP_FINISHED_FILES },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

QueuePage::QueuePage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_AUTOPRIO, wxEmptyString), wxVERTICAL);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(2, 6, 0, 0);
	fgSizer1->AddGrowableCol(2);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	highestPrioLabel = new wxStaticText(this, IDC_SETTINGS_PRIO_HIGHEST, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	highestPrioLabel->Wrap(-1);
	fgSizer1->Add(highestPrioLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
	
	highestPrioSpin = new wxSpinCtrl(this, IDC_PRIO_HIGHEST_SIZE, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 10, 0);
	fgSizer1->Add(highestPrioSpin, 0, wxALL, 5);
	
	highestKibLabel = new wxStaticText(this, IDC_SETTINGS_KB3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	highestKibLabel->Wrap(-1);
	fgSizer1->Add(highestKibLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	
	highPrioLabel = new wxStaticText(this, IDC_SETTINGS_PRIO_HIGH, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	highPrioLabel->Wrap(-1);
	fgSizer1->Add(highPrioLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
	
	highPrioSpin = new wxSpinCtrl(this, IDC_PRIO_HIGH_SIZE, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 10, 0);
	fgSizer1->Add(highPrioSpin, 0, wxALL, 5);
	
	highKibLabel = new wxStaticText(this, IDC_SETTINGS_KB4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	highKibLabel->Wrap(-1);
	fgSizer1->Add(highKibLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	
	normPrioLabel = new wxStaticText(this, IDC_SETTINGS_PRIO_NORMAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	normPrioLabel->Wrap(-1);
	fgSizer1->Add(normPrioLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
	
	normPrioSpin = new wxSpinCtrl(this, IDC_PRIO_NORMAL_SIZE, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 10, 0);
	fgSizer1->Add(normPrioSpin, 0, wxALL, 5);
	
	normKibLabel = new wxStaticText(this, IDC_SETTINGS_KB5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	normKibLabel->Wrap(-1);
	fgSizer1->Add(normKibLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	
	lowPrioLabel = new wxStaticText(this, IDC_SETTINGS_PRIO_LOW, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	lowPrioLabel->Wrap(-1);
	fgSizer1->Add(lowPrioLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
	
	lowPrioSpin = new wxSpinCtrl(this, IDC_PRIO_LOW_SIZE, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 10, 0);
	fgSizer1->Add(lowPrioSpin, 0, wxALL, 5);
	
	lowKibLabel = new wxStaticText(this, IDC_SETTINGS_KB6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	lowKibLabel->Wrap(-1);
	fgSizer1->Add(lowKibLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	
	sbSizer1->Add(fgSizer1, 1, wxEXPAND, 5);
	
	bSizer1->Add(sbSizer1, 0, wxALL | wxEXPAND, 5);
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_SEGMENT, wxEmptyString), wxVERTICAL);
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer(3, 3, 0, 0);
	fgSizer2->AddGrowableCol(0);
	fgSizer2->SetFlexibleDirection(wxBOTH);
	fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	autoSearchTthCheck = new wxCheckBox(this, IDC_AUTOSEGMENT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	
	fgSizer2->Add(autoSearchTthCheck, 1, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxLEFT, 5);
	
	autoSearchTthSpin = new wxSpinCtrl(this, IDC_SEARCH_EDIT, wxEmptyString, wxDefaultPosition, wxSize(65,-1), wxSP_ARROW_KEYS, 5, 60, 10);
	fgSizer2->Add(autoSearchTthSpin, 0, wxALL, 5);
	
	autoSearchTthLabel = new wxStaticText(this, IDC_MINUTES, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	autoSearchTthLabel->Wrap(-1);
	fgSizer2->Add(autoSearchTthLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxTOP, 5);
	
	noNewSegCheck = new wxCheckBox(this, IDC_DONTBEGIN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	
	fgSizer2->Add(noNewSegCheck, 1, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	noNewSegSpin = new wxSpinCtrl(this, IDC_BEGIN_EDIT, wxEmptyString, wxDefaultPosition, wxSize(65,-1), wxSP_ARROW_KEYS, 2, 100000, 512);
	fgSizer2->Add(noNewSegSpin, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	noNewSegLabel = new wxStaticText(this, IDC_KBPS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	noNewSegLabel->Wrap(-1);
	fgSizer2->Add(noNewSegLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	
	segNumCheck = new wxCheckBox(this, IDC_CHUNKCOUNT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	
	fgSizer2->Add(segNumCheck, 1, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	segNumSpin = new wxSpinCtrl(this, IDC_SEG_NUMBER, wxEmptyString, wxDefaultPosition, wxSize(65,-1), wxSP_ARROW_KEYS, 2, 10, 3);
	fgSizer2->Add(segNumSpin, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	sbSizer2->Add(fgSizer2, 0, wxEXPAND, 5);
	
	bSizer1->Add(sbSizer2, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);
	
	wxArrayString queueOptsListChoices;
	queueOptsList = new wxCheckListBox(this, IDC_OTHER_QUEUE_OPTIONS, wxDefaultPosition, wxSize(-1,160), queueOptsListChoices, 0);
	bSizer1->Add(queueOptsList, 0, wxALL | wxEXPAND, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();

	SettingsPage::translate(texts);
	SettingsPage::read(items);
	SettingsPage::read(items, optionItems, queueOptsList);
}

QueuePage::~QueuePage()
{
}

void QueuePage::write()
{
	SettingsPage::write(items);
	SettingsPage::write(items, optionItems, queueOptsList);
}