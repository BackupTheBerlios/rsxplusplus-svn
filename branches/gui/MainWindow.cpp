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
 
#include "stdafx.h"
#include "MainWindow.h"

#include "AboutDlg.h"
#include "ADLSearchFrame.h"
#include "DebugFrame.h"
#include "DirectoryListingFrame.h"
#include "FavHubsFrame.h"
#include "FavUsersFrame.h"
#include "FinishedFrame.h"
#include "HubFrame.h"
#include "NetStatsFrame.h"
#include "NotepadFrame.h"
#include "PublicHubsFrame.h"
#include "QueueFrame.h"
#include "RecentHubsFrame.h"
#include "SearchFrame.h"
#include "SearchSpyFrame.h"
#include "SettingsDlg.h"
#include "SystemLog.h"
#include "TransferView.h"
#include "UpdateDialog.h"
#include "UploadQueueFrame.h"
#include "WinUtil.h"
#include "wxAuiArts.h"

#include "../client/ADLSearch.h"
#include "../client/ConnectionManager.h"
#include "../client/ConnectivityManager.h"
#include "../client/Download.h"
#include "../client/DownloadManager.h"
#include "../client/QueueItem.h"
#include "../client/QueueManager.h"
#include "../client/SettingsManager.h"
#include "../client/ShareManager.h"
#include "../client/StringTokenizer.h"
#include "../client/Thread.h"
#include "../client/ThrottleManager.h"
#include "../client/UploadManager.h"
#include "../client/UPnPManager.h"
#include "../client/version.h"

#include "../dht/dht.h"

#include <wx/laywin.h>
#include <wx/srchctrl.h>
#include <wx/taskbar.h>
#include <wx/tooltip.h>
#include <wx/wx.h>

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_ACTIVATE(MainWindow::OnActivate)
	EVT_ICONIZE(MainWindow::OnMinimize)
	EVT_SIZE(MainWindow::OnSize)
	EVT_CLOSE(MainWindow::OnClose)
	EVT_END_SESSION(MainWindow::OnEndSession)
	EVT_AUINOTEBOOK_TAB_RIGHT_UP(wxID_ANY, MainWindow::OnTabContextMenu)
	EVT_TEXT_ENTER(IDC_SEARCH_EDIT, MainWindow::OnQuickSearch)
	//EVT_SEARCHCTRL_SEARCH_BTN(IDC_SEARCH_EDIT, MainWindow::OnQuickSearch)

	EVT_MENU(ID_SEARCH_HISTORY, MainWindow::OnQuickSearch)
	EVT_MENU(ID_CMD_AWAY, MainWindow::OnAway)
	EVT_MENU(ID_CMD_DISABLE_SOUNDS, MainWindow::OnDisableSounds)
	EVT_MENU(ID_CMD_RECONNECT, MainWindow::OnReconnect)
	EVT_MENU(ID_CMD_FOLLOW, MainWindow::OnFollow)
	EVT_MENU(ID_CMD_GET_TTH, MainWindow::OnGetTTH)
	EVT_MENU(ID_CMD_HASH_PROGRESS, MainWindow::OnHashProgress)
	EVT_MENU(ID_CMD_LIMITER, MainWindow::OnLimiter)
	EVT_MENU(ID_CMD_MATCH_ALL, MainWindow::OnMatchAll)
	EVT_MENU(ID_CMD_OPEN_DOWNLOADS, MainWindow::OnOpenDownloads)
	EVT_MENU(ID_CMD_OPEN_FILE_LIST, MainWindow::OnOpenFileList)
	EVT_MENU(ID_CMD_OPEN_MY_LIST, MainWindow::OnOpenMyList)
	EVT_MENU(ID_CMD_QUICK_CONNECT, MainWindow::OnQuickConnect)
	EVT_MENU(ID_CMD_QUIT, MainWindow::OnQuit)
	EVT_MENU(ID_CMD_REFRESH_FILE_LIST, MainWindow::OnRefreshFileList)
	EVT_MENU(ID_CMD_SETTINGS, MainWindow::OnSettings)
	EVT_MENU(ID_CMD_SHUTDOWN, MainWindow::OnShutdown)
	EVT_MENU(ID_CMD_UPDATE, MainWindow::OnUpdate)
	EVT_MENU(ID_CMD_ABOUT, MainWindow::OnAbout)

	EVT_MENU_RANGE(ID_CMD_ADL_SEARCH, ID_CMD_UPLOAD_QUEUE, MainWindow::OnOpenWindow)
	EVT_MENU_RANGE(ID_CMD_VIEW_TOOLBAR, ID_CMD_VIEW_TRANSFER_VIEW, MainWindow::OnToggleView)
	EVT_MENU_RANGE(ID_CMD_HOMEPAGE,	ID_CMD_DONATE, MainWindow::OnLink)
END_EVENT_TABLE()

struct toolbarButton 
{
	int id, image;
	bool check;
	ResourceManager::Strings tooltip;
};

static const toolbarButton ToolbarButtons[] = 
{
	{ID_CMD_CONNECT,			 0, true,	ResourceManager::MENU_PUBLIC_HUBS		},
	{ID_CMD_RECONNECT,			 1, false,	ResourceManager::MENU_RECONNECT			},
	{ID_CMD_FOLLOW,				 2, false,	ResourceManager::MENU_FOLLOW_REDIRECT	},
	{ID_CMD_FAVORITES,			 3, true,	ResourceManager::MENU_FAVORITE_HUBS		},
	{ID_CMD_FAVUSERS,			 4, true,	ResourceManager::MENU_FAVORITE_USERS	},
	{ID_CMD_RECENTS,			 5, true,	ResourceManager::MENU_FILE_RECENT_HUBS	},
	{ID_CMD_QUEUE,				 6, true,	ResourceManager::MENU_DOWNLOAD_QUEUE	},
	{ID_CMD_FINISHED_DL,		 7, true,	ResourceManager::FINISHED_DOWNLOADS		},
	{ID_CMD_UPLOAD_QUEUE,		 8, true,	ResourceManager::UPLOAD_QUEUE			},
	{ID_CMD_FINISHED_UL,		 9, true,	ResourceManager::FINISHED_UPLOADS		},
	{ID_CMD_SEARCH,				10, false,	ResourceManager::MENU_SEARCH			},
	{ID_CMD_ADL_SEARCH,			11, true,	ResourceManager::MENU_ADL_SEARCH		},
	{ID_CMD_SEARCH_SPY,			12, true,	ResourceManager::MENU_SEARCH_SPY		},
	{ID_CMD_NET_STATS,			13, true,	ResourceManager::NETWORK_STATISTICS		},
	{ID_CMD_OPEN_FILE_LIST,		14, false,	ResourceManager::MENU_OPEN_FILE_LIST	},
	{ID_CMD_SETTINGS,			15, false,	ResourceManager::MENU_SETTINGS			},
	{ID_CMD_NOTEPAD,			16, true,	ResourceManager::MENU_NOTEPAD			},
	{ID_CMD_AWAY,				17, true,	ResourceManager::AWAY					},
	{ID_CMD_SHUTDOWN,			18, true,	ResourceManager::SHUTDOWN				},
	{ID_CMD_LIMITER,			19, true,	ResourceManager::ENABLE_LIMITING		},
	{ID_CMD_UPDATE,				20, false,	ResourceManager::UPDATE_CHECK			},
	{ID_CMD_DISABLE_SOUNDS,		21, true,	ResourceManager::DISABLE_SOUNDS			}
};

MainWindow::MainWindow(const wxPoint &position, const wxSize& size) : wxAuiMDIParentFrame(NULL, ID_MAIN_WINDOW, COMPLETEVERSIONSTRING, position, size, wxDEFAULT_FRAME_STYLE),
	lastTick(GET_TICK()), lastUp(0), lastDown(0), c(new HttpConnection()), taskBarIcon(NULL), oldShutdown(false), shutdown(false), 
	currentShutdownTime(0), isShutdownStatus(false), awayByMinimize(false), closing(false), missedAutoConnect(false), toolBar(NULL)
{
	//SetDoubleBuffered(true);
	manager.SetFlags( wxAUI_MGR_DEFAULT | wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_LIVE_RESIZE );
	manager.GetArtProvider()->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE, 0);
	manager.SetManagedWindow(this);

	TimerManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);
	WebServerManager::getInstance()->addListener(this);
	
	if(BOOLSETTING(WEBSERVER)) 
	{
		try 
		{
			WebServerManager::getInstance()->Start();
		} 
		catch(const Exception& e) 
		{
			wxMessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), wxICON_STOP | wxOK);
		}
	}

	WinUtil::init(this);

	TimerManager::getInstance()->start();

	SetIcon(wxIcon(wxT("IDI_APPICON")));
	taskBarIcon = new TaskBarIcon(this);

	createMenu();
	createToolBar();
	createStatusBar();

	long auistyle = wxAUI_NB_TAB_SPLIT |
					wxAUI_NB_TAB_MOVE |
					wxAUI_NB_TAB_EXTERNAL_MOVE |
					wxAUI_NB_TAB_FIXED_WIDTH |
					wxAUI_NB_WINDOWLIST_BUTTON |
					wxAUI_NB_CLOSE_ON_ALL_TABS |
					wxAUI_NB_SCROLL_BUTTONS |
					wxAUI_NB_MIDDLE_CLICK_CLOSE;
	
	tabPos = SETTING(TABS_POS);
	if (tabPos == 1) // bottom 
		auistyle |= wxAUI_NB_BOTTOM;
	else
		auistyle |= wxAUI_NB_TOP;
	
	GetClientWindow()->SetWindowStyleFlag(auistyle);
	GetClientWindow()->SetArtProvider(new wxAuiCustomTabArt());

	wxAuiNotebook* bottomNotebook = new wxAuiNotebook(this);

	// workaround for event bug when wxAuiNotebook docked in wxAuiMDIParentFrame
	bottomNotebook->Bind(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, [this](wxAuiNotebookEvent& /*event*/) {  });

	transferView = new TransferView(bottomNotebook);

	bottomNotebook->SetSize(1000, SETTING(TRANSFER_SPLIT_SIZE));
	bottomNotebook->SetWindowStyleFlag(wxAUI_NB_BOTTOM | wxAUI_NB_TAB_MOVE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxAUI_NB_TAB_SPLIT);
	
	bottomNotebook->AddPage(new SystemLog(bottomNotebook), _("System log"), true);	// TODO: some better log
	bottomNotebook->AddPage(transferView, _("Transfers"), false);
	bottomNotebook->AddPage(new DebugFrame(bottomNotebook), _("Debug"), false);

	manager.AddPane(bottomNotebook, wxAuiPaneInfo().Bottom()
		.Dock()
		.Floatable(false)
		.Gripper(false)
		.CloseButton(true)
		.CaptionVisible(false));

	manager.Update();

	c->addListener(this);
	c->downloadFile(VERSION_URL);

	if(BOOLSETTING(OPEN_PUBLIC))				WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_CONNECT);
	if(BOOLSETTING(OPEN_FAVORITE_HUBS))			WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_FAVORITES);
	if(BOOLSETTING(OPEN_FAVORITE_USERS))		WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_FAVUSERS);
	if(BOOLSETTING(OPEN_RECENT_HUBS))			WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_RECENTS);	
	if(BOOLSETTING(OPEN_QUEUE))					WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_QUEUE);
	if(BOOLSETTING(OPEN_FINISHED_DOWNLOADS))	WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_FINISHED_DL);
	if(BOOLSETTING(OPEN_UPLOAD_QUEUE))			WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_UPLOAD_QUEUE);
	if(BOOLSETTING(OPEN_FINISHED_UPLOADS))		WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_FINISHED_UL);
	if(BOOLSETTING(OPEN_SEARCH_SPY))			WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_SEARCH_SPY);
	if(BOOLSETTING(OPEN_NETWORK_STATISTICS))	WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_NET_STATS);
	if(BOOLSETTING(OPEN_NOTEPAD))				WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_NOTEPAD);

	if(!BOOLSETTING(SHOW_STATUSBAR))			WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_VIEW_STATUS_BAR);
	if(!BOOLSETTING(SHOW_TOOLBAR))				WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_VIEW_TOOLBAR);
	if(!BOOLSETTING(SHOW_TRANSFERVIEW))			WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_VIEW_TRANSFER_VIEW);
	if(!BOOLSETTING(SHOW_QUICK_SEARCH))			WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_TOGGLE_QSEARCH);

	
	if(!wxGetKeyState(WXK_SHIFT))
		callAsync(std::bind(&MainWindow::autoConnect, this, FavoriteManager::getInstance()->getFavoriteHubs()));
	
	/* TODO WinUtil::asyncEvent(this, wxEVT_SPEAKER, PARSE_COMMAND_LINE);*/

	try 
	{
		File::ensureDirectory(SETTING(LOG_DIRECTORY));
	} 
	catch (const FileException) 
	{	
	}

	try 
	{
		ConnectivityManager::getInstance()->setup(true, SettingsManager::INCOMING_DIRECT);
	} 
	catch (const Exception& e) {
		// TODO showPortsError(e.getError());
	}

	Util::setAway(BOOLSETTING(AWAY));

	toolBar->ToggleTool(ID_CMD_AWAY, BOOLSETTING(AWAY));
	toolBar->ToggleTool(ID_CMD_LIMITER, BOOLSETTING(THROTTLE_ENABLE));
	toolBar->ToggleTool(ID_CMD_DISABLE_SOUNDS, BOOLSETTING(SOUNDS_DISABLED));

	if(SETTING(NICK).empty()) 
	{
		WinUtil::asyncEvent(this, wxEVT_COMMAND_MENU_SELECTED, ID_CMD_SETTINGS);
	}
}

MainWindow::~MainWindow(void)
{
	WinUtil::uninit();

	delete taskBarIcon;
	manager.UnInit();
}

void MainWindow::stopper()
{
	for(size_t i = 0; i < GetClientWindow()->GetPageCount(); ++i)
	{
		wxWindow* page = GetClientWindow()->GetPage(i);

		wxCloseEvent event(wxEVT_CLOSE_WINDOW, page->GetId());
		event.SetEventObject(page);
		event.SetCanVeto(true);
		wxPostEvent(page, event);
	}

	wxCloseEvent event(wxEVT_CLOSE_WINDOW, GetId());
	event.SetEventObject(this);
	event.SetCanVeto(true);
	wxPostEvent(this, event);
}

void MainWindow::createMenu()
{
	wxMenuBar* mainMenu = new wxMenuBar();

	wxBitmap bitmapList = wxBitmap(wxT("IDB_TOOLBAR"), wxBITMAP_TYPE_BMP_RESOURCE);
	WinUtil::premultiplyAlpha(bitmapList);

	wxMenu* file = new wxMenu();
	file->Append(createMenuItem(file, ID_CMD_OPEN_FILE_LIST, CTSTRING(MENU_OPEN_FILE_LIST), bitmapList, 13));
	file->Append(createMenuItem(file, ID_CMD_OPEN_MY_LIST, CTSTRING(MENU_OPEN_OWN_LIST)));
	file->Append(createMenuItem(file, ID_CMD_MATCH_ALL, CTSTRING(MENU_OPEN_MATCH_ALL)));
	file->Append(createMenuItem(file, ID_CMD_REFRESH_FILE_LIST, CTSTRING(MENU_REFRESH_FILE_LIST)));
	file->Append(createMenuItem(file, ID_CMD_OPEN_DOWNLOADS, CTSTRING(MENU_OPEN_DOWNLOADS_DIR)));
	file->AppendSeparator();
	file->Append(createMenuItem(file, ID_CMD_QUICK_CONNECT, CTSTRING(MENU_QUICK_CONNECT)));
	file->Append(createMenuItem(file, ID_CMD_FOLLOW, CTSTRING(MENU_FOLLOW_REDIRECT)));
	file->Append(createMenuItem(file, ID_CMD_RECONNECT, CTSTRING(MENU_RECONNECT), bitmapList, 1));
	file->AppendSeparator();
	file->Append(createMenuItem(file, ID_CMD_SETTINGS, CTSTRING(MENU_SETTINGS), bitmapList, 14));
	file->Append(createMenuItem(file, ID_CMD_GET_TTH, CTSTRING(MENU_TTH), bitmapList, 23));
	file->Append(createMenuItem(file, ID_CMD_UPDATE, CTSTRING(UPDATE_CHECK), bitmapList, 24));
	file->AppendSeparator();
	file->Append(createMenuItem(file, ID_CMD_QUIT, CTSTRING(MENU_EXIT)));

	mainMenu->Append(file, CTSTRING(MENU_FILE));

	wxMenu* view = new wxMenu();
	view->Append(createMenuItem(view, ID_CMD_CONNECT, CTSTRING(MENU_PUBLIC_HUBS), bitmapList, 0));
	view->AppendSeparator();
	view->Append(createMenuItem(view, ID_CMD_RECENTS, CTSTRING(MENU_FILE_RECENT_HUBS), bitmapList, 3));
	view->AppendSeparator();
	view->Append(createMenuItem(view, ID_CMD_FAVORITES, CTSTRING(MENU_FAVORITE_HUBS), bitmapList, 4));
	view->Append(createMenuItem(view, ID_CMD_FAVUSERS, CTSTRING(MENU_FAVORITE_USERS), bitmapList, 5));
	view->AppendSeparator();
	view->Append(createMenuItem(view, ID_CMD_SEARCH, CTSTRING(MENU_SEARCH), bitmapList, 10));
	view->Append(createMenuItem(view, ID_CMD_ADL_SEARCH, CTSTRING(MENU_ADL_SEARCH), bitmapList, 11));
	view->Append(createMenuItem(view, ID_CMD_SEARCH_SPY, CTSTRING(MENU_SEARCH_SPY), bitmapList, 12));
	view->AppendSeparator();
	view->Append(createMenuItem(view, ID_CMD_NOTEPAD, CTSTRING(MENU_NOTEPAD), bitmapList, 15));
	view->Append(createMenuItem(view, ID_CMD_HASH_PROGRESS, CTSTRING(MENU_HASH_PROGRESS)));
	view->AppendSeparator();
	view->AppendCheckItem(ID_CMD_VIEW_TOOLBAR, CTSTRING(MENU_TOOLBAR));
	view->AppendCheckItem(ID_CMD_TOGGLE_QSEARCH, CTSTRING(TOGGLE_QSEARCH));	
	view->AppendCheckItem(ID_CMD_VIEW_STATUS_BAR, CTSTRING(MENU_STATUS_BAR));
	view->AppendCheckItem(ID_CMD_VIEW_TRANSFER_VIEW, CTSTRING(MENU_TRANSFER_VIEW));

	view->Check(ID_CMD_VIEW_TOOLBAR, true);
	view->Check(ID_CMD_TOGGLE_QSEARCH, true);
	view->Check(ID_CMD_VIEW_STATUS_BAR, true);
	view->Check(ID_CMD_VIEW_TRANSFER_VIEW, true);

	mainMenu->Append(view, CTSTRING(MENU_VIEW));

	wxMenu* transfers = new wxMenu();
	transfers->Append(createMenuItem(transfers, ID_CMD_QUEUE, CTSTRING(MENU_DOWNLOAD_QUEUE), bitmapList, 6));
	transfers->Append(createMenuItem(transfers, ID_CMD_FINISHED_DL, CTSTRING(FINISHED_DOWNLOADS), bitmapList, 7));
	transfers->AppendSeparator();
	transfers->Append(createMenuItem(transfers, ID_CMD_UPLOAD_QUEUE, CTSTRING(UPLOAD_QUEUE), bitmapList, 8));
	transfers->Append(createMenuItem(transfers, ID_CMD_FINISHED_UL, CTSTRING(FINISHED_UPLOADS), bitmapList, 9));
	transfers->AppendSeparator();
	transfers->Append(createMenuItem(transfers, ID_CMD_NET_STATS, CTSTRING(MENU_NETWORK_STATISTICS), bitmapList, 16));

	mainMenu->Append(transfers, CTSTRING(MENU_TRANSFERS));
/* TODO: wxMDIParentFrame has its own windowmenu but it will need either translation or using our menu
	wxMenu *window = new wxMenu(0);
	window->Append(ID_WINDOW_CASCADE, CTSTRING(MENU_CASCADE));
	window->Append(ID_WINDOW_TILE_HORZ, CTSTRING(MENU_HORIZONTAL_TILE));
	window->Append(ID_WINDOW_TILE_VERT, CTSTRING(MENU_VERTICAL_TILE));
	window->Append(ID_WINDOW_ARRANGE, CTSTRING(MENU_ARRANGE));
	window->Append(ID_WINDOW_MINIMIZE_ALL, CTSTRING(MENU_MINIMIZE_ALL));
	window->Append(ID_WINDOW_RESTORE_ALL, CTSTRING(MENU_RESTORE_ALL));
	window->AppendSeparator();
	window->Append(ID_CMD_CLOSE_DISCONNECTED, CTSTRING(MENU_CLOSE_DISCONNECTED));
	window->Append(ID_CMD_CLOSE_ALL_PM, CTSTRING(MENU_CLOSE_ALL_PM));
	window->Append(ID_CMD_CLOSE_ALL_OFFLINE_PM, CTSTRING(MENU_CLOSE_ALL_OFFLINE_PM));
	window->Append(ID_CMD_CLOSE_ALL_DIR_LIST, CTSTRING(MENU_CLOSE_ALL_DIR_LIST));
	window->Append(ID_CMD_CLOSE_ALL_SEARCH_FRAME, CTSTRING(MENU_CLOSE_ALL_SEARCHFRAME));

	mainMenu->Append(window, CTSTRING(MENU_WINDOW));
*/
	wxMenu *help = new wxMenu();
	help->Append(ID_CMD_ABOUT, CTSTRING(MENU_ABOUT));
	help->AppendSeparator();
	help->Append(ID_CMD_HOMEPAGE, CTSTRING(MENU_HOMEPAGE));
	help->Append(ID_CMD_GEOIPFILE, CTSTRING(MENU_HELP_GEOIPFILE));
	help->Append(ID_CMD_TRANSLATIONS, CTSTRING(MENU_HELP_TRANSLATIONS));
	help->Append(ID_CMD_FAQ, CTSTRING(MENU_FAQ));
	help->Append(ID_CMD_DISCUSS, CTSTRING(MENU_DISCUSS));
	help->Append(ID_CMD_DONATE, CTSTRING(MENU_DONATE));

	mainMenu->Append(help, CTSTRING(MENU_HELP));

	SetMenuBar(mainMenu);
}

void MainWindow::createToolBar()
{
	if(toolBar == NULL)
	{
		toolBar = new wxAuiToolBar(this, ID_MAIN_TOOLBAR, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW/* | wxAUI_TB_NO_AUTORESIZE*/);
	}

	toolBar->ClearTools();

	wxBitmap bitmapList;
	if(SETTING(TOOLBARIMAGE) == "")
	{
		bitmapList = wxBitmap(wxT("IDB_TOOLBAR20"), wxBITMAP_TYPE_BMP_RESOURCE);
		WinUtil::premultiplyAlpha(bitmapList);
	} 
	else
	{
		wxImage img(Text::toT(SETTING(TOOLBARIMAGE)).c_str(), wxBITMAP_TYPE_BMP);
		bitmapList = wxBitmap(img);
	}

	StringTokenizer<string> t(SETTING(TOOLBAR), ',');
	StringList& l = t.getTokens();
	
	int buttonsCount = sizeof(ToolbarButtons) / sizeof(ToolbarButtons[0]);
	for(StringList::const_iterator k = l.begin(); k != l.end(); ++k) 
	{
		int i = Util::toInt(*k);		
		
		if(i == -1)
		{
			toolBar->AddSeparator();
		}
		else if(i >= 0 && i < buttonsCount) 
		{
			wxRect r(ToolbarButtons[i].image * 20, 0, 20, 20);
			wxBitmap bmp = bitmapList.GetSubBitmap(r);

			wxItemKind kind = ToolbarButtons[i].check ? wxITEM_CHECK : wxITEM_NORMAL;
			toolBar->AddTool(ToolbarButtons[i].id, wxEmptyString, bmp, CTSTRING_I((ResourceManager::Strings)ToolbarButtons[i].tooltip), 
				kind);
		} 
		else 
		{
			continue;
		}
	}

	toolBar->SetToolBitmapSize(wxSize(20, 20));
	toolBar->SetToolSeparation(11);
	toolBar->Realize();	

	// search bar
	searchBar = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE/* | wxAUI_TB_OVERFLOW*/);
	searchBar->AddControl(searchBox = new wxSearchCtrl(searchBar, IDC_SEARCH_EDIT));
	searchBar->Realize();
	updateQuickSearches();

	// add toolbars into AUI manager (don't allow dockable bars now)
	manager.AddPane(toolBar, wxAuiPaneInfo().ToolbarPane().Top().Floatable(false).Movable(false));
	manager.AddPane(searchBar, wxAuiPaneInfo().ToolbarPane().Top().Floatable(false).Movable(false));
}


void MainWindow::updateQuickSearches() 
{
	wxMenu* historyMenu = NULL;

	if(!SearchFrame::getLastSearches().empty())
	{
		historyMenu = new wxMenu();
	
		for(std::set<tstring>::const_iterator i = SearchFrame::getLastSearches().begin(); i != SearchFrame::getLastSearches().end(); ++i) 
		{
			historyMenu->Append(ID_SEARCH_HISTORY, *i);
		}
	}

	searchBox->SetMenu(historyMenu);
}

void MainWindow::OnActivate(wxActivateEvent& event)
{
	taskBarIcon->setNormalIcon();

	// forward to active MDI window
	wxAuiMDIChildFrame* activeChild = this->GetActiveChild();
	if(activeChild != NULL)
		activeChild->GetEventHandler()->ProcessEvent(event);

	event.Skip();
}

void MainWindow::createStatusBar()
{
	wxStatusBar* statusBar = CreateStatusBar(11, wxSTB_SIZEGRIP | wxSTB_SHOW_TIPS | wxFULL_REPAINT_ON_RESIZE);
	shutdownIcon = new wxStaticBitmap(statusBar, wxID_ANY, wxIcon(), wxDefaultPosition, wxSize(16, 16));

	updateStatus();
}


void MainWindow::autoConnect(const FavoriteHubEntry::List& fl) 
{
	missedAutoConnect = false;
	for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i) 
	{
		FavoriteHubEntry* entry = *i;
		if(entry->getConnect()) {
 			if(!entry->getNick().empty() || !SETTING(NICK).empty()) 
			{
				RecentHubEntry r;
				r.setName(entry->getName());
				r.setDescription(entry->getDescription());
				r.setUsers("*");
				r.setShared("*");
				r.setServer(entry->getServer());
				FavoriteManager::getInstance()->addRecent(r);
				HubFrame::openWindow(Text::toT(entry->getServer())
					, entry->getRawOne()
					, entry->getRawTwo()
					, entry->getRawThree()
					, entry->getRawFour()
					, entry->getRawFive()
					, entry->getChatUserSplit(), entry->getUserListState());
 			} else
 				missedAutoConnect = true;
 		}				
	}
}

void MainWindow::OnMinimize(wxIconizeEvent& event)
{
	if(!event.Iconized())
		return;

	//SetProcessWorkingSetSize(GetCurrentProcess(), 0xffffffff, 0xffffffff);
	if(BOOLSETTING(AUTO_AWAY)) 
	{
		if(Util::getAway() == true) 
		{
			awayByMinimize = false;
		} 
		else 
		{
			awayByMinimize = true;
			Util::setAway(true);
			toolBar->ToggleTool(ID_CMD_AWAY, true);
			ClientManager::getInstance()->infoUpdated();
		}
	}

	if(BOOLSETTING(MINIMIZE_TRAY) && !wxGetKeyState(WXK_SHIFT))
	{
		Show(false);
		SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
	}
}

void MainWindow::OnSize(wxSizeEvent& event)
{
	if(IsShown())
	{
		int w, h;
		GetClientSize(&w, &h);

		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		if(BOOLSETTING(AUTO_AWAY)) 
		{
			if(awayByMinimize)
			{
				awayByMinimize = false;
				Util::setAway(false);
				toolBar->ToggleTool(ID_CMD_AWAY, false);
				ClientManager::getInstance()->infoUpdated();
			}
		}

		// position icon in the last statusbar field
		wxRect sr;
		GetStatusBar()->GetFieldRect(10, sr);
		sr.Offset(2, (sr.GetHeight() - 16) / 2);
		sr.SetSize(wxSize(16, 16));
		shutdownIcon->SetSize(sr);
	}
	
	event.Skip();
}

void MainWindow::OnClose(wxCloseEvent& event)
{
	if(!closing) 
	{
		if( oldShutdown ||(!BOOLSETTING(CONFIRM_EXIT)) || (wxMessageBox(CTSTRING(REALLY_EXIT), _T(APPNAME) _T(" ") _T(VERSIONSTRING), wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT) == wxYES) ) 
		{
			if(c != NULL) {
				c->removeListener(this);
				delete c;
				c = NULL;
			}

			taskBarIcon->RemoveIcon();

			if(!IsIconized() && IsShown())
			{
				if(!IsMaximized())
				{
					SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, this->GetPosition().x);
					SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, this->GetPosition().y);
					SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, this->GetSize().GetWidth());
					SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, this->GetSize().GetHeight());
				}

				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)IsMaximized());
			}		

			SearchManager::getInstance()->disconnect();
			ConnectionManager::getInstance()->disconnect();
			// TODO listQueue.shutdown();

			transferView->prepareClose();

			WebServerManager::getInstance()->removeListener(this);
			QueueManager::getInstance()->removeListener(this);
			TimerManager::getInstance()->removeListener(this);

			boost::thread( &MainWindow::stopper, this ).swap( stopperThread );
			closing = true;
		}

		return;
	}

	// This should end immediately, as it only should be the stopper that sends another WM_CLOSE
	stopperThread.timed_join( boost::posix_time::minutes( 1 ) );
	event.Skip();
}

void MainWindow::OnEndSession(wxCloseEvent& /*event*/)
{
	if(c != NULL) 
	{
		c->removeListener(this);
		delete c;
		c = NULL;
	}

	if(!IsIconized())
	{
		if(!IsMaximized() && IsShown())
		{
			SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, this->GetPosition().x);
			SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, this->GetPosition().y);
			SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, this->GetSize().GetWidth());
			SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, this->GetSize().GetHeight());
		}

		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)IsMaximized());
	}	

	QueueManager::getInstance()->saveQueue();
	SettingsManager::getInstance()->save();
}

void MainWindow::OnTabContextMenu(wxAuiNotebookEvent& event)
{
	wxWindow* child = GetClientWindow()->GetPage(event.GetSelection());
	if(!child->GetEventHandler()->ProcessEvent(event))
	{
		wxMenu mnu;
		mnu.Append(IDC_CLOSE_WINDOW, CTSTRING(CLOSE));
		PopupMenu(&mnu);
	}
}

void MainWindow::OnQuickSearch(wxCommandEvent& event)
{
	if(event.GetId() == ID_SEARCH_HISTORY)
	{
	}
	else
	{
		SearchFrame::openWindow(tstring(searchBox->GetValue()));
	}

}

void MainWindow::OnAway(wxCommandEvent& /*event*/) 
{ 
	Util::setAway(!Util::getAway());
	ClientManager::getInstance()->infoUpdated();
}

void MainWindow::OnDisableSounds(wxCommandEvent& /*event*/) 
{
	SettingsManager::getInstance()->set(SettingsManager::SOUNDS_DISABLED, !BOOLSETTING(SOUNDS_DISABLED));
}

void MainWindow::OnReconnect(wxCommandEvent& event)
{
	// forward to active MDI window
	wxAuiMDIChildFrame* activeChild = this->GetActiveChild();
	if(activeChild != NULL)
		activeChild->GetEventHandler()->ProcessEvent(event);
}

void MainWindow::OnFollow(wxCommandEvent& event) 
{ 
	// forward to active MDI window
	wxAuiMDIChildFrame* activeChild = this->GetActiveChild();
	if(activeChild != NULL)
		activeChild->GetEventHandler()->ProcessEvent(event);
}

void MainWindow::OnGetTTH(wxCommandEvent& /*event*/) 
{ 
	Thread::start();
}

void MainWindow::OnHashProgress(wxCommandEvent& /*event*/) 
{
	HashProgressDlg dlg(this);
	dlg.setAutoClose(false);
	dlg.ShowModal();
}

void MainWindow::OnLimiter(wxCommandEvent& /*event*/) 
{ 
	SettingsManager::getInstance()->set(SettingsManager::THROTTLE_ENABLE, !BOOLSETTING(THROTTLE_ENABLE));
	ClientManager::getInstance()->infoUpdated();
}

void MainWindow::OnMatchAll(wxCommandEvent& /*event*/) 
{ }

void MainWindow::OnOpenDownloads(wxCommandEvent& /*event*/) 
{ }

void MainWindow::OnOpenFileList(wxCommandEvent& /*event*/) 
{ }

void MainWindow::OnOpenMyList(wxCommandEvent& /*event*/) 
{ 
	if(!ShareManager::getInstance()->getOwnListFile().empty())
	{
		DirectoryListingFrame::openWindow(Text::toT(ShareManager::getInstance()->getOwnListFile()), Text::toT(Util::emptyString), HintedUser(ClientManager::getInstance()->getMe(), Util::emptyString), 0);
	}	
}

void MainWindow::OnQuickConnect(wxCommandEvent& /*event*/) 
{ 
	wxTextEntryDialog dlg(this, TSTRING(HUB_ADDRESS), TSTRING(QUICK_CONNECT));
	if(dlg.ShowModal() == wxID_OK)
	{
		if(SETTING(NICK).empty())
			return;

		tstring tmp = dlg.GetValue();
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
}

void MainWindow::OnQuit(wxCommandEvent& /*event*/) 
{ 
	Close();
}

void MainWindow::OnRefreshFileList(wxCommandEvent& /*event*/) 
{ 
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true);
}

void MainWindow::OnSettings(wxCommandEvent& /*event*/) 
{ 
	SettingsDlg dlg(this, SettingsManager::getInstance());

	unsigned short lastTCP = static_cast<unsigned short>(SETTING(TCP_PORT));
	unsigned short lastUDP = static_cast<unsigned short>(SETTING(UDP_PORT));
	unsigned short lastTLS = static_cast<unsigned short>(SETTING(TLS_PORT));
	unsigned short lastDHT = static_cast<unsigned short>(SETTING(DHT_PORT));
	
	int lastConn = SETTING(INCOMING_CONNECTIONS);
	bool lastDHTConn = BOOLSETTING(USE_DHT);

	bool lastSortFavUsersFirst = BOOLSETTING(SORT_FAVUSERS_FIRST);

	if(dlg.ShowModal() == wxID_OK) 
	{
		SettingsManager::getInstance()->save();
		if(missedAutoConnect && !SETTING(NICK).empty()) 
		{
			callAsync(std::bind(&MainWindow::autoConnect, this, FavoriteManager::getInstance()->getFavoriteHubs()));
		}
		
		try 
		{
			ConnectivityManager::getInstance()->setup(SETTING(INCOMING_CONNECTIONS) != lastConn || SETTING(TCP_PORT) != lastTCP || SETTING(UDP_PORT) != lastUDP || SETTING(TLS_PORT) != lastTLS ||
				SETTING(DHT_PORT) != lastDHT || BOOLSETTING(USE_DHT) != lastDHTConn, lastConn);
		} 
		catch (const Exception& e) 
		{
			// TODO: showPortsError(e.getError());
		}
 
		if(BOOLSETTING(SORT_FAVUSERS_FIRST) != lastSortFavUsersFirst)
			HubFrame::resortUsers();

		/* TODO if(BOOLSETTING(URL_HANDLER)) 
		{
			WinUtil::registerDchubHandler();
			WinUtil::registerADChubHandler();
			WinUtil::urlDcADCRegistered = true;
		}
		else if(WinUtil::urlDcADCRegistered) 
		{
			WinUtil::unRegisterDchubHandler();
			WinUtil::unRegisterADChubHandler();
			WinUtil::urlDcADCRegistered = false;
		}
		
		if(BOOLSETTING(MAGNET_REGISTER)) 
		{
			WinUtil::registerMagnetHandler();
			WinUtil::urlMagnetRegistered = true; 
		} 
		else if(WinUtil::urlMagnetRegistered) 
		{
			WinUtil::unRegisterMagnetHandler();
			WinUtil::urlMagnetRegistered = false;
		}*/

		if(BOOLSETTING(THROTTLE_ENABLE)) 
			toolBar->ToggleTool(IDC_LIMITER, true);
		else 
			toolBar->ToggleTool(IDC_LIMITER, false);

		if(Util::getAway()) 
			toolBar->ToggleTool(IDC_AWAY, true);
		else 
			toolBar->ToggleTool(IDC_AWAY, false);

		if(getShutDown()) 
			toolBar->ToggleTool(IDC_SHUTDOWN, true);
		else 
			toolBar->ToggleTool(IDC_SHUTDOWN, false);
	
		if(tabPos != SETTING(TABS_POS)) 
		{
			tabPos = SETTING(TABS_POS);
			long auistyle = GetClientWindow()->GetWindowStyle();
			auistyle &= ~(wxAUI_NB_BOTTOM | wxAUI_NB_TOP);

			if (SETTING(TABS_POS) == 1) // bottom 
				auistyle |= wxAUI_NB_BOTTOM;
			else
				auistyle |= wxAUI_NB_TOP;
			
			GetClientWindow()->SetWindowStyleFlag(auistyle);

			wxAuiMDIChildFrame* activeChild = this->GetActiveChild();
			if(activeChild)
				activeChild->Refresh();
		}

		ClientManager::getInstance()->infoUpdated();
	}
}

void MainWindow::OnShutdown(wxCommandEvent& /*event*/) 
{ 
	setShutDown(!getShutDown());
}

void MainWindow::OnUpdate(wxCommandEvent& /*event*/) 
{ 
	UpdateDialog dlg(this);
	dlg.ShowModal();
}

void MainWindow::OnAbout(wxCommandEvent& /*event*/) 
{
	AboutDlg dlg(this);
	dlg.ShowModal();
}

void MainWindow::OnOpenWindow(wxCommandEvent& event)
{
	// TODO: open windows
	switch(event.GetId())
	{
		case ID_CMD_ADL_SEARCH:			ADLSearchFrame::openWindow(); break;
		case ID_CMD_CONNECT:			PublicHubsFrame::openWindow(); break;
		case ID_CMD_FAVORITES:			FavHubsFrame::openWindow(); break;
		case ID_CMD_FAVUSERS:			FavUsersFrame::openWindow(); break;
		case ID_CMD_FINISHED_DL:		FinishedFrame<true>::openWindow(); break;
		case ID_CMD_FINISHED_UL:		FinishedFrame<false>::openWindow(); break;
		case ID_CMD_NET_STATS:			NetStatsFrame::openWindow(); break;
		case ID_CMD_NOTEPAD:			NotepadFrame::openWindow(); break;
		case ID_CMD_QUEUE:				QueueFrame::openWindow(); break;
		case ID_CMD_RECENTS:			RecentHubsFrame::openWindow(); break;
		case ID_CMD_SEARCH:				SearchFrame::openWindow(); break;
		case ID_CMD_SEARCH_SPY:			SearchSpyFrame::openWindow(); break;
		case ID_CMD_UPLOAD_QUEUE:		UploadQueueFrame::openWindow(); break;
	}
}

void MainWindow::OnToggleView(wxCommandEvent& event) 
{
	bool visible = true;
	switch(event.GetId())
	{
		case ID_CMD_VIEW_TOOLBAR:
			visible = !this->toolBar->IsShown();
			this->toolBar->Show(visible); 
			SettingsManager::getInstance()->set(SettingsManager::SHOW_TOOLBAR, visible);
			break;

		case ID_CMD_TOGGLE_QSEARCH:
			visible = !searchBar->IsShown();
			searchBar->Show(visible);
			SettingsManager::getInstance()->set(SettingsManager::SHOW_QUICK_SEARCH, visible);
			break;

		case ID_CMD_VIEW_STATUS_BAR:
			visible = !this->GetStatusBar()->IsShown();
			this->GetStatusBar()->Show(visible);
			SettingsManager::getInstance()->set(SettingsManager::SHOW_STATUSBAR, visible);
			break;

		case ID_CMD_VIEW_TRANSFER_VIEW:
			visible = !transferView->IsShown();
			transferView->Show(visible);
			SettingsManager::getInstance()->set(SettingsManager::SHOW_TRANSFERVIEW, visible);
			break;
	}

	this->GetMenuBar()->Check(event.GetId(), visible);
}

void MainWindow::OnLink(wxCommandEvent& event) 
{
	tstring site;
	switch(event.GetId())
	{
		case ID_CMD_HOMEPAGE: site = _T("http://strongdc.sf.net"); break;
		case ID_CMD_GEOIPFILE: site = _T("http://www.maxmind.com/download/geoip/database/GeoIPCountryCSV.zip"); break;
		case ID_CMD_TRANSLATIONS: site = _T("http://strongdc.sf.net/forum/viewtopic.php?t=5201"); break;
		case ID_CMD_FAQ: site = _T("http://strongdc.sf.net/forum/viewtopic.php?t=4067"); break;
		case ID_CMD_DISCUSS: site = _T("http://strongdc.sf.net/forum/index.php"); break;
		case ID_CMD_DONATE: site = _T("http://strongdc.sf.net/donate.php"); break;
	}

	WinUtil::openLink(site);
}

// TimerManagerListener
void MainWindow::on(TimerManagerListener::Second, uint64_t aTick) throw()
{
	callAsync(std::bind(&MainWindow::updateStatus, this));
}

void MainWindow::updateStatus()
{
	uint64_t now = GET_TICK();
	uint64_t tdiff = now - lastTick;
	if(tdiff < 100)
	{
		tdiff = 1;
	}

	uint64_t up = Socket::getTotalUp();
	uint64_t down = Socket::getTotalDown();
	uint64_t updiff = up - lastUp;
	uint64_t downdiff = down - lastDown;

	lastTick = now;
	lastUp = up;
	lastDown = down;

	// save stats
	SettingsManager::getInstance()->set(SettingsManager::TOTAL_UPLOAD, SETTING(TOTAL_UPLOAD) + updiff);
	SettingsManager::getInstance()->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + downdiff);

	SetStatusText(Util::getAway() ? TSTRING(AWAY) : _T(""), 1);
	SetStatusText(_T("DHT: ") + (DHT::getInstance()->isConnected() ? Util::toStringW(DHT::getInstance()->getNodesCount()) : _T("-")), 2);
	SetStatusText(TSTRING(SHARED) + _T(": ") + Util::formatBytesW(ShareManager::getInstance()->getSharedSize()), 3);
	SetStatusText(_T("H: ") + Text::toT(Client::getCounts()), 4);
	SetStatusText(TSTRING(SLOTS) + _T(": ") + Util::toStringW(UploadManager::getInstance()->getFreeSlots()) + _T('/') + Util::toStringW(UploadManager::getInstance()->getSlots()) + _T(" (") + Util::toStringW(UploadManager::getInstance()->getFreeExtraSlots()) + _T('/') + Util::toStringW(SETTING(EXTRA_SLOTS)) + _T(")"), 5);
	SetStatusText(_T("D: ") + Util::formatBytesW(Socket::getTotalDown()), 6);
	SetStatusText(_T("U: ") + Util::formatBytesW(Socket::getTotalUp()), 7);
	
	tstring downInfo = _T("D: [") + Util::toStringW(DownloadManager::getInstance()->getDownloadCount()) + _T("][");
	tstring upInfo = _T("U: [") + Util::toStringW(UploadManager::getInstance()->getUploadCount()) + _T("][");
	if(BOOLSETTING(THROTTLE_ENABLE)) 
	{
		downInfo += Util::formatBytesW(ThrottleManager::getInstance()->getDownloadLimit()) + _T("/s");
		upInfo += Util::formatBytesW(ThrottleManager::getInstance()->getUploadLimit()) + _T("/s");
	} 
	else 
	{
		downInfo += _T("-");
		upInfo += _T("-");
	}
	
	SetStatusText(downInfo + _T("] ") + Util::formatBytesW(downdiff * 1000 / tdiff) + _T("/s"), 8);
	SetStatusText(upInfo + _T("] ") + Util::formatBytesW(updiff * 1000 / tdiff) + _T("/s"), 9);
	
	// update taskbar icon tooltip
	wxString tip = _T("D: ") + Util::formatBytesW(DownloadManager::getInstance()->getRunningAverage()) + _T("/s (") + 
		Util::toStringW(DownloadManager::getInstance()->getDownloadCount()) + _T(")\r\nU: ") +
		Util::formatBytesW(UploadManager::getInstance()->getRunningAverage()) + _T("/s (") + 
		Util::toStringW(UploadManager::getInstance()->getUploadCount()) + _T(")")
		+ _T("\r\nUptime: ") + Util::formatSeconds(time(NULL) - Util::getStartTime());
			
	taskBarIcon->setToolTip(tip);

	// position shutdown icon
	if (shutdown) 
	{
		uint64_t iSec = GET_TICK() / 1000;
		if(!isShutdownStatus) 
		{
			shutdownIcon->SetIcon(wxIcon(wxT("IDI_SHUTDOWN"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));
			isShutdownStatus = true;
		}
		if (DownloadManager::getInstance()->getDownloadCount() > 0) 
		{
			currentShutdownTime = iSec;
			this->GetStatusBar()->SetStatusText(wxT(""), 10);
		} 
		else 
		{
			int64_t timeLeft = SETTING(SHUTDOWN_TIMEOUT) - (iSec - currentShutdownTime);
			// FIXME: something better than tabulator + 3× space
			this->GetStatusBar()->SetStatusText((_T("\t   ") + Util::formatSeconds(timeLeft, timeLeft < 3600)).c_str(), 10);
			if (currentShutdownTime + SETTING(SHUTDOWN_TIMEOUT) <= iSec) 
			{
				bool bDidShutDown = false;
				// TODO bDidShutDown = WinUtil::shutDown(SETTING(SHUTDOWN_ACTION));
				if (bDidShutDown) 
				{
					// Should we go faster here and force termination?
					// We "could" do a manual shutdown of this app...
				} 
				else 
				{
					LogManager::getInstance()->message(STRING(FAILED_TO_SHUTDOWN), LogManager::LOG_ERROR);
					this->GetStatusBar()->SetStatusText(wxT(""), 10);
				}
				// We better not try again. It WON'T work...
				shutdown = false;
			}
		}
	} 
	else 
	{
		if(isShutdownStatus) 
		{
			shutdownIcon->SetIcon(wxIcon());
			isShutdownStatus = false;
		}
		this->GetStatusBar()->SetStatusText(wxT(""), 10);
	}

	// update status sizes
	int widths[11] = { 0 };
	widths[0] = -1; // first field will take the remaining width
	widths[1] = WinUtil::getTextWidth(TSTRING(AWAY), GetStatusBar()); // for "AWAY" segment
	widths[10] = 50;

	for(size_t i = 2; i < 10; ++i)
	{
		int textWidth = WinUtil::getTextWidth(GetStatusBar()->GetStatusText(i), GetStatusBar());
		int partWidth = GetStatusBar()->GetStatusWidth(i);

		widths[i] = max(textWidth, partWidth);
	}
	GetStatusBar()->SetStatusWidths(11, widths);
}
	
// HttpConnectionListener
void MainWindow::on(HttpConnectionListener::Complete, HttpConnection* /*conn*/, string const& /*aLine*/, bool /*fromCoral*/) throw()
{
	try {
		SimpleXML xml;
		xml.fromXML(versionInfo);
		xml.stepIn();

		string url;
		if(xml.findChild("URL")) {
			url = xml.getChildData();
		}

		xml.resetCurrentChild();
		if(xml.findChild("Version")) {
			if(Util::toDouble(xml.getChildData()) > VERSIONFLOAT) {
				xml.resetCurrentChild();
				xml.resetCurrentChild();
				if(xml.findChild("Title")) {
					const string& title = xml.getChildData();
					xml.resetCurrentChild();
					if(xml.findChild("Message") && !BOOLSETTING(DONT_ANNOUNCE_NEW_VERSIONS)) {
						if(url.empty()) {
							const string& msg = xml.getChildData();
							wxMessageBox(Text::toT(msg).c_str(), Text::toT(title).c_str(), wxOK);
						} else {
							string msg = xml.getChildData() + "\r\n" + STRING(OPEN_DOWNLOAD_PAGE);

							// TODO: should we open update dialog or just show some kind of notification?
							UpdateDialog dlg(this);
							dlg.ShowModal();
						}
					}
				}
				xml.resetCurrentChild();
				if(xml.findChild("VeryOldVersion")) {
					if(Util::toDouble(xml.getChildData()) >= VERSIONFLOAT) {
						string msg = xml.getChildAttrib("Message", "Your version of StrongDC++ contains a serious bug that affects all users of the DC network or the security of your computer.");
						wxMessageBox(Text::toT(msg + "\r\nPlease get a new one at " + url).c_str());
						oldShutdown = true;
						this->Close();
					}
				}
				xml.resetCurrentChild();
				if(xml.findChild("BadVersion")) {
					xml.stepIn();
					while(xml.findChild("BadVersion")) {
						double v = Util::toDouble(xml.getChildAttrib("Version"));
						if(v == VERSIONFLOAT) {
							string msg = xml.getChildAttrib("Message", "Your version of DC++ contains a serious bug that affects all users of the DC network or the security of your computer.");
							wxMessageBox(Text::toT(msg + "\r\nPlease get a new one at " + url).c_str(), _T("Bad DC++ version"), wxOK | wxICON_EXCLAMATION);
							oldShutdown = true;
							this->Close();
						}
					}
				}
			}
		}
	} catch (const Exception&) {
		// ...
	}
}

void MainWindow::on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw()
{
	versionInfo += string((const char*)buf, len);
}
	
// WebServerListener
void MainWindow::on(WebServerListener::Setup)
{
	// TODO
}

void MainWindow::on(WebServerListener::ShutdownPC, int)
{
	// TODO
}

// QueueManagerListener
void MainWindow::on(QueueManagerListener::Finished, const QueueItem* qi, const string& dir, const Download* download) throw()
{
	if(qi->isSet(QueueItem::FLAG_CLIENT_VIEW)) 
	{
		if(qi->isSet(QueueItem::FLAG_USER_LIST)) 
		{
			// This is a file listing, show it...
			tstring file = Text::toT(qi->getListName());
			HintedUser user = qi->getDownloads()[0]->getHintedUser();
			int64_t speed = static_cast<int64_t>(download->getAverageSpeed());
			callAsync([this, file, dir, user, speed] { DirectoryListingFrame::openWindow(file, Text::toT(dir), user, speed); });
		} 
		else if(qi->isSet(QueueItem::FLAG_TEXT)) 
		{
			string target = qi->getTarget();
			callAsync([this, target] {
				// TODO: TextFrame::openWindow(Text::toT(target));
				File::deleteFile(target);
			});

		}
	} 
	else if(qi->isSet(QueueItem::FLAG_USER_CHECK)) 
	{
/*	TODO:
		DirectoryListInfo* i = new DirectoryListInfo(qi->getDownloads()[0]->getHintedUser(), Text::toT(qi->getListName()), Text::toT(dir), static_cast<int64_t>(download->getAverageSpeed()));
		
		if(listQueue.stop) 
		{
			listQueue.stop = false;
			listQueue.start();
		}

		{
			::Lock l(listQueue.cs);
			listQueue.fileLists.push_back(i);
		}

		listQueue.s.signal();
*/
	}
}

void MainWindow::on(PartialList, const HintedUser& aUser, const string& text) throw()
{
	callAsync([this, aUser, text] { DirectoryListingFrame::openWindow(aUser, text, 0); });
}

int MainWindow::run() 
{
	tstring file;
	wxFileDialog fileDlg(this);
	this->GetMenuBar()->Enable(ID_CMD_GET_TTH, false);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		file = fileDlg.GetPath();
		Thread::setThreadPriority(Thread::LOW);

		boost::scoped_array<char> buf(new char[512 * 1024]);

		try
		{
			File f(Text::fromT(file), File::READ, File::OPEN);
			TigerTree tth(TigerTree::calcBlockSize(f.getSize(), 1));

			if(f.getSize() > 0)
			{
				size_t n = 512*1024;
				while( (n = f.read(&buf[0], n)) > 0)
				{
					tth.update(&buf[0], n);
					n = 512*1024;
				}
			}
			else
			{
				tth.update("", 0);
			}
			tth.finalize();

			tstring tthRoot = Text::toT(tth.getRoot().toBase32());
			tstring magnet = _T("magnet:?xt=urn:tree:tiger:") + Text::toT(tth.getRoot().toBase32()) + _T("&xl=") + 
				Util::toStringW(f.getSize()) + _T("&dn=") + Text::toT(Util::encodeURI(Text::fromT(Util::getFileName(file))));

			f.close();

			callAsync([this, file, tthRoot, magnet] {
				GetTTHDialog dlg(this);
				dlg.fillValues(file, tthRoot, magnet);
				dlg.ShowModal();
			});
		}
		catch(...) 
		{ }

		Thread::setThreadPriority(Thread::NORMAL);
	}
	this->GetMenuBar()->Enable(ID_CMD_GET_TTH, true);

	return 0;
}

/* TODO
int MainWindow::FileListQueue::run() 
{
	setThreadPriority(Thread::LOW);

	while(true) {
		s.wait(15000);
		if(stop || fileLists.empty()) 
		{
			break;
		}

		DirectoryListInfo* i;
		{
			::Lock l(cs);
			i = fileLists.front();
			fileLists.pop_front();
		}
		if(Util::fileExists(Text::fromT(i->file))) 
		{
			DirectoryListing* dl = new DirectoryListing(i->user);
			try 
			{
				dl->loadFile(Text::fromT(i->file));
				ADLSearchManager::getInstance()->matchListing(*dl);
				ClientManager::getInstance()->checkCheating(i->user, dl);
			} 
			catch(...) 
			{
			}
			delete dl;
		}
		delete i;
	}
	stop = true;
	return 0;
}
*/