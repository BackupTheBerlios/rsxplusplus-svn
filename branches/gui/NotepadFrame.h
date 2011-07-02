/*
 * Copyright (C) 2010 adrian_007 adrian-007 on o2 point pl
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

#ifndef _NOTEPAD_FRAME_H_
#define _NOTEPAD_FRAME_H_

#include "StaticFrame.h"
#include "../client/SettingsManager.h"

class NotepadFrame : public StaticFrame<NotepadFrame>, private SettingsManagerListener {
public:
	typedef StaticFrame<NotepadFrame> BaseType;

	NotepadFrame();
	~NotepadFrame();

private:
	DECLARE_EVENT_TABLE();

	void UpdateLayout();

	void on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw();

	// events
	void OnClose(wxCloseEvent& event);

	wxTextCtrl* textCtrl;
};

#endif
