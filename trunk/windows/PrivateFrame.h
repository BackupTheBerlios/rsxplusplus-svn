/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(PRIVATE_FRAME_H)
#define PRIVATE_FRAME_H

#include "../client/User.h"
#include "../client/ClientManagerListener.h"
#include "../client/ResourceManager.h"

#include "FlatTabCtrl.h"
#include "WinUtil.h"
#include "UCHandler.h"
#include "EmoticonsDlg.h"
#include "HubFrame.h"

#include "ChatCtrl.h"

#define PM_MESSAGE_MAP 8		// This could be any number, really...

class Client; //rsx

class PrivateFrame : public MDITabChildWindowImpl<PrivateFrame, TABDEFCLR, IDR_PRIVATE, IDR_PRIVATE_OFF>, 
	private ClientManagerListener, public UCHandler<PrivateFrame>, private SettingsManagerListener
{
public:
	static void gotMessage(const Identity& from, const UserPtr& to, const UserPtr& replyTo, const tstring& aMessage);
	static void openWindow(const UserPtr& replyTo, const tstring& aMessage = Util::emptyStringT);
	static bool isOpen(const UserPtr u) { return frames.find(u) != frames.end(); }
	static void closeAll();
	static void closeAllOffline();

	enum {
		USER_UPDATED
	};

	DECLARE_FRAME_WND_CLASS_EX(_T("PrivateFrame"), IDR_PRIVATE, 0, COLOR_3DFACE);

	typedef MDITabChildWindowImpl<PrivateFrame, TABDEFCLR, IDR_PRIVATE, IDR_PRIVATE_OFF> baseClass;
	typedef UCHandler<PrivateFrame> ucBase;

	BEGIN_MSG_MAP(PrivateFrame)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(FTM_CONTEXTMENU, onTabContextMenu)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_MATCH_QUEUE, onMatchQueue)
		COMMAND_RANGE_HANDLER(IDC_GRANTSLOT, IDC_UNGRANTSLOT, onGrantSlot)
		COMMAND_ID_HANDLER(IDC_ADD_TO_FAVORITES, onAddToFavorites)
		COMMAND_ID_HANDLER(IDC_SEND_MESSAGE, onSendMessage)
		COMMAND_ID_HANDLER(IDC_CLOSE_WINDOW, onCloseWindow)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, onEditCopy)
		COMMAND_ID_HANDLER(ID_EDIT_SELECT_ALL, onEditSelectAll)
		COMMAND_ID_HANDLER(ID_EDIT_CLEAR_ALL, onEditClearAll)
		COMMAND_ID_HANDLER(IDC_COPY_ACTUAL_LINE, onCopyActualLine)
		COMMAND_ID_HANDLER(IDC_OPEN_USER_LOG, onOpenUserLog)
		COMMAND_ID_HANDLER(IDC_COPY_URL, onCopyURL)
		COMMAND_ID_HANDLER(IDC_EMOT, onEmoticons)
		//RSX++
		COMMAND_ID_HANDLER(IDC_CUSTOM_KICK, onCustomKick)
		COMMAND_ID_HANDLER(IDC_MULTIHUB_KICK, onMultihubKick)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_EXACT_SHARE, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_DESCRIPTION, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_TAG, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_EMAIL_ADDRESS, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_IP, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_CONNECTION, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_MYINFO, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_CLIENT, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_CHEAT, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_HOST, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_SUB_CID, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_NICK, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_EXACT_SHARE, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_CONNECTION, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_MYINFO, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_CLIENT, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_CHEAT, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_HOST, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_CID, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_TAG, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_DESCRIPTION, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_EMAIL_ADDRESS, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_AUTOSCROLL_CHAT, onAutoScrollChat)		
		COMMAND_ID_HANDLER(IDC_BAN_IP, onBanIP)
		COMMAND_ID_HANDLER(IDC_UNBAN_IP, onUnBanIP)
		COMMAND_ID_HANDLER(IDC_WHOIS_IP, onWhoisIP)
		COMMAND_ID_HANDLER(IDC_COPY_IP, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_COPY_ALL, onCopyUserInfo)
		COMMAND_ID_HANDLER(IDC_GET_USER_RESPONSES, onGetUserResponse)
		COMMAND_ID_HANDLER(IDC_CHECKLIST, onCheckFileList)
		COMMAND_ID_HANDLER(IDC_REPORT, onReportUser)
		COMMAND_ID_HANDLER(IDC_CLEAN_USER_D, onCleanUser)
		COMMAND_ID_HANDLER(IDC_UNIGNORE, onUnIgnore)
		COMMAND_ID_HANDLER(IDC_IGNORE, onIgnore)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPM)
		//END
		COMMAND_RANGE_HANDLER(IDC_EMOMENU, IDC_EMOMENU + menuItems, onEmoPackChange);
		CHAIN_COMMANDS(ucBase)
		CHAIN_MSG_MAP(baseClass)
		NOTIFY_HANDLER(IDC_CLIENT, EN_LINK, onClientEnLink)
	ALT_MSG_MAP(PM_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
		MESSAGE_HANDLER(BM_SETCHECK, onSoundActive) //RSX++
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, onLButton)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onEditClearAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCopyActualLine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClientEnLink(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onOpenUserLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCopyURL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onEmoPackChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	//RSX++
	LRESULT onCopyUserInfo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAutoScrollChat(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBanIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onUnBanIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onWhoIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onWhoisIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGetUserResponse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCheckFileList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onReportUser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCleanUser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onIgnore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onUnIgnore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onPM(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCustomKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMultihubKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onSoundActive(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		replyTo->setSoundActive(wParam == BST_CHECKED);
		if (getUser()->getSoundActive()) {
			replyTo->setSoundActive(true);
		} else {
			replyTo->setSoundActive(false);
		}
		return 0;
    }
	LRESULT onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	//END
  	LRESULT onEmoticons(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled) {
  		if (hWndCtl != ctrlEmoticons.m_hWnd) {
  			bHandled = false;
  	        return 0;
  	    }
  	 
  		EmoticonsDlg dlg;
  		ctrlEmoticons.GetWindowRect(dlg.pos);
  		dlg.DoModal(m_hWnd);
  		if (!dlg.result.empty()) {
  			TCHAR* message = new TCHAR[ctrlMessage.GetWindowTextLength()+1];
  			ctrlMessage.GetWindowText(message, ctrlMessage.GetWindowTextLength()+1);
  			tstring s(message, ctrlMessage.GetWindowTextLength());
  			delete[] message;
  			
			ctrlMessage.SetWindowText((s+dlg.result).c_str());
  			ctrlMessage.SetFocus();
  			ctrlMessage.SetSel( ctrlMessage.GetWindowTextLength(), ctrlMessage.GetWindowTextLength() );
  		}
  		return 0;
  	}

	void addLine(const tstring& aLine, CHARFORMAT2& cf);
	void addLine(const Identity&, const tstring& aLine);
	void addLine(const Identity&, const tstring& aLine, CHARFORMAT2& cf);
	void onEnter();
	void UpdateLayout(BOOL bResizeBars = TRUE);	
	void runUserCommand(UserCommand& uc);
	void readLog();
	
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		
	LRESULT onSendMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		onEnter();
		return 0;
	}

	LRESULT onCloseWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT PrivateFrame::onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */) {
		updateTitle();
		return 0;
	}
	
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlClient.m_hWnd || hWnd == ctrlMessage.m_hWnd) {
			::SetBkColor(hDC, WinUtil::bgColor);
			::SetTextColor(hDC, WinUtil::textColor);
			return (LRESULT)WinUtil::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	}

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlMessage.SetFocus();
		ctrlClient.GoToEnd();
		return 0;
	}
	
	void addClientLine(const tstring& aLine) {
		if(!created) {
			CreateEx(WinUtil::mdiClient);
		}
		ctrlStatus.SetText(0, (_T("[") + Text::toT(Util::getShortTimeString()) + _T("] ") + aLine).c_str());
		if (BOOLSETTING(BOLD_PM)) {
			setDirty();
		}
	}
	
	void sendMessage(const tstring& msg);
	
	const UserPtr& getUser() const { return replyTo; }
	//RSX++
	const UserPtr& getSelectedUser() const { return selUser ? selUser : replyTo; }
	//END
private:
	PrivateFrame(const UserPtr& replyTo_) : replyTo(replyTo_), 
		created(false), closed(false), isoffline(false), curCommandPosition(0),  
		ctrlMessageContainer(WC_EDIT, this, PM_MESSAGE_MAP), menuItems(0),
		ctrlClientContainer(WC_EDIT, this, PM_MESSAGE_MAP),
		soundActiveContainer(WC_BUTTON, this, PM_MESSAGE_MAP) { } //RSX++
	
	~PrivateFrame() { }

	bool created;
	typedef unordered_map<UserPtr, PrivateFrame*, User::Hash> FrameMap;
	typedef FrameMap::const_iterator FrameIter;
	static FrameMap frames;
	ChatCtrl ctrlClient;
	CEdit ctrlMessage;
	CStatusBarCtrl ctrlStatus;

	int menuItems;
	OMenu emoMenu;
	CButton ctrlEmoticons;
	HBITMAP hEmoticonBmp;

	//RSX++
	CButton ctrlSoundActive;
	CContainedWindow soundActiveContainer;

	Client* client;
	bool PreparePopupMenu(const tstring& sNick, OMenu& pMenu);
	bool isOp, customProtection;
	string hubName, myNick;
	OMenu copySubMenu;
	OMenu copyMenu;
	UserPtr selUser;

	void putClient() { ctrlClient.setClient(NULL); client = NULL; }
	void setClient(const UserPtr& user) {
		client = ClientManager::getInstance()->getUserClient(user);
		ctrlClient.setClient(client);
	}
	Client* getClient() { return client; }

	string getCustomAway() {
		string defAway = Util::getAwayMessage();
		if(client) {
			string customAway = FavoriteManager::getInstance()->getAwayMessage(client->getHubUrl());
			if((defAway.compare(SETTING(DEFAULT_AWAY_MESSAGE)) != 0))
				return defAway; //might be custom away but not from favs :P
			return customAway;
		}
		return defAway;
	}
	//END

	UserPtr replyTo;
	CContainedWindow ctrlMessageContainer;
	CContainedWindow ctrlClientContainer;

	bool closed;
	bool isoffline;

	StringMap ucLineParams;

	void updateTitle();
	
	TStringList prevCommands;
	tstring currentCommand;
	TStringList::size_type curCommandPosition;

	// ClientManagerListener
	void on(ClientManagerListener::UserUpdated, const OnlineUser& aUser) throw() {
		if(aUser.getUser() == replyTo)
			PostMessage(WM_SPEAKER, USER_UPDATED);
	}
	void on(ClientManagerListener::UserConnected, const UserPtr& aUser) throw() {
		if(aUser == replyTo) {
			PostMessage(WM_SPEAKER, USER_UPDATED);
			//RSX++
			setClient(aUser);
			if(customProtection && !replyTo->isSet(User::PROTECTED)) //set protection ie after reconnect
				replyTo->setFlag(User::PROTECTED);
		}
	}
	void on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) throw() {
		if(aUser == replyTo)
			PostMessage(WM_SPEAKER, USER_UPDATED);
	}
	void on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw();
	//RSX++
	void on(ClientDisconnected, const Client* c) throw() { 
		if(client == c) {
			putClient();
		}
	}
	//END
};

#endif // !defined(PRIVATE_FRAME_H)

/**
 * @file
 * $Id: PrivateFrame.h 326 2007-09-01 16:55:01Z bigmuscle $
 */
