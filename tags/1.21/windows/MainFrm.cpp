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

#include "MainFrm.h"
#include "AboutDlg.h"
#include "HubFrame.h"
#include "SearchFrm.h"
#include "PublicHubsFrm.h"
#include "PropertiesDlg.h"
#include "UsersFrame.h"
#include "DirectoryListingFrm.h"
#include "RecentsFrm.h"
#include "FavoritesFrm.h"
#include "NotepadFrame.h"
#include "QueueFrame.h"
#include "SpyFrame.h"
#include "FinishedFrame.h"
#include "ADLSearchFrame.h"
#include "FinishedULFrame.h"
#include "TextFrame.h"
#include "StatsFrame.h"
#include "UploadQueueFrame.h"
#include "LineDlg.h"
#include "HashProgressDlg.h"
#include "UPnP_COM.h"
#include "PrivateFrame.h"
#include "WinUtil.h"
#include "CDMDebugFrame.h"
#include "InputBox.h"
#include "PopupManager.h"

#include "../client/ConnectionManager.h"
#include "../client/DownloadManager.h"
#include "../client/HashManager.h"
#include "../client/UploadManager.h"
#include "../client/StringTokenizer.h"
#include "../client/SimpleXML.h"
#include "../client/ShareManager.h"
#include "../client/LogManager.h"
#include "../client/WebServerManager.h"
#include "../client/Thread.h"
//RSX++
#include "../rsx/RsxUtil.h"
#include "../rsx/UpdateManager.h"
#include "../rsx/HTTPDownloadManager.h"
#include "../client/PluginsManager.h"
#include "../client/NetworkConfiguration.hpp"
#include "UpdateDialog.h"
#include "PluginsListDlg.h"
#include "ShutdownManager.h"
#include "ShutdownDlg.h"
//END
#include "../dht/dht.h"

MainFrame* MainFrame::anyMF = NULL;

MainFrame::MainFrame() : trayMessage(0), maximized(false), lastUpload(-1), lastUpdate(0), 
	lastUp(0), lastDown(0), oldshutdown(false), stopperThread(NULL), 
	closing(false), awaybyminimize(false), missedAutoConnect(false), lastTTHdir(Util::emptyStringT), tabPos(1),
	bTrayIcon(false), bAppMinimized(false), bIsPM(false), m_bDisableAutoComplete(false),
	QuickSearchBoxContainer(WC_COMBOBOX, this, QUICK_SEARCH_MAP), QuickSearchEditContainer(WC_EDIT ,this, QUICK_SEARCH_MAP),
	shutdownMng(0) //RSX++
{ 
		memzero(statusSizes, sizeof(statusSizes));
		anyMF = this;
		//RSX++
		ClientManager::getInstance()->openFrame = boost::bind(&MainFrame::openHub, this, _1);
		ClientManager::getInstance()->closeFrame = boost::bind(&MainFrame::closeHub, this, _1);
		//END
}

MainFrame::~MainFrame() {
	m_CmdBar.m_hImageList = NULL;

	images.Destroy();
	largeImages.Destroy();
	largeImagesHot.Destroy();

	WinUtil::uninit();
}

DWORD WINAPI MainFrame::stopper(void* p) {
	MainFrame* mf = (MainFrame*)p;
	HWND wnd, wnd2 = NULL;

	while( (wnd=::GetWindow(mf->m_hWndMDIClient, GW_CHILD)) != NULL) {
		if(wnd == wnd2)
			Sleep(100);
		else { 
			::PostMessage(wnd, WM_CLOSE, 0, 0);
			wnd2 = wnd;
		}
	}

	mf->PostMessage(WM_CLOSE);	
	return 0;
}

class ListMatcher : public Thread {
public:
	ListMatcher(StringList files_) : files(files_) {

	}
	int run() {
		for(StringIter i = files.begin(); i != files.end(); ++i) {
			UserPtr u = DirectoryListing::getUserFromFilename(*i);
			if(!u)
				continue;
				
			HintedUser user(u, Util::emptyString);
			DirectoryListing dl(user);
			try {
				dl.loadFile(*i);
				string tmp;
				tmp.resize(STRING(MATCHED_FILES).size() + 16);
				tmp.resize(snprintf(&tmp[0], tmp.size(), CSTRING(MATCHED_FILES), QueueManager::getInstance()->matchListing(dl)));
				LogManager::getInstance()->message(Util::toString(ClientManager::getInstance()->getNicks(user)) + ": " + tmp);
			} catch(const Exception&) {

			}
		}
		delete this;
		return 0;
	}
	StringList files;
};

LRESULT MainFrame::onMatchAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ListMatcher* matcher = new ListMatcher(File::findFiles(Util::getListPath(), "*.xml*"));
	try {
		matcher->start();
	} catch(const ThreadException&) {
		///@todo add error message
		delete matcher;
	}
	
	return 0;
}

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	TimerManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);
	LogManager::getInstance()->addListener(this);
	WebServerManager::getInstance()->addListener(this);
	
	if(BOOLSETTING(WEBSERVER)) {
		try {
			WebServerManager::getInstance()->Start();
		} catch(const Exception& e) {
			MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
	}
	//RSX++
	switch(RSXPP_SETTING(DEFAULT_PRIO)) {
		case 0:	RsxUtil::changeProcessPriority(RsxUtil::REALTIME);		break;
		case 1:	RsxUtil::changeProcessPriority(RsxUtil::HIGH);			break;
		case 2:	RsxUtil::changeProcessPriority(RsxUtil::ABOVE_NORMAL);	break;
		case 3:	RsxUtil::changeProcessPriority(RsxUtil::NORMAL);		break;
		case 4:	RsxUtil::changeProcessPriority(RsxUtil::BELOW_NORMAL);	break;
		case 5:	RsxUtil::changeProcessPriority(RsxUtil::IDLE);			break;
		default: break;
	}
	//--
	WinUtil::init(m_hWnd);

	trayMessage = RegisterWindowMessage(_T("TaskbarCreated"));

	TimerManager::getInstance()->start();

	// Set window name
	SetWindowText(COMPLETEVERSIONSTRING);
	UpdateManager::getInstance()->addListener(this); //RSX++

	// Load images
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);

	m_hMenu = WinUtil::mainMenu;

	hShutdownIcon = (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_SHUTDOWN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	// attach menu
	m_CmdBar.AttachMenu(m_hMenu);
	// load command bar images
	ResourceLoader::LoadImageList(IDP_TOOLBAR, images, 16, 16);
	m_CmdBar.m_hImageList = images;

	m_CmdBar.m_arrCommand.Add(ID_FILE_CONNECT);
	m_CmdBar.m_arrCommand.Add(ID_FILE_RECONNECT);
	m_CmdBar.m_arrCommand.Add(IDC_FOLLOW);
	m_CmdBar.m_arrCommand.Add(IDC_RECENTS);
	m_CmdBar.m_arrCommand.Add(IDC_FAVORITES);
	m_CmdBar.m_arrCommand.Add(IDC_FAVUSERS);
	m_CmdBar.m_arrCommand.Add(IDC_QUEUE);
	m_CmdBar.m_arrCommand.Add(IDC_FINISHED);
	m_CmdBar.m_arrCommand.Add(IDC_UPLOAD_QUEUE);
	m_CmdBar.m_arrCommand.Add(IDC_FINISHED_UL);
	m_CmdBar.m_arrCommand.Add(ID_FILE_SEARCH);
	m_CmdBar.m_arrCommand.Add(IDC_FILE_ADL_SEARCH);
	m_CmdBar.m_arrCommand.Add(IDC_SEARCH_SPY);
	m_CmdBar.m_arrCommand.Add(IDC_OPEN_FILE_LIST);
	m_CmdBar.m_arrCommand.Add(ID_FILE_SETTINGS);
	m_CmdBar.m_arrCommand.Add(IDC_NOTEPAD);
	m_CmdBar.m_arrCommand.Add(IDC_NET_STATS);
	m_CmdBar.m_arrCommand.Add(IDC_CDMDEBUG_WINDOW);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_CASCADE);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_TILE_HORZ);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_TILE_VERT);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_MINIMIZE_ALL);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_RESTORE_ALL);
	m_CmdBar.m_arrCommand.Add(ID_GET_TTH);	
	m_CmdBar.m_arrCommand.Add(IDC_UPDATE);	
	//RSX++
	m_CmdBar.m_arrCommand.Add(IDC_HASH_PROGRESS);
	m_CmdBar.m_arrCommand.Add(IDC_VIEW_PLUGINS_LIST);
	m_CmdBar.m_arrCommand.Add(IDC_OPEN_MY_LIST);
	m_CmdBar.m_arrCommand.Add(IDC_MATCH_ALL);
	m_CmdBar.m_arrCommand.Add(IDC_REFRESH_FILE_LIST);
	m_CmdBar.m_arrCommand.Add(IDC_OPEN_DOWNLOADS);
	m_CmdBar.m_arrCommand.Add(ID_FILE_QUICK_CONNECT);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_ARRANGE);
	m_CmdBar.m_arrCommand.Add(IDC_CLOSE_DISCONNECTED);
	m_CmdBar.m_arrCommand.Add(IDC_CLOSE_ALL_PM);
	m_CmdBar.m_arrCommand.Add(IDC_CLOSE_ALL_OFFLINE_PM);
	m_CmdBar.m_arrCommand.Add(IDC_CLOSE_ALL_DIR_LIST);
	m_CmdBar.m_arrCommand.Add(IDC_CLOSE_ALL_SEARCH_FRAME);
	m_CmdBar.m_arrCommand.Add(IDC_RECONNECT_DISCONNECTED);
	m_CmdBar.m_arrCommand.Add(ID_APP_ABOUT);
	m_CmdBar.m_arrCommand.Add(IDC_HELP_HOMEPAGE);
	m_CmdBar.m_arrCommand.Add(IDC_HELP_GEOIPFILE);
	m_CmdBar.m_arrCommand.Add(IDC_HELP_FAQ);
	m_CmdBar.m_arrCommand.Add(IDC_HELP_DISCUSS);
	m_CmdBar.m_arrCommand.Add(ID_APP_EXIT);
	//END

	// use Vista-styled menus on Vista/Win7
	if(WinUtil::getOsMajor() >= 6)
		m_CmdBar._AddVistaBitmapsFromImageList(0, m_CmdBar.m_arrCommand.GetSize());

	// remove old menu
	SetMenu(NULL);

	tbarcreated = false;
	HWND hWndToolBar = createToolbar();
	HWND hWndQuickSearchBar = createQuickSearchBar();

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	AddSimpleReBarBand(hWndQuickSearchBar, NULL, FALSE, 200, TRUE);
	CreateSimpleStatusBar();
	
	RECT toolRect = {0};
	::GetWindowRect(hWndToolBar, &toolRect);

	ctrlStatus.Attach(m_hWndStatusBar);
	ctrlStatus.SetSimple(FALSE);
	int w[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	ctrlStatus.SetParts(11, w);
	statusSizes[0] = WinUtil::getTextWidth(TSTRING(AWAY), ctrlStatus.m_hWnd); // for "AWAY" segment

	CToolInfo ti(TTF_SUBCLASS, ctrlStatus.m_hWnd);

	ctrlLastLines.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP /*| TTS_BALLOON*/, WS_EX_TOPMOST);
	ctrlLastLines.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	ctrlLastLines.AddTool(&ti);
	ctrlLastLines.SetDelayTime(TTDT_AUTOPOP, 15000);

	CreateMDIClient();
	m_CmdBar.SetMDIClient(m_hWndMDIClient);
	WinUtil::mdiClient = m_hWndMDIClient;

	ctrlTab.Create(m_hWnd, rcDefault);
	WinUtil::tabCtrl = &ctrlTab;
	tabPos = SETTING(TABS_POS);

	transferView.Create(m_hWnd);

	SetSplitterPanes(m_hWndMDIClient, transferView.m_hWnd);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_nProportionalPos = SETTING(TRANSFER_SPLIT_SIZE);
	UIAddToolBar(hWndToolBar);
	UIAddToolBar(hWndQuickSearchBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UISetCheck(ID_VIEW_TRANSFER_VIEW, 1);
	UISetCheck(ID_TOGGLE_QSEARCH, 1);
	
	// load bars settings
	WinUtil::loadReBarSettings(m_hWndToolBar);	

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	//RSX++ //Process Priority
	prioMenu.CreatePopupMenu();
	prioMenu.AppendMenu(MF_STRING, IDC_CHANGE_PRIO_ABOVE,		CTSTRING(MENU_PRIO_ABOVE));
	prioMenu.AppendMenu(MF_STRING, IDC_CHANGE_PRIO_NORMAL,		CTSTRING(MENU_PRIO_NORMAL));
	prioMenu.AppendMenu(MF_STRING, IDC_CHANGE_PRIO_BELOW,		CTSTRING(MENU_PRIO_BELOW));
	prioMenu.AppendMenu(MF_SEPARATOR);
	prioMenu.AppendMenu(MF_STRING, IDC_CHANGE_PRIO_IDLE,		CTSTRING(MENU_PRIO_IDLE));
	prioMenu.AppendMenu(MF_STRING, IDC_CHANGE_PRIO_HIGH,		CTSTRING(MENU_PRIO_HIGH));
	prioMenu.AppendMenu(MF_STRING, IDC_CHANGE_PRIO_REALTIME,	CTSTRING(MENU_PRIO_REALTIME));
	//END

	trayMenu.CreatePopupMenu();
	trayMenu.AppendMenu(MF_STRING, IDC_TRAY_SHOW, CTSTRING(MENU_SHOW));
	trayMenu.AppendMenu(MF_STRING, IDC_OPEN_DOWNLOADS, CTSTRING(MENU_OPEN_DOWNLOADS_DIR));
	trayMenu.AppendMenu(MF_SEPARATOR);
	trayMenu.AppendMenu(MF_STRING, IDC_REFRESH_FILE_LIST, CTSTRING(MENU_REFRESH_FILE_LIST));
	trayMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)prioMenu, CTSTRING(MENU_CHANGE_PRIO)); //RSX++
	trayMenu.AppendMenu(MF_SEPARATOR);
	trayMenu.AppendMenu(MF_STRING, ID_APP_ABOUT, CTSTRING(MENU_ABOUT));
	trayMenu.AppendMenu(MF_STRING, ID_APP_EXIT, CTSTRING(MENU_EXIT));

	if(BOOLSETTING(OPEN_PUBLIC)) PostMessage(WM_COMMAND, ID_FILE_CONNECT);
	if(BOOLSETTING(OPEN_FAVORITE_HUBS)) PostMessage(WM_COMMAND, IDC_FAVORITES);
	if(BOOLSETTING(OPEN_FAVORITE_USERS)) PostMessage(WM_COMMAND, IDC_FAVUSERS);
	if(BOOLSETTING(OPEN_RECENT_HUBS)) PostMessage(WM_COMMAND, IDC_RECENTS);	
	if(BOOLSETTING(OPEN_QUEUE)) PostMessage(WM_COMMAND, IDC_QUEUE);
	if(BOOLSETTING(OPEN_FINISHED_DOWNLOADS)) PostMessage(WM_COMMAND, IDC_FINISHED);
	if(BOOLSETTING(OPEN_UPLOAD_QUEUE)) PostMessage(WM_COMMAND, IDC_UPLOAD_QUEUE);
	if(BOOLSETTING(OPEN_FINISHED_UPLOADS)) PostMessage(WM_COMMAND, IDC_FINISHED_UL);
	if(BOOLSETTING(OPEN_SEARCH_SPY)) PostMessage(WM_COMMAND, IDC_SEARCH_SPY);
	if(BOOLSETTING(OPEN_NETWORK_STATISTICS)) PostMessage(WM_COMMAND, IDC_NET_STATS);
	if(BOOLSETTING(OPEN_NOTEPAD)) PostMessage(WM_COMMAND, IDC_NOTEPAD);

	if(!BOOLSETTING(SHOW_STATUSBAR)) PostMessage(WM_COMMAND, ID_VIEW_STATUS_BAR);
	if(!BOOLSETTING(SHOW_TOOLBAR)) PostMessage(WM_COMMAND, ID_VIEW_TOOLBAR);
	if(!BOOLSETTING(SHOW_TRANSFERVIEW))	PostMessage(WM_COMMAND, ID_VIEW_TRANSFER_VIEW);
	if(!BOOLSETTING(SHOW_QUICK_SEARCH))	PostMessage(WM_COMMAND, ID_TOGGLE_QSEARCH);

	if(!WinUtil::isShift())
		PostMessage(WM_SPEAKER, AUTO_CONNECT);

	PostMessage(WM_SPEAKER, PARSE_COMMAND_LINE);

	try {
		File::ensureDirectory(SETTING(LOG_DIRECTORY));
		//RSX++
		File::ensureDirectory(Util::getPath(Util::PATH_GLOBAL_CONFIG) + "Plugins" PATH_SEPARATOR_STR);
		File::ensureDirectory(Util::getPath(Util::PATH_EMOPACKS));
		//END
	} catch (const FileException) {	}

	PluginsManager::getInstance()->load(); //RSX++

	startSocket();
	
	normalicon.hIcon = (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	pmicon.hIcon = (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_TRAY_PM), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	updateTray( BOOLSETTING( MINIMIZE_TRAY ) );

	Util::setAway(BOOLSETTING(AWAY));

	ctrlToolbar.CheckButton(IDC_AWAY,BOOLSETTING(AWAY));
	ctrlToolbar.CheckButton(IDC_LIMITER,BOOLSETTING(THROTTLE_ENABLE));
	ctrlToolbar.CheckButton(IDC_DISABLE_SOUNDS, BOOLSETTING(SOUNDS_DISABLED));
#ifndef SVNBUILD
	if(SETTING(NICK).empty()) {
		PostMessage(WM_COMMAND, ID_FILE_SETTINGS);
	}
#endif
	UpdateManager::getInstance()->runUpdate(); //RSX++

	// We want to pass this one on to the splitter...hope it get's there...
	bHandled = FALSE;
	return 0;
}

HWND MainFrame::createQuickSearchBar() {
	ctrlQuickSearchBar.Create(m_hWnd, NULL, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 0, ATL_IDW_TOOLBAR);

	TBBUTTON tb[1];
	memzero(&tb, sizeof(tb));

	tb[0].iBitmap = 200;
	tb[0].fsStyle = TBSTYLE_SEP;

	ctrlQuickSearchBar.SetButtonStructSize();
	ctrlQuickSearchBar.AddButtons(1, tb);
	ctrlQuickSearchBar.AutoSize();

	CRect rect;
	ctrlQuickSearchBar.GetItemRect(0, &rect);
	rect.bottom += 100;
	rect.left += 2;

	QuickSearchBox.Create(ctrlQuickSearchBar.m_hWnd, rect , NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL , 0);
	updateQuickSearches();
	QuickSearchBox.SetWindowText(CTSTRING(QSEARCH_STR));
	QuickSearchBoxContainer.SubclassWindow(QuickSearchBox.m_hWnd);
	QuickSearchBox.SetExtendedUI();

	QuickSearchBox.SetFont(WinUtil::systemFont, FALSE);

	POINT pt;
	pt.x = 10;
	pt.y = 10;
	HWND hWnd = QuickSearchBox.ChildWindowFromPoint(pt);
	if(hWnd != NULL && !QuickSearchEdit.IsWindow() && hWnd != QuickSearchBox.m_hWnd) {
		QuickSearchEdit.Attach(hWnd);
		QuickSearchEditContainer.SubclassWindow(QuickSearchEdit.m_hWnd);
	}

	return ctrlQuickSearchBar.m_hWnd;
}

LRESULT MainFrame::onQuickSearchChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled){
	if(uMsg == WM_CHAR)
		if(wParam == VK_BACK)
			m_bDisableAutoComplete = true;
		else
			m_bDisableAutoComplete = false;

	switch(wParam) {
		case VK_DELETE:
			if(uMsg == WM_KEYDOWN) {
				m_bDisableAutoComplete = true;
			}
			bHandled = FALSE;
			break;
		case VK_RETURN:
			if( WinUtil::isShift() || WinUtil::isCtrl() || WinUtil::isAlt() ) {
				bHandled = FALSE;
			} else {
				if(uMsg == WM_KEYDOWN) {
					tstring s(QuickSearchEdit.GetWindowTextLength() + 1, _T('\0'));
					QuickSearchEdit.GetWindowText(&s[0], s.size());
					s.resize(s.size()-1);
					SearchFrame::openWindow(s);
					
					updateQuickSearches();
				}
			}
			break;
		default:
			bHandled = FALSE;
	}
	return 0;
}

LRESULT MainFrame::onQuickSearchColor(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HDC hDC = (HDC)wParam;
	::SetBkColor(hDC, WinUtil::bgColor);
	::SetTextColor(hDC, WinUtil::textColor);
	return (LRESULT)WinUtil::bgBrush;
}

LRESULT MainFrame::onQuickSearchEditChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	uint32_t nTextLen = 0, nMatchedTextLen = 0;
	HWND hWndCombo = QuickSearchBox.m_hWnd;
	_TCHAR *pStrMatchedText = NULL, *pEnteredText = NULL;
	DWORD dwStartSel = 0, dwEndSel = 0;

	// Get the text length from the combobox, then copy it into a newly allocated buffer.
	nTextLen = ::SendMessage(hWndCombo, WM_GETTEXTLENGTH, NULL, NULL);
	pEnteredText = new _TCHAR[nTextLen + 1];
	::SendMessage(hWndCombo, WM_GETTEXT, (WPARAM)nTextLen + 1, (LPARAM)pEnteredText);
	::SendMessage(hWndCombo, CB_GETEDITSEL, (WPARAM)&dwStartSel, (LPARAM)&dwEndSel);

	// Check to make sure autocompletion isn't disabled due to a backspace or delete
	// Also, the user must be typing at the end of the string, not somewhere in the middle.
	if (! m_bDisableAutoComplete && (dwStartSel == dwEndSel) && (dwStartSel == nTextLen)) {
		// Try and find a string that matches the typed substring.  If one is found,
		// set the text of the combobox to that string and set the selection to mask off
		// the end of the matched string.
		int nMatch = ::SendMessage(hWndCombo, CB_FINDSTRING, (WPARAM)-1, (LPARAM)pEnteredText);
		if (nMatch != CB_ERR) {
			nMatchedTextLen = ::SendMessage(hWndCombo, CB_GETLBTEXTLEN, (WPARAM)nMatch, 0);
			if (nMatchedTextLen != CB_ERR) {
				// Since the user may be typing in the same string, but with different case (e.g. "/port --> /PORT")
				// we copy whatever the user has already typed into the beginning of the matched string,
				// then copy the whole shebang into the combobox.  We then set the selection to mask off
				// the inferred portion.
				pStrMatchedText = new _TCHAR[nMatchedTextLen + 1];
				::SendMessage(hWndCombo, CB_GETLBTEXT, (WPARAM)nMatch, (LPARAM)pStrMatchedText);				
				memcpy((void*)pStrMatchedText, (void*)pEnteredText, nTextLen * sizeof(_TCHAR));
				::SendMessage(hWndCombo, WM_SETTEXT, 0, (WPARAM)pStrMatchedText);
				::SendMessage(hWndCombo, CB_SETEDITSEL, 0, MAKELPARAM(nTextLen, -1));
				delete[] pStrMatchedText;
			}
		}
	}

	delete[] pEnteredText;
	bHandled = TRUE;	

    return 0;
}

void MainFrame::updateQuickSearches() {
	QuickSearchBox.ResetContent();
	
	for(std::set<tstring>::const_iterator i = SearchFrame::getLastSearches().begin(); i != SearchFrame::getLastSearches().end(); ++i) {
		QuickSearchBox.InsertString(0, i->c_str());
	}	
	
	if(BOOLSETTING(CLEAR_SEARCH) && ::IsWindow(QuickSearchEdit.m_hWnd)) {
		QuickSearchBox.SetWindowText(CTSTRING(QSEARCH_STR));
	}
}

void MainFrame::startSocket() {
	//RSX++
	if(!NetworkConfiguration::getInstance()->restart() && SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_NAT_PMP) {
		SettingsManager::getInstance()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
		MessageBox(CTSTRING(UPNP_FAILED_TO_CREATE_MAPPINGS), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK | MB_ICONWARNING);
	}
	//END
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();
	DHT::getInstance()->stop();

	//if(ClientManager::getInstance()->isActive()) {
		try {
			ConnectionManager::getInstance()->listen();
		} catch(const Exception&) {
			MessageBox(CTSTRING(TCP_PORT_BUSY), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
		try {
			SearchManager::getInstance()->listen();
		} catch(const Exception&) {
			MessageBox(CTSTRING(TCP_PORT_BUSY), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
		
		try {
			DHT::getInstance()->start();
		} catch(const Exception&) {
			MessageBox(CTSTRING(TCP_PORT_BUSY), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
	//}

	startUPnP();
}

void MainFrame::startUPnP() {
	stopUPnP();

	if(SETTING(INCOMING_CONNECTIONS) != SettingsManager::INCOMING_FIREWALL_UPNP)
		return;

	pUPnP.reset(new UPnP_COM());

	if(!initUPnP()) {
		/// @todo try again with a different impl if we have one

		/// @todo the UPnP impl might return a meaningful error, show it to the user
		LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_CREATE_MAPPINGS));
		MessageBox(CTSTRING(UPNP_FAILED_TO_CREATE_MAPPINGS), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK | MB_ICONWARNING);

		stopUPnP();
	}
}

bool MainFrame::initUPnP() {
	if(!pUPnP->init())
		return false;

	uint16_t port = ConnectionManager::getInstance()->getPort();
	if(port != 0 && !pUPnP->open(port, UPnP::PROTOCOL_TCP, APPNAME " Transfer Port (" + Util::toString(port) + " TCP)"))
		return false;

	port = ConnectionManager::getInstance()->getSecurePort();
	if(port != 0 && !pUPnP->open(port, UPnP::PROTOCOL_TCP, APPNAME " Encrypted Transfer Port (" + Util::toString(port) + " TCP)"))
		return false;

	port = SearchManager::getInstance()->getPort();
	if(port != 0 && !pUPnP->open(port, UPnP::PROTOCOL_UDP, APPNAME " Search Port (" + Util::toString(port) + " TCP)"))
		return false;

	port = DHT::getInstance()->getPort();
	if(port != 0 && !pUPnP->open(port, UPnP::PROTOCOL_UDP, APPNAME " DHT Port (" + Util::toString(port) + " UDP)"))
		return false;

	if(!BOOLSETTING(NO_IP_OVERRIDE)) {
		// now lets configure the external IP (connect to me) address
		string ExternalIP = pUPnP->getExternalIP();
		if(!ExternalIP.empty()) {
			// woohoo, we got the external IP from the UPnP framework
			SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, ExternalIP);
		} else {
			//:-( Looks like we have to rely on the user setting the external IP manually
			// no need to do cleanup here because the mappings work
			LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_GET_EXTERNAL_IP));
			MessageBox(CTSTRING(UPNP_FAILED_TO_GET_EXTERNAL_IP), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK | MB_ICONWARNING);
		}
	}

	return true;
}

void MainFrame::stopUPnP() {
	if(pUPnP.get()) {
		if(!pUPnP->close()) {
			LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_REMOVE_MAPPINGS));
		}
		pUPnP.reset();
	}
}

HWND MainFrame::createToolbar() {
	if(!tbarcreated) {
		if(SETTING(TOOLBARIMAGE) == "")
			ResourceLoader::LoadImageList(IDP_TOOLBAR20, largeImages, 20, 20);
		else
			ResourceLoader::LoadImageList(Text::toT(SETTING(TOOLBARIMAGE)).c_str(), largeImages, 20, 20);

		if(SETTING(TOOLBARHOTIMAGE) == "")
			ResourceLoader::LoadImageList(IDP_TOOLBAR20_HOT, largeImagesHot, 20, 20);
		else
			ResourceLoader::LoadImageList(Text::toT(SETTING(TOOLBARHOTIMAGE)).c_str(), largeImagesHot, 20, 20);

		ctrlToolbar.Create(m_hWnd, NULL, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | TBSTYLE_LIST, 0, ATL_IDW_TOOLBAR);
		ctrlToolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		ctrlToolbar.SetImageList(largeImages);
		ctrlToolbar.SetHotImageList(largeImagesHot);
		tbarcreated = true;
	}

	while(ctrlToolbar.GetButtonCount()>0)
		ctrlToolbar.DeleteButton(0);

	ctrlToolbar.SetButtonStructSize();
	StringTokenizer<string> t(SETTING(TOOLBAR), ',');
	StringList& l = t.getTokens();
	
	int buttonsCount = sizeof(ToolbarButtons) / sizeof(ToolbarButtons[0]);
	for(StringList::const_iterator k = l.begin(); k != l.end(); ++k) {
		int i = Util::toInt(*k);		
		
		TBBUTTON nTB;
		memzero(&nTB, sizeof(TBBUTTON));

		if(i == -1) {
			nTB.fsStyle = TBSTYLE_SEP;			
		} else if(i >= 0 && i < buttonsCount) {
			nTB.iBitmap = ToolbarButtons[i].image;
			nTB.idCommand = ToolbarButtons[i].id;
			nTB.fsState = TBSTATE_ENABLED;
			nTB.fsStyle = TBSTYLE_AUTOSIZE | ((ToolbarButtons[i].check == true)? TBSTYLE_CHECK : TBSTYLE_BUTTON);
			nTB.iString = ctrlToolbar.AddStrings(CTSTRING_I((ResourceManager::Strings)ToolbarButtons[i].tooltip));
		} else {
			continue;
		}

		ctrlToolbar.AddButtons(1, &nTB);
	}	

	ctrlToolbar.AutoSize();

	return ctrlToolbar.m_hWnd;
}



LRESULT MainFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		
	if(wParam == DOWNLOAD_LISTING) {
		auto_ptr<DirectoryListInfo> i(reinterpret_cast<DirectoryListInfo*>(lParam));
		DirectoryListingFrame::openWindow(i->file, i->dir, i->user, i->speed);
	} else if(wParam == BROWSE_LISTING) {
		auto_ptr<DirectoryBrowseInfo> i(reinterpret_cast<DirectoryBrowseInfo*>(lParam));
		DirectoryListingFrame::openWindow(i->user, i->text, 0);
	} else if(wParam == VIEW_FILE_AND_DELETE) {
		auto_ptr<tstring> file(reinterpret_cast<tstring*>(lParam));
		TextFrame::openWindow(*file);
		File::deleteFile(Text::fromT(*file));
	} else if(wParam == STATS) {
		auto_ptr<TStringList> pstr(reinterpret_cast<TStringList*>(lParam));
		const TStringList& str = *pstr;
		if(ctrlStatus.IsWindow()) {
			bool u = false;
			ctrlStatus.SetText(1, str[0].c_str());
			for(int i = 1; i < 9; i++) {
				int w = WinUtil::getTextWidth(str[i], ctrlStatus.m_hWnd);
				
				if(statusSizes[i] < w) {
					statusSizes[i] = w;
					u = true;
				}
				ctrlStatus.SetText(i+1, str[i].c_str());
			}

			if(u)
				UpdateLayout(TRUE);
			//RSX++
			if(shutdownMng) {
				bool bClose = false, bShutdown = false;
				uint64_t shTimeLeft = shutdownMng->getTimeLeft(bClose, bShutdown);
				if(bShutdown) {
					if(bClose) {
						oldshutdown = true; // nasty...
						PostMessage(WM_CLOSE);
					} else {
						shutdownMng->shutdown();
					}
					setShutDown(false);
				} else {
					ctrlStatus.SetIcon(10, hShutdownIcon);
					if(shTimeLeft >= 0)
						ctrlStatus.SetText(10, Util::formatSeconds(shTimeLeft, shTimeLeft < 3600).c_str());
					else
						ctrlStatus.SetText(10, NULL);
				}
			} else {
				ctrlStatus.SetIcon(10, NULL);
				ctrlStatus.SetText(10, NULL);
			}
			//END
		}
	} else if(wParam == AUTO_CONNECT) {
		autoConnect(FavoriteManager::getInstance()->getFavoriteHubs());
	} else if(wParam == PARSE_COMMAND_LINE) {
		parseCommandLine(GetCommandLine());
	} else if(wParam == STATUS_MESSAGE) {
		tstring* msg = (tstring*)lParam;
		if(ctrlStatus.IsWindow()) {
			tstring line = Text::toT("[" + Util::getShortTimeString() + "] ") + *msg;

			ctrlStatus.SetText(0, line.c_str());
			while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
				lastLinesList.erase(lastLinesList.begin());
			if (line.find(_T('\r')) == tstring::npos) {
				lastLinesList.push_back(line);
			} else {
				lastLinesList.push_back(line.substr(0, line.find(_T('\r'))));
			}
		}
		delete msg;
	} else if(wParam == SHOW_POPUP) {
		Popup* msg = (Popup*)lParam;
		PopupManager::getInstance()->Show(msg->Message, msg->Title, msg->Icon);
		delete msg;
	} else if(wParam == WM_CLOSE) {
		PopupManager::getInstance()->Remove((int)lParam);
	} else if(wParam == REMOVE_POPUP){
		PopupManager::getInstance()->AutoRemove();
	} else if(wParam == SET_PM_TRAY_ICON) {
		if(bIsPM == false && (!WinUtil::isAppActive || bAppMinimized) && bTrayIcon == true) {
			NOTIFYICONDATA nid;
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = m_hWnd;
			nid.uID = 0;
			nid.uFlags = NIF_ICON;
			nid.hIcon = pmicon.hIcon;
			::Shell_NotifyIcon(NIM_MODIFY, &nid);
			bIsPM = true;
		}
    }

	return 0;
}

void MainFrame::parseCommandLine(const tstring& cmdLine)
{
	string::size_type i = 0;
	string::size_type j;

	if( (j = cmdLine.find(_T("dchub://"), i)) != string::npos) {
		WinUtil::parseDchubUrl(cmdLine.substr(j), false);
	}
	if( (j = cmdLine.find(_T("nmdcs://"), i)) != string::npos) {
		WinUtil::parseDchubUrl(cmdLine.substr(j), true);
	}	
	if( (j = cmdLine.find(_T("adc://"), i)) != string::npos) {
		WinUtil::parseADChubUrl(cmdLine.substr(j), false);
	}
	if( (j = cmdLine.find(_T("adcs://"), i)) != string::npos) {
		WinUtil::parseADChubUrl(cmdLine.substr(j), true);
	}
	if( (j = cmdLine.find(_T("magnet:?"), i)) != string::npos) {
		WinUtil::parseMagnetUri(cmdLine.substr(j));
	}
}

LRESULT MainFrame::onCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	tstring cmdLine = (LPCTSTR) (((COPYDATASTRUCT *)lParam)->lpData);
	parseCommandLine(Text::toT(WinUtil::getAppName() + " ") + cmdLine);
	return true;
}

LRESULT MainFrame::onHashProgress(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HashProgressDlg(false).DoModal(m_hWnd);
	return 0;
}

LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	AboutDlg dlg;
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT MainFrame::onOpenWindows(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch(wID) {
		case ID_FILE_SEARCH: SearchFrame::openWindow(); break;
		case ID_FILE_CONNECT: PublicHubsFrame::openWindow(); break;
		case IDC_FAVORITES: FavoriteHubsFrame::openWindow(); break;
		case IDC_FAVUSERS: UsersFrame::openWindow(); break;
		case IDC_NOTEPAD: NotepadFrame::openWindow(); break;
		case IDC_QUEUE: QueueFrame::openWindow(); break;
		case IDC_SEARCH_SPY: SpyFrame::openWindow(); break;
		case IDC_FILE_ADL_SEARCH: ADLSearchFrame::openWindow(); break;
		case IDC_NET_STATS: StatsFrame::openWindow(); break; 
		case IDC_FINISHED: FinishedFrame::openWindow(); break;
		case IDC_FINISHED_UL: FinishedULFrame::openWindow(); break;
		case IDC_UPLOAD_QUEUE: UploadQueueFrame::openWindow(); break;
		case IDC_CDMDEBUG_WINDOW: CDMDebugFrame::openWindow(); break;
		case IDC_RECENTS: RecentHubsFrame::openWindow(); break;
		default: dcassert(0); break;
	}
	return 0;
}

LRESULT MainFrame::OnFileSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PropertiesDlg dlg(m_hWnd, SettingsManager::getInstance());

	unsigned short lastTCP = static_cast<unsigned short>(SETTING(TCP_PORT));
	unsigned short lastUDP = static_cast<unsigned short>(SETTING(UDP_PORT));
	unsigned short lastTLS = static_cast<unsigned short>(SETTING(TLS_PORT));
	unsigned short lastDHT = static_cast<unsigned short>(SETTING(DHT_PORT));
	
	int lastConn = SETTING(INCOMING_CONNECTIONS);
	bool lastDHTConn = BOOLSETTING(USE_DHT);

	bool lastSortFavUsersFirst = BOOLSETTING(SORT_FAVUSERS_FIRST);

	if(dlg.DoModal(m_hWnd) == IDOK) 
	{
		SettingsManager::getInstance()->save();
		if(missedAutoConnect && !SETTING(NICK).empty()) {
			PostMessage(WM_SPEAKER, AUTO_CONNECT);
		}
		if(	SETTING(INCOMING_CONNECTIONS) != lastConn || SETTING(TCP_PORT) != lastTCP || SETTING(UDP_PORT) != lastUDP || SETTING(TLS_PORT) != lastTLS ||
			SETTING(DHT_PORT) != lastDHT || BOOLSETTING(USE_DHT) != lastDHTConn)
		{
			startSocket();
		} else if(SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP && !pUPnP.get()) {
			// previous UPnP mappings had failed; try again
			startUPnP();
		} 

		if(BOOLSETTING(SORT_FAVUSERS_FIRST) != lastSortFavUsersFirst)
			HubFrame::resortUsers();

		if(BOOLSETTING(URL_HANDLER)) {
			WinUtil::registerDchubHandler();
			WinUtil::registerADChubHandler();
			WinUtil::urlDcADCRegistered = true;
		} else if(WinUtil::urlDcADCRegistered) {
			WinUtil::unRegisterDchubHandler();
			WinUtil::unRegisterADChubHandler();
			WinUtil::urlDcADCRegistered = false;
		}
		if(BOOLSETTING(MAGNET_REGISTER)) {
			WinUtil::registerMagnetHandler();
			WinUtil::urlMagnetRegistered = true; 
		} else if(WinUtil::urlMagnetRegistered) {
			WinUtil::unRegisterMagnetHandler();
			WinUtil::urlMagnetRegistered = false;
		}

		if(BOOLSETTING(THROTTLE_ENABLE)) 
			ctrlToolbar.CheckButton(IDC_LIMITER, true);
		else 
			ctrlToolbar.CheckButton(IDC_LIMITER, false);

		if(Util::getAway()) 
			ctrlToolbar.CheckButton(IDC_AWAY, true);
		else 
			ctrlToolbar.CheckButton(IDC_AWAY, false);

		if(getShutDown()) 
			ctrlToolbar.CheckButton(IDC_SHUTDOWN, true);
		else 
			ctrlToolbar.CheckButton(IDC_SHUTDOWN, false);
	
		updateTray(BOOLSETTING(MINIMIZE_TRAY));
		if(tabPos != SETTING(TABS_POS)) {
			tabPos = SETTING(TABS_POS);
			ctrlTab.updateTabs();
			UpdateLayout();
		}

		ClientManager::getInstance()->infoUpdated();
	}
	return 0;
}
//RSX++
void MainFrame::on(UpdateManagerListener::VersionUpdated, const VersionInfo::Client& client, const VersionInfo::Profiles& profiles) throw() {
#ifndef SVNBUILD
	if(client.veryOldVersion >= SVN_REVISION) {
		string msg = "Your version of RSX++ contains a serious bug that affects all users of the DC network or the security of your computer.";
		string url = client.url.empty() ? RSXPP_SITE : client.url;
		if(MessageBox(Text::toT(msg + "\r\nPlease get a new one at " + url).c_str(), _T("Old RSX++ version"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			WinUtil::openLink(Text::toT(url));
		oldshutdown = true;
		PostMessage(WM_CLOSE);
		return;
	}

	for(vector<double>::const_iterator i = client.badVersions.begin(); i != client.badVersions.end(); ++i) {
		if(*i == SVN_REVISION) {
			string msg = "Your version of RSX++ contains a serious bug that affects all users of the DC network or the security of your computer.";
			string url = client.url.empty() ? RSXPP_SITE : client.url;
			if(MessageBox(Text::toT(msg + "\r\nPlease get a new one at " + url).c_str(), _T("Bad RSX++ version"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				WinUtil::openLink(Text::toT(url));
			oldshutdown = true;
			PostMessage(WM_CLOSE);
			return;
		}
	}
#endif
	if(::IsWindow(GetDlgItem(IDD_UPDATE)))
		return;

	UpdateDialog dlg(client, profiles);
	if(dlg.isAnyNewVersion(true)) {
		dlg.DoModal();
	}
}

LRESULT MainFrame::onWebServerSocket(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	WebServerManager::getInstance()->getServerSocket().incoming();
	return 0;
}

LRESULT MainFrame::onGetToolTip(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMTTDISPINFO pDispInfo = (LPNMTTDISPINFO)pnmh;
	pDispInfo->szText[0] = 0;

	if(!((idCtrl != 0) && !(pDispInfo->uFlags & TTF_IDISHWND))) {
		// if we're really in the status bar, this should be detected intelligently
		lastLines.clear();
		for(TStringIter i = lastLinesList.begin(); i != lastLinesList.end(); ++i) {
			lastLines += *i;
			lastLines += _T("\r\n");
		}
		if(lastLines.size() > 2) {
			lastLines.erase(lastLines.size() - 2);
		}
		pDispInfo->lpszText = const_cast<TCHAR*>(lastLines.c_str());
	}
	return 0;
}

void MainFrame::autoConnect(const FavoriteHubEntry::List& fl) {
	missedAutoConnect = false;
	for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
		FavoriteHubEntry* entry = *i;
		if(entry->getConnect()) {
 			if(!entry->getNick().empty() || !SETTING(NICK).empty()) {
				RecentHubEntry r;
				r.setName(entry->getName());
				r.setDescription(entry->getDescription());
				r.setUsers("*");
				r.setShared("*");
				r.setServer(entry->getServer());
				FavoriteManager::getInstance()->addRecent(r);
				HubFrame::openWindow(Text::toT(entry->getServer())
					, entry->getChatUserSplit(), entry->getUserListState());
 			} else
 				missedAutoConnect = true;
 		}				
	}
}

void MainFrame::updateTray(bool add /* = true */) {
	if(add) {
		if(bTrayIcon == false) {
			NOTIFYICONDATA nid;
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = m_hWnd;
			nid.uID = 0;
			nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
			nid.uCallbackMessage = WM_APP + 242;
			nid.hIcon = normalicon.hIcon;
			_tcsncpy(nid.szTip, _T(APPNAME), 64);
			nid.szTip[63] = '\0';
			lastMove = GET_TICK() - 1000;
			::Shell_NotifyIcon(NIM_ADD, &nid);
			bTrayIcon = true;
		}
	} else {
		if(bTrayIcon) {
			NOTIFYICONDATA nid;
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = m_hWnd;
			nid.uID = 0;
			nid.uFlags = 0;
			::Shell_NotifyIcon(NIM_DELETE, &nid);
			ShowWindow(SW_SHOW);
			bTrayIcon = false;
		}
	}
}

LRESULT MainFrame::onSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if(wParam == SIZE_MINIMIZED) {
		SetProcessWorkingSetSize(GetCurrentProcess(), 0xffffffff, 0xffffffff);
		if(BOOLSETTING(AUTO_AWAY)) {
			if(bAppMinimized == false)
			if(Util::getAway() == true) {
				awaybyminimize = false;
			} else {
				awaybyminimize = true;
				Util::setAway(true);
				setAwayButton(true);
				ClientManager::getInstance()->infoUpdated();
			}
		}

		bAppMinimized = true;
		if(BOOLSETTING(MINIMIZE_TRAY) != WinUtil::isShift()) {
			updateTray(true);
			ShowWindow(SW_HIDE);
			//RsxUtil::changeProcessPriority(1);
		} else {
			updateTray(false);
		}
		maximized = IsZoomed() > 0;
	} else if( (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) ) {
		//RsxUtil::changeProcessPriority(2);
		if(BOOLSETTING(AUTO_AWAY)) {
			if(awaybyminimize == true) {
				awaybyminimize = false;
				Util::setAway(false);
				setAwayButton(false);
				ClientManager::getInstance()->infoUpdated();
			}
		}
		if(bIsPM && bTrayIcon == true) {
			NOTIFYICONDATA nid;
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = m_hWnd;
			nid.uID = 0;
			nid.uFlags = NIF_ICON;
			nid.hIcon = normalicon.hIcon;
			::Shell_NotifyIcon(NIM_MODIFY, &nid);
			bIsPM = false;
		}
		bAppMinimized = false;
	}

	bHandled = FALSE;
	return 0;
}

LRESULT MainFrame::onEndSession(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);
	
	CRect rc;
	GetWindowRect(rc);
	
	if(wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWNORMAL) {
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, rc.left);
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, rc.top);
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, rc.Width());
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, rc.Height());
	}
	if(wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_MAXIMIZE)
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)wp.showCmd);
	
	QueueManager::getInstance()->saveQueue();
	SettingsManager::getInstance()->save();
	
	return 0;
}

LRESULT MainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closing) {
		if( oldshutdown ||(!BOOLSETTING(CONFIRM_EXIT)) || (MessageBox(CTSTRING(REALLY_EXIT), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) ) {
			updateTray(false);
			string tmp1;
			string tmp2;

			WINDOWPLACEMENT wp;
			wp.length = sizeof(wp);
			GetWindowPlacement(&wp);

			CRect rc;
			GetWindowRect(rc);
			if(BOOLSETTING(SHOW_TRANSFERVIEW)) {
				SettingsManager::getInstance()->set(SettingsManager::TRANSFER_SPLIT_SIZE, m_nProportionalPos);
			}
			if(wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWNORMAL) {
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, rc.left);
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, rc.top);
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, rc.Width());
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, rc.Height());
			}
			if(wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_MAXIMIZE)
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)wp.showCmd);

			// save bars settings
			WinUtil::saveReBarSettings(m_hWndToolBar);
			
			ShowWindow(SW_HIDE);
			transferView.prepareClose();
			
			WebServerManager::getInstance()->removeListener(this);
			SearchManager::getInstance()->disconnect();
			ConnectionManager::getInstance()->disconnect();
			listQueue.shutdown();

			stopUPnP();

			DWORD id;
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
			closing = true;
		}
		bHandled = TRUE;
	} else {
		// This should end immediately, as it only should be the stopper that sends another WM_CLOSE
		WaitForSingleObject(stopperThread, 60*1000);
		CloseHandle(stopperThread);
		stopperThread = NULL;
		DestroyIcon(normalicon.hIcon);
		DestroyIcon(hShutdownIcon); 	
		DestroyIcon(pmicon.hIcon);
		bHandled = FALSE;
	}

	return 0;
}

LRESULT MainFrame::onLink(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	tstring site;
	bool isFile = false;
	switch(wID) {
		case IDC_HELP_HOMEPAGE: site = _T(RSXPP_SITE); break;
		case IDC_HELP_GEOIPFILE: site = _T("http://www.maxmind.com/download/geoip/database/GeoIPCountryCSV.zip"); break;
		case IDC_HELP_TRANSLATIONS: site = _T(RSXPP_SITE); break;
		case IDC_HELP_FAQ: site = _T(RSXPP_TRAC); break;
		case IDC_HELP_DISCUSS: site = _T(RSXPP_FORUM); break;
		case IDC_HELP_DONATE: site = _T(RSXPP_DONATE); break;
		default: dcassert(0);
	}

	if(isFile)
		WinUtil::openFile(site);
	else
		WinUtil::openLink(site);

	return 0;
}

int MainFrame::run() {
	tstring file;
	if(WinUtil::browseFile(file, m_hWnd, false, lastTTHdir) == IDOK) {
		WinUtil::mainMenu.EnableMenuItem(ID_GET_TTH, MF_GRAYED);
		Thread::setThreadPriority(Thread::LOW);
		lastTTHdir = Util::getFilePath(file);

		string TTH;
		TTH.resize(192*8/(5*8)+1);

		boost::scoped_array<char> buf(new char[512 * 1024]);

		try {
			File f(Text::fromT(file), File::READ, File::OPEN);
			TigerTree tth(TigerTree::calcBlockSize(f.getSize(), 1));

			if(f.getSize() > 0) {
				size_t n = 512*1024;
				while( (n = f.read(&buf[0], n)) > 0) {
					tth.update(&buf[0], n);
					n = 512*1024;
				}
			} else {
				tth.update("", 0);
			}
			tth.finalize();

			strcpy(&TTH[0], tth.getRoot().toBase32().c_str());

			CInputBox ibox(m_hWnd);

			string magnetlink = "magnet:?xt=urn:tree:tiger:"+ TTH +"&xl="+Util::toString(f.getSize())+"&dn="+Util::encodeURI(Text::fromT(Util::getFileName(file)));
			f.close();
			
			ibox.DoModal(_T("Tiger Tree Hash"), file.c_str(), Text::toT(TTH).c_str(), Text::toT(magnetlink).c_str());
		} catch(...) { }
		Thread::setThreadPriority(Thread::NORMAL);
		WinUtil::mainMenu.EnableMenuItem(ID_GET_TTH, MF_ENABLED);
	}
	return 0;
}

LRESULT MainFrame::onGetTTH(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	Thread::start();
	return 0;
}

void MainFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */)
{
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow() && ctrlLastLines.IsWindow()) {
		CRect sr;
		int w[11];
		ctrlStatus.GetClientRect(sr);
		w[10] = sr.right - 20;
		w[9] = w[10] - 60;
#define setw(x) w[x] = max(w[x+1] - statusSizes[x], 0)
		setw(8); setw(7); setw(6); setw(5); setw(4); setw(3); setw(2); setw(1); setw(0);

		ctrlStatus.SetParts(11, w);
		ctrlLastLines.SetMaxTipWidth(w[0]);
	}
	CRect rc = rect;
	CRect rc2 = rect;

	if(tabPos == 0) {
		rc.bottom = rc.top + ctrlTab.getHeight();
		rc2.top = rc.bottom;
	} else if(tabPos == 1) {
		rc.top = rc.bottom - ctrlTab.getHeight();
		rc2.bottom = rc.top;
	} else if(tabPos == 2) {
		rc.left = 0;
		rc.right = 150;
		rc2.left = rc.right;
	} else if(tabPos == 3) {
		rc.left = rc.right - 150;
		rc2.right = rc.left;
	}
	
	if(ctrlTab.IsWindow())
		ctrlTab.MoveWindow(rc);

	SetSplitterRect(rc2);
}

static const TCHAR types[] = _T("File Lists\0*.xml.bz2\0All Files\0*.*\0");

LRESULT MainFrame::onOpenFileList(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring file;
	
	if(wID == IDC_OPEN_MY_LIST){
		if(!ShareManager::getInstance()->getOwnListFile().empty()){
			DirectoryListingFrame::openWindow(Text::toT(ShareManager::getInstance()->getOwnListFile()), Text::toT(Util::emptyString), HintedUser(ClientManager::getInstance()->getMe(), Util::emptyString), 0);
		}
		return 0;
	}

	if(WinUtil::browseFile(file, m_hWnd, false, Text::toT(Util::getListPath()), types)) {
		UserPtr u = DirectoryListing::getUserFromFilename(Text::fromT(file));
		if(u) {
			DirectoryListingFrame::openWindow(file, Text::toT(Util::emptyString), HintedUser(u, Util::emptyString), 0);
		} else {
			MessageBox(CTSTRING(INVALID_LISTNAME), _T(APPNAME) _T(" ") _T(VERSIONSTRING));
		}
	}
	return 0;
}

LRESULT MainFrame::onRefreshFileList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true);
	return 0;
}

LRESULT MainFrame::onTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	if (lParam == WM_LBUTTONUP) {
		if(bAppMinimized) {
			ShowWindow(SW_SHOW);
			ShowWindow(maximized ? SW_MAXIMIZE : SW_RESTORE);
		} else {
			SetForegroundWindow(m_hWnd);
		}
	} else if(lParam == WM_MOUSEMOVE && ((lastMove + 1000) < GET_TICK()) ) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_TIP;
		tstring s_tip = _T(APPNAME) _T(" v") _T(VERSIONSTRING)	_T("\r\n\r\nD: ") + 
			Util::formatBytesW(DownloadManager::getInstance()->getRunningAverage()) + _T("/s (") + 
			Util::toStringW(DownloadManager::getInstance()->getDownloadCount()) + _T(")\r\nU: ") +
			Util::formatBytesW(UploadManager::getInstance()->getRunningAverage()) + _T("/s (") + 
			Util::toStringW(UploadManager::getInstance()->getUploadCount()) + _T(")") +
			_T("\r\nUptime: ") + Util::formatSeconds(time(NULL) - Util::getStartTime());
		_tcsncpy(nid.szTip, s_tip.c_str(), 80);
		
		::Shell_NotifyIcon(NIM_MODIFY, &nid);
		lastMove = GET_TICK();
	} else if (lParam == WM_RBUTTONUP) {
		CPoint pt(GetMessagePos());		
		SetForegroundWindow(m_hWnd);
		setDefPrioMenu(); //RSX++
		trayMenu.TrackPopupMenu(TPM_RIGHTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);		
		PostMessage(WM_NULL, 0, 0);
	}
	return 0;
}

void MainFrame::ShowBalloonTip(tstring szMsg, tstring szTitle, DWORD dwInfoFlags) {
	Popup* p = new Popup;
	p->Title = szTitle;
	p->Message = szMsg;
	p->Icon = dwInfoFlags;

	PostMessage(WM_SPEAKER, SHOW_POPUP, (LPARAM)p);
}

LRESULT MainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	SettingsManager::getInstance()->set(SettingsManager::SHOW_TOOLBAR, bVisible);
	return 0;
}

LRESULT MainFrame::OnViewQuickSearchBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 2);	// toolbar is 3rd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_TOGGLE_QSEARCH, bVisible);
	UpdateLayout();
	SettingsManager::getInstance()->set(SettingsManager::SHOW_QUICK_SEARCH, bVisible);
	return 0;
}
//RSX++
LRESULT MainFrame::onViewPluginsList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PluginsListDlg dlg;
	dlg.DoModal();
	return 0;
}
//END
LRESULT MainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	SettingsManager::getInstance()->set(SettingsManager::SHOW_STATUSBAR, bVisible);
	return 0;
}

LRESULT MainFrame::OnViewTransferView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !transferView.IsWindowVisible();
	if(!bVisible) {	
		if(GetSinglePaneMode() == SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_TOP);
	} else { 
		if(GetSinglePaneMode() != SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_NONE);
	}
	UISetCheck(ID_VIEW_TRANSFER_VIEW, bVisible);
	UpdateLayout();
	SettingsManager::getInstance()->set(SettingsManager::SHOW_TRANSFERVIEW, bVisible);
	return 0;
}

LRESULT MainFrame::onCloseWindows(WORD , WORD wID, HWND , BOOL& ) {
	switch(wID) {
	case IDC_CLOSE_DISCONNECTED:		HubFrame::closeDisconnected();		break;
	case IDC_CLOSE_ALL_PM:				PrivateFrame::closeAll();			break;
	case IDC_CLOSE_ALL_OFFLINE_PM:		PrivateFrame::closeAllOffline();	break;
	case IDC_CLOSE_ALL_DIR_LIST:		DirectoryListingFrame::closeAll();	break;
	case IDC_CLOSE_ALL_SEARCH_FRAME:	SearchFrame::closeAll();			break;
	case IDC_RECONNECT_DISCONNECTED:	HubFrame::reconnectDisconnected();	break;
	}
	return 0;
}

LRESULT MainFrame::onLimiter(WORD , WORD , HWND, BOOL& ) {
	SettingsManager::getInstance()->set(SettingsManager::THROTTLE_ENABLE, !BOOLSETTING(THROTTLE_ENABLE));
	ClientManager::getInstance()->infoUpdated();
	return 0;
}

LRESULT MainFrame::onQuickConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	LineDlg dlg;
	dlg.description = TSTRING(HUB_ADDRESS);
	dlg.title = TSTRING(QUICK_CONNECT);
	if(dlg.DoModal(m_hWnd) == IDOK){
		if(SETTING(NICK).empty())
			return 0;

		tstring tmp = dlg.line;
		// Strip out all the spaces
		string::size_type i;
		while((i = tmp.find(' ')) != string::npos)
			tmp.erase(i, 1);

		RecentHubEntry r;
		r.setName("*");
		r.setDescription("*");
		r.setUsers("*");
		r.setShared("*");
		r.setServer(Text::fromT(tmp));
		FavoriteManager::getInstance()->addRecent(r);
		HubFrame::openWindow(tmp);
	}
	return 0;
}

void MainFrame::on(TimerManagerListener::Second, uint64_t aTick) throw() {
	int64_t diff = (int64_t)((lastUpdate == 0) ? aTick - 1000 : aTick - lastUpdate);
	int64_t updiff = Socket::getTotalUp() - lastUp;
	int64_t downdiff = Socket::getTotalDown() - lastDown;

	TStringList* str = new TStringList();
	str->push_back(Util::getAway() ? TSTRING(AWAY) : _T(""));
	
	dht::DHT* dhtManager = DHT::getInstance();
	str->push_back(_T("DHT: ") + (dhtManager->isConnected() ? Util::toStringW(dhtManager->getNodesCount()) : _T("-")));
	str->push_back(TSTRING(SHARED) + _T(": ") + Util::formatBytesW(ShareManager::getInstance()->getSharedSize()));
	str->push_back(_T("H: ") + Text::toT(Client::getCounts()));
	str->push_back(TSTRING(SLOTS) + _T(": ") + Util::toStringW(UploadManager::getInstance()->getFreeSlots()) + _T('/') + Util::toStringW(UploadManager::getInstance()->getSlots()) + _T(" (") + Util::toStringW(UploadManager::getInstance()->getFreeExtraSlots()) + _T('/') + Util::toStringW(SETTING(EXTRA_SLOTS)) + _T(")"));

	str->push_back(boost::str(boost::wformat(_T("D: %1%/s   T: %2%")) % Util::formatBytesW(downdiff*1000I64/diff) % Util::formatBytesW(Socket::getTotalDown())));
	str->push_back(boost::str(boost::wformat(_T("U: %1%/s   T: %2%")) % Util::formatBytesW(updiff*1000I64/diff) % Util::formatBytesW(Socket::getTotalUp())));

	if(BOOLSETTING(THROTTLE_ENABLE)) {
		str->push_back(boost::str(boost::wformat(_T("D Lim: %1%/s")) % Util::formatBytesW(SETTING(MAX_DOWNLOAD_SPEED_LIMIT) * 1024)));
		str->push_back(boost::str(boost::wformat(_T("U Lim: %1%/s")) % Util::formatBytesW(SETTING(MAX_UPLOAD_SPEED_LIMIT) * 1024)));
	} else {
		str->push_back(_T("D Lim: -"));
		str->push_back(_T("U Lim: -"));
	}

	PostMessage(WM_SPEAKER, STATS, (LPARAM)str);

	SettingsManager::getInstance()->set(SettingsManager::TOTAL_UPLOAD, SETTING(TOTAL_UPLOAD) + updiff);
	SettingsManager::getInstance()->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + downdiff);
	lastUpdate = aTick;
	lastUp = Socket::getTotalUp();
	lastDown = Socket::getTotalDown();

	if(SETTING(DISCONNECT_SPEED) < 1) {
		SettingsManager::getInstance()->set(SettingsManager::DISCONNECT_SPEED, 1);
	}
}

void MainFrame::on(PartialList, const HintedUser& aUser, const string& text) throw() {
	PostMessage(WM_SPEAKER, BROWSE_LISTING, (LPARAM)new DirectoryBrowseInfo(aUser, text));
}

void MainFrame::on(QueueManagerListener::Finished, const QueueItem* qi, const string& dir, const Download* download) throw() {
	if(qi->isSet(QueueItem::FLAG_CLIENT_VIEW)) {
		if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
			// This is a file listing, show it...
			DirectoryListInfo* i = new DirectoryListInfo(qi->getDownloads()[0]->getHintedUser(), Text::toT(qi->getListName()), Text::toT(dir), static_cast<int64_t>(download->getAverageSpeed()));

			PostMessage(WM_SPEAKER, DOWNLOAD_LISTING, (LPARAM)i);
		} else if(qi->isSet(QueueItem::FLAG_TEXT)) {
			PostMessage(WM_SPEAKER, VIEW_FILE_AND_DELETE, (LPARAM) new tstring(Text::toT(qi->getTarget())));
		}
	} else if(qi->isSet(QueueItem::FLAG_USER_LIST) && qi->isSet(QueueItem::FLAG_CHECK_FILE_LIST)) {
		DirectoryListInfo* i = new DirectoryListInfo(qi->getDownloads()[0]->getHintedUser(), Text::toT(qi->getListName()), Text::toT(dir), static_cast<int64_t>(download->getAverageSpeed()));
		
		if(listQueue.stop) {
			listQueue.stop = false;
			listQueue.start();
		}
		{
			Lock l(listQueue.cs);
			listQueue.fileLists.push_back(i);
		}
		listQueue.s.signal();
	}	
}

LRESULT MainFrame::onActivateApp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	bHandled = FALSE;
	WinUtil::isAppActive = (wParam == 1);	//wParam == TRUE if window is activated, FALSE if deactivated
	if(wParam == 1) {
		if(bIsPM && bTrayIcon == true) {
			NOTIFYICONDATA nid;
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = m_hWnd;
			nid.uID = 0;
			nid.uFlags = NIF_ICON;
			nid.hIcon = normalicon.hIcon;
			::Shell_NotifyIcon(NIM_MODIFY, &nid);
			bIsPM = false;
		}
	}
	return 0;
}

LRESULT MainFrame::onAppCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
	if(GET_APPCOMMAND_LPARAM(lParam) == APPCOMMAND_BROWSER_FORWARD) {
		ctrlTab.SwitchTo();
	} else if(GET_APPCOMMAND_LPARAM(lParam) == APPCOMMAND_BROWSER_BACKWARD) {
		ctrlTab.SwitchTo(false);
	} else {
		bHandled = FALSE;
	}
	
	return FALSE;
}

LRESULT MainFrame::onAway(WORD , WORD , HWND, BOOL& ) {
	if(Util::getAway()) { 
		setAwayButton(false);
		Util::setAway(false);
	} else {
		setAwayButton(true);
		Util::setAway(true);
	}
	ClientManager::getInstance()->infoUpdated();
	return 0;
}

LRESULT MainFrame::onUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	UpdateDialog dlg;
	dlg.DoModal();
	return S_OK;
}

LRESULT MainFrame::onDisableSounds(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	SettingsManager::getInstance()->set(SettingsManager::SOUNDS_DISABLED, !BOOLSETTING(SOUNDS_DISABLED));
	return 0;
}

LRESULT MainFrame::onShutDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(shutdownMng) {
		delete shutdownMng;
		shutdownMng = 0;
	} else {
		ShutdownDlg dlg(hShutdownIcon);
		if(dlg.DoModal() == IDOK) {
			shutdownMng = new ShutdownManager(dlg.action, dlg.type, dlg.timeout);
			ctrlToolbar.CheckButton(IDC_SHUTDOWN, TRUE);
		}
	}
	ctrlToolbar.CheckButton(IDC_SHUTDOWN, shutdownMng != 0);
	return S_OK;
}

void MainFrame::setShutDown(bool b) {
	if(!b) {
		if(anyMF->shutdownMng) {
			delete anyMF->shutdownMng;
			anyMF->shutdownMng = 0;
		}
		anyMF->ctrlToolbar.CheckButton(IDC_SHUTDOWN, FALSE);
	} else {
		anyMF->PostMessage(WM_COMMAND, IDC_SHUTDOWN);
	}
}

void MainFrame::on(WebServerListener::Setup) throw() {
	WSAAsyncSelect(WebServerManager::getInstance()->getServerSocket().getSock(), m_hWnd, WEBSERVER_SOCKET_MESSAGE, FD_ACCEPT);
}

void MainFrame::on(WebServerListener::ShutdownPC, int action) throw() {
	WinUtil::shutDown(action);
}

int MainFrame::FileListQueue::run() {
	setThreadPriority(Thread::LOW);

	while(true) {
		s.wait(15000);
		if(stop || fileLists.empty()) {
			break;
		}

		DirectoryListInfo* i;
		{
			Lock l(cs);
			i = fileLists.front();
			fileLists.pop_front();
		}
		if(Util::fileExists(Text::fromT(i->file))) {
			DirectoryListing* dl = new DirectoryListing(i->user);
			try {
				dl->loadFile(Text::fromT(i->file));
				ADLSearchManager::getInstance()->matchListing(*dl);
				ClientManager::getInstance()->checkCheating(i->user, dl);
			} catch(...) {
			}
			delete dl;
			//RSX++
			if(RSXPP_BOOLSETTING(DELETE_CHECKED_FILELISTS)) {
				File::deleteFile(Text::fromT(i->file));
			}
			//END
		}
		delete i;
	}
	stop = true;
	return 0;
}

LRESULT MainFrame::onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(shutdownMng) {
		delete shutdownMng;
		shutdownMng = 0;
	}

	LogManager::getInstance()->removeListener(this);
	QueueManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);

	if(bTrayIcon) {
		updateTray(false);
	}
	bHandled = FALSE;
	return 0;
}
//RSX++ //switching tabs by number
LRESULT MainFrame::onSwitchWindow(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlTab.SwitchWindow(wID - ID_SWINDOW_1);
	return 0;
}
//RSX++ //Process Priority
LRESULT MainFrame::onChangePriority(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch(wID){
		case IDC_CHANGE_PRIO_IDLE:		RsxUtil::changeProcessPriority(RsxUtil::IDLE);			break;
		case IDC_CHANGE_PRIO_BELOW:		RsxUtil::changeProcessPriority(RsxUtil::BELOW_NORMAL);	break;
		case IDC_CHANGE_PRIO_NORMAL:	RsxUtil::changeProcessPriority(RsxUtil::NORMAL);		break;
		case IDC_CHANGE_PRIO_ABOVE:		RsxUtil::changeProcessPriority(RsxUtil::ABOVE_NORMAL);	break;
		case IDC_CHANGE_PRIO_HIGH:		RsxUtil::changeProcessPriority(RsxUtil::HIGH);			break;
		case IDC_CHANGE_PRIO_REALTIME:	RsxUtil::changeProcessPriority(RsxUtil::REALTIME);		break;
	}
	return 0;
}

void MainFrame::setDefPrioMenu() {
	switch(GetPriorityClass(GetCurrentProcess())) {
		case IDLE_PRIORITY_CLASS:			prioMenu.SetMenuDefaultItem(IDC_CHANGE_PRIO_IDLE);		break;
		case BELOW_NORMAL_PRIORITY_CLASS:	prioMenu.SetMenuDefaultItem(IDC_CHANGE_PRIO_BELOW);		break;
		case NORMAL_PRIORITY_CLASS:			prioMenu.SetMenuDefaultItem(IDC_CHANGE_PRIO_NORMAL);	break;
		case ABOVE_NORMAL_PRIORITY_CLASS:	prioMenu.SetMenuDefaultItem(IDC_CHANGE_PRIO_ABOVE);		break;
		case HIGH_PRIORITY_CLASS:			prioMenu.SetMenuDefaultItem(IDC_CHANGE_PRIO_HIGH);		break;
		case REALTIME_PRIORITY_CLASS:		prioMenu.SetMenuDefaultItem(IDC_CHANGE_PRIO_REALTIME);	break;
		default:							prioMenu.SetMenuDefaultItem(IDC_CHANGE_PRIO_NORMAL);	break;
	}
}

void MainFrame::openHub(const std::string& url) {
	HubFrame::openWindow(Text::toT(url));
}

void MainFrame::closeHub(const std::string& url) {
	HubFrame::closeHub(Text::toT(url));
}
//END
/**
 * @file
 * $Id: MainFrm.cpp,v 1.20 2004/07/21 13:15:15 bigmuscle Exp
 */