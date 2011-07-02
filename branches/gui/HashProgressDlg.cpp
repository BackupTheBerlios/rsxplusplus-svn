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

#include "HashProgressDlg.h"

BEGIN_EVENT_TABLE(HashProgressDlg, wxDialog)
	EVT_SPINCTRL(IDC_MAX_HASH_SPEED, HashProgressDlg::onSpeedChange)
	EVT_BUTTON(IDC_PAUSE, HashProgressDlg::onPause)
	EVT_TIMER(ID_TIMER, HashProgressDlg::onTimer)
END_EVENT_TABLE()

HashProgressDlg::HashProgressDlg(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	hashIndexLabel = new wxStaticText(this, IDC_HASH_INDEXING, CTSTRING(HASH_PROGRESS_TEXT), wxDefaultPosition, wxDefaultSize, 0);
	hashIndexLabel->Wrap(-1);
	bSizer1->Add(hashIndexLabel, 0, wxALL, 5);
	
	curFileLabel = new wxStaticText(this, IDC_CURRENT_FILE, _("Current file..."), wxDefaultPosition, wxDefaultSize, 0);
	curFileLabel->Wrap(-1);
	bSizer1->Add(curFileLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_STATISTICS, CTSTRING(HASH_PROGRESS_STATS)), wxVERTICAL);
	
	filesHourLabel = new wxStaticText(this, IDC_FILES_PER_HOUR, _("0.0 files/hour"), wxDefaultPosition, wxDefaultSize, 0);
	filesHourLabel->Wrap(-1);
	sbSizer1->Add(filesHourLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	hashSpeedLabel = new wxStaticText(this, IDC_HASH_SPEED, _("0.0 MiB/s"), wxDefaultPosition, wxDefaultSize, 0);
	hashSpeedLabel->Wrap(-1);
	sbSizer1->Add(hashSpeedLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	timeLeftLabel = new wxStaticText(this, IDC_TIME_LEFT, _("0:00:00 left"), wxDefaultPosition, wxDefaultSize, 0);
	timeLeftLabel->Wrap(-1);
	sbSizer1->Add(timeLeftLabel, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	bSizer1->Add(sbSizer1, 1, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	hashProgress = new wxGauge(this, IDC_HASH_PROGRESS, 100, wxDefaultPosition, wxSize(550,25), wxGA_HORIZONTAL);
	hashProgress->SetValue(0); 
	bSizer1->Add(hashProgress, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, 5);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(1, 5, 0, 0);
	fgSizer1->AddGrowableCol(0);
	fgSizer1->AddGrowableCol(2);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	maxSpeedLabel = new wxStaticText(this, IDC_SETTINGS_MAX_HASH_SPEED, CTSTRING(SETTINGS_MAX_HASH_SPEED), wxDefaultPosition,
		wxDefaultSize, 0);
	maxSpeedLabel->Wrap(-1);
	fgSizer1->Add(maxSpeedLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_RIGHT, 5);
	
	maxSpeedSpin = new wxSpinCtrl(this, IDC_MAX_HASH_SPEED, Text::toT(Util::toString(SETTING(MAX_HASH_SPEED))).c_str(),
		wxDefaultPosition, wxSize(50,-1), wxSP_ARROW_KEYS, 0, 10, 0);
	fgSizer1->Add(maxSpeedSpin, 0, wxBOTTOM | wxRIGHT, 5);
	
	mbsLabel = new wxStaticText(this, IDC_SETTINGS_MBS, _("MiB/s"), wxDefaultPosition, wxDefaultSize, 0);
	mbsLabel->Wrap(-1);
	fgSizer1->Add(mbsLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT, 5);
	
	pauseButton = new wxButton(this, IDC_PAUSE, HashManager::getInstance()->isHashingPaused() ? CTSTRING(RESUME) : CTSTRING(PAUSE),
		wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(pauseButton, 0, wxBOTTOM | wxRIGHT, 5);
	
	runInBackButton = new wxButton(this, wxID_OK, CTSTRING(HASH_PROGRESS_BACKGROUND), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(runInBackButton, 0, wxBOTTOM | wxRIGHT, 5);
	
	bSizer1->Add(fgSizer1, 0, wxEXPAND, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);
	
	Center();
	init = true;

	string tmp;
	startTime = GET_TICK();
	HashManager::getInstance()->getStats(tmp, startBytes, startFiles);

	maxSpeedSpin->SetRange(0, 999);

	hashProgress->SetRange(10000);
	updateStats();

	HashManager::getInstance()->setPriority(Thread::NORMAL);
	
	timer = new wxTimer(this, ID_TIMER);
	timer->Start(1000);
}

HashProgressDlg::~HashProgressDlg()
{
	HashManager::getInstance()->setPriority(Thread::IDLE);
	timer->Stop();
	wxDELETE(timer);
}

void HashProgressDlg::onSpeedChange(wxSpinEvent &WXUNUSED(event))
{
	if(init)
	{
		SettingsManager::getInstance()->set(SettingsManager::MAX_HASH_SPEED, maxSpeedSpin->GetValue());
	}
}

void HashProgressDlg::onPause(wxCommandEvent &WXUNUSED(event))
{
	if(HashManager::getInstance()->isHashingPaused())
	{
		HashManager::getInstance()->resumeHashing();
		pauseButton->SetLabel(CTSTRING(PAUSE));
	}
	else
	{
		HashManager::getInstance()->pauseHashing();
		pauseButton->SetLabel(CTSTRING(RESUME));
	}
}

void HashProgressDlg::onTimer(wxTimerEvent &WXUNUSED(event))
{
	updateStats();
}

void HashProgressDlg::updateStats()
{
	string file;
	int64_t bytes = 0;
	size_t files = 0;
	uint64_t tick = GET_TICK();

	HashManager::getInstance()->getStats(file, bytes, files);
	if(bytes > startBytes)
		startBytes = bytes;

	if(files > startFiles)
		startFiles = files;

	if(autoClose && files == 0)
	{
		Close();
		return;
	}

	double diff = static_cast<double>(tick - startTime);
	bool paused = HashManager::getInstance()->isHashingPaused();
	if (diff < 1000 || files == 0 || bytes == 0 || paused)
	{
		filesHourLabel->SetLabel(Text::toT("-.-- " + STRING(FILES_PER_HOUR) + ", " + Util::toString((uint32_t)files) + " " +
			STRING(FILES_LEFT)).c_str());
		hashSpeedLabel->SetLabel(wxString(_T("-.-- B/s, ") + Util::formatBytesW(bytes) + _T(" ") + TSTRING(LEFT)).c_str());
		if(paused)
		{
			timeLeftLabel->SetLabel(Text::toT("( " + STRING(PAUSED) + " )").c_str());
		}
		else
		{
			timeLeftLabel->SetLabel(Text::toT("-:--:-- " + STRING(LEFT)).c_str());
			hashProgress->SetValue(0);
		}
	}
	else
	{
		double filestat = (((double)(startFiles - files)) * 60 * 60 * 1000) / diff;
		double speedStat = (((double)(startBytes - bytes)) * 1000) / diff;

		filesHourLabel->SetLabel(Text::toT(Util::toString(filestat) + " " + STRING(FILES_PER_HOUR) + ", " +
			Util::toString((uint32_t)files) + " " + STRING(FILES_LEFT)).c_str());
		hashSpeedLabel->SetLabel(wxString(Util::formatBytesW((int64_t)speedStat) + _T("/s, ") + Util::formatBytesW(bytes)
			+ _T(" ") + TSTRING(LEFT)).c_str());

		if (filestat == 0 || speedStat == 0)
		{
			timeLeftLabel->SetLabel(Text::toT("-:--:-- " + STRING(LEFT)).c_str());
		}
		else
		{
			double fs = files * 60 * 60 / filestat;
			double ss = bytes / speedStat;

			timeLeftLabel->SetLabel(wxString(Util::formatSeconds((int64_t)(fs + ss) / 2) + _T(" ") + TSTRING(LEFT)).c_str());
		}
	}

	if(files == 0)
	{
		curFileLabel->SetLabel(CTSTRING(DONE));
	}
	else
	{
		curFileLabel->SetLabel(Text::toT(file).c_str());
	}

	if (startFiles == 0 || startBytes == 0)
	{
		hashProgress->SetValue(0);
	}
	else
	{
		hashProgress->SetValue((int)(10000 * ((0.5 * (startFiles - files)/startFiles) + 0.5 * (startBytes - bytes) / startBytes)));
	}
		
	pauseButton->SetLabel(paused ? CTSTRING(RESUME) : CTSTRING(PAUSE));
}

void HashProgressDlg::setAutoClose(bool value)
{
	autoClose = value;
}

bool HashProgressDlg::getAutoClose()
{
	return autoClose;
}