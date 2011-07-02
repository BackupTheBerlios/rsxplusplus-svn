/*
 * Copyright (C) 2010 adrian_007 adrian-007 on o2 point pl
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

#ifndef _ADL_SEARCH_FRAME_H_
#define _ADL_SEARCH_FRAME_H_

#include "StaticFrame.h"
#include "ListViewCtrl.h"

class ADLSearchFrame : public StaticFrame<ADLSearchFrame> {
public:
	typedef StaticFrame<ADLSearchFrame> BaseType;

	ADLSearchFrame();
private:
	DECLARE_EVENT_TABLE();

	void UpdateLayout();
	void LoadAll();
	void UpdateSearch(int index, bool doDelete = true);

	// events
	void OnClose(wxCloseEvent& event);

	void OnAdd(wxCommandEvent& event);
	void OnEdit(wxCommandEvent& event);
	void OnRemove(wxCommandEvent& event);
	void OnMoveUp(wxCommandEvent& event);
	void OnMoveDown(wxCommandEvent& event);

	void OnItemSelected(wxListEvent& event);
	void OnItemDeselected(wxListEvent& event);
	void OnContextMenu(wxListEvent& /*event*/) { /* todo */ };
	void OnActivate(wxListEvent& /*event*/) { wxCommandEvent e; OnEdit(e); }

	ListViewCtrl* ctrlList;

	wxButton* ctrlAdd;
	wxButton* ctrlProps;
	wxButton* ctrlRemove;
	wxButton* ctrlUp;
	wxButton* ctrlDown;

	// Column order
	enum {
		COLUMN_FIRST = 0,
		COLUMN_ACTIVE_SEARCH_STRING = COLUMN_FIRST,
		COLUMN_SOURCE_TYPE,
		COLUMN_DEST_DIR,
		COLUMN_MIN_FILE_SIZE,
		COLUMN_MAX_FILE_SIZE,
		COLUMN_LAST
	};

	static int columnIndexes[];
	static int columnSizes[];
};

#endif
