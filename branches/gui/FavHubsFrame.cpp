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
#include "FavHubsFrame.h"

#include "FavHubProperties.h"
#include "HubFrame.h"

#include "../client/FavoriteManager.h"
#include "../client/SettingsManager.h"
#include "../client/version.h"

#include <wx/wx.h>

BEGIN_EVENT_TABLE(FavHubsFrame, BaseType)
	EVT_CLOSE(FavHubsFrame::OnClose)
	EVT_BUTTON(IDC_CONNECT, FavHubsFrame::OnConnect)
	EVT_BUTTON(IDC_REMOVE, FavHubsFrame::OnRemove)
	EVT_BUTTON(IDC_EDIT, FavHubsFrame::OnEdit)
	EVT_BUTTON(IDC_NEWFAV, FavHubsFrame::OnNew)
	EVT_BUTTON(IDC_MOVE_UP, FavHubsFrame::OnMoveUp)
	EVT_BUTTON(IDC_MOVE_DOWN, FavHubsFrame::OnMoveDown)
	EVT_BUTTON(IDC_MANAGE_GROUPS, FavHubsFrame::OnManageGroups)
	EVT_MENU(IDC_OPEN_HUB_LOG, FavHubsFrame::OnOpenHubLog)
	EVT_MENU(IDC_CONNECT, FavHubsFrame::OnConnect)
	EVT_MENU(IDC_NEWFAV, FavHubsFrame::OnNew)
	EVT_MENU(IDC_MOVE_UP, FavHubsFrame::OnMoveUp)
	EVT_MENU(IDC_MOVE_DOWN, FavHubsFrame::OnMoveDown)
	EVT_MENU(IDC_REMOVE, FavHubsFrame::OnRemove)
	EVT_MENU(IDC_EDIT, FavHubsFrame::OnEdit)
	EVT_LIST_ITEM_SELECTED(ID_LIST_CTRL, FavHubsFrame::OnItemSelected)
	EVT_LIST_ITEM_DESELECTED(ID_LIST_CTRL, FavHubsFrame::OnItemDeselected)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_LIST_CTRL, FavHubsFrame::OnContextMenu)
	EVT_LIST_ITEM_ACTIVATED(ID_LIST_CTRL, FavHubsFrame::OnActivate)
END_EVENT_TABLE()

int FavHubsFrame::columnIndexes[]	= { COLUMN_NAME, COLUMN_DESCRIPTION, COLUMN_NICK, COLUMN_PASSWORD, COLUMN_SERVER, COLUMN_USERDESCRIPTION };
int FavHubsFrame::columnSizes[]		= { 200, 290, 125, 100, 100, 125 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_CONNECT, ResourceManager::DESCRIPTION, ResourceManager::NICK, 
	ResourceManager::PASSWORD, ResourceManager::SERVER, ResourceManager::USER_DESCRIPTION
};

FavHubsFrame::FavHubsFrame(void) : BaseType(ID_CMD_FAVORITES, CTSTRING(FAVORITE_HUBS), wxColour(0, 0, 0), wxT("IDI_FAVORITES")), nosave(true)
{
	ctrlHubs = new ListViewCtrl(this, ID_LIST_CTRL, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxHSCROLL | wxLC_REPORT);
	ctrlHubs->SetBackgroundColour(WinUtil::bgColor);
	ctrlHubs->SetForegroundColour(WinUtil::textColor);

#ifdef __WXMSW__
	/* TODO: use groups for Windows build only now
	 *			rewrite it later to some portable way (maybe wxDataViewTreeCtrl?)
	 */
	::SendMessage((HWND)ctrlHubs->GetHWND(), LVM_ENABLEGROUPVIEW, TRUE, 0L);

	LVGROUPMETRICS metrics = {0};
	metrics.cbSize = sizeof(metrics);
	metrics.mask = LVGMF_TEXTCOLOR;
	metrics.crHeader = SETTING(TEXT_GENERAL_FORE_COLOR);
	::SendMessage((HWND)ctrlHubs->GetHWND(), LVM_SETGROUPMETRICS, 0, (LPARAM)&metrics);
#endif
	
	/* TODO
	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(FAVORITESFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(FAVORITESFRAME_WIDTHS), COLUMN_LAST);
	*/

	for(int j = 0; j < COLUMN_LAST; ++j)
	{
		int fmt = wxLIST_FORMAT_LEFT;
		ctrlHubs->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j]);
	}
/*	
	ctrlHubs.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
*/

	ctrlConnect = new wxButton(this, IDC_CONNECT, CTSTRING(CONNECT), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlConnect->Enable(false);
	ctrlConnect->SetFont(WinUtil::font);

	ctrlNew = new wxButton(this, IDC_NEWFAV, CTSTRING(NEW), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlNew->SetFont(WinUtil::font);

	ctrlProps = new wxButton(this, IDC_EDIT, CTSTRING(PROPERTIES), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlProps->Enable(false);
	ctrlProps->SetFont(WinUtil::font);

	ctrlRemove = new wxButton(this, IDC_REMOVE, CTSTRING(REMOVE), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlRemove->Enable(false);
	ctrlRemove->SetFont(WinUtil::font);

	ctrlUp = new wxButton(this, IDC_MOVE_UP, CTSTRING(MOVE_UP), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlUp->SetFont(WinUtil::font);

	ctrlDown = new wxButton(this, IDC_MOVE_DOWN, CTSTRING(MOVE_DOWN), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlDown->SetFont(WinUtil::font);

	ctrlManageGroups = new wxButton(this, IDC_MANAGE_GROUPS, CTSTRING(MANAGE_GROUPS), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlManageGroups->SetFont(WinUtil::font);

	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonsSizer->Add(ctrlNew);
	buttonsSizer->Add(ctrlProps);
	buttonsSizer->Add(ctrlRemove, 0, wxRIGHT, 5);
	buttonsSizer->Add(ctrlUp);
	buttonsSizer->Add(ctrlDown, 0, wxRIGHT, 5);
	buttonsSizer->Add(ctrlConnect, 0, wxRIGHT, 5);
	buttonsSizer->Add(ctrlManageGroups);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(ctrlHubs, 1, wxEXPAND);
	sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);

	SetSizerAndFit(sizer);

	FavoriteManager::getInstance()->addListener(this);
	SettingsManager::getInstance()->addListener(this);

	fillList();

	nosave = false;
}

FavHubsFrame::~FavHubsFrame(void)
{
}

FavHubsFrame::StateKeeper::StateKeeper(ListViewCtrl& hubs_, bool ensureVisible_) :
	hubs(hubs_), ensureVisible(ensureVisible_)
{
	hubs.Freeze();

	// in grouped mode, the indexes of each item are completely random, so use entry pointers instead
	int i = -1;
	while( (i = hubs.GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1) 
	{
		selected.push_back((FavoriteHubEntry*)hubs.GetItemData(i));
	}

	scroll = hubs.GetScrollPos(wxVERTICAL);
}

FavHubsFrame::StateKeeper::~StateKeeper() 
{
	// restore visual updating now, otherwise it doesn't always scroll
	hubs.Thaw();

	for(FavoriteHubEntryList::const_iterator i = selected.begin(), iend = selected.end(); i != iend; ++i) 
	{
		for(int j = 0; j < hubs.GetItemCount(); ++j) {
			if((FavoriteHubEntry*)hubs.GetItemData(j) == *i)
			{
				hubs.SetItemState(j, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
				if(ensureVisible)
					hubs.EnsureVisible(j);
				break;
			}
		}
	}

	hubs.ScrollList(0, scroll);
}

const FavoriteHubEntryList& FavHubsFrame::StateKeeper::getSelection() const 
{
	return selected;
}

void FavHubsFrame::OnClose(wxCloseEvent& event)
{
	FavoriteManager::getInstance()->removeListener(this);
	SettingsManager::getInstance()->removeListener(this);

	WinUtil::setButtonPressed(GetId(), false);
	event.Skip();
}

void FavHubsFrame::UpdateLayout()
{
}

void FavHubsFrame::addEntry(const FavoriteHubEntry* entry, int pos, int groupIndex)
{
	TStringList l;
	l.push_back(Text::toT(entry->getName()));
	l.push_back(Text::toT(entry->getDescription()));
	l.push_back(Text::toT(entry->getNick(false)));
	l.push_back(tstring(entry->getPassword().size(), '*'));
	l.push_back(Text::toT(entry->getServer()));
	l.push_back(Text::toT(entry->getUserDescription()));	

// TODO	bool b = entry->getConnect();
	int i = ctrlHubs->insert(pos, l, 0, (void*)entry);

	//ctrlHubs.SetCheckState(i, b);
#ifdef __WXMSW__
	LVITEM lvItem = { 0 };
    lvItem.mask = LVIF_GROUPID;
    lvItem.iItem = i;
    lvItem.iSubItem = 0;
    lvItem.iGroupId = groupIndex;
	::SendMessage((HWND)ctrlHubs->GetHWND(), LVM_SETITEM, 0, (LPARAM)&lvItem);
#endif
}

TStringList FavHubsFrame::getSortedGroups() const 
{
	set<tstring, noCaseStringLess> sorted_groups;
	const FavHubGroups& favHubGroups = FavoriteManager::getInstance()->getFavHubGroups();
	for(FavHubGroups::const_iterator i = favHubGroups.begin(), iend = favHubGroups.end(); i != iend; ++i)
		sorted_groups.insert(Text::toT(i->first));

	TStringList groups(sorted_groups.begin(), sorted_groups.end());
	groups.insert(groups.begin(), Util::emptyStringT); // default group (otherwise, hubs without group don't show up)
	return groups;
}

void FavHubsFrame::fillList()
{
	bool old_nosave = nosave;
	nosave = true;

	ctrlHubs->DeleteAllItems();
	
	// sort groups
	TStringList groups(getSortedGroups());

#ifdef __WXMSW__
	for(size_t i = 0; i < groups.size(); ++i) 
	{
		// insert groups
		LVGROUP lg = {0};
		lg.cbSize = sizeof(lg);
		lg.iGroupId = i;
		lg.state = LVGS_NORMAL | (wxPlatformInfo::Get().GetOSMajorVersion()  >= 6 ? LVGS_COLLAPSIBLE : 0);
		lg.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
		lg.uAlign = LVGA_HEADER_LEFT;

		// Header-title must be unicode (Convert if necessary)
		lg.pszHeader = (LPWSTR)groups[i].c_str();
		lg.cchHeader = groups[i].size();
		::SendMessage((HWND)ctrlHubs->GetHWND(), LVM_INSERTGROUP, i, (LPARAM)&lg);
	}
#endif

	const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();
	for(FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) 
	{
		const string& group = (*i)->getGroup();

		int index = 0;
		if(!group.empty()) 
		{
			TStringList::const_iterator groupI = find(groups.begin() + 1, groups.end(), Text::toT(group));
			if(groupI != groups.end())
				index = groupI - groups.begin();
		}

		addEntry(*i, ctrlHubs->GetItemCount(), index);
	}

	nosave = old_nosave;
}

void FavHubsFrame::openSelected()
{
	int i = -1;
	while( (i = ctrlHubs->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1) 
	{
		FavoriteHubEntry* entry = (FavoriteHubEntry*)ctrlHubs->GetItemData(i);

		if(!checkNick(entry))
			return;

		RecentHubEntry r;
		r.setName(entry->getName());
		r.setDescription(entry->getDescription());
		r.setUsers("*");
		r.setShared("*");
		r.setServer(entry->getServer());
		FavoriteManager::getInstance()->addRecent(r);

		HubFrame::openWindow(Text::toT(entry->getServer()), entry->getRawOne(), entry->getRawTwo(), entry->getRawThree(),
			entry->getRawFour(), entry->getRawFive(), entry->getChatUserSplit(), entry->getUserListState());
	}
}

void FavHubsFrame::OnRemove(wxCommandEvent& /*event*/)
{
	if(!BOOLSETTING(CONFIRM_HUB_REMOVAL) || wxMessageBox(CTSTRING(REALLY_REMOVE), wxT(APPNAME) wxT(" ") wxT(VERSIONSTRING), wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT) == wxYES) 
	{
		int i = -1;
		while( (i = ctrlHubs->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1) 
		{
			FavoriteManager::getInstance()->removeFavorite((FavoriteHubEntry*)ctrlHubs->GetItemData(i));
		}
	}
}

void FavHubsFrame::OnEdit(wxCommandEvent& /*event*/)
{
	int i = -1;
	if((i = ctrlHubs->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1)
	{
		FavoriteHubEntry* e = (FavoriteHubEntry*)ctrlHubs->GetItemData(i);
		dcassert(e != NULL);
		FavHubProperties dlg(this, e);
		if (dlg.ShowModal() == wxID_OK)
		{
			StateKeeper keeper(*ctrlHubs);
			fillList();
		}
	}
}

void FavHubsFrame::OnNew(wxCommandEvent& /*event*/)
{
	FavoriteHubEntry e;
	FavHubProperties dlg(this, &e);
	if (dlg.ShowModal() == wxID_OK)
	{
		if (FavoriteManager::getInstance()->isFavoriteHub(e.getServer()))
		{
			wxMessageBox(CTSTRING(FAVORITE_HUB_ALREADY_EXISTS), _T(""), wxICON_ERROR | wxOK);
		}
		else
		{
			FavoriteManager::getInstance()->addFavorite(e);
			fillList();
		}
	}
}

void FavHubsFrame::OnMoveUp(wxCommandEvent& /*event*/)
{
	handleMove(true);
}

void FavHubsFrame::OnMoveDown(wxCommandEvent& /*event*/)
{
	handleMove(false);
}

void FavHubsFrame::handleMove(bool up)
{
	FavoriteHubEntryList& fh = FavoriteManager::getInstance()->getFavoriteHubs();
	if(fh.size() <= 1)
		return;
	
	StateKeeper keeper(*ctrlHubs);
	const FavoriteHubEntryList& selected = keeper.getSelection();

	FavoriteHubEntryList fh_copy = fh;
	if(!up)
		reverse(fh_copy.begin(), fh_copy.end());
	FavoriteHubEntryList moved;
	for(FavoriteHubEntryList::iterator i = fh_copy.begin(); i != fh_copy.end(); ++i) 
	{
		if(find(selected.begin(), selected.end(), *i) == selected.end())
			continue;
		if(find(moved.begin(), moved.end(), *i) != moved.end())
			continue;
		const string& group = (*i)->getGroup();
		for(FavoriteHubEntryList::iterator j = i; ;) 
		{
			if(j == fh_copy.begin()) 
			{
				// couldn't move within the same group; change group.
				TStringList groups(getSortedGroups());
				if(!up)
					reverse(groups.begin(), groups.end());
				TStringIterC ig = find(groups.begin(), groups.end(), Text::toT(group));
				if(ig != groups.begin()) 
				{
					FavoriteHubEntryPtr f = *i;
					f->setGroup(Text::fromT(*(ig - 1)));
					i = fh_copy.erase(i);
					fh_copy.push_back(f);
					moved.push_back(f);
				}
				break;
			}
			if((*--j)->getGroup() == group)
			{
				swap(*i, *j);
				break;
			}
		}
	}
	if(!up)
		reverse(fh_copy.begin(), fh_copy.end());
	fh = fh_copy;
	FavoriteManager::getInstance()->save();

	fillList();
}

void FavHubsFrame::OnManageGroups(wxCommandEvent& /*event*/)
{
	// TODO
}

void FavHubsFrame::OnOpenHubLog(wxCommandEvent& /*event*/)
{
	int i = -1;
	while( (i = ctrlHubs->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1) 
	{
		FavoriteHubEntry* entry = (FavoriteHubEntry*)ctrlHubs->GetItemData(i);
		StringMap params;
		params["hubNI"] = entry->getName();
		params["hubURL"] = entry->getServer();
		params["myNI"] = entry->getNick(); 

		tstring file = Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_MAIN_CHAT), params, false)));
		if(Util::fileExists(Text::fromT(file)))
		{
			WinUtil::openFile(file);
		} 
		else 
		{
			wxMessageBox(CTSTRING(NO_LOG_FOR_HUB), CTSTRING(NO_LOG_FOR_HUB), wxOK);	  
		}
	}
}

void FavHubsFrame::OnItemSelected(wxListEvent& /*event*/)
{
	ctrlConnect->Enable(true);
	ctrlRemove->Enable(true);
	ctrlProps->Enable(true);
}

void FavHubsFrame::OnItemDeselected(wxListEvent& /*event*/)
{
	ctrlConnect->Enable(false);
	ctrlRemove->Enable(false);
	ctrlProps->Enable(false);
}

void FavHubsFrame::OnContextMenu(wxListEvent& event)
{
	wxPoint pt = event.GetPoint();

	wxMenu mnu;
	
	if (ctrlHubs->GetSelectedItemCount() == 1) 
	{
		FavoriteHubEntry* f = (FavoriteHubEntry*)ctrlHubs->GetItemData(ctrlHubs->GetFirstSelected());
		mnu.SetTitle(Text::toT(f->getName())); // TODO: limit max title size to avoid huge menus
	}

	mnu.Append(IDC_OPEN_HUB_LOG, CTSTRING(OPEN_HUB_LOG));
	mnu.AppendSeparator();
	mnu.Append(IDC_CONNECT, CTSTRING(CONNECT));
	mnu.Append(IDC_NEWFAV, CTSTRING(NEW));
	mnu.Append(IDC_MOVE_UP, CTSTRING(MOVE_UP));
	mnu.Append(IDC_MOVE_DOWN, CTSTRING(MOVE_DOWN));
	mnu.AppendSeparator();
	mnu.Append(IDC_REMOVE, CTSTRING(REMOVE));
	mnu.AppendSeparator();
	mnu.Append(IDC_EDIT, CTSTRING(PROPERTIES));

	PopupMenu(&mnu, pt.x, pt.y); // TODO: TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN
}

bool FavHubsFrame::checkNick(const FavoriteHubEntry* entry) 
{
	if(SETTING(NICK).empty() && entry->getNick().empty()) 
	{
		wxMessageBox(CTSTRING(ENTER_NICK), wxT(APPNAME) _T(" ") _T(VERSIONSTRING), wxICON_STOP | wxOK);
		return false;
	}
	return true;
}
