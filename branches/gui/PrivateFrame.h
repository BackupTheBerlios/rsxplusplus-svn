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

#ifndef _PRIVATEFRAME_H
#define _PRIVATEFRAME_H

#include "ChatPanel.h"
#include "MDIChildFrame.h"

#include "../client/ClientManagerListener.h"

class PrivateFrame :
	public MDIChildFrame, private ClientManagerListener
{
public:

	static void gotMessage(const Identity& from, const UserPtr& to, const UserPtr& replyTo, const tstring& aMessage, Client* c);
	static void openWindow(const HintedUser& replyTo, const tstring& aMessage = Util::emptyStringT, Client* c = NULL);
	static bool isOpen(const UserPtr u) { return frames.find(u) != frames.end(); }

private:

	DECLARE_EVENT_TABLE();

	// events
	void OnClose(wxCloseEvent& event);
	void OnSetFocus(wxFocusEvent& /*event*/) { ctrlChat->SetFocus(); }
	void OnSendMessage(wxCommandEvent& event);

	// attributes
	typedef unordered_map<UserPtr, PrivateFrame*, User::Hash> FrameMap;
	typedef FrameMap::const_iterator FrameIter;
	static FrameMap frames;

	bool isOffline;
	bool priv;

	HintedUser	replyTo;
	ChatPanel*	ctrlChat;

	// methods
	PrivateFrame(const HintedUser& replyTo_, Client* c);
	~PrivateFrame(void);

	void UpdateLayout() { }
	void updateTitle();

	void fillLogParams(StringMap& params) const;
	string getLogFile() const;

	void addLine(const tstring& aLine, const wxRichTextAttr& cf);
	void addLine(const Identity&, const tstring& aLine);
	void addLine(const Identity&, const tstring& aLine, const wxRichTextAttr& cf);

	void addStatusLine(const tstring& aLine);

	void sendMessage(const tstring& msg, bool thirdPerson = false);

	// ClientManagerListener
	void on(ClientManagerListener::UserUpdated, const OnlineUser& aUser) throw();
	void on(ClientManagerListener::UserConnected, const UserPtr& aUser) throw();
	void on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) throw();
};

#endif	// _PRIVATEFRAME_H