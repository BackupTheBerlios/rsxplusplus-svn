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

#ifndef _HASHPROGRESSDLG_H
#define _HASHPROGRESSDLG_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/gauge.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>

#include "../client/HashManager.h"

class HashProgressDlg : public wxDialog
{
private:
	DECLARE_EVENT_TABLE()

	enum
	{
		ID_TIMER
	};

	wxTimer *timer;

	bool autoClose;
	int64_t startBytes;
	size_t startFiles;
	uint64_t startTime;
	bool init;

	void updateStats();

protected:		
	wxStaticText* hashIndexLabel;
	wxStaticText* curFileLabel;
	wxStaticText* filesHourLabel;
	wxStaticText* hashSpeedLabel;
	wxStaticText* timeLeftLabel;
	wxGauge* hashProgress;
	wxStaticText* maxSpeedLabel;
	wxSpinCtrl* maxSpeedSpin;
	wxStaticText* mbsLabel;
	wxButton* pauseButton;
	wxButton* runInBackButton;

	void onSpeedChange(wxSpinEvent &WXUNUSED(event));
	void onPause(wxCommandEvent &WXUNUSED(event));
	void onTimer(wxTimerEvent &WXUNUSED(event));
	
public:
	HashProgressDlg(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = CTSTRING(HASH_PROGRESS),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
	~HashProgressDlg();
	void setAutoClose(bool value);
	bool getAutoClose();
	
};

#endif // _HASHPROGRESSDLG_H
