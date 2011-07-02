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

#ifndef _LISTVIEWCTRL_H
#define _LISTVIEWCTRL_H

#include <wx/listctrl.h>

class ListViewCtrl :
	public wxListView
{
public:

	enum 
	{	
 		SORT_STRING = 2,
		SORT_STRING_NOCASE,
		SORT_INT,
		SORT_FLOAT,
		SORT_BYTES
	};

	ListViewCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);
	~ListViewCtrl(void);

	int insert(int nItem, TStringList& aList, int iImage = 0, void* lParam = NULL);
	int insert(TStringList& aList, int iImage = 0, void* lParam = NULL);
	int insert(int nItem, const tstring& aString, int iImage = 0, void* lParam = NULL);

	const wxString GetItemText(int nItem, int nSubItem) const
	{
		wxListItem item;
		item.SetId(nItem);
		item.SetColumn(nSubItem);
		item.SetMask(wxLIST_MASK_TEXT);

		GetItem(item);

		return item.GetText();
	}

	void setSort(int aColumn, int aType, bool aAscending = true);
	void resort();

private:

	DECLARE_EVENT_TABLE();

	// events
	void OnEraseBackground(wxEraseEvent& /*event*/) { }
	void OnColumnClick(wxListEvent& event);

	// attributes
	int sortColumn;
	int sortType;
	bool ascending;

	static int wxCALLBACK CompareFunc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);

	template<class T> static int compare(const T& a, const T& b) 
	{
		return (a < b) ? -1 : ( (a == b) ? 0 : 1);
	}


};

#endif	// _LISTVIEWCTRL _H