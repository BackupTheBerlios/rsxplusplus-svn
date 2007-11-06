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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "PrivateFrame.h"
#include "SearchFrm.h"
#include "WinUtil.h"
#include "MainFrm.h"
#include "AGEmotionSetup.h"

#include "../client/Client.h"
#include "../client/ClientManager.h"
#include "../client/Util.h"
#include "../client/LogManager.h"
#include "../client/UploadManager.h"
#include "../client/ShareManager.h"
#include "../client/FavoriteManager.h"
#include "../client/QueueManager.h"
#include "../client/StringTokenizer.h"
#include "../client/IgnoreManager.h" //RSX++

PrivateFrame::FrameMap PrivateFrame::frames;
tstring pSelectedLine = Util::emptyStringT;
tstring pSelectedURL = Util::emptyStringT;

extern CAGEmotionSetup* g_pEmotionsSetup;

LRESULT PrivateFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlClient.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE, IDC_CLIENT);
	
	ctrlClient.LimitText(0);
	ctrlClient.SetFont(WinUtil::font);
	
	ctrlClient.SetBackgroundColor( SETTING(BACKGROUND_COLOR) ); 
	ctrlClient.SetAutoURLDetect(false);
	ctrlClient.SetEventMask( ctrlClient.GetEventMask() | ENM_LINK );
	ctrlMessage.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_AUTOHSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, WS_EX_CLIENTEDGE);
	
	ctrlMessageContainer.SubclassWindow(ctrlMessage.m_hWnd);
	ctrlClientContainer.SubclassWindow(ctrlClient.m_hWnd); //RSX++

	ctrlMessage.SetFont(WinUtil::font);
	ctrlMessage.SetLimitText(9999);

	//RSX++ //custom pm sounds
	ctrlSoundActive.Create(ctrlStatus.m_hWnd, rcDefault, CTSTRING(SOUND_ACTIF_DESACTIF), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlSoundActive.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlSoundActive.SetFont(WinUtil::systemFont);
	ctrlSoundActive.SetCheck(getUser()->getSoundActive());
	soundActiveContainer.SubclassWindow(ctrlSoundActive.m_hWnd);
	//END
	ctrlEmoticons.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | BS_FLAT | BS_BITMAP | BS_CENTER, 0, IDC_EMOT);

	hEmoticonBmp = (HBITMAP) ::LoadImage(_Module.get_m_hInst(), MAKEINTRESOURCE(IDB_EMOTICON), IMAGE_BITMAP, 0, 0, LR_SHARED);
  	ctrlEmoticons.SetBitmap(hEmoticonBmp);

	PostMessage(WM_SPEAKER, USER_UPDATED);
	created = true;

	ClientManager::getInstance()->addListener(this);
	SettingsManager::getInstance()->addListener(this);

	readLog();

	//RSX++
	setClient(replyTo);  //find user and get his client ;] ... case sensitive, specialy when user reconnect or back online
	if(client) {
		//just in case...
		isOp = client->isOp();
		hubName = client->getHubName();
		myNick = client->getMyNick();
	}
	customProtection = false;
	if(RSXBOOLSETTING(PROTECT_PM_USERS)) {
		if(!replyTo->isSet(User::PROTECTED)) {
			replyTo->setFlag(User::PROTECTED);
			customProtection = true;
		}
	}
	//END
	bHandled = FALSE;
	return 1;
}

void PrivateFrame::gotMessage(const Identity& from, const UserPtr& to, const UserPtr& replyTo, const tstring& aMessage) {
	PrivateFrame* p = NULL;
	bool myPM = replyTo == ClientManager::getInstance()->getMe();
	const UserPtr& user = myPM ? to : replyTo;
	
	FrameIter i = frames.find(user);
	if(i == frames.end()) {
		if(frames.size() > 200) return;
		p = new PrivateFrame(user);
		frames[user] = p;
		p->addLine(from, aMessage);
		if(Util::getAway()) {
			if(!(BOOLSETTING(NO_AWAYMSG_TO_BOTS) && (user->isSet(User::BOT) || from.isBot() || from.isHub())))
				p->sendMessage(Text::toT(p->getCustomAway()));
		}

		if(BOOLSETTING(POPUP_NEW_PM)) {
			pair<tstring, bool> hubs = WinUtil::getHubNames(replyTo);
			MainFrame::getMainFrame()->ShowBalloonTip((WinUtil::getNicks(replyTo) + _T(" - ") + hubs.first).c_str(), CTSTRING(PRIVATE_MESSAGE));
		}

		if((BOOLSETTING(PRIVATE_MESSAGE_BEEP) || BOOLSETTING(PRIVATE_MESSAGE_BEEP_OPEN)) && (!BOOLSETTING(SOUNDS_DISABLED)) && user->getSoundActive()) { //RSX++
			if (SETTING(BEEPFILE).empty()) {
				MessageBeep(MB_OK);
			} else {
				::PlaySound(Text::toT(SETTING(BEEPFILE)).c_str(), NULL, SND_FILENAME | SND_ASYNC);
			}
		}
	} else {
		if(!myPM) {
			if(BOOLSETTING(POPUP_PM)) {
				pair<tstring, bool> hubs = WinUtil::getHubNames(replyTo);
				MainFrame::getMainFrame()->ShowBalloonTip((WinUtil::getNicks(replyTo) + _T(" - ") + hubs.first).c_str(), CTSTRING(PRIVATE_MESSAGE));
			}

			if((BOOLSETTING(PRIVATE_MESSAGE_BEEP)) && (!BOOLSETTING(SOUNDS_DISABLED)) && user->getSoundActive()) { //RSX++
				if (SETTING(BEEPFILE).empty()) {
					MessageBeep(MB_OK);
				} else {
					::PlaySound(Text::toT(SETTING(BEEPFILE)).c_str(), NULL, SND_FILENAME | SND_ASYNC);
				}
			}
		}
		i->second->addLine(from, aMessage);
	}
}

void PrivateFrame::openWindow(const UserPtr& replyTo, const tstring& msg) {
	PrivateFrame* p = NULL;
	FrameIter i = frames.find(replyTo);
	if(i == frames.end()) {
		if(frames.size() > 200) return;
		p = new PrivateFrame(replyTo);
		frames[replyTo] = p;
		p->CreateEx(WinUtil::mdiClient);
	} else {
		p = i->second;
		if(::IsIconic(p->m_hWnd))
			::ShowWindow(p->m_hWnd, SW_RESTORE);
		p->MDIActivate(p->m_hWnd);
	}
	if(!msg.empty())
		p->sendMessage(msg);
}

LRESULT PrivateFrame::onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	if (uMsg != WM_KEYDOWN) {
		switch(wParam) {
			case VK_RETURN:
				if( WinUtil::isShift() || WinUtil::isCtrl() ||  WinUtil::isAlt() ) {
					bHandled = FALSE;
				}
				break;
		case VK_TAB:
				bHandled = TRUE;
  				break;
  			default:
  				bHandled = FALSE;
				break;
		}
		if ((uMsg == WM_CHAR) && (GetFocus() == ctrlMessage.m_hWnd) && (wParam != VK_RETURN) && (wParam != VK_TAB) && (wParam != VK_BACK)) {
			if ((!SETTING(SOUND_TYPING_NOTIFY).empty()) && (!BOOLSETTING(SOUNDS_DISABLED)))
				PlaySound(Text::toT(SETTING(SOUND_TYPING_NOTIFY)).c_str(), NULL, SND_FILENAME | SND_ASYNC);
		}
		return 0;
	}
	switch(wParam) {
	case VK_RETURN:
		if( (GetKeyState(VK_SHIFT) & 0x8000) || 
			(GetKeyState(VK_CONTROL) & 0x8000) || 
			(GetKeyState(VK_MENU) & 0x8000) ) {
			bHandled = FALSE;
		} else {
			if(uMsg == WM_KEYDOWN) {
				onEnter();
			}
		}
		break;
	case VK_UP:
		if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
			//scroll up in chat command history
			//currently beyond the last command?
			if (curCommandPosition > 0) {
				//check whether current command needs to be saved
				if (curCommandPosition == prevCommands.size()) {
					auto_ptr<TCHAR> messageContents(new TCHAR[ctrlMessage.GetWindowTextLength()+2]);
					ctrlMessage.GetWindowText(messageContents.get(), ctrlMessage.GetWindowTextLength()+1);
					currentCommand = tstring(messageContents.get());
				}
				//replace current chat buffer with current command
				ctrlMessage.SetWindowText(prevCommands[--curCommandPosition].c_str());
			}
		} else {
			bHandled = FALSE;
		}
		break;
	case VK_DOWN:
		if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
			//scroll down in chat command history
			//currently beyond the last command?
			if (curCommandPosition + 1 < prevCommands.size()) {
				//replace current chat buffer with current command
				ctrlMessage.SetWindowText(prevCommands[++curCommandPosition].c_str());
			} else if (curCommandPosition + 1 == prevCommands.size()) {
				//revert to last saved, unfinished command
				ctrlMessage.SetWindowText(currentCommand.c_str());
				++curCommandPosition;
			}
		} else {
			bHandled = FALSE;
		}
		break;
	case VK_HOME:
		if (!prevCommands.empty() && (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
			curCommandPosition = 0;
			auto_ptr<TCHAR> messageContents(new TCHAR[ctrlMessage.GetWindowTextLength()+2]);
			ctrlMessage.GetWindowText(messageContents.get(), ctrlMessage.GetWindowTextLength()+1);
			currentCommand = tstring(messageContents.get());
			ctrlMessage.SetWindowText(prevCommands[curCommandPosition].c_str());
		} else {
			bHandled = FALSE;
		}
		break;
	case VK_END:
		if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
			curCommandPosition = prevCommands.size();
			ctrlMessage.SetWindowText(currentCommand.c_str());
		} else {
			bHandled = FALSE;
		}
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

void PrivateFrame::onEnter()
{
	bool resetText = true;

	if(ctrlMessage.GetWindowTextLength() > 0) {
		AutoArray<TCHAR> msg(ctrlMessage.GetWindowTextLength()+1);
		ctrlMessage.GetWindowText(msg, ctrlMessage.GetWindowTextLength()+1);
		tstring s(msg, ctrlMessage.GetWindowTextLength());

		// save command in history, reset current buffer pointer to the newest command
		curCommandPosition = prevCommands.size();		//this places it one position beyond a legal subscript
		if (!curCommandPosition || curCommandPosition > 0 && prevCommands[curCommandPosition - 1] != s) {
			++curCommandPosition;
			prevCommands.push_back(s);
		}
		currentCommand = Util::emptyStringT;

		// Process special commands
		if(s[0] == '/') {
			tstring m = s;
			tstring param;
			tstring message;
			tstring status;
			if(WinUtil::checkCommand(s, param, message, status)) {
				if(!message.empty()) {
					sendMessage(message);
				}
				if(!status.empty()) {
					addClientLine(status);
				}
			} else if((Util::stricmp(s.c_str(), _T("clear")) == 0) || (Util::stricmp(s.c_str(), _T("cls")) == 0)) {
				ctrlClient.SetWindowText(_T(""));
			} else if(Util::stricmp(s.c_str(), _T("grant")) == 0) {
				UploadManager::getInstance()->reserveSlot(getUser(), 600);
				addClientLine(TSTRING(SLOT_GRANTED));
			} else if(Util::stricmp(s.c_str(), _T("close")) == 0) {
				PostMessage(WM_CLOSE);
			} else if((Util::stricmp(s.c_str(), _T("favorite")) == 0) || (Util::stricmp(s.c_str(), _T("fav")) == 0)) {
				FavoriteManager::getInstance()->addFavoriteUser(getUser());
				addClientLine(TSTRING(FAVORITE_USER_ADDED));
			} else if(Util::stricmp(s.c_str(), _T("getlist")) == 0) {
				BOOL bTmp;
				onGetList(0,0,0,bTmp);
			} else if(Util::stricmp(s.c_str(), _T("log")) == 0) {
				StringMap params;

				params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(replyTo->getCID()));
				params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(replyTo->getCID()));
				params["userCID"] = replyTo->getCID().toBase32(); 
				params["userNI"] = ClientManager::getInstance()->getNicks(replyTo->getCID())[0];
				params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
				WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params, false))));
			} else if(Util::stricmp(s.c_str(), _T("stats")) == 0) {
				addLine(Text::toT(WinUtil::generateStats()), WinUtil::m_ChatTextMyOwn);
			} else if(Util::stricmp(s.c_str(), _T("pstats")) == 0) {
				sendMessage(Text::toT(WinUtil::generateStats()));
			} else if(Util::stricmp(s.c_str(), _T("help")) == 0) {
				addLine(_T("*** ") + WinUtil::commands + _T(", /getlist, /clear, /grant, /close, /favorite, /winamp"), WinUtil::m_ChatTextSystem);
			} else {
				if(replyTo->isOnline()) {
					sendMessage(tstring(m));
				} else {
					ctrlStatus.SetText(0, CTSTRING(USER_WENT_OFFLINE));
					resetText = false;
				}
			}
		} else {
			if(replyTo->isOnline()) {
				if(BOOLSETTING(CZCHARS_DISABLE))
					s = WinUtil::disableCzChars(s);

				sendMessage(s);
			} else {
				ctrlStatus.SetText(0, CTSTRING(USER_WENT_OFFLINE));
				resetText = false;
			}
		}
		if(resetText)
			ctrlMessage.SetWindowText(_T(""));
	} 
}

void PrivateFrame::sendMessage(const tstring& msg) {
	ClientManager::getInstance()->privateMessage(replyTo, Text::fromT(msg));
}

LRESULT PrivateFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	//RSX++
	if(customProtection && replyTo)
		replyTo->unsetFlag(User::PROTECTED);
	//END
	if(!closed) {
		DeleteObject(hEmoticonBmp);
		ClientManager::getInstance()->removeListener(this);
		SettingsManager::getInstance()->removeListener(this);
		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		frames.erase(replyTo);

		bHandled = FALSE;
		return 0;
	}
}

void PrivateFrame::addLine(const tstring& aLine, CHARFORMAT2& cf) {
	Identity i = Identity(NULL, 0);
    addLine(i, aLine, cf);
}

void PrivateFrame::addLine(const Identity& from, const tstring& aLine) {
	addLine(from, aLine, WinUtil::m_ChatTextGeneral );
}

void PrivateFrame::addLine(const Identity& from, const tstring& aLine, CHARFORMAT2& cf) {
	if(!created) {
		if(BOOLSETTING(POPUNDER_PM))
			WinUtil::hiddenCreateEx(this);
		else
			CreateEx(WinUtil::mdiClient);
	}
	ctrlClient.AdjustTextSize();

	CRect r;
	ctrlClient.GetClientRect(r);

	if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
		StringMap params;
		params["message"] = Text::fromT(aLine);
		params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(replyTo->getCID()));
		params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(replyTo->getCID()));
		params["userCID"] = replyTo->getCID().toBase32(); 
		params["userNI"] = ClientManager::getInstance()->getNicks(replyTo->getCID())[0];
		params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
		LOG(LogManager::PM, params);
	}

	//rsx++ was here...
	tstring currentNick = Text::toT(client ? client->getCurrentNick() : SETTING(NICK));

	if(BOOLSETTING(TIME_STAMPS)) {
		ctrlClient.AppendText(from, currentNick, Text::toT("[" + Util::getShortTimeString() + "] "), aLine.c_str(), cf);
	} else {
		ctrlClient.AppendText(from, currentNick, _T(""), aLine.c_str(), cf);
	}
	//... end
	addClientLine(CTSTRING(LAST_CHANGE) +  Text::toT(Util::getTimeString()));

	if (BOOLSETTING(BOLD_PM)) {
		setDirty();
	}
}

LRESULT PrivateFrame::onEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlClient.Copy();
	return 0;
}

LRESULT PrivateFrame::onEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlClient.SetSelAll();
	return 0;
}

LRESULT PrivateFrame::onEditClearAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlClient.SetWindowText(_T(""));
	return 0;
}

LRESULT PrivateFrame::onCopyActualLine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (pSelectedLine != _T("")) {
		WinUtil::setClipboard(pSelectedLine);
	}
	return 0;
}

LRESULT PrivateFrame::onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 

	OMenu tabMenu;
	tabMenu.CreatePopupMenu();	

	tabMenu.InsertSeparatorFirst(Text::toT(ClientManager::getInstance()->getNicks(getSelectedUser()->getCID())[0]));
	if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
		tabMenu.AppendMenu(MF_STRING, IDC_OPEN_USER_LOG,  CTSTRING(OPEN_USER_LOG));
		tabMenu.AppendMenu(MF_SEPARATOR);
	}
	tabMenu.AppendMenu(MF_STRING, ID_EDIT_CLEAR_ALL, CTSTRING(CLEAR));
	tabMenu.AppendMenu(MF_SEPARATOR);
	tabMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
	tabMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
	tabMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)WinUtil::grantMenu, CTSTRING(GRANT_SLOTS_MENU));
	tabMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));

	prepareMenu(tabMenu, UserCommand::CONTEXT_CHAT, ClientManager::getInstance()->getHubs(replyTo->getCID()));
	if(!(tabMenu.GetMenuState(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION) & MF_SEPARATOR)) {	
		tabMenu.AppendMenu(MF_SEPARATOR);
	}
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CTSTRING(CLOSE));

	tabMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	return TRUE;
}

void PrivateFrame::runUserCommand(UserCommand& uc) {

	if(!WinUtil::getUCParams(m_hWnd, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	ClientManager::getInstance()->userCommand(getSelectedUser(), uc, ucParams, true);
}

LRESULT PrivateFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try { //rsx was here
		QueueManager::getInstance()->addList(getSelectedUser(), QueueItem::FLAG_CLIENT_VIEW);
	} catch(const Exception& e) {
		addClientLine(Text::toT(e.getError()));
	}
	return 0;
}

LRESULT PrivateFrame::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try { //rsx was here
		QueueManager::getInstance()->addList(getSelectedUser(), QueueItem::FLAG_MATCH_QUEUE);
	} catch(const Exception& e) {
		addClientLine(Text::toT(e.getError()));
	}
	return 0;
}

LRESULT PrivateFrame::onGrantSlot(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	uint64_t time = 0;
	switch(wID) {
		case IDC_GRANTSLOT:			time = 600; break;
		case IDC_GRANTSLOT_DAY:		time = 3600; break;
		case IDC_GRANTSLOT_HOUR:	time = 24*3600; break;
		case IDC_GRANTSLOT_WEEK:	time = 7*24*3600; break;
		case IDC_UNGRANTSLOT:		time = 0; break;
	}
	
	if(time > 0)
		UploadManager::getInstance()->reserveSlot(getSelectedUser(), time);
	else
		UploadManager::getInstance()->unreserveSlot(getSelectedUser());

	return 0;
}

LRESULT PrivateFrame::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	//rsx was here
	FavoriteManager::getInstance()->addFavoriteUser(getSelectedUser());
	return 0;
}

void PrivateFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		//RSX++ //custom pm sound button
		int w[2];
		ctrlStatus.GetClientRect(sr);
		
		int larg = WinUtil::getTextWidth(TSTRING(SOUND_ACTIF_DESACTIF), ctrlStatus.m_hWnd);

		w[0] = sr.right - (larg + 16);
		w[1] = w[0] + larg;

		ctrlStatus.SetParts(2, w);

		ctrlStatus.GetRect(0, sr);
		sr.left = sr.right + 2;
		sr.right = sr.left + larg;
		ctrlSoundActive.MoveWindow(sr);
		//END
	}
	
	int h = WinUtil::fontHeight + 4;

	CRect rc = rect;
	rc.bottom -= h + 10;
	ctrlClient.MoveWindow(rc);
	
	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - h - 5;
	rc.left +=2;
	rc.right -= 2 + 24;
	ctrlMessage.MoveWindow(rc);
	
	rc.left = rc.right + 2;
  	rc.right += 24;
  	 
  	ctrlEmoticons.MoveWindow(rc);
}

void PrivateFrame::updateTitle() {
	pair<tstring, bool> hubs = WinUtil::getHubNames(replyTo);
	if(hubs.second) {	
		unsetIconState();
		//setTabColor(RGB(0, 255,	255));
		if(isoffline) {
			tstring status = _T(" *** ") + TSTRING(USER_WENT_ONLINE) + _T(" [") + WinUtil::getNicks(replyTo) + Text::toT(" - " + hubName) + _T("] ***");
			if(BOOLSETTING(STATUS_IN_CHAT)) {
				addLine(status, WinUtil::m_ChatTextServer);
			} else {
				addClientLine(status);
			}
		}
		isoffline = false;
	} else {
		setIconState();
		//setTabColor(RGB(255, 0, 0));
		tstring status = _T(" *** ") + TSTRING(USER_WENT_OFFLINE) + _T(" [") + WinUtil::getNicks(replyTo) + Text::toT(" - " + hubName) + _T("] ***");
		if(BOOLSETTING(STATUS_IN_CHAT)) {
			addLine(status, WinUtil::m_ChatTextServer);
		} else {
			addClientLine(status);
		}
		isoffline = true;
	}
	SetWindowText((WinUtil::getNicks(replyTo) + _T(" - ") + hubs.first).c_str());
}

LRESULT PrivateFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	bHandled = FALSE;

	POINT p;
	p.x = GET_X_LPARAM(lParam);
	p.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(ctrlClient.m_hWnd, &p);

	POINT cpt;
	GetCursorPos(&cpt);

	CRect rc;            // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click

	OMenu Mnu;
	ChatCtrl::sSelectedUser = Util::emptyStringT;
	pSelectedLine = _T("");

	bool bHitURL = ctrlClient.HitURL();
	if (!bHitURL)
		pSelectedURL = _T("");

	if(reinterpret_cast<HWND>(wParam) == ctrlEmoticons) { 
		if(emoMenu != NULL) emoMenu.DestroyMenu();
		emoMenu.CreatePopupMenu();
		menuItems = 0;
		emoMenu.AppendMenu(MF_STRING, IDC_EMOMENU, _T("Disabled"));
		if (SETTING(EMOTICONS_FILE)=="Disabled") emoMenu.CheckMenuItem( IDC_EMOMENU, MF_BYCOMMAND | MF_CHECKED );
		// nacteme seznam emoticon packu (vsechny *.xml v adresari EmoPacks)
		WIN32_FIND_DATA data;
		HANDLE hFind;
		hFind = FindFirstFile(Text::toT(Util::getDataPath()+"EmoPacks\\*.xml").c_str(), &data);
		if(hFind != INVALID_HANDLE_VALUE) {
			do {
				tstring name = data.cFileName;
				tstring::size_type i = name.rfind('.');
				name = name.substr(0, i);

				menuItems++;
				emoMenu.AppendMenu(MF_STRING, IDC_EMOMENU + menuItems, name.c_str());
				if(name == Text::toT(SETTING(EMOTICONS_FILE))) emoMenu.CheckMenuItem( IDC_EMOMENU + menuItems, MF_BYCOMMAND | MF_CHECKED );
			} while(FindNextFile(hFind, &data));
			FindClose(hFind);
		}
		emoMenu.InsertSeparatorFirst(_T("Emoticons Pack"));
		if(menuItems>0) emoMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		emoMenu.RemoveFirstItem();
		return TRUE;
	}
	//RSX++ //give pms some colors and rclick menus! :)
	if(reinterpret_cast<HWND>(wParam) == ctrlClient) { 
		ChatCtrl::sSelectedUser = Text::toT(ClientManager::getInstance()->getFirstNick(replyTo->getCID()));

		if(pt.x == -1 && pt.y == -1) {
			CRect erc;
			ctrlClient.GetRect(&erc);
			pt.x = erc.Width() / 2;
			pt.y = erc.Height() / 2;
			ctrlClient.ClientToScreen(&pt);
		}
		POINT ptCl = pt;
		ctrlClient.ScreenToClient(&ptCl); 
		ctrlClient.OnRButtonDown(ptCl);
		pSelectedLine = ctrlClient.LineFromPos(p);

		selUser = NULL;
		if(!ChatCtrl::sSelectedUser.empty())
			selUser = client->findUser(Text::fromT(ChatCtrl::sSelectedUser))->getUser();

		if(PreparePopupMenu(ChatCtrl::sSelectedUser, Mnu )) {
			if(ChatCtrl::sSelectedUser.empty()) {
				Mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
				return TRUE;
			} else {
				if(client) { //no client? no fun
					prepareMenu(Mnu, ::UserCommand::CONTEXT_CHAT, client->getHubUrl());
					Mnu.AppendMenu(MF_STRING, ID_EDIT_CLEAR_ALL, CTSTRING(CLEAR));
					Mnu.AppendMenu(MF_SEPARATOR);
					Mnu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CTSTRING(CLOSE));

					Mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
					return TRUE;
				}
			}
		}
	}
	//END
	return FALSE;
}

LRESULT PrivateFrame::onClientEnLink(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	ENLINK* pEL = (ENLINK*)pnmh;

	if ( pEL->msg == WM_LBUTTONUP ) {
		long lBegin = pEL->chrg.cpMin, lEnd = pEL->chrg.cpMax;
		TCHAR* sURLTemp = new TCHAR[(lEnd - lBegin)+1];
		if(sURLTemp) {
			ctrlClient.GetTextRange(lBegin, lEnd, sURLTemp);
			tstring sURL = sURLTemp;
			WinUtil::openLink(sURL);
			delete[] sURLTemp;
		}
	} else if ( pEL->msg == WM_RBUTTONUP ) {
		pSelectedURL = _T("");
		long lBegin = pEL->chrg.cpMin, lEnd = pEL->chrg.cpMax;
		TCHAR* sURLTemp = new TCHAR[(lEnd - lBegin)+1];
		if(sURLTemp) {
			ctrlClient.GetTextRange(lBegin, lEnd, sURLTemp);
			pSelectedURL = sURLTemp;
			delete[] sURLTemp;
		}

		ctrlClient.SetSel( lBegin, lEnd );
		ctrlClient.InvalidateRect( NULL );
		return 0;
	}
	return 0;
}

void PrivateFrame::readLog() {
	if (SETTING(SHOW_LAST_LINES_LOG) == 0) return;

	StringMap params;
	
	params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(replyTo->getCID()));
	params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(replyTo->getCID()));
	params["userCID"] = replyTo->getCID().toBase32(); 
	params["userNI"] = ClientManager::getInstance()->getNicks(replyTo->getCID())[0];
	params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
		
	string path = Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params, false));

	try {
		File f(path, File::READ, File::OPEN);
		
		int64_t size = f.getSize();

		if(size > 32*1024) {
			f.setPos(size - 32*1024);
		}
		string buf = f.read(32*1024);
		StringList lines;

		if(Util::strnicmp(buf.c_str(), "\xef\xbb\xbf", 3) == 0)
			lines = StringTokenizer<string>(buf.substr(3), "\r\n").getTokens();
		else
			lines = StringTokenizer<string>(buf, "\r\n").getTokens();

		int linesCount = lines.size();

		int i = linesCount > (SETTING(SHOW_LAST_LINES_LOG) + 1) ? linesCount - SETTING(SHOW_LAST_LINES_LOG) : 0;

		for(; i < linesCount; ++i){
			ctrlClient.AppendText(Identity(NULL, 0), _T("- "), _T(""), (Text::toT(lines[i])).c_str(), WinUtil::m_ChatTextLog, true);
		}

		f.close();
	} catch(const FileException&){
	}
}

void PrivateFrame::closeAll(){
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		i->second->PostMessage(WM_CLOSE, 0, 0);
}

void PrivateFrame::closeAllOffline() {
	for(FrameIter i = frames.begin(); i != frames.end(); ++i) {
		if(!i->first->isOnline())
			i->second->PostMessage(WM_CLOSE, 0, 0);
	}
}

LRESULT PrivateFrame::onOpenUserLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {	
	StringMap params;
	params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(getSelectedUser()->getCID()));
	params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(getSelectedUser()->getCID()));
	params["userCID"] = getSelectedUser()->getCID().toBase32(); 
	params["userNI"] = ClientManager::getInstance()->getFirstNick(getSelectedUser()->getCID());
	params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();

	string file = Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params, false));
	if(Util::fileExists(file)) {
		ShellExecute(NULL, NULL, Text::toT(file).c_str(), NULL, NULL, SW_SHOWNORMAL);
	} else {
		MessageBox(CTSTRING(NO_LOG_FOR_USER), CTSTRING(NO_LOG_FOR_USER), MB_OK );	  
	}	

	return 0;
}

LRESULT PrivateFrame::onCopyURL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (pSelectedURL != _T("")) {
		WinUtil::setClipboard(pSelectedURL);
	}
	return 0;
}

void PrivateFrame::on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw() {
	ctrlClient.SetBackgroundColor(WinUtil::bgColor);
	RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

LRESULT PrivateFrame::onEmoPackChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	TCHAR buf[256];
	emoMenu.GetMenuString(wID, buf, 256, MF_BYCOMMAND);
	if (buf!=Text::toT(SETTING(EMOTICONS_FILE))) {
		SettingsManager::getInstance()->set(SettingsManager::EMOTICONS_FILE, Text::fromT(buf));
		g_pEmotionsSetup->Unload();
		g_pEmotionsSetup->Load();
	}
	return 0;
}
//RSX++ //rclick menu
bool PrivateFrame::PreparePopupMenu(const tstring& sNick, OMenu& pMenu ) {
	if (copyMenu.m_hMenu != NULL) {
		copyMenu.DestroyMenu();
		copySubMenu.DestroyMenu();
		copyMenu.m_hMenu = NULL;
		copySubMenu.m_hMenu = NULL;
	}

	copySubMenu.CreatePopupMenu();
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_EXACT_SHARE,		CTSTRING(COPY_EXACT_SHARE));
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_DESCRIPTION,		CTSTRING(COPY_DESCRIPTION));
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_TAG,				CTSTRING(COPY_TAG));
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_EMAIL_ADDRESS,	CTSTRING(COPY_EMAIL_ADDRESS));
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_IP,				CTSTRING(COPY_IP));
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_CONNECTION,		CTSTRING(COPY_CONNECTION));
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_MYINFO,			CTSTRING(COPY_MYINFO));
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_CLIENT,			CTSTRING(COPY_CLIENT));
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_CHEAT,			CTSTRING(COPY_CHEAT));
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_HOST,			CTSTRING(COPY_HOST));
	copySubMenu.AppendMenu(MF_STRING, IDC_COPY_SUB_CID,				CTSTRING(COPY_CID));
	copySubMenu.InsertSeparator(0, TRUE, _T("Nick + ..."));

	copyMenu.CreatePopupMenu();
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_NICK, CTSTRING(COPY_NICK));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_EXACT_SHARE, CTSTRING(COPY_EXACT_SHARE));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_DESCRIPTION, CTSTRING(COPY_DESCRIPTION));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_TAG, CTSTRING(COPY_TAG));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_EMAIL_ADDRESS, CTSTRING(COPY_EMAIL_ADDRESS));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_IP, CTSTRING(COPY_IP));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_CONNECTION, CTSTRING(COPY_CONNECTION));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_MYINFO, CTSTRING(COPY_MYINFO));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_CLIENT, CTSTRING(COPY_CLIENT));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_CHEAT, CTSTRING(COPY_CHEAT));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_HOST, CTSTRING(COPY_HOST));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_CID, CTSTRING(COPY_CID));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_ALL, CTSTRING(COPY_ALL));
	copyMenu.AppendMenu(MF_SEPARATOR);
	copyMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)copySubMenu, _T("Nick + ..."));
	copyMenu.InsertSeparator(0, TRUE, TSTRING(COPY));

	pMenu.CreatePopupMenu();

	if(sNick.empty()) {
		if(!ChatCtrl::sSelectedIP.empty()) {
			pMenu.InsertSeparator(0, TRUE, ChatCtrl::sSelectedIP);
			pMenu.AppendMenu(MF_STRING, IDC_WHOIS_IP, (CTSTRING(WHO_IS) + ChatCtrl::sSelectedIP).c_str() );
			if(isOp) {
				pMenu.AppendMenu(MF_SEPARATOR);
				pMenu.AppendMenu(MF_STRING, IDC_BAN_IP, (_T("!banip ") + ChatCtrl::sSelectedIP).c_str());
				pMenu.SetMenuDefaultItem(IDC_BAN_IP);
				pMenu.AppendMenu(MF_STRING, IDC_UNBAN_IP, (_T("!unban ") + ChatCtrl::sSelectedIP).c_str());
				pMenu.AppendMenu(MF_SEPARATOR);
			}
		} else pMenu.InsertSeparator(0, TRUE, _T("Text"));
		pMenu.AppendMenu(MF_STRING, ID_EDIT_COPY, CTSTRING(COPY));
		pMenu.AppendMenu(MF_STRING, IDC_COPY_ACTUAL_LINE,  CTSTRING(COPY_LINE));
		if(!pSelectedURL.empty()) 
  			pMenu.AppendMenu(MF_STRING, IDC_COPY_URL, CTSTRING(COPY_URL));
		pMenu.AppendMenu(MF_SEPARATOR);
		pMenu.AppendMenu(MF_STRING, ID_EDIT_SELECT_ALL, CTSTRING(SELECT_ALL));
		pMenu.AppendMenu(MF_STRING, ID_EDIT_CLEAR_ALL, CTSTRING(CLEAR));
		pMenu.AppendMenu(MF_SEPARATOR);
		pMenu.AppendMenu(MF_STRING, IDC_AUTOSCROLL_CHAT, CTSTRING(ASCROLL_CHAT));
		
		if(ctrlClient.GetAutoScroll())
			pMenu.CheckMenuItem(IDC_AUTOSCROLL_CHAT, MF_BYCOMMAND | MF_CHECKED);
	} else {
		bool bIsMe = (Text::toLower(sNick) == Text::toLower(Text::toT(myNick)));

		pMenu.InsertSeparator(0, TRUE, sNick);
		if(bIsMe == false && !BOOLSETTING(LOG_PRIVATE_CHAT)) {
			pMenu.AppendMenu(MF_STRING | MF_DISABLED, (UINT_PTR)0,  CTSTRING(OPEN_USER_LOG));
			pMenu.AppendMenu(MF_SEPARATOR);
		} else if(bIsMe) {
			pMenu.AppendMenu(MF_STRING, IDC_OPEN_USER_LOG,  CTSTRING(OPEN_USER_LOG));
			pMenu.AppendMenu(MF_SEPARATOR);
		}				
		pMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)copyMenu, CTSTRING(COPY));

		if(bIsMe == false) {
			pMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)WinUtil::grantMenu, CTSTRING(GRANT_SLOTS_MENU));
			if(client) {
				if(isOp || !ClientManager::getInstance()->isOp(getSelectedUser(), client->getHubUrl())) {
					pMenu.AppendMenu(MF_SEPARATOR);
					if(IgnoreManager::getInstance()->isIgnored(Text::fromT(sNick))) {
						pMenu.AppendMenu(MF_STRING, IDC_UNIGNORE, CTSTRING(UNIGNORE_USER));
					} else {  
						pMenu.AppendMenu(MF_STRING, IDC_IGNORE, CTSTRING(IGNORE_USER));
					}
				}
			}

			if(Text::toLower(sNick) != Text::toLower(Text::toT(ClientManager::getInstance()->getFirstNick(replyTo->getCID())))) {
				pMenu.AppendMenu(MF_SEPARATOR);
				pMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CTSTRING(SEND_PRIVATE_MESSAGE));
			}
			pMenu.AppendMenu(MF_SEPARATOR);
			pMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
			pMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
			pMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
		}

		switch(SETTING(CHAT_DBLCLICK)) {
			case 0:
				pMenu.SetMenuDefaultItem(IDC_SELECT_USER);
				break;
			case 1:
				pMenu.SetMenuDefaultItem(IDC_PUBLIC_MESSAGE);
				break;
			case 2:
				pMenu.SetMenuDefaultItem(IDC_PRIVATEMESSAGE);
				break;
			case 3:
				pMenu.SetMenuDefaultItem(IDC_GETLIST);
				break;
			case 4:
				pMenu.SetMenuDefaultItem(IDC_MATCH_QUEUE);
				break;
			case 6:
				pMenu.SetMenuDefaultItem(IDC_ADD_TO_FAVORITES);
				break;
		} 
	}
	return true;
}

LRESULT PrivateFrame::onLButton(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	HWND focus = GetFocus();
	bHandled = false;
	if(focus == ctrlClient.m_hWnd) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		int i = ctrlClient.CharFromPos(pt);
		int line = ctrlClient.LineFromChar(i);
		int c = LOWORD(i) - ctrlClient.LineIndex(line);
		int len = ctrlClient.LineLength(i) + 1;
		if(len < 3) {
			return 0;
		}

		TCHAR* buf = new TCHAR[len];
		ctrlClient.GetLine(line, buf, len);
		tstring x = tstring(buf, len-1);
		delete[] buf;

		string::size_type start = x.find_last_of(_T(" <\t\r\n"), c);

		if(start == string::npos)
			start = 0;
		else
			start++;
					

		string::size_type end = x.find_first_of(_T(" >\t"), start+1);

			if(end == string::npos) // get EOL as well
				end = x.length();
			else if(end == start + 1)
				return 0;

			// Nickname click, let's see if we can find one like it in the name list...
			tstring nick = x.substr(start, end - start);
			if(!client)
				return 0;
			OnlineUser* ui = client->findUser(Text::fromT(nick));
			if(ui) {
				bHandled = true;
				if (wParam & MK_CONTROL) { // MK_CONTROL = 0x0008
					PrivateFrame::openWindow(ui->getUser());
				} else if (wParam & MK_SHIFT) {
					try {
						QueueManager::getInstance()->addList(ui->getUser(), QueueItem::FLAG_CLIENT_VIEW);
					} catch(const Exception& e) {
						addLine(Text::toT(e.getError()), WinUtil::m_ChatTextSystem);
					}
				} else {
				switch(SETTING(CHAT_DBLCLICK)) {
					case 1: {
					     tstring sUser = ui->getText(OnlineUser::COLUMN_NICK);
					     int iSelBegin, iSelEnd;
					     ctrlMessage.GetSel(iSelBegin, iSelEnd);

					     if((iSelBegin == 0) && (iSelEnd == 0)) {
							sUser += _T(": ");
							if(ctrlMessage.GetWindowTextLength() == 0) {   
			                    ctrlMessage.SetWindowText(sUser.c_str());
			                    ctrlMessage.SetFocus();
			                    ctrlMessage.SetSel(ctrlMessage.GetWindowTextLength(), ctrlMessage.GetWindowTextLength());
							} else {
			                    ctrlMessage.ReplaceSel(sUser.c_str());
								ctrlMessage.SetFocus();
					        }
					     } else {
					          sUser += _T(" ");
					          ctrlMessage.ReplaceSel(sUser.c_str());
					          ctrlMessage.SetFocus();
					     }
					     break;
					}
					case 2:
						if(ui->getUser() != (ClientManager::getInstance()->getMe() || replyTo))
					          ui->pm();
					     break;
					case 3:
					     ui->getList();
					     break;
					case 4:
					     ui->matchQueue();
					     break;
					case 5:
					     ui->grant();
					     break;
					case 6:
					     ui->addFav();
					     break;
				}
			}
		}
	}
	return 0;
}

LRESULT PrivateFrame::onPM(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	//send pm from pm =) [good one to use with opchats]
	if(client && !ChatCtrl::sSelectedUser.empty()) {
		if(getUser() != getSelectedUser())
			openWindow(getSelectedUser());
	}
	return 0;
}

LRESULT PrivateFrame::onAutoScrollChat(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlClient.SetAutoScroll(!ctrlClient.GetAutoScroll() );
	return 0;
}

LRESULT PrivateFrame::onBanIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ChatCtrl::sSelectedIP != _T("")) {
		tstring s = _T("!banip ") + ChatCtrl::sSelectedIP + _T(" ");
		if(client) {
			client->hubMessage(Text::fromT(s));
		}
	}
	return 0;
}

LRESULT PrivateFrame::onUnBanIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ChatCtrl::sSelectedIP != _T("")) {
		tstring s = _T("!unban ") + ChatCtrl::sSelectedIP + _T(" "); //verli NEED a unban reason at all
		if(client) {
			client->hubMessage(Text::fromT(s));
		}
	}
	return 0;
}

LRESULT PrivateFrame::onWhoisIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ChatCtrl::sSelectedIP != _T("")) {
 		WinUtil::openLink(_T("http://www.ripe.net/perl/whois?form_type=simple&full_query_string=&searchtext=") + ChatCtrl::sSelectedIP);
 	}
	return 0;
}

LRESULT PrivateFrame::onGetUserResponse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		QueueManager::getInstance()->addTestSUR(getSelectedUser());
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError());		
	}
	return 0;
}

LRESULT PrivateFrame::onCheckFileList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		QueueManager::getInstance()->addList(getSelectedUser(), QueueItem::FLAG_CHECK_FILE_LIST);
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError());		
	}
	return 0;
}

LRESULT PrivateFrame::onReportUser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ClientManager::getInstance()->reportUser(getSelectedUser());
	return 0;
}

LRESULT PrivateFrame::onCleanUser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ClientManager::getInstance()->cleanUser(getSelectedUser());
	return 0;
}

LRESULT PrivateFrame::onIgnore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	IgnoreManager::getInstance()->storeIgnore(getSelectedUser());
	return 0;
}

LRESULT PrivateFrame::onUnIgnore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	IgnoreManager::getInstance()->removeIgnore(getSelectedUser());
	return 0;
}

LRESULT PrivateFrame::onCopyUserInfo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring sCopy;
	if(!ChatCtrl::sSelectedUser.empty() && client != NULL) {
		const OnlineUser* ui = client->findUser(Text::fromT(ChatCtrl::sSelectedUser));
		if(ui) {
			switch (wID) {
				case IDC_COPY_NICK:
					sCopy += Text::toT(ui->getNick());
					break;
				case IDC_COPY_EXACT_SHARE:
					sCopy += Util::formatExactSize(ui->getIdentity().getBytesShared());
					break;
				case IDC_COPY_DESCRIPTION:
					sCopy += Text::toT(ui->getIdentity().getDescription());
					break;
				case IDC_COPY_TAG:
					sCopy += Text::toT(ui->getIdentity().getTag());
					break;
				case IDC_COPY_EMAIL_ADDRESS:
					sCopy += Text::toT(ui->getIdentity().getEmail());
					break;
				case IDC_COPY_IP:
					sCopy += Text::toT(ui->getIdentity().getIp());
					break;
				case IDC_COPY_SUB_EXACT_SHARE:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Util::formatExactSize(ui->getIdentity().getBytesShared());
					break;
				case IDC_COPY_SUB_DESCRIPTION:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Text::toT(ui->getIdentity().getDescription());
					break;
				case IDC_COPY_SUB_TAG:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Text::toT(ui->getIdentity().getTag());
					break;
				case IDC_COPY_SUB_EMAIL_ADDRESS:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Text::toT(ui->getIdentity().getEmail());
					break;
				case IDC_COPY_SUB_IP:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Text::toT(ui->getIdentity().getIp());
					break;
				case IDC_COPY_SUB_CONNECTION:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Text::toT(ui->getIdentity().getConnection());
					break;
				case IDC_COPY_SUB_MYINFO:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Text::toT(ui->getIdentity().getMyInfoType());
					break;
				case IDC_COPY_SUB_CLIENT:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Text::toT(ui->getIdentity().get("CT"));
					break;
				case IDC_COPY_SUB_CHEAT:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Text::toT(ui->getIdentity().get("CS"));
					break;
				case IDC_COPY_SUB_HOST:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Text::toT(ui->getIdentity().get("HT"));
					break;
				case IDC_COPY_SUB_CID:
					sCopy += Text::toT(ui->getIdentity().getNick()) + _T(" ") + Text::toT(ui->getUser()->getCID().toBase32());
					break;
				case IDC_COPY_CONNECTION:
					sCopy += Text::toT(ui->getIdentity().getConnection());
					break;
				case IDC_COPY_MYINFO:
					sCopy += Text::toT(ui->getIdentity().getMyInfoType());
					break;
				case IDC_COPY_CLIENT:
					sCopy += Text::toT(ui->getIdentity().get("CT"));
					break;
				case IDC_COPY_CHEAT:
					sCopy += Text::toT(ui->getIdentity().get("CS"));
					break;
				case IDC_COPY_HOST:
					sCopy += Text::toT(ui->getIdentity().get("HT"));
					break;
				case IDC_COPY_CID:
					sCopy += Text::toT(ui->getUser()->getCID().toBase32());
					break;
				case IDC_COPY_ALL:
					sCopy += _T("Info User:\r\n")
						_T("\tNick: ") + Text::toT(ui->getIdentity().getNick()) + _T("\r\n") + 
						_T("\tShare: ") + Util::formatBytesW(ui->getIdentity().getBytesShared()) + _T("\r\n") + 
						_T("\tDescription: ") + Text::toT(ui->getIdentity().getDescription()) + _T("\r\n") +
						_T("\tTag: ") + Text::toT(ui->getIdentity().getTag()) + _T("\r\n") +
						_T("\tConnection: ") + Text::toT(ui->getIdentity().getConnection()) + _T("\r\n") + 
						_T("\tE-Mail: ") + Text::toT(ui->getIdentity().getEmail()) + _T("\r\n") +
						_T("\tClient: ") + Text::toT(ui->getIdentity().get("CT")) + _T("\r\n") + 
						_T("\tMyInfo: ") + Text::toT(ui->getIdentity().getMyInfoType()) + _T("\r\n")+
						_T("\tVersion: ") + Text::toT(ui->getIdentity().get("VE")) + _T("\r\n") +
						_T("\tMode: ") + ui->getText(OnlineUser::COLUMN_MODE) + _T("\r\n") +
						_T("\tHubs: ") + ui->getText(OnlineUser::COLUMN_HUBS) + _T("\r\n") +
						_T("\tSlots: ") + ui->getText(OnlineUser::COLUMN_SLOTS) + _T("\r\n") +
						_T("\tUpLimit: ") + ui->getText(OnlineUser::COLUMN_UPLOAD_SPEED) + _T("\r\n") +
						_T("\tIP: ") + Text::toT(ui->getIdentity().getIp()) + _T("\r\n") +
						_T("\tHost: ") + Text::toT(ui->getIdentity().get("HT")) + _T("\r\n") +
						_T("\tPk String: ") + Text::toT(ui->getIdentity().get("PK")) + _T("\r\n") +
						_T("\tLock: " )+ Text::toT(ui->getIdentity().get("LO")) + _T("\r\n")+
						_T("\tSupports: ") + Text::toT(ui->getIdentity().get("SU")) + _T("\r\n")+
						_T("\tCheating description: ") + Text::toT(ui->getIdentity().get("CS")) + _T("\r\n") +
						_T("\tComment: ") + Text::toT(ui->getIdentity().get("CM"));
					break;		
				default:
					dcdebug("HUBFRAME DON'T GO HERE\n");
					return 0;
			}
		}
	}
	if (!sCopy.empty())
		WinUtil::setClipboard(sCopy);

	return 0;
}

LRESULT PrivateFrame::onCustomKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!ChatCtrl::sSelectedUser.empty() && client) {
		OnlineUser* ui = client->findUser(Text::fromT(ChatCtrl::sSelectedUser));
		if(ui) {
			ui->customKick();
		}
	}
	return 0;
}

LRESULT PrivateFrame::onMultihubKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!ChatCtrl::sSelectedUser.empty() && client) {
		OnlineUser* ui = client->findUser(Text::fromT(ChatCtrl::sSelectedUser));
		if(ui) {
			ui->multiHubKick();
		}
	}
	return 0;
}
//END
/**
 * @file
 * $Id: PrivateFrame.cpp 312 2007-07-25 20:49:11Z bigmuscle $
 */
