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

#include "LimitsPage.h"

BEGIN_EVENT_TABLE(LimitsPage, SettingsPage)
	EVT_CHECKBOX(IDC_THROTTLE_ENABLE, LimitsPage::onEnableCheck)
	EVT_CHECKBOX(IDC_TIME_LIMITING, LimitsPage::onAltCheck)
END_EVENT_TABLE()

SettingsPage::TextItem LimitsPage::texts[] = {
	{ IDC_THROTTLE_ENABLE, ResourceManager::ENABLE_LIMITING },
	{ IDC_STRONGDC_TRANSFER_LIMITING, ResourceManager::SETSTRONGDC_TRANSFER_LIMITING },
	{ IDC_STRONGDC_UP_SPEED, ResourceManager::SETSTRONGDC_UPLOAD_SPEED },
	{ IDC_STRONGDC_UP_SPEED1, ResourceManager::SETSTRONGDC_UPLOAD_SPEED },
	{ IDC_SETTINGS_KBPS1, ResourceManager::KBPS },
	{ IDC_SETTINGS_KBPS2, ResourceManager::KBPS },
	{ IDC_SETTINGS_KBPS3, ResourceManager::KBPS },
	{ IDC_SETTINGS_KBPS4, ResourceManager::KBPS },
	{ IDC_SETTINGS_KBPS5, ResourceManager::KBPS },
	{ IDC_SETTINGS_KBPS6, ResourceManager::KBPS },
	{ IDC_SETTINGS_KBPS7, ResourceManager::KBPS },
	{ IDC_SETTINGS_MINUTES, ResourceManager::SECONDS },
	{ IDC_STRONGDC_DW_SPEED, ResourceManager::SETSTRONGDC_DOWNLOAD_SPEED },
	{ IDC_STRONGDC_DW_SPEED1, ResourceManager::SETSTRONGDC_DOWNLOAD_SPEED },
	{ IDC_TIME_LIMITING, ResourceManager::SETSTRONGDC_ALTERNATE_LIMITING },
	{ IDC_STRONGDC_TO, ResourceManager::SETSTRONGDC_TO },
	//{ IDC_STRONGDC_SECONDARY_TRANSFER, ResourceManager::SETSTRONGDC_SECONDARY_LIMITING },
	{ IDC_STRONGDC_UP_NOTE, ResourceManager::SETSTRONGDC_NOTE_UPLOAD },
	{ IDC_STRONGDC_DW_NOTE, ResourceManager::SETSTRONGDC_NOTE_DOWNLOAD },
	{ IDC_STRONGDC_SLOW_DISCONNECT, ResourceManager::SETSTRONGDC_SLOW_DISCONNECT },
	{ IDC_SEGMENTED_ONLY, ResourceManager::SETTINGS_AUTO_DROP_SEGMENTED_SOURCE },
	{ IDC_STRONGDC_I_DOWN_SPEED, ResourceManager::SETSTRONGDC_I_DOWN_SPEED },
	{ IDC_STRONGDC_TIME_DOWN, ResourceManager::SETSTRONGDC_TIME_DOWN },
	{ IDC_STRONGDC_H_DOWN_SPEED, ResourceManager::SETSTRONGDC_H_DOWN_SPEED },
	{ IDC_STRONGDC_MIN_FILE_SIZE, ResourceManager::SETSTRONGDC_MIN_FILE_SIZE },
	{ IDC_SETTINGS_MB, ResourceManager::MB },
	{ IDC_REMOVE_IF, ResourceManager::NEW_DISCONNECT },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
}; 

SettingsPage::Item LimitsPage::items[] = {
	{ IDC_MX_UP_SP_LMT_NORMAL, SettingsManager::MAX_UPLOAD_SPEED_LIMIT, SettingsPage::T_INT },
	{ IDC_MX_DW_SP_LMT_NORMAL, SettingsManager::MAX_DOWNLOAD_SPEED_LIMIT, SettingsPage::T_INT },
	{ IDC_TIME_LIMITING, SettingsManager::TIME_DEPENDENT_THROTTLE, SettingsPage::T_BOOL },
	{ IDC_MX_UP_SP_LMT_TIME, SettingsManager::MAX_UPLOAD_SPEED_LIMIT_TIME, SettingsPage::T_INT },
	{ IDC_MX_DW_SP_LMT_TIME, SettingsManager::MAX_DOWNLOAD_SPEED_LIMIT_TIME, SettingsPage::T_INT },
	//{ IDC_BW_START_TIME, SettingsManager::BANDWIDTH_LIMIT_START, SettingsPage::T_INT },
	//{ IDC_BW_END_TIME, SettingsManager::BANDWIDTH_LIMIT_END, SettingsPage::T_INT },
	{ IDC_THROTTLE_ENABLE, SettingsManager::THROTTLE_ENABLE, SettingsPage::T_BOOL },
	{ IDC_I_DOWN_SPEED, SettingsManager::DISCONNECT_SPEED, SettingsPage::T_INT },
	{ IDC_TIME_DOWN, SettingsManager::DISCONNECT_TIME, SettingsPage::T_INT },
	{ IDC_H_DOWN_SPEED, SettingsManager::DISCONNECT_FILE_SPEED, SettingsPage::T_INT },
	{ IDC_SEGMENTED_ONLY, SettingsManager::DROP_MULTISOURCE_ONLY, SettingsPage::T_BOOL },
	{ IDC_MIN_FILE_SIZE, SettingsManager::DISCONNECT_FILESIZE, SettingsPage::T_INT },
	{ IDC_REMOVE_IF_BELOW, SettingsManager::REMOVE_SPEED, SettingsPage::T_INT },
	{ 0, 0, SettingsPage::T_END }
};

LimitsPage::LimitsPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_STRONGDC_SLOW_DISCONNECT, wxEmptyString), wxVERTICAL);
	
	segmentedCheck = new wxCheckBox(this, IDC_SEGMENTED_ONLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	
	sbSizer1->Add(segmentedCheck, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(5, 3, 0, 0);
	fgSizer1->AddGrowableCol(1);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	disconnectLabel = new wxStaticText(this, IDC_STRONGDC_I_DOWN_SPEED, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	disconnectLabel->Wrap(-1);
	fgSizer1->Add(disconnectLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	disconnectSpin = new wxSpinCtrl(this, IDC_I_DOWN_SPEED, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 1, 99999, 1);
	fgSizer1->Add(disconnectSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	kbps1Label = new wxStaticText(this, IDC_SETTINGS_KBPS5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	kbps1Label->Wrap(-1);
	fgSizer1->Add(kbps1Label, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	moreLabel = new wxStaticText(this, IDC_STRONGDC_TIME_DOWN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	moreLabel->Wrap(-1);
	fgSizer1->Add(moreLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	moreSpin = new wxSpinCtrl(this, IDC_TIME_DOWN, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 10, 180, 10);
	fgSizer1->Add(moreSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	secLabel = new wxStaticText(this, IDC_SETTINGS_MINUTES, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	secLabel->Wrap(-1);
	fgSizer1->Add(secLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	fileSpeedLabel = new wxStaticText(this, IDC_STRONGDC_H_DOWN_SPEED, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fileSpeedLabel->Wrap(-1);
	fgSizer1->Add(fileSpeedLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	fileSpeedSpin = new wxSpinCtrl(this, IDC_H_DOWN_SPEED, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 4096, 0);
	fgSizer1->Add(fileSpeedSpin, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	kbps2Label = new wxStaticText(this, IDC_SETTINGS_KBPS6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	kbps2Label->Wrap(-1);
	fgSizer1->Add(kbps2Label, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	fileSizeLabel = new wxStaticText(this, IDC_STRONGDC_MIN_FILE_SIZE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fileSizeLabel->Wrap(-1);
	fgSizer1->Add(fileSizeLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	fileSizeSpin = new wxSpinCtrl(this, IDC_MIN_FILE_SIZE, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 4096, 0);
	fgSizer1->Add(fileSizeSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	mbpsLabel = new wxStaticText(this, IDC_SETTINGS_MB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mbpsLabel->Wrap(-1);
	fgSizer1->Add(mbpsLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	removeLabel = new wxStaticText(this, IDC_REMOVE_IF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	removeLabel->Wrap(-1);
	fgSizer1->Add(removeLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	removeSpin = new wxSpinCtrl(this, IDC_REMOVE_IF_BELOW, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 99999, 0);
	fgSizer1->Add(removeSpin, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	kbps3Label = new wxStaticText(this, IDC_SETTINGS_KBPS7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	kbps3Label->Wrap(-1);
	fgSizer1->Add(kbps3Label, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	sbSizer1->Add(fgSizer1, 0, wxEXPAND, 5);
	
	bSizer1->Add(sbSizer1, 0, wxEXPAND | wxALL, 5);
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_STRONGDC_TRANSFER_LIMITING, wxEmptyString), wxVERTICAL);
	
	enableLimitCheck = new wxCheckBox(this, IDC_THROTTLE_ENABLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	
	sbSizer2->Add(enableLimitCheck, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer(1, 6, 0, 0);
	fgSizer2->AddGrowableCol(2);
	fgSizer2->SetFlexibleDirection(wxBOTH);
	fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	downLabel = new wxStaticText(this, IDC_STRONGDC_DW_SPEED, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	downLabel->Wrap(-1);
	fgSizer2->Add(downLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	downSpin = new wxSpinCtrl(this, IDC_MX_DW_SP_LMT_NORMAL, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 99999, 0);
	fgSizer2->Add(downSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	kbps4Label = new wxStaticText(this, IDC_SETTINGS_KBPS1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	kbps4Label->Wrap(-1);
	fgSizer2->Add(kbps4Label, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	upLabel = new wxStaticText(this, IDC_STRONGDC_UP_SPEED, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	upLabel->Wrap(-1);
	fgSizer2->Add(upLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	upSpin = new wxSpinCtrl(this, IDC_MX_UP_SP_LMT_NORMAL, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 99999, 0);
	fgSizer2->Add(upSpin, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	kbps5Label = new wxStaticText(this, IDC_SETTINGS_KBPS2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	kbps5Label->Wrap(-1);
	fgSizer2->Add(kbps5Label, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	sbSizer2->Add(fgSizer2, 0, wxEXPAND, 5);
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer(1, 4, 0, 0);
	fgSizer3->SetFlexibleDirection(wxBOTH);
	fgSizer3->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	altCheck = new wxCheckBox(this, IDC_TIME_LIMITING, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	
	fgSizer3->Add(altCheck, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	beginCombo = new wxComboBox(this, IDC_BW_START_TIME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer3->Add(beginCombo, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	toLabel = new wxStaticText(this, IDC_STRONGDC_TO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	toLabel->Wrap(-1);
	fgSizer3->Add(toLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	endCombo = new wxComboBox(this, IDC_BW_END_TIME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY); 
	fgSizer3->Add(endCombo, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	sbSizer2->Add(fgSizer3, 1, wxEXPAND, 5);
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer(1, 6, 0, 0);
	fgSizer4->AddGrowableCol(2);
	fgSizer4->SetFlexibleDirection(wxBOTH);
	fgSizer4->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	downAltLabel = new wxStaticText(this, IDC_STRONGDC_DW_SPEED1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	downAltLabel->Wrap(-1);
	fgSizer4->Add(downAltLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	downAltSpin = new wxSpinCtrl(this, IDC_MX_DW_SP_LMT_TIME, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 99999, 0);
	fgSizer4->Add(downAltSpin, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	kbps6Label = new wxStaticText(this, IDC_SETTINGS_KBPS3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	kbps6Label->Wrap(-1);
	fgSizer4->Add(kbps6Label, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	upAltLabel = new wxStaticText(this, IDC_STRONGDC_UP_SPEED1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	upAltLabel->Wrap(-1);
	fgSizer4->Add(upAltLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	upAltSpin = new wxSpinCtrl(this, IDC_MX_UP_SP_LMT_TIME, wxEmptyString, wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, 0, 99999, 0);
	fgSizer4->Add(upAltSpin, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	kbps7Label = new wxStaticText(this, IDC_SETTINGS_KBPS4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	kbps7Label->Wrap(-1);
	fgSizer4->Add(kbps7Label, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	sbSizer2->Add(fgSizer4, 0, wxEXPAND, 5);
	
	upNoteLabel = new wxStaticText(this, IDC_STRONGDC_UP_NOTE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	upNoteLabel->Wrap(-1);
	sbSizer2->Add(upNoteLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	downNoteLabel = new wxStaticText(this, IDC_STRONGDC_DW_NOTE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	downNoteLabel->Wrap(-1);
	sbSizer2->Add(downNoteLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	bSizer1->Add(sbSizer2, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);
	
	SettingsPage::translate(texts);
	SettingsPage::read(items);

	beginCombo->AppendString(_("Midnight"));
	endCombo->AppendString(_("Midnight"));
	for (int i = 1; i < 12; ++i)
	{
		beginCombo->AppendString((Util::toStringW(i) + _(" AM")).c_str());
		endCombo->AppendString((Util::toStringW(i) + _(" AM")).c_str());
	}
	beginCombo->AppendString(_("Noon"));
	endCombo->AppendString(_("Noon"));
	for (int i = 1; i < 12; ++i)
	{
		beginCombo->AppendString((Util::toStringW(i) + _(" PM")).c_str());
		endCombo->AppendString((Util::toStringW(i) + _(" PM")).c_str());
	}

	beginCombo->SetSelection(SETTING(BANDWIDTH_LIMIT_START));
	endCombo->SetSelection(SETTING(BANDWIDTH_LIMIT_END));

	doCheck();
}

LimitsPage::~LimitsPage()
{
}

void LimitsPage::onEnableCheck(wxCommandEvent &WXUNUSED(event))
{
	doCheck();
}

void LimitsPage::onAltCheck(wxCommandEvent &WXUNUSED(event))
{
	doCheck();
}

void LimitsPage::doCheck()
{
	bool altValue = altCheck->GetValue();
	bool enableValue = enableLimitCheck->GetValue();

	downAltSpin->Enable(altValue & enableValue);
	upAltSpin->Enable(altValue & enableValue);
	beginCombo->Enable(altValue & enableValue);
	endCombo->Enable(altValue & enableValue);

	altCheck->Enable(enableValue);
	downSpin->Enable(enableValue);
	upSpin->Enable(enableValue);
}

void LimitsPage::write()
{
	SettingsPage::write(items);

	settings->set(SettingsManager::BANDWIDTH_LIMIT_START, beginCombo->GetSelection());
	settings->set(SettingsManager::BANDWIDTH_LIMIT_END, endCombo->GetSelection());
}