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
#include "ListViewCtrl.h"

#include "../client/SettingsManager.h"
#include "../client/Util.h"

#include <wx/wx.h>

#ifdef _WIN32
# include <uxtheme.h>
# pragma comment(lib, "uxtheme.lib")
#endif

BEGIN_EVENT_TABLE(ListViewCtrl, wxListView)
	EVT_ERASE_BACKGROUND(ListViewCtrl::OnEraseBackground)
	EVT_LIST_COL_CLICK(ID_LIST_CTRL, ListViewCtrl::OnColumnClick)
END_EVENT_TABLE()

ListViewCtrl::ListViewCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : 
	wxListView(parent, id, pos, size, style), sortType(SORT_STRING), ascending(true), sortColumn(-1)
{
#ifdef _WIN32
	::SendMessage((HWND)GetHWND(), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, ::SendMessage((HWND)GetHWND(), LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0L) | LVS_EX_DOUBLEBUFFER);

	int major, minor;
	wxGetOsVersion(&major, &minor);
	if(BOOLSETTING(USE_EXPLORER_THEME) &&
		((major >= 5 && minor >= 1) //WinXP & WinSvr2003
		|| (major >= 6))) //Vista & Win7
	{
		SetWindowTheme((HWND)GetHWND(), L"explorer", NULL);
	}
#endif	
}

ListViewCtrl::~ListViewCtrl(void)
{

}

int ListViewCtrl::insert(int nItem, TStringList& aList, int iImage, void* lParam)
{
	dcassert(aList.size() > 0);

	int i = insert(nItem, aList[0], iImage, lParam);

	int k = 0;
	for(TStringIter j = aList.begin(); j != aList.end(); ++j, k++) {
		SetItem(i, k, j->c_str());
	}
	return i;
}

int ListViewCtrl::insert(TStringList& aList, int iImage, void* lParam)
{
	// TODO
	return -1;
}
	
int ListViewCtrl::insert(int nItem, const tstring& aString, int iImage, void* lParam) 
{
	wxListItem item;
	item.SetId(nItem);
	item.SetText(aString);
	item.SetImage(iImage);
	item.SetData(lParam);

	return InsertItem(item);
}

void ListViewCtrl::OnColumnClick(wxListEvent& event)
{
	if(event.GetColumn() == sortColumn)
	{
		if (!ascending)
			setSort(-1, sortType);
		else
			setSort(sortColumn, sortType, false);
	} 
	else 
	{
		setSort(event.GetColumn(), SORT_STRING_NOCASE);
	}
}

void ListViewCtrl::setSort(int aColumn, int aType, bool aAscending) 
{
	bool doUpdateArrow = (aColumn != sortColumn || aAscending != ascending);
		
	sortColumn = aColumn;
	sortType = aType;
	ascending = aAscending;
	resort();
	/* TODO if (doUpdateArrow)
		updateArrow();*/
}

void ListViewCtrl::resort() 
{
	if(sortColumn != -1) 
	{
		SortItems(CompareFunc, (long)this);
	}
}

int wxCALLBACK ListViewCtrl::CompareFunc(long item1, long item2, wxIntPtr sortData)
{
	ListViewCtrl* p = (ListViewCtrl*) sortData;
	wxString buf, buf2;

	int na = (int)p->FindItem(-1, item1);
	int nb = (int)p->FindItem(-1, item2);

	int result = p->sortType;
	if(result == SORT_STRING) 
	{
		buf = p->GetItemText(na, p->sortColumn);
		buf2 = p->GetItemText(nb, p->sortColumn);
		result = buf.Cmp(buf2);
	} 
	else if(result == SORT_STRING_NOCASE) 
	{
		buf = p->GetItemText(na, p->sortColumn);
		buf2 = p->GetItemText(nb, p->sortColumn);
		result = buf.CmpNoCase(buf2);
	} 
	else if(result == SORT_INT) 
	{
		buf = p->GetItemText(na, p->sortColumn);
		buf2 = p->GetItemText(nb, p->sortColumn);
		result = compare(Util::toInt(buf), Util::toInt(buf2));
	} 
	else if(result == SORT_FLOAT) 
	{
		buf = p->GetItemText(na, p->sortColumn);
		buf2 = p->GetItemText(nb, p->sortColumn);
		result = compare(Util::toDouble(buf), Util::toDouble(buf2));
	} /* TODO else if(result == SORT_BYTES) {
		buf = p->GetItemText(na, p->sortColumn);
		buf2 = p->GetItemText(nb, p->sortColumn);
		result = compare(WinUtil::toBytes(buf), WinUtil::toBytes(buf2));
	}*/
	if(!p->ascending)
		result = -result;
	return result;
}