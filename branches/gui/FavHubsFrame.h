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

#ifndef _FAVHUBSFRAME_H
#define _FAVHUBSFRAME_H

#include "ListViewCtrl.h"
#include "StaticFrame.h"

#include "../client/FavoriteManager.h"

class FavHubsFrame :
	public StaticFrame<FavHubsFrame>, private FavoriteManagerListener, private SettingsManagerListener
{
public:

	typedef StaticFrame<FavHubsFrame> BaseType;

	FavHubsFrame(void);
	~FavHubsFrame(void);

private:

	enum 
	{
		COLUMN_FIRST,
		COLUMN_NAME = COLUMN_FIRST,
		COLUMN_DESCRIPTION,
		COLUMN_NICK,
		COLUMN_PASSWORD,
		COLUMN_SERVER,
		COLUMN_USERDESCRIPTION,
		COLUMN_LAST
	};

	struct StateKeeper {
		StateKeeper(ListViewCtrl& hubs_, bool ensureVisible_ = true);
		~StateKeeper();

		const FavoriteHubEntryList& getSelection() const;

	private:
		ListViewCtrl& hubs;
		bool ensureVisible;
		FavoriteHubEntryList selected;
		int scroll;
	};

	DECLARE_EVENT_TABLE();

	// events
	void OnClose(wxCloseEvent& event);

	void OnConnect(wxCommandEvent& /*event*/) { openSelected(); }
	void OnRemove(wxCommandEvent& event);
	void OnEdit(wxCommandEvent& event);
	void OnNew(wxCommandEvent& event);
	void OnMoveUp(wxCommandEvent& event);
	void OnMoveDown(wxCommandEvent& event);
	void OnManageGroups(wxCommandEvent& event);
	void OnOpenHubLog(wxCommandEvent& event);
	void OnItemSelected(wxListEvent& event);
	void OnItemDeselected(wxListEvent& event);
	void OnContextMenu(wxListEvent& event);
	void OnActivate(wxListEvent& /*event*/) { openSelected(); }

	// attributes
	bool nosave;

	ListViewCtrl* ctrlHubs;
	static int	columnIndexes[];
	static int	columnSizes[];

	wxButton* ctrlConnect;
	wxButton* ctrlNew;
	wxButton* ctrlProps;
	wxButton* ctrlRemove;
	wxButton* ctrlUp;
	wxButton* ctrlDown;
	wxButton* ctrlManageGroups;

	// methods
	void addEntry(const FavoriteHubEntry* entry, int pos, int groupIndex);
	void handleMove(bool up);
	TStringList getSortedGroups() const;
	void fillList();
	bool checkNick(const FavoriteHubEntry* entry);
	void openSelected();

	void UpdateLayout();

	void on(FavoriteAdded, const FavoriteHubEntry* /*e*/)  throw() { /* TODO StateKeeper keeper(ctrlHubs);*/ fillList(); }
	void on(FavoriteRemoved, const FavoriteHubEntry* e) throw() { ctrlHubs->DeleteItem(ctrlHubs->FindItem(-1, (wxUIntPtr)e)); }
	// TODO: void on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw();
};

#endif	// _FAVHUBSFRAME_H