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

#ifndef _RICHCHATCTRL_H
#define _RICHCHATCTRL_H

#include "UCHandler.h"

#include "../client/User.h"

#include <wx/richtext/richtextctrl.h>

class RichChatCtrl :
	public wxRichTextCtrl,
	public UCHandler<RichChatCtrl>
{
public:
	RichChatCtrl(wxWindow* parent, long style);
	~RichChatCtrl(void);

	void AppendText(const Identity& i, const tstring& myNick, const tstring& time, tstring msg, const wxRichTextAttr& attr, bool useEmoticons = true);

	const tstring& getSelectedUser() const { return selectedUser; }

	void runUserCommand(const UserCommand& uc); 

	GETSET(Client*, client, Client);

private:

	DECLARE_EVENT_TABLE();
	
	void OnSize(wxSizeEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnDblClick(wxMouseEvent& event);
	void OnURL(wxTextUrlEvent& event);

	void OnCopyActualLine(wxCommandEvent& event);
	void OnEditCopy(wxCommandEvent& event);
	void OnEditSelectAll(wxCommandEvent& event);
	void OnEditClearAll(wxCommandEvent& event);
	void OnBanIP(wxCommandEvent& event);
	void OnUnBanIP(wxCommandEvent& event);
	void OnCopyURL(wxCommandEvent& event);
	void OnWhoisIP(wxCommandEvent& event);

	void OnOpenUserLog(wxCommandEvent& event);
	void OnPrivateMessage(wxCommandEvent& event);
	void OnGetList(wxCommandEvent& event);
	void OnMatchQueue(wxCommandEvent& event);
	void OnGrantSlot(wxCommandEvent& event);
	void OnAddToFavorites(wxCommandEvent& event);
	void OnIgnore(wxCommandEvent& event);
	void OnUnignore(wxCommandEvent& event);
	void OnCopyUserInfo(wxCommandEvent& event);
	void OnReport(wxCommandEvent& event);
	void OnCheckList(wxCommandEvent& event);

	void FormatChatLine(const tstring& myNick, const tstring& msg, const wxRichTextAttr& attr, bool isMyMessage, const tstring& author, LONG lSelBegin, bool useEmoticons);
	void FormatEmoticonsAndLinks(const tstring& sText, const tstring& sTextLower, long lSelBegin, bool bUseEmo);

	void handleSelection(const wxPoint& p);

//	static EmoticonsManager* emoticonsManager;

	tstring selectedIP;
	tstring selectedUser;
	wxString selectedLine;
	wxString selectedURL;

};

#endif	// #define _RICHCHATCTRL_H