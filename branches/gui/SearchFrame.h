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

#ifndef _SEARCHFRAME_H
#define _SEARCHFRAME_H

#include "MDIChildFrame.h"
#include "ProgressBarRenderer.h"
#include "TypedTreeListCtrl.h"
#include "UCHandler.h"
#include "UserInfoBaseHandler.h"

#include "../client/SearchManager.h"
#include "../client/SearchResult.h"

#include <wx/checklst.h>
#include <wx/splitter.h>

class SearchFrame :
	public MDIChildFrame, 
	private SearchManagerListener, 
	private ClientManagerListener,
	public UCHandler<SearchFrame>,
	public UserInfoBaseHandler<SearchFrame>,
	private SettingsManagerListener, 
	private TimerManagerListener
{
private:

	class SearchInfo : public UserInfoBase 
	{
	public:
		typedef SearchInfo* Ptr;
		typedef vector<Ptr> List;
		typedef List::const_iterator Iter;

		SearchInfo::List subItems;

		SearchInfo(const SearchResultPtr& aSR) : sr(aSR), collapsed(true), parent(NULL), flagIndex(0), hits(0) { }

		~SearchInfo() {	}

		const UserPtr& getUser() const { return sr->getUser(); }

		bool collapsed;
		size_t hits;
		SearchInfo* parent;

		void getList();
		void browseList();

		void view();
		struct Download 
		{
			Download(const tstring& aTarget, SearchFrame* aSf) : tgt(aTarget), sf(aSf) { }
			void operator()(SearchInfo* si);
			const tstring& tgt;
			SearchFrame* sf;
		};
		struct DownloadWhole 
		{
			DownloadWhole(const tstring& aTarget) : tgt(aTarget) { }
				void operator()(SearchInfo* si);
			const tstring& tgt;
		};
		struct DownloadTarget 
		{
			DownloadTarget(const tstring& aTarget) : tgt(aTarget) { }
				void operator()(SearchInfo* si);
			const tstring& tgt;
		};
		struct CheckTTH 
		{
			CheckTTH() : op(true), firstHubs(true), hasTTH(false), firstTTH(true) { }
				void operator()(SearchInfo* si);
			bool firstHubs;
			StringList hubs;
			bool op;
			bool hasTTH;
			bool firstTTH;
			tstring tth;
		};
	
		wxString getText(uint8_t col) const 
		{
			switch(col) 
			{
				case COLUMN_FILENAME:
					if(sr->getType() == SearchResult::TYPE_FILE) 
					{
						if(sr->getFile().rfind(_T('\\')) == tstring::npos) 
						{
							return Text::toT(sr->getFile());
						} 
						else 
						{
	    					return Text::toT(Util::getFileName(sr->getFile()));
						}      
					} 
					else 
					{
						return Text::toT(sr->getFileName());
					}
				case COLUMN_HITS: return hits == 0 ? Util::emptyStringT : Util::toStringW(hits + 1) + _T(' ') + TSTRING(USERS);
				case COLUMN_NICK: return WinUtil::getNicks(sr->getUser(), sr->getHubURL());
				case COLUMN_TYPE:
					if(sr->getType() == SearchResult::TYPE_FILE) 
					{
						tstring type = Text::toT(Util::getFileExt(Text::fromT(tstring(getText(COLUMN_FILENAME)))));
						if(!type.empty() && type[0] == _T('.'))
							type.erase(0, 1);
						return type;
					} 
					else 
					{
						return TSTRING(DIRECTORY);
					}
				case COLUMN_SIZE: 
					if(sr->getType() == SearchResult::TYPE_FILE) 
					{
						return Util::formatBytesW(sr->getSize());
					} 
					else 
					{
						return Util::emptyStringT;
					}					
				case COLUMN_PATH:
					if(sr->getType() == SearchResult::TYPE_FILE) 
					{
						return Text::toT(Util::getFilePath(sr->getFile()));
					} 
					else 
					{
						return Text::toT(sr->getFile());
					}
				case COLUMN_SLOTS: return Text::toT(sr->getSlotString());
				case COLUMN_CONNECTION: return Text::toT(ClientManager::getInstance()->getConnection(getUser()->getCID()));
				case COLUMN_HUB: return Text::toT(sr->getHubName());
				case COLUMN_EXACT_SIZE: return sr->getSize() > 0 ? Util::formatExactSize(sr->getSize()) : Util::emptyStringT;
				case COLUMN_IP: 
				{
					string ip = sr->getIP();
					if (!ip.empty()) 
					{
						// Only attempt to grab a country mapping if we actually have an IP address
						string tmpCountry = Util::getIpCountry(ip);
						if(!tmpCountry.empty()) 
						{
							ip = tmpCountry + " (" + ip + ")";
						}
					}
					return Text::toT(ip);
				}
				case COLUMN_TTH: return sr->getType() == SearchResult::TYPE_FILE ? Text::toT(sr->getTTH().toBase32()) : Util::emptyStringT;
				default: return Util::emptyStringT;
			}
		}
	
		static int compareItems(const SearchInfo* a, const SearchInfo* b, uint8_t col) 
		{
			if(!a->sr || !b->sr)
				return 0;
	
			switch(col) {
				case COLUMN_TYPE: 
					if(a->sr->getType() == b->sr->getType())
						return lstrcmpi(a->getText(COLUMN_TYPE).c_str(), b->getText(COLUMN_TYPE).c_str());
					else
						return(a->sr->getType() == SearchResult::TYPE_DIRECTORY) ? -1 : 1;
				case COLUMN_HITS: return compare(a->hits, b->hits);
				case COLUMN_SLOTS: 
					if(a->sr->getFreeSlots() == b->sr->getFreeSlots())
						return compare(a->sr->getSlots(), b->sr->getSlots());
					else
						return compare(a->sr->getFreeSlots(), b->sr->getFreeSlots());
				case COLUMN_SIZE:
				case COLUMN_EXACT_SIZE: return compare(a->sr->getSize(), b->sr->getSize());
				default: return lstrcmpi(a->getText(col).c_str(), b->getText(col).c_str());
			}
		}

		int getImageIndex() const 
		{
			int image = 0;
			if (BOOLSETTING(USE_SYSTEM_ICONS)) 
			{
				image = sr->getType() == SearchResult::TYPE_FILE ? WinUtil::getIconIndex(Text::toT(sr->getFile())) : WinUtil::getDirIconIndex();
			} 
			else 
			{
				string tmp = ClientManager::getInstance()->getConnection(sr->getUser()->getCID());
				if( (tmp == "28.8Kbps") ||
					(tmp == "33.6Kbps") ||
					(tmp == "56Kbps") ||
					(tmp == "Modem") ||
					(tmp == "Satellite") ||
					(tmp == "Wireless") ||
					(tmp == "ISDN") ) 
				{
					image = 1;
				} 
				else if( (tmp == "Cable") ||
					(tmp == "DSL") ) 
				{
					image = 2;
				}
				 else if( (tmp == "LAN(T1)") ||
					(tmp == "LAN(T3)") ) 
				{
					image = 3;
				}
				if(sr->getType() == SearchResult::TYPE_FILE)
					image += 4;
			}
			return image;
		}
		
		inline SearchInfo* createParent() { return this; }
		inline const TTHValue& getGroupCond() const { return sr->getTTH(); }

		SearchResultPtr sr;
		GETSET(uint8_t, flagIndex, FlagIndex);
	};

	struct HubInfo : public FastAlloc<HubInfo> 
	{
		HubInfo(const Client* client) : url(Text::toT(client->getHubUrl())),
			name(Text::toT(client->getHubName())), op(client->getMyIdentity().isOp()) { }

		tstring url;
		tstring name;
		bool op;
	};

public:

	SearchFrame(const tstring& str = Util::emptyStringW, int64_t size = 0, SearchManager::SizeModes mode = SearchManager::SIZE_ATLEAST, 
		SearchManager::TypeModes type = SearchManager::TYPE_ANY);
	~SearchFrame(void);

	static void openWindow(const tstring& str = Util::emptyStringW, int64_t size = 0, SearchManager::SizeModes mode = SearchManager::SIZE_ATLEAST, 
		SearchManager::TypeModes type = SearchManager::TYPE_ANY);

	typedef TypedTreeListCtrl<SearchInfo, TTHValue, hash<TTHValue*>, equal_to<TTHValue*>> SearchInfoList;
	SearchInfoList& getUserList() { return *ctrlResults; }

	static const std::set<tstring>& getLastSearches() { return lastSearches; }

	static bool GetItemAttr(void* data, int64_t& position, int64_t& size, wxColour& backColour, wxColour& textColour);
	
	void runUserCommand(const UserCommand& uc); 

private:

	enum 
	{
		COLUMN_FIRST,
		COLUMN_FILENAME = COLUMN_FIRST,
		COLUMN_HITS,
		COLUMN_NICK,
		COLUMN_TYPE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_SLOTS,
		COLUMN_CONNECTION,
		COLUMN_HUB,
		COLUMN_EXACT_SIZE,
		COLUMN_IP,		
		COLUMN_TTH,
		COLUMN_LAST
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

	DECLARE_EVENT_TABLE();

	// events
	void OnClose(wxCloseEvent& event);
	void OnSetFocus(wxFocusEvent& /*event*/) { searchBox->SetFocus(); }
	void OnEnter(wxCommandEvent& event);
	void OnPause(wxCommandEvent& event);
	void OnPurge(wxCommandEvent& event);
	void OnShowUI(wxCommandEvent& event);
	void OnFreeSlots(wxCommandEvent& /*event*/) { onlyFree = ctrlSlots->IsChecked(); }
	void OnCollapsed(wxCommandEvent& /*event*/) { expandSR = ctrlCollapsed->IsChecked(); }
	void OnFilterSelection(wxCommandEvent& event);
	void OnFilterChange(wxCommandEvent& event);
	void OnFilterEnter(wxCommandEvent& event);
	void OnDblClick(wxDataViewEvent& event);
	void OnContextMenu(wxDataViewEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnCheckListBox(wxCommandEvent& event);

	void OnDownload(wxCommandEvent& /*event*/)			{ ctrlResults->forEachSelectedT(SearchInfo::Download(Text::toT(SETTING(DOWNLOAD_DIRECTORY)), this)); }
	void OnDownloadDirectory(wxCommandEvent& /*event*/) { ctrlResults->forEachSelectedT(SearchInfo::DownloadWhole(Text::toT(SETTING(DOWNLOAD_DIRECTORY)))); }
	void OnDownloadTo(wxCommandEvent& event);
	void OnDownloadDirectoryTo(wxCommandEvent& event);
	void OnDownloadFavoriteDirs(wxCommandEvent& event);
	void OnDownloadWholeFavoriteDirs(wxCommandEvent& event);
	void OnDownloadTarget(wxCommandEvent& event);
	void OnDownloadWholeTarget(wxCommandEvent& event);
	void OnViewAsText(wxCommandEvent& /*event*/)		{ ctrlResults->forEachSelected(&SearchInfo::view); }
	void OnSearchForAlternates(wxCommandEvent& event);
	void OnBitziLookup(wxCommandEvent& event);
	void OnRemove(wxCommandEvent& event);
	void OnCopy(wxCommandEvent& event);

	// attributes
	static int columnIndexes[];
	static int columnSizes[];

	bool				onlyFree;
	bool				isHash;
	bool				expandSR;
	bool				running;
	bool				exactSize1;
	bool				useGrouping;
	bool				waiting;
	
	size_t				droppedResults;	
	size_t				resultsCount;
	int64_t				exactSize2;
	uint64_t			searchEndTime;
	uint64_t			searchStartTime;

	wxSplitterWindow*	splitter;
	wxPanel*			controlPanel;
	SearchInfoList*		ctrlResults;
	wxComboBox*			searchBox;	// TODO: use wxSearchCtrl when it becomes better
	wxComboBox*			ctrlMode;
	wxComboBox*			ctrlSizeMode;
	wxComboBox*			ctrlFiletype;
	wxComboBox*			ctrlFilterSel;
	wxTextCtrl*			ctrlSize;
	wxTextCtrl*			ctrlFilter;
	wxCheckBox*			ctrlSlots;
	wxCheckBox*			ctrlShowUI;
	wxCheckBox*			ctrlCollapsed;
	wxButton*			ctrlPauseSearch;
	
	ProgressBarRenderer<SearchFrame>	progress;
	wxBitmap							progressBitmap;

	wxCheckListBox*		ctrlHubs;
	vector<HubInfo*>	hubs;

	string				token;
	tstring				target;
	tstring				filter;
	StringList			targets;
	TStringList			search;
	StringMap			ucLineParams;
	static std::set<tstring> lastSearches;

	SearchInfo::List	pausedResults;

	CriticalSection		cs;
	
	// methods
	void UpdateLayout();
	void addSearchResult(SearchInfo* si);

	bool matchFilter(SearchInfo* si, int sel, bool doSizeCompare = false, FilterModes mode = NONE, int64_t size = 0);
	bool parseFilter(FilterModes& mode, int64_t& size);
	void updateSearchList(SearchInfo* si = NULL);

	void initHubs();
	void onHubAdded(HubInfo* info);
	void onHubChanged(HubInfo* info);
	void onHubRemoved(HubInfo* info);

	void onFilterResult();
	void onQueueStats(tstring* str);

	void on(SearchManagerListener::SR, const SearchResultPtr& aResult) throw();
	void on(TimerManagerListener::Second, uint64_t aTick) throw();

	// ClientManagerListener
	void on(ClientConnected, const Client* c) throw() { callAsync(std::bind(&SearchFrame::onHubAdded, this, new HubInfo(c))); }
	void on(ClientUpdated, const Client* c) throw() { callAsync(std::bind(&SearchFrame::onHubChanged, this, new HubInfo(c))); }
	void on(ClientDisconnected, const Client* c) throw() { callAsync(std::bind(&SearchFrame::onHubRemoved, this, new HubInfo(c))); }
	
	void on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw() { }
};

#endif	// _SEARCHFRAME_H