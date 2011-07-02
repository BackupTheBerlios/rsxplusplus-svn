/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
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

#ifndef _CHATPANEL_H
#define _CHATPANEL_H

#include "RichChatCtrl.h"

#include <wx/splitter.h>
#include <wx/textctrl.h>
#include <wx/window.h>

BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_LOCAL_EVENT_TYPE(SEND_MESSAGE, 14000)
END_DECLARE_EVENT_TYPES()

class ChatPanel :
	public wxSplitterWindow
{
public:

	ChatPanel(wxWindow* parent, long style);
	~ChatPanel(void) { }

	void readLog(const string& logFile, unsigned int maxLines);
	void Clear() { ctrlChat->Clear(); }
	
	wxTextCtrl* getCtrlMessage() const { return ctrlMessage; }
	const tstring& getSelectedUser() const { return ctrlChat->getSelectedUser(); }

	void setClient(Client* client) { ctrlChat->setClient(client); }

	void AppendText(const Identity& i, const tstring& myNick, const tstring& time, tstring msg, const wxRichTextAttr& attr, bool useEmoticons = true)
	{
		ctrlChat->AppendText(i, myNick, time, msg, attr, useEmoticons);
	}

private:

	// events
	DECLARE_EVENT_TABLE();

	void OnMessageChar(wxKeyEvent& event);
	void OnEmoticons(wxCommandEvent& event);
	void OnEmotContextMenu(wxContextMenuEvent& /*event*/);

	// attributes
	RichChatCtrl*		ctrlChat;
	wxTextCtrl*			ctrlMessage;

	// command history
	tstring currentCommand;
	TStringList::size_type curCommandPosition;		//can't use an iterator because StringList is a vector, and vector iterators become invalid after resizing
	TStringList prevCommands;
};

#endif	// _CHATPANEL_H
