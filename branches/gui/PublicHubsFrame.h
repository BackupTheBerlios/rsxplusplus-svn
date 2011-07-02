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

#ifndef _PUBLICHUBSFRAME_H
#define _PUBLICHUBSFRAME_H

#include "StaticFrame.h"
#include "TypedDataViewCtrl.h"
#include "WinUtil.h"

#include "../client/FavoriteManagerListener.h"

#include <wx/textctrl.h>

class PublicHubsFrame :
	public StaticFrame<PublicHubsFrame>, private FavoriteManagerListener
{
public:

	typedef StaticFrame<PublicHubsFrame> BaseType;

	PublicHubsFrame(void);
	~PublicHubsFrame(void);

private:

	enum 
	{
		COLUMN_FIRST,
		COLUMN_NAME = COLUMN_FIRST,
		COLUMN_DESCRIPTION,
		COLUMN_USERS,
		COLUMN_SERVER,
		COLUMN_COUNTRY,
		COLUMN_SHARED,
		COLUMN_MINSHARE,
		COLUMN_MINSLOTS,
		COLUMN_MAXHUBS,
		COLUMN_MAXUSERS,
		COLUMN_RELIABILITY,
		COLUMN_RATING,
		COLUMN_LAST
	};

	enum 
	{
		FINISHED,
		SET_TEXT
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

	class HubInfo 
	{
	public:
		HubInfo(const HubEntry* entry_) : entry(entry_) { }

		static int compareItems(const HubInfo* a, const HubInfo* b, int col);
		wxString getText(int column) const;
		uint8_t getImageIndex() const;

		const HubEntry* entry;
	};

	DECLARE_EVENT_TABLE();

	// events
	void OnClose(wxCloseEvent& event);
	void OnFilterEnter(wxCommandEvent& event);
	void OnRefresh(wxCommandEvent& event);
	void OnConfigure(wxCommandEvent& event);
	void OnListSelection(wxCommandEvent& event);
	void OnContextMenu(wxDataViewEvent& event);
	void OnDblClick(wxDataViewEvent& event);

	void OnConnect(wxCommandEvent& event);
	void OnAdd(wxCommandEvent& event);
	void OnCopy(wxCommandEvent& event);

	void OnFinished(const tstring& str);

	// attributes
	typedef TypedDataViewCtrl<HubInfo> HubList;
	HubList*		ctrlHubs;
	wxTextCtrl*		ctrlFilter;
	wxComboBox*		ctrlFilterSel;
	wxComboBox*		ctrlHubLists;

	HubEntryList	hubs;
	string			filter;

	static int columnIndexes[];
	static int columnSizes[];
	
	// methods
	void UpdateLayout();
	void updateList();
	void updateDropDown();

	bool checkNick();

	bool parseFilter(FilterModes& mode, double& size);
	bool matchFilter(const HubEntry& entry, const int& sel, bool doSizeCompare, const FilterModes& mode, const double& size);

	void on(DownloadStarting, const string& l) throw();
	void on(DownloadFailed, const string& l) throw();
	void on(DownloadFinished, const string& l, bool fromCoral) throw();
	void on(LoadedFromCache, const string& l, const string& d) throw();
	void on(Corrupted, const string& l) throw();
};

#endif	// _PUBLICHUBSFRAME_H