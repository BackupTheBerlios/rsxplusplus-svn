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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include "AsyncHandler.h"
#include "ListViewCtrl.h"
#include "TaskBarIcon.h"
#include "TransferView.h"
#include "GetTTHDialog.h"
#include "HashProgressDlg.h"

#include "../client/HttpConnection.h"
#include "../client/QueueManagerListener.h"
#include "../client/Thread.h"
#include "../client/TimerManager.h"
#include "../client/WebServerManager.h"

#include <wx/wx.h>

#include <wx/aui/aui.h>
#include <wx/laywin.h>
#include <wx/mdi.h>
#include <wx/imaglist.h>
#include <wx/srchctrl.h>
#include <wx/splitter.h>

class MainWindow : 
		public wxAuiMDIParentFrame,
		public Thread,
		public AsyncHandler<MainWindow>,
		private TimerManagerListener, 
		private HttpConnectionListener, 
		private QueueManagerListener, 
		private WebServerListener

{
public:
	MainWindow(const wxPoint &position, const wxSize& size);
	~MainWindow(void);

	void updateQuickSearches();

	void setShutDown(bool _shutdown) 
	{
		if (_shutdown)
			currentShutdownTime = GET_TICK() / 1000;
		shutdown = _shutdown;
	}

	bool getShutDown() const { return shutdown; }

	void setPMIcon() { if(!IsActive()) taskBarIcon->setPMIcon(); }
	TaskBarIcon* getTaskBarIcon() const { return taskBarIcon; }

	wxAuiManager& getAuiManager() { return manager; }
	wxAuiToolBar* getToolBar() const { return toolBar; }

private:

	class DirectoryBrowseInfo 
	{
	public:
		DirectoryBrowseInfo(const HintedUser& ptr, string aText) : user(ptr), text(aText) { }
		HintedUser user;
		string text;
	};
/* TODO
	class FileListQueue: public Thread 
	{
	public:
		bool stop;
		Semaphore s;
		CriticalSection cs;
		list<DirectoryListInfo*> fileLists;

		FileListQueue() : stop(true) {}
		~FileListQueue() throw() {
			shutdown();
		}

		int run();
		void shutdown() {
			stop = true;
			s.signal();
		}
	};
*/
	DECLARE_EVENT_TABLE();

	// events
	void OnActivate(wxActivateEvent& event);
	void OnMinimize(wxIconizeEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnEndSession(wxCloseEvent& event);
	void OnTabContextMenu(wxAuiNotebookEvent& event);
	void OnQuickSearch(wxCommandEvent& event);

	void OnAway(wxCommandEvent& event);
	void OnDisableSounds(wxCommandEvent& event);
	void OnFollow(wxCommandEvent& event);
	void OnGetTTH(wxCommandEvent& event);
	void OnHashProgress(wxCommandEvent& event);
	void OnLimiter(wxCommandEvent& event);
	void OnMatchAll(wxCommandEvent& event);
	void OnOpenDownloads(wxCommandEvent& event);
	void OnOpenFileList(wxCommandEvent& event);
	void OnOpenMyList(wxCommandEvent& event);
	void OnQuickConnect(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnReconnect(wxCommandEvent& event);
	void OnRefreshFileList(wxCommandEvent& event);
	void OnSettings(wxCommandEvent& event);
	void OnShutdown(wxCommandEvent& event);
	void OnUpdate(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

	void OnOpenWindow(wxCommandEvent& event);
	void OnToggleView(wxCommandEvent& event);
	void OnLink(wxCommandEvent& event);
	
	// attributes
	bool				closing;
	bool				awayByMinimize;
	bool				shutdown;
	bool				isShutdownStatus;
	bool				missedAutoConnect;
	uint64_t			currentShutdownTime;
	bool				oldShutdown;
	int					tabPos;
	
	uint64_t			lastTick;
	int64_t				lastUp;
	int64_t				lastDown;
	
	string				versionInfo;

	TransferView*		transferView;
	TaskBarIcon*		taskBarIcon;
	wxSearchCtrl*		searchBox;
	wxStaticBitmap*		shutdownIcon;

	HttpConnection*		c;
	// TODO FileListQueue		listQueue;

	boost::thread		stopperThread;
	
	wxAuiManager		manager;
	wxAuiToolBar*		toolBar;
	wxAuiToolBar*		searchBar;

	// methods
	int run();

	void createMenu();
	void createToolBar();
	void createStatusBar();

	void updateStatus();

	void autoConnect(const FavoriteHubEntry::List& fl);

	inline wxMenuItem* createMenuItem(wxMenu* parent, int id, const tstring& title, const wxBitmap& bmp = wxBitmap(), int iconIndex = -1)
	{ 
		wxMenuItem* item = new wxMenuItem(parent, id, title);
		if(iconIndex != -1)
			item->SetBitmap(bmp.GetSubBitmap(wxRect(iconIndex * bmp.GetHeight(), 0, bmp.GetHeight(), bmp.GetHeight())));	
		return item;
	} 

	void stopper();

	// TimerManagerListener
	void on(TimerManagerListener::Second, uint64_t aTick) throw();
	
	// HttpConnectionListener
	void on(HttpConnectionListener::Complete, HttpConnection* conn, string const& /*aLine*/, bool /*fromCoral*/) throw();
	void on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw();	
	// WebServerListener
	void on(WebServerListener::Setup);
	void on(WebServerListener::ShutdownPC, int);

	// QueueManagerListener
	void on(QueueManagerListener::Finished, const QueueItem* qi, const string& dir, const Download*) throw();
	void on(PartialList, const HintedUser&, const string& text) throw();

};

#endif	// _MAINWINDOW_H