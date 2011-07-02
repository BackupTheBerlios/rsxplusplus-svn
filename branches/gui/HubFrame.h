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

#ifndef _HUBFRAME_H
#define _HUBFRAME_H

#include "ChatPanel.h"
#include "MDIChildFrame.h"
#include "TypedDataViewCtrl.h"
#include "UCHandler.h"
#include "UserInfoBaseHandler.h"

#include "../client/Client.h"
#include "../client/FavoriteManagerListener.h"
#include "../client/SettingsManager.h"
#include "../client/TaskQueue.h"
#include "../client/Util.h"

#include <wx/splitter.h>
#include <wx/textctrl.h>

class HubFrame :
	public MDIChildFrame, 
	private ClientListener, 
	private FavoriteManagerListener, 
	private TimerManagerListener,
	public UCHandler<HubFrame>,
	public UserInfoBaseHandler<HubFrame>, 
	private SettingsManagerListener
{
public:

	static void openWindow(const tstring& server, const string& rawOne = Util::emptyString, const string& rawTwo = Util::emptyString,
		const string& rawThree = Util::emptyString, const string& rawFour = Util::emptyString, const string& rawFive = Util::emptyString,
		int chatUserSplit = 0, bool userListState = true, string sColumsOrder = Util::emptyString, string sColumsWidth = Util::emptyString, 
		string sColumsVisible = Util::emptyString);

	typedef TypedDataViewCtrl<OnlineUser> UserList;
	UserList& getUserList() { return *ctrlUsers; }

	static void resortUsers();

	void runUserCommand(const UserCommand& uc); 

private:

	friend class RichChatCtrl;

	enum Tasks 
	{ 
		UPDATE_USER_JOIN, UPDATE_USER, REMOVE_USER
	};

	enum FilterModes
	{
		NONE,
		EQUAL,
		GREATER_EQUAL,
		LESS_EQUAL,
		GREATER,
		LESS,
		NOT_EQUAL
	};

	struct UserTask : public Task 
	{
		UserTask(const OnlineUserPtr& ou) : onlineUser(ou) { }
		~UserTask() { }
		
		const OnlineUserPtr onlineUser;
	};

	DECLARE_EVENT_TABLE();

	// events
	void OnClose(wxCloseEvent& event);
	void OnErase(wxEraseEvent& /*event*/) { }
	void OnTabContextMenu(wxAuiNotebookEvent& event);
	void OnGetAttributes(wxCommandEvent& event);
	void OnSendMessage(wxCommandEvent& event);
	void OnShowUsers(wxCommandEvent& event);
	void OnUnsplit(wxSplitterEvent& event);
	void OnUserListContextMenu(wxDataViewEvent& event);
	void OnUserListDblClick(wxDataViewEvent& event);
	void OnFilterSelection(wxCommandEvent& event);
	void OnFilterChange(wxCommandEvent& event);
	void OnFilterEnter(wxCommandEvent& event);
	void OnSetFocus(wxFocusEvent& /*event*/) { ctrlChat->SetFocus(); }

	void OnFollow(wxCommandEvent& event);
	void OnCopyHubInfo(wxCommandEvent& event);
	void OnOpenHubLog(wxCommandEvent& event);
	void OnEditClearAll(wxCommandEvent& event);
	void OnAddFavorite(wxCommandEvent& event);
	void OnReconnect(wxCommandEvent& event);
	void OnCloseWindow(wxCommandEvent& /*event*/) { Close(); }
	void OnCopyUserInfo(wxCommandEvent& event);
	void OnOpenUserLog(wxCommandEvent& event);
	void OnPublicMessage(wxCommandEvent& event);
	void OnIgnore(wxCommandEvent& event);
	void OnUnignore(wxCommandEvent& event);
	void OnRefresh(wxCommandEvent& event);

	void OnTasks();
	void OnConnected();
	void OnDisconnected();
	void OnGetPassword();
	void OnChatMessage(const Identity& from, const tstring& msg);
	void OnPrivateMessage(const Identity& from, const UserPtr& to, const UserPtr& replyTo, bool hub, bool bot, const tstring& msg);

	// attributes
	typedef unordered_map<tstring, HubFrame*> FrameMap;
	typedef FrameMap::const_iterator FrameIter;
	static FrameMap frames;

	Client* client;
	tstring server;
	tstring redirect;

	TaskQueue tasks;
	bool updateUsers;
	bool resort;
	bool timeStamps;
	bool showJoins;
	bool favShowJoins;
	bool waitingForPW;
	bool tabMenuShown;

	int chatUserSplitSize;
	int statusSizes[6];
	static int columnIndexes[OnlineUser::COLUMN_LAST];
	static int columnSizes[OnlineUser::COLUMN_LAST];
	static ResourceManager::Strings columnNames[OnlineUser::COLUMN_LAST];

	tstring		complete;
	tstring		filter;
	StringMap	ucLineParams;
	
	enum { MAX_CLIENT_LINES = 5 };
	TStringList lastLinesList;

	UserList*			ctrlUsers;
	ChatPanel*			ctrlChat;
	wxSplitterWindow*	chatUsersSplitter;
	wxCheckBox*			ctrlShowUsers;
	wxTextCtrl*			ctrlFilter;
	wxComboBox*			ctrlFilterSel;

	wxPanel*			userListPanel;

	// methods
	HubFrame(const tstring& aServer, const string& aRawOne, const string& aRawTwo, const string& aRawThree, const string& aRawFour,
		const string& aRawFive, int chatUserSplit, bool userListState);
	~HubFrame(void);

	void addAsFavorite();
	void removeFavoriteHub();

	void addLine(const tstring& aLine);
	void addLine(const tstring& aLine, const wxRichTextAttr& cf, bool bUseEmo = true);
	void addLine(const Identity& i, const tstring& aLine, const wxRichTextAttr& cf, bool bUseEmo = true);
	void addStatus(const tstring& aLine, const wxRichTextAttr& cf = WinUtil::m_ChatTextSystem, bool inChat = true);

	string getLogFile() const;

	bool updateUser(const UserTask& u);
	void removeUser(const OnlineUserPtr& aUser);

	void updateUserList(OnlineUserPtr ui = NULL);
	bool parseFilter(FilterModes& mode, int64_t& size);
	bool matchFilter(const OnlineUser& ui, int sel, bool doSizeCompare = false, FilterModes mode = NONE, int64_t size = 0);

	void UpdateLayout();
	void clearUserList();
	void clearTaskList();

	void OnTab(wxKeyEvent& event);

	void resortForFavsFirst(bool justDoIt = false);

	// FavoriteManagerListener
	void on(FavoriteManagerListener::UserAdded, const FavoriteUser& /*aUser*/) throw();
	void on(FavoriteManagerListener::UserRemoved, const FavoriteUser& /*aUser*/) throw();

	// TimerManagerListener
	void on(TimerManagerListener::Second, uint64_t /*aTick*/) throw();
	void on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw();

	// ClientListener
	void on(Connecting, const Client*) throw();
	void on(Connected, const Client*) throw();
	void on(UserUpdated, const Client*, const OnlineUserPtr&) throw();
	void on(UsersUpdated, const Client*, const OnlineUserList&) throw();
	void on(ClientListener::UserRemoved, const Client*, const OnlineUserPtr&) throw();
	void on(Redirect, const Client*, const string&) throw();
	void on(Failed, const Client*, const string&) throw();
	void on(GetPassword, const Client*) throw();
	void on(HubUpdated, const Client*) throw();
	void on(Message, const Client*, const ChatMessage&) throw();
	void on(StatusMessage, const Client*, const string&, int = ClientListener::FLAG_NORMAL) throw();
	void on(NickTaken, const Client*) throw();
	void on(SearchFlood, const Client*, const string&) throw();
	void on(CheatMessage, const Client*, const string&) throw();	
	void on(HubTopic, const Client*, const string&) throw();
	void on(UserReport, const Client*, const Identity&) throw();

	void speak(Tasks s, const OnlineUserPtr& u);
};

#endif	// _HUBFRAME_H