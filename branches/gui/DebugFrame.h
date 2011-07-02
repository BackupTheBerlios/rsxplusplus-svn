/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
 * Copyright (C) 2010-2011 adrian_007, adrian-007 on o2 point pl

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

#ifndef _DEBUGFRAME_H
#define _DEBUGFRAME_H

#include "AsyncHandler.h"

#include "../client/DebugManager.h"

class DebugFrame :
	public wxWindow,
	private DebugManagerListener,
	public AsyncHandler<DebugFrame>
{
public:
	DebugFrame(wxWindow* parent);
	~DebugFrame(void);

private:
	DECLARE_EVENT_TABLE();

	// events
	void OnSize(wxSizeEvent& event);
	void OnClear(wxCommandEvent& event);

	wxStatusBar* statusBar;
	wxTextCtrl* textCtrl;
	wxButton* ctrlClear;
	wxCheckBox* ctrlShowHubCommands;
	wxCheckBox* ctrlShowClientCommands;
	wxCheckBox* ctrlShowDetectionCommands;
	wxCheckBox* ctrlEnableFilter;
	wxTextCtrl* ctrlFilter;

	void addCommand(const std::string& line);
	bool checkFilter(const std::string& ip);

	void on(DebugManagerListener::DebugDetection, const string& aLine) throw();
	void on(DebugManagerListener::DebugCommand, const string& aLine, int typeDir, const string& ip) throw();

	void setControlInStatusBar(int pos, wxWindow* wnd, int offset = 0);
};

#endif	// _DEBUGFRAME_H
