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
#include "PublicHubsFrame.h"

#include "CountryFlags.h"
#include "HubFrame.h"
#include "PublicHubListDlg.h"

#include "../client/FavoriteManager.h"
#include "../client/version.h"

#include <wx/wx.h>

BEGIN_EVENT_TABLE(PublicHubsFrame, BaseType)
	EVT_CLOSE(PublicHubsFrame::OnClose)
	EVT_TEXT_ENTER(ID_FILTER_TEXTBOX, PublicHubsFrame::OnFilterEnter)
	EVT_BUTTON(IDC_REFRESH, PublicHubsFrame::OnRefresh)
	EVT_BUTTON(IDC_PUB_LIST_CONFIG, PublicHubsFrame::OnConfigure)

	EVT_COMBOBOX(IDC_PUB_LIST_DROPDOWN, PublicHubsFrame::OnListSelection)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(ID_LIST_CTRL, PublicHubsFrame::OnContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(ID_LIST_CTRL, PublicHubsFrame::OnDblClick)

	EVT_MENU(IDC_CONNECT, PublicHubsFrame::OnConnect)
	EVT_MENU(IDC_ADD, PublicHubsFrame::OnAdd)
	EVT_MENU_RANGE(IDC_COPY, IDC_COPY + PublicHubsFrame::COLUMN_LAST, PublicHubsFrame::OnCopy)
END_EVENT_TABLE()

int PublicHubsFrame::columnIndexes[] = { COLUMN_NAME, COLUMN_DESCRIPTION, COLUMN_USERS, COLUMN_SERVER, COLUMN_COUNTRY, 
	COLUMN_SHARED, COLUMN_MINSHARE, COLUMN_MINSLOTS, COLUMN_MAXHUBS, COLUMN_MAXUSERS, COLUMN_RELIABILITY, COLUMN_RATING };

int PublicHubsFrame::columnSizes[] = { 200, 290, 50, 100, 100, 100, 100, 100, 100, 100, 100, 100 };

static ResourceManager::Strings columnNames[] = { ResourceManager::HUB_NAME, ResourceManager::DESCRIPTION, ResourceManager::USERS, 
	ResourceManager::HUB_ADDRESS, ResourceManager::COUNTRY, ResourceManager::SHARED, ResourceManager::MIN_SHARE, ResourceManager::MIN_SLOTS,
	ResourceManager::MAX_HUBS, ResourceManager::MAX_USERS, ResourceManager::RELIABILITY, ResourceManager::RATING };

int PublicHubsFrame::HubInfo::compareItems(const HubInfo* a, const HubInfo* b, int col)
{
	switch(col) 
	{
		case COLUMN_USERS: return compare(a->entry->getUsers(), b->entry->getUsers());
		case COLUMN_MINSLOTS: return compare(a->entry->getMinSlots(), b->entry->getMinSlots());
		case COLUMN_MAXHUBS: return compare(a->entry->getMaxHubs(), b->entry->getMaxHubs());
		case COLUMN_MAXUSERS: return compare(a->entry->getMaxUsers(), b->entry->getMaxUsers());
		case COLUMN_RELIABILITY: return compare(a->entry->getReliability(), b->entry->getReliability());
		case COLUMN_SHARED: return compare(a->entry->getShared(), b->entry->getShared());
		case COLUMN_MINSHARE: return compare(a->entry->getMinShare(), b->entry->getMinShare());
		case COLUMN_RATING: return compare(a->entry->getRating(), b->entry->getRating());

		case COLUMN_NAME: return lstrcmpi(Text::toT(a->entry->getName()).c_str(), Text::toT(b->entry->getName()).c_str());
		case COLUMN_DESCRIPTION: return lstrcmpi(Text::toT(a->entry->getDescription()).c_str(), Text::toT(b->entry->getDescription()).c_str());
		case COLUMN_SERVER: return lstrcmpi(Text::toT(a->entry->getServer()).c_str(), Text::toT(b->entry->getServer()).c_str());
		case COLUMN_COUNTRY: return lstrcmpi(Text::toT(a->entry->getCountry()).c_str(), Text::toT(b->entry->getCountry()).c_str());
		default: return 0;
	}
}

wxString PublicHubsFrame::HubInfo::getText(int column) const
{
	switch(column)
	{
		case COLUMN_NAME: return Text::toT(entry->getName());
		case COLUMN_DESCRIPTION: return Text::toT(entry->getDescription());
		case COLUMN_USERS: return Util::toStringW(entry->getUsers());
		case COLUMN_SERVER: return Text::toT(entry->getServer());
		case COLUMN_COUNTRY: return Text::toT(entry->getCountry());
		case COLUMN_SHARED: return Util::formatBytesW(entry->getShared());
		case COLUMN_MINSHARE: return Util::formatBytesW(entry->getMinShare());
		case COLUMN_MINSLOTS: return Util::toStringW(entry->getMinSlots());
		case COLUMN_MAXHUBS: return Util::toStringW(entry->getMaxHubs());
		case COLUMN_MAXUSERS: return Util::toStringW(entry->getMaxUsers());
		case COLUMN_RELIABILITY: return Util::toStringW(entry->getReliability());
		case COLUMN_RATING: return Text::toT(entry->getRating());
		default: return wxEmptyString;
	}
}

uint8_t PublicHubsFrame::HubInfo::getImageIndex() const
{
	return CountryFlags::getFlagIndexByName(entry->getCountry().c_str());
}

PublicHubsFrame::PublicHubsFrame(void) : BaseType(ID_CMD_CONNECT, CTSTRING(PUBLIC_HUBS), wxColour(0, 0, 0), wxT("IDI_PUBLICHUBS"))
{
	CreateStatusBar(3, wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	ctrlHubs = new HubList(this, ID_LIST_CTRL, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxHSCROLL);
	ctrlHubs->SetBackgroundColour(WinUtil::bgColor);
	ctrlHubs->SetForegroundColour(WinUtil::textColor);
	ctrlHubs->setImageList(&CountryFlags::getFlagsImages());

	sizer->Add(ctrlHubs, 1, wxEXPAND, 0);

	wxBoxSizer* bottomPanel = new wxBoxSizer(wxHORIZONTAL);
	
	wxStaticBoxSizer* box1 = new wxStaticBoxSizer(wxHORIZONTAL, this, TSTRING(FILTER));
	wxStaticBoxSizer* box2 = new wxStaticBoxSizer(wxHORIZONTAL, this, TSTRING(CONFIGURED_HUB_LISTS));
	
	ctrlFilter = new wxTextCtrl(this, ID_FILTER_TEXTBOX, wxEmptyString, wxDefaultPosition, wxSize(0, -1), wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxTE_PROCESS_ENTER);
	ctrlFilter->SetFont(WinUtil::font);
	ctrlFilter->SetBackgroundColour(WinUtil::bgColor);
	ctrlFilter->SetForegroundColour(WinUtil::textColor);

	ctrlFilterSel = new wxComboBox(this, ID_FILTER_COMBOBOX, wxEmptyString, wxDefaultPosition, wxSize(0, -1), 0, NULL, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxCB_READONLY);
	ctrlFilterSel->SetFont(WinUtil::font);
	ctrlFilterSel->SetBackgroundColour(WinUtil::bgColor);
	ctrlFilterSel->SetForegroundColour(WinUtil::textColor);

	box1->Add(ctrlFilter, 2, wxRIGHT | wxBOTTOM | wxEXPAND, 1);
	box1->Add(ctrlFilterSel, 1, wxLEFT | wxBOTTOM | wxEXPAND, 1);

	ctrlHubLists = new wxComboBox(this, IDC_PUB_LIST_DROPDOWN, wxEmptyString, wxDefaultPosition, wxSize(0, -1), 0, NULL, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxCB_READONLY);
	ctrlHubLists->SetFont(WinUtil::font);
	ctrlHubLists->SetBackgroundColour(WinUtil::bgColor);
	ctrlHubLists->SetForegroundColour(WinUtil::textColor);

	box2->Add(ctrlHubLists, 1, wxRIGHT | wxBOTTOM | wxEXPAND, 1);
	box2->Add(new wxButton(this, IDC_PUB_LIST_CONFIG, CTSTRING(CONFIGURE), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN), 0, wxRIGHT | wxBOTTOM, 1);

	wxButton* refresh = new wxButton(this, IDC_REFRESH, wxEmptyString, wxDefaultPosition, wxSize(30, 30), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	wxBitmap bmp(wxT("IDB_TOOLBAR20"));
	WinUtil::premultiplyAlpha(bmp);
	refresh->SetBitmap(bmp.GetSubBitmap(wxRect(20, 0, 20, 20)));

	bottomPanel->Add(box1, 1, wxALL | wxEXPAND, 2);
	bottomPanel->Add(box2, 1, wxALL | wxEXPAND, 2);
	bottomPanel->Add(refresh, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
	sizer->Add(bottomPanel, 0, wxEXPAND, 0);

	SetSizerAndFit(sizer);

	for(int j = 0; j < COLUMN_LAST; ++j) 
	{
		wxAlignment fmt = (j == COLUMN_USERS) ? wxALIGN_RIGHT : wxALIGN_LEFT;
		ctrlHubs->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j]);
		ctrlFilterSel->Append(CTSTRING_I(columnNames[j]));
	}

	ctrlFilterSel->Append(TSTRING(ANY));
	ctrlFilterSel->SetSelection(COLUMN_LAST);
	ctrlHubs->setSortColumn(COLUMN_USERS, false);

	FavoriteManager::getInstance()->addListener(this);

	hubs = FavoriteManager::getInstance()->getPublicHubs();
	if(FavoriteManager::getInstance()->isDownloading())
		GetStatusBar()->SetStatusText(TSTRING(DOWNLOADING_HUB_LIST), 0);
	else if(hubs.empty())
		FavoriteManager::getInstance()->refresh();

	updateDropDown();
	updateList();
}

PublicHubsFrame::~PublicHubsFrame(void)
{
}

void PublicHubsFrame::OnClose(wxCloseEvent& event)
{
	FavoriteManager::getInstance()->removeListener(this);

	unsigned int n = ctrlHubs->getItemCount();
	for(unsigned int i = 0; i < n; ++i)
		delete ctrlHubs->getItemData(i);

	ctrlHubs->deleteAllItems();

	WinUtil::setButtonPressed(GetId(), false);
	event.Skip();
}

void PublicHubsFrame::OnFinished(const tstring& str)
{
	hubs = FavoriteManager::getInstance()->getPublicHubs();
	updateList();
		
	GetStatusBar()->SetStatusText(str, 0);
} 

void PublicHubsFrame::OnFilterEnter(wxCommandEvent& /*event*/)
{
	filter = ctrlFilter->GetValue();
	updateList();
}

void PublicHubsFrame::OnRefresh(wxCommandEvent& /*event*/)
{
	GetStatusBar()->SetStatusText(CTSTRING(DOWNLOADING_HUB_LIST), 0);
	FavoriteManager::getInstance()->refresh(true);
	updateDropDown();
}

void PublicHubsFrame::OnConfigure(wxCommandEvent& /*event*/)
{
	PublicHubListDlg dlg(this);
	dlg.ShowModal();
}

void PublicHubsFrame::OnListSelection(wxCommandEvent& /*event*/)
{
	FavoriteManager::getInstance()->setHubList(ctrlHubLists->GetSelection());
	hubs = FavoriteManager::getInstance()->getPublicHubs();
	updateList();
}

void PublicHubsFrame::OnContextMenu(wxDataViewEvent& /*event*/)
{
	wxMenu menu;

	int i = ctrlHubs->getFirstSelectedItem();
	if (i != -1) 
	{
		HubInfo* f = ctrlHubs->getItemData(i);
		menu.SetTitle(f->getText(COLUMN_NAME)); // TODO: limit max title size to avoid huge menus
	}

	menu.Append(IDC_CONNECT, CTSTRING(CONNECT));
	menu.Append(IDC_ADD, CTSTRING(ADD_TO_FAVORITES));
	menu.AppendSubMenu(ctrlHubs->getCopyMenu(), TSTRING(COPY));

	PopupMenu(&menu);
}

void PublicHubsFrame::OnDblClick(wxDataViewEvent& /*event*/)
{
	wxCommandEvent evt;
	OnConnect(evt);
}

void PublicHubsFrame::OnConnect(wxCommandEvent& /*event*/)
{
	if(!checkNick())
		return;

	wxDataViewItemArray sel;
	ctrlHubs->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		HubInfo* hub = ctrlHubs->getItemData(sel[i]);

		/*RecentHubEntry r;
		ctrlHubs.GetItemText(i, COLUMN_NAME, buf, 256);
		r.setName(Text::fromT(buf));
		ctrlHubs.GetItemText(i, COLUMN_DESCRIPTION, buf, 256);
		r.setDescription(Text::fromT(buf));
		ctrlHubs.GetItemText(i, COLUMN_USERS, buf, 256);
		r.setUsers(Text::fromT(buf));
		ctrlHubs.GetItemText(i, COLUMN_SHARED, buf, 256);
		r.setShared(Text::fromT(buf));
		ctrlHubs.GetItemText(i, COLUMN_SERVER, buf, 256);
		r.setServer(Text::fromT(buf));
		FavoriteManager::getInstance()->addRecent(r);*/
				
		HubFrame::openWindow(tstring(hub->getText(COLUMN_SERVER)));
	}
}

void PublicHubsFrame::OnAdd(wxCommandEvent& /*event*/)
{
	if(!checkNick())
		return;
	
	wxDataViewItemArray sel;
	ctrlHubs->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const HubInfo* hub = ctrlHubs->getItemData(sel[i]);
		FavoriteManager::getInstance()->addFavorite(*hub->entry);
	}
}

void PublicHubsFrame::OnCopy(wxCommandEvent& event)
{	
	tstring sCopy;

	wxDataViewItemArray sel;
	ctrlHubs->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const HubInfo* hub = ctrlHubs->getItemData(sel[i]);
	
		if(!sCopy.empty())
			sCopy += _T("\r\n");

		sCopy += hub->getText(static_cast<uint8_t>(event.GetId() - IDC_COPY));
	}

	if (!sCopy.empty())
		WinUtil::setClipboard(sCopy);
}

void PublicHubsFrame::UpdateLayout()
{
	int widths[3] = { -1, 100, 100 };
	GetStatusBar()->SetFieldsCount(3, widths);
}

void PublicHubsFrame::updateList()
{
	unsigned int n = ctrlHubs->getItemCount();
	for(unsigned int i = 0; i < n; ++i)
		delete ctrlHubs->getItemData(i);

	ctrlHubs->deleteAllItems();

	unsigned int users = 0;
	unsigned int visibleHubs = 0;
	
	ctrlHubs->Freeze();
	
	double size = -1;
	FilterModes mode = NONE;

	int sel = ctrlFilterSel->GetSelection();

	bool doSizeCompare = parseFilter(mode, size);
	
	for(HubEntry::List::const_iterator i = hubs.begin(); i != hubs.end(); ++i) 
	{
		if(matchFilter(*i, sel, doSizeCompare, mode, size)) 
		{
			ctrlHubs->insertItem(new HubInfo(&(*i)));

			visibleHubs++;
			users += i->getUsers();
		}
	}
	
	ctrlHubs->Thaw();
	ctrlHubs->resort();

	// update statusbar
	GetStatusBar()->SetStatusText(TSTRING(HUBS) + _T(": ") + Util::toStringW(visibleHubs), 1);
	GetStatusBar()->SetStatusText(TSTRING(USERS) + _T(": ") + Util::toStringW(users), 2);
}

void PublicHubsFrame::updateDropDown() 
{
	ctrlHubLists->Clear();
	StringList lists(FavoriteManager::getInstance()->getHubLists());
	for(StringList::iterator idx = lists.begin(); idx != lists.end(); ++idx) 
	{
		ctrlHubLists->Append(Text::toT(*idx));
	}
	ctrlHubLists->SetSelection(FavoriteManager::getInstance()->getSelectedHubList());
}

bool PublicHubsFrame::checkNick() 
{
	if(SETTING(NICK).empty()) 
	{
		wxMessageBox(TSTRING(ENTER_NICK), wxT(APPNAME) wxT(" ") wxT(VERSIONSTRING), wxICON_STOP | wxOK);
		return false;
	}
	return true;
}

bool PublicHubsFrame::parseFilter(FilterModes& mode, double& size) 
{
	string::size_type start = (string::size_type)string::npos;
	string::size_type end = (string::size_type)string::npos;
	int64_t multiplier = 1;

	if(filter.empty()) 
		return false;

	if(filter.compare(0, 2, ">=") == 0) {
		mode = GREATER_EQUAL;
		start = 2;
	} else if(filter.compare(0, 2, "<=") == 0) {
		mode = LESS_EQUAL;
		start = 2;
	} else if(filter.compare(0, 2, "==") == 0) {
		mode = EQUAL;
		start = 2;
	} else if(filter.compare(0, 2, "!=") == 0) {
		mode = NOT_EQUAL;
		start = 2;
	} else if(filter[0] == _T('<')) {
		mode = LESS;
		start = 1;
	} else if(filter[0] == _T('>')) {
		mode = GREATER;
		start = 1;
	} else if(filter[0] == _T('=')) {
		mode = EQUAL;
		start = 1;
	}

	if(start == string::npos)
		return false;
	if(filter.length() <= start)
		return false;

	if((end = Util::findSubString(filter, "TiB")) != tstring::npos) {
		multiplier = 1024LL * 1024LL * 1024LL * 1024LL;
	} else if((end = Util::findSubString(filter, "GiB")) != tstring::npos) {
		multiplier = 1024*1024*1024;
	} else if((end = Util::findSubString(filter, "MiB")) != tstring::npos) {
		multiplier = 1024*1024;
	} else if((end = Util::findSubString(filter, "KiB")) != tstring::npos) {
		multiplier = 1024;
	} else if((end = Util::findSubString(filter, "TB")) != tstring::npos) {
		multiplier = 1000LL * 1000LL * 1000LL * 1000LL;
	} else if((end = Util::findSubString(filter, "GB")) != tstring::npos) {
		multiplier = 1000*1000*1000;
	} else if((end = Util::findSubString(filter, "MB")) != tstring::npos) {
		multiplier = 1000*1000;
	} else if((end = Util::findSubString(filter, "kB")) != tstring::npos) {
		multiplier = 1000;
	} else if((end = Util::findSubString(filter, "B")) != tstring::npos) {
		multiplier = 1;
	}
	

	if(end == string::npos) 
	{
		end = filter.length();
	}

	string tmpSize = filter.substr(start, end-start);
	size = Util::toDouble(tmpSize) * multiplier;

	return true;
}

bool PublicHubsFrame::matchFilter(const HubEntry& entry, const int& sel, bool doSizeCompare, const FilterModes& mode, const double& size) 
{
	if(filter.empty())
		return true;

	double entrySize = 0;
	string entryString = "";

	switch(sel) 
	{
		case COLUMN_NAME: entryString = entry.getName(); doSizeCompare = false; break;
		case COLUMN_DESCRIPTION: entryString = entry.getDescription(); doSizeCompare = false; break;
		case COLUMN_USERS: entrySize = entry.getUsers(); break;
		case COLUMN_SERVER: entryString = entry.getServer(); doSizeCompare = false; break;
		case COLUMN_COUNTRY: entryString = entry.getCountry(); doSizeCompare = false; break;
		case COLUMN_SHARED: entrySize = (double)entry.getShared(); break;
		case COLUMN_MINSHARE: entrySize = (double)entry.getMinShare(); break;
		case COLUMN_MINSLOTS: entrySize = entry.getMinSlots(); break;
		case COLUMN_MAXHUBS: entrySize = entry.getMaxHubs(); break;
		case COLUMN_MAXUSERS: entrySize = entry.getMaxUsers(); break;
		case COLUMN_RELIABILITY: entrySize = entry.getReliability(); break;
		case COLUMN_RATING: entryString = entry.getRating(); doSizeCompare = false; break;
		default: break;
	}

	bool insert = false;
	if(doSizeCompare) 
	{
		switch(mode) 
		{
			case EQUAL: insert = (size == entrySize); break;
			case GREATER_EQUAL: insert = (size <=  entrySize); break;
			case LESS_EQUAL: insert = (size >=  entrySize); break;
			case GREATER: insert = (size < entrySize); break;
			case LESS: insert = (size > entrySize); break;
			case NOT_EQUAL: insert = (size != entrySize); break;
		}
	} 
	else 
	{
		if(sel >= COLUMN_LAST) 
		{
			if( Util::findSubString(entry.getName(), filter) != string::npos ||
				Util::findSubString(entry.getDescription(), filter) != string::npos ||
				Util::findSubString(entry.getServer(), filter) != string::npos ||
				Util::findSubString(entry.getCountry(), filter) != string::npos ||
				Util::findSubString(entry.getRating(), filter) != string::npos ) 
				{
					insert = true;
				}
		}
		if(Util::findSubString(entryString, filter) != string::npos)
			insert = true;
	}

	return insert;
}

void PublicHubsFrame::on(DownloadStarting, const string& l) throw() 
{ 
	callAsync([l, this] { GetStatusBar()->SetStatusText(TSTRING(DOWNLOADING_HUB_LIST) + _T(" (") + Text::toT(l) + _T(")")); });
}

void PublicHubsFrame::on(DownloadFailed, const string& l) throw() 
{ 
	callAsync([l, this] { GetStatusBar()->SetStatusText(TSTRING(DOWNLOAD_FAILED) + _T(" ") + Text::toT(l)); });
}

void PublicHubsFrame::on(DownloadFinished, const string& l, bool /* TODO fromCoral*/) throw() 
{ 
	callAsync(std::bind(&PublicHubsFrame::OnFinished, this, TSTRING(HUB_LIST_DOWNLOADED) + _T(" (") + Text::toT(l) + _T(")")));
}

void PublicHubsFrame::on(LoadedFromCache, const string& l, const string& /* TODO d*/) throw() 
{ 
	callAsync(std::bind(&PublicHubsFrame::OnFinished, this, TSTRING(HUB_LIST_LOADED_FROM_CACHE) + _T(" (") + Text::toT(l) + _T(")"))); 
}

void PublicHubsFrame::on(Corrupted, const string& l) throw() 
{
	if (l.empty()) 
	{
		callAsync(std::bind(&PublicHubsFrame::OnFinished, this, TSTRING(HUBLIST_CACHE_CORRUPTED)));
	} 
	else 
	{	
		callAsync(std::bind(&PublicHubsFrame::OnFinished, this, TSTRING(HUBLIST_DOWNLOAD_CORRUPTED) + _T(" (") + Text::toT(l) + _T(")")));
	}
}