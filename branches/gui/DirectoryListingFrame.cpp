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
#include "DirectoryListingFrame.h"

#include "WinUtil.h"

#include "../client/ClientManager.h"
#include "../client/QueueManager.h"

BEGIN_EVENT_TABLE(DirectoryListingFrame, MDIChildFrame)
	EVT_CLOSE(DirectoryListingFrame::OnClose)
	EVT_TREE_SEL_CHANGED(wxID_ANY, DirectoryListingFrame::OnTreeSelChanged)
	EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, DirectoryListingFrame::OnTreeContextMenu)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(ID_LIST_CTRL, DirectoryListingFrame::OnFilesContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(ID_LIST_CTRL, DirectoryListingFrame::OnDblClick)
	EVT_BUTTON(IDC_FILELIST_DIFF, DirectoryListingFrame::OnListDiff)
	EVT_BUTTON(IDC_MATCH_QUEUE, DirectoryListingFrame::OnMatchQueue)
	EVT_BUTTON(IDC_FIND, DirectoryListingFrame::OnFind)
	EVT_BUTTON(IDC_NEXT, DirectoryListingFrame::OnFindNext)

	EVT_MENU(ID_DOWNLOAD, DirectoryListingFrame::OnDownload)
	EVT_MENU(ID_DOWNLOADTO, DirectoryListingFrame::OnDownloadTo)
	EVT_MENU(ID_DOWNLOADDIR, DirectoryListingFrame::OnDownloadDir)
	EVT_MENU(ID_DOWNLOADDIRTO, DirectoryListingFrame::OnDownloadDirTo)
	EVT_MENU(ID_VIEW_AS_TEXT, DirectoryListingFrame::OnViewAsText)
	EVT_MENU(ID_SEARCH_ALTERNATES, DirectoryListingFrame::OnSearchForAlternates)
	EVT_MENU(IDC_GO_TO_DIRECTORY, DirectoryListingFrame::OnGoToDirectory)
	EVT_MENU_RANGE(IDC_PRIORITY_PAUSED, IDC_PRIORITY_HIGHEST, DirectoryListingFrame::OnPriority)
	EVT_MENU_RANGE(IDC_PRIORITY_PAUSED+90, IDC_PRIORITY_HIGHEST+90, DirectoryListingFrame::OnPriorityDir)
END_EVENT_TABLE()

int DirectoryListingFrame::columnIndexes[] = { COLUMN_FILENAME, COLUMN_TYPE, COLUMN_EXACTSIZE, COLUMN_SIZE, COLUMN_TTH };
int DirectoryListingFrame::columnSizes[] = { 300, 60, 100, 100, 200 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILE, ResourceManager::TYPE, ResourceManager::EXACT_SIZE, ResourceManager::SIZE, ResourceManager::TTH_ROOT };

DirectoryListingFrame::UserMap DirectoryListingFrame::lists;

DirectoryListingFrame::DirectoryListingFrame(const HintedUser& aUser, int64_t aSpeed) : MDIChildFrame(wxID_ANY, "DirectoryListingFrame", wxColour(255, 0, 255), wxT("IDI_DIRECTORY")),
	loading(true), searching(false), updating(false), dl(new DirectoryListing(aUser)), speed(aSpeed)
{
	CreateStatusBar(STATUS_LAST, wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE);
	memzero(statusSizes, sizeof(statusSizes));
	statusSizes[STATUS_TEXT] = -1;
	statusSizes[STATUS_FILE_LIST_DIFF] = WinUtil::getTextWidth(TSTRING(FILE_LIST_DIFF), GetStatusBar()) + 8;
	statusSizes[STATUS_MATCH_QUEUE] = WinUtil::getTextWidth(TSTRING(MATCH_QUEUE), GetStatusBar()) + 8;
	statusSizes[STATUS_FIND] = WinUtil::getTextWidth(TSTRING(FIND), GetStatusBar()) + 8;
	statusSizes[STATUS_NEXT] = WinUtil::getTextWidth(TSTRING(NEXT), GetStatusBar()) + 8;
	
	splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxSP_3D | wxSP_LIVE_UPDATE);

	ctrlDirs = new wxTreeCtrl(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxTR_DEFAULT_STYLE);
	ctrlDirs->SetFont(WinUtil::font);
	ctrlDirs->SetBackgroundColour(WinUtil::bgColor);
	ctrlDirs->SetForegroundColour(WinUtil::textColor);
	ctrlDirs->SetImageList(&WinUtil::fileImages);

	string nick = ClientManager::getInstance()->getNicks(dl->getHintedUser())[0];
	ctrlDirs->AddRoot(nick, WinUtil::getDirIconIndex(), WinUtil::getDirIconIndex(), new TreeData(dl->getRoot()));
	ctrlDirs->Enable(false);

	ctrlFiles = new TypedDataViewCtrl<ItemInfo>(splitter, ID_LIST_CTRL, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxHSCROLL);
	ctrlFiles->SetFont(WinUtil::font);
	ctrlFiles->SetBackgroundColour(WinUtil::bgColor);
	ctrlFiles->SetForegroundColour(WinUtil::textColor);
	ctrlFiles->setImageList(&WinUtil::fileImages);

	for(uint8_t j = 0; j < COLUMN_LAST; j++) 
	{
		wxAlignment fmt = ((j == COLUMN_SIZE) || (j == COLUMN_EXACTSIZE) || (j == COLUMN_TYPE)) ? wxALIGN_RIGHT : wxALIGN_LEFT;
		ctrlFiles->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j]);
	}

	ctrlFiles->setSortColumn(COLUMN_FILENAME);

	splitter->SplitVertically(ctrlDirs, ctrlFiles, GetClientRect().GetWidth() / 4);
	splitter->SetMinimumPaneSize(1);

	ctrlListDiff = new wxButton(GetStatusBar(), IDC_FILELIST_DIFF, TSTRING(FILE_LIST_DIFF), wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlMatchQueue = new wxButton(GetStatusBar(), IDC_MATCH_QUEUE, TSTRING(MATCH_QUEUE), wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlFind = new wxButton(GetStatusBar(), IDC_FIND, TSTRING(FIND), wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlFindNext = new wxButton(GetStatusBar(), IDC_NEXT, TSTRING(NEXT), wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(splitter, 1, wxEXPAND);
	SetSizerAndFit(sizer);

	SetTitle(WinUtil::getNicks(dl->getHintedUser()) + _T(" - ") + WinUtil::getHubNames(dl->getHintedUser()).first);
}

DirectoryListingFrame::~DirectoryListingFrame(void)
{
	dcassert(lists.find(dl->getUser()) != lists.end());
	lists.erase(dl->getUser());
}

void DirectoryListingFrame::openWindow(const tstring& aFile, const tstring& aDir, const HintedUser& aUser, int64_t aSpeed)
{
	UserIter i = lists.find(aUser);
	if(i == lists.end()) 
	{
		DirectoryListingFrame* frm = new DirectoryListingFrame(aUser, aSpeed);
		frm->Show();
		lists[aUser] = frm;

		frm->loadFile(aFile, aDir);
	} 
	else 
	{
		if(i->second->IsIconized())
			i->second->Restore();

		i->second->Activate();
	}
}

void DirectoryListingFrame::openWindow(const HintedUser& aUser, const string& txt, int64_t aSpeed)
{
	UserIter i = lists.find(aUser);
	if(i == lists.end()) 
	{
		DirectoryListingFrame* frm = new DirectoryListingFrame(aUser, aSpeed);
		frm->Show();
		lists[aUser] = frm;

		frm->loadXML(txt);
	} 
	else 
	{
		i->second->speed = aSpeed;
		i->second->loadXML(txt);

		if(i->second->IsIconized())
			i->second->Restore();

		i->second->Activate();
	}
}

void DirectoryListingFrame::OnClose(wxCloseEvent& event)
{
	if(loading)
	{
		//tell the thread to abort and wait until we get a notification
		//that it's done.
		dl->setAbort(true);		
		return;
	}

	int j = ctrlFiles->getItemCount();
	for(int i = 0; i < j; ++i)
	{
		delete ctrlFiles->getItemData(i);
	}

	event.Skip();
}

void DirectoryListingFrame::OnFinished()
{
	loading = false;
	initStatus();
	GetStatusBar()->SetStatusText(TSTRING(LOADED_FILE_LIST), STATUS_TEXT);
	ctrlDirs->Enable(true);

	//notify the user that we've loaded the list
	setDirty();
}

void DirectoryListingFrame::OnAborted()
{
	loading = false;
	Close();
}

void DirectoryListingFrame::OnTreeSelChanged(wxTreeEvent& event)
{
	if(event.GetItem().IsOk() && ctrlDirs->IsSelected(event.GetItem()))
	{
		DirectoryListing::Directory* d = ((TreeData*)ctrlDirs->GetItemData(event.GetItem()))->dir;
		changeDir(d, true);
	}
}

void DirectoryListingFrame::OnTreeContextMenu(wxTreeEvent& /*event*/)
{
	wxMenu directoryMenu;
	wxMenu* targetDirMenu = new wxMenu();
	wxMenu* priorityDirMenu = new wxMenu();

	directoryMenu.Append(IDC_DOWNLOADDIR, CTSTRING(DOWNLOAD));
	directoryMenu.AppendSubMenu(targetDirMenu, CTSTRING(DOWNLOAD_TO));
	directoryMenu.AppendSubMenu(priorityDirMenu, CTSTRING(DOWNLOAD_WITH_PRIORITY));

	priorityDirMenu->SetTitle(CTSTRING(DOWNLOAD_WITH_PRIORITY));
	priorityDirMenu->Append(IDC_PRIORITY_PAUSED+90, CTSTRING(PAUSED));
	priorityDirMenu->Append(IDC_PRIORITY_LOWEST+90, CTSTRING(LOWEST));
	priorityDirMenu->Append(IDC_PRIORITY_LOW+90, CTSTRING(LOW));
	priorityDirMenu->Append(IDC_PRIORITY_NORMAL+90, CTSTRING(NORMAL));
	priorityDirMenu->Append(IDC_PRIORITY_HIGH+90, CTSTRING(HIGH));
	priorityDirMenu->Append(IDC_PRIORITY_HIGHEST+90, CTSTRING(HIGHEST));

	// Strange, windows doesn't change the selection on right-click... (!)

	targetDirMenu->SetTitle(CTSTRING(DOWNLOAD_TO));
	int n = 0;
	//Append Favorite download dirs
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if (spl.size() > 0) 
	{
		for(StringPairIter i = spl.begin(); i != spl.end(); i++) 
		{
			targetDirMenu->Append(ID_DOWNLOAD_WHOLE_FAVORITE_DIRS + n, Text::toT(i->second).c_str());
			n++;
		}
		targetDirMenu->AppendSeparator();
	}

	n = 0;
	targetDirMenu->Append(ID_DOWNLOADDIRTO, CTSTRING(BROWSE));

	if(WinUtil::lastDirs.size() > 0) 
	{
		targetDirMenu->AppendSeparator();
		for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) 
		{
			targetDirMenu->Append(IDC_DOWNLOAD_TARGET_DIR + (++n), i->c_str());
		}
	}
			
	directoryMenu.SetTitle(TSTRING(DIRECTORY));
	PopupMenu(&directoryMenu);
}

void DirectoryListingFrame::OnFilesContextMenu(wxDataViewEvent& /*event*/)
{
	wxMenu fileMenu;
	wxMenu* targetMenu = new wxMenu();
	wxMenu* priorityMenu = new wxMenu();

	targetMenu->SetTitle(CTSTRING(DOWNLOAD_TO));

	fileMenu.Append(ID_DOWNLOAD, CTSTRING(DOWNLOAD));
	fileMenu.AppendSubMenu(targetMenu, CTSTRING(DOWNLOAD_TO));
	fileMenu.AppendSubMenu(priorityMenu, CTSTRING(DOWNLOAD_WITH_PRIORITY));
	fileMenu.Append(ID_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT));
	fileMenu.Append(ID_SEARCH_ALTERNATES, CTSTRING(SEARCH_FOR_ALTERNATES));
	fileMenu.AppendSeparator();
	fileMenu.AppendSubMenu(ctrlFiles->getCopyMenu(), CTSTRING(COPY));

	priorityMenu->SetTitle(CTSTRING(DOWNLOAD_WITH_PRIORITY));
	priorityMenu->Append(IDC_PRIORITY_PAUSED, CTSTRING(PAUSED));
	priorityMenu->Append(IDC_PRIORITY_LOWEST, CTSTRING(LOWEST));
	priorityMenu->Append(IDC_PRIORITY_LOW, CTSTRING(LOW));
	priorityMenu->Append(IDC_PRIORITY_NORMAL, CTSTRING(NORMAL));
	priorityMenu->Append(IDC_PRIORITY_HIGH, CTSTRING(HIGH));
	priorityMenu->Append(IDC_PRIORITY_HIGHEST, CTSTRING(HIGHEST));

	int n = 0;

	wxDataViewItemArray sel;
	ctrlFiles->GetSelections(sel);

	const ItemInfo* ii = ctrlFiles->getItemData(sel[0]);

	if(sel.size() == 1 && ii->type == ItemInfo::FILE) 
	{
		fileMenu.SetTitle(Text::toT(Util::getFileName(ii->file->getName())));
			
		//Append Favorite download dirs
		StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
		if (spl.size() > 0) 
		{
			for(StringPairIter i = spl.begin(); i != spl.end(); i++) 
			{
				targetMenu->Append(ID_DOWNLOAD_FAVORITE_DIRS + n, Text::toT(i->second).c_str());
				n++;
			}
			targetMenu->AppendSeparator();
		}

		n = 0;
		targetMenu->Append(ID_DOWNLOADTO, CTSTRING(BROWSE));
		targets.clear();
		QueueManager::getInstance()->getTargets(ii->file->getTTH(), targets);

		if(targets.size() > 0) 
		{
			targetMenu->AppendSeparator();
			for(StringIter i = targets.begin(); i != targets.end(); ++i) 
			{
				targetMenu->Append(ID_DOWNLOAD_TARGET + (++n), Text::toT(*i).c_str());
			}
		}
		if(WinUtil::lastDirs.size() > 0) 
		{
			targetMenu->AppendSeparator();
			for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) 
			{
				targetMenu->Append(ID_DOWNLOAD_TARGET + (++n), i->c_str());
			}
		}

		if(ii->file->getAdls())
		{
			fileMenu.Append(IDC_GO_TO_DIRECTORY, CTSTRING(GO_TO_DIRECTORY));
		}

		prepareMenu(fileMenu, UserCommand::CONTEXT_FILELIST, ClientManager::getInstance()->getHubs(dl->getHintedUser().user->getCID(), dl->getHintedUser().hint));
		PopupMenu(&fileMenu);
	} 
	else 
	{
		fileMenu.Enable(ID_SEARCH_ALTERNATES, false);

		//Append Favorite download dirs
		StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
		if (spl.size() > 0) 
		{
			for(StringPairIter i = spl.begin(); i != spl.end(); i++) 
			{
				targetMenu->Append(ID_DOWNLOAD_FAVORITE_DIRS + n, Text::toT(i->second).c_str());
				n++;
			}
			targetMenu->AppendSeparator();
		}

		n = 0;
		targetMenu->Append(ID_DOWNLOADTO, CTSTRING(BROWSE));
		if(WinUtil::lastDirs.size() > 0) 
		{
			targetMenu->AppendSeparator();
			for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) 
			{
				targetMenu->Append(ID_DOWNLOAD_TARGET + (++n), i->c_str());
			}
		}
		
		if(ii->type == ItemInfo::DIRECTORY)
		{
			if(sel.size() == 1)
				fileMenu.SetTitle(Text::toT(Util::getFileName(ii->dir->getName())));

			if(ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()) 
				fileMenu.Append(IDC_GO_TO_DIRECTORY, CTSTRING(GO_TO_DIRECTORY));
		}

		prepareMenu(fileMenu, UserCommand::CONTEXT_FILELIST, ClientManager::getInstance()->getHubs(dl->getUser()->getCID(), dl->getHintedUser().hint));
		PopupMenu(&fileMenu);
	}
}

void DirectoryListingFrame::UpdateLayout()
{
	GetStatusBar()->SetFieldsCount(STATUS_LAST, statusSizes);

	wxRect rc;
	
	// position buttons in statusbar
	GetStatusBar()->GetFieldRect(STATUS_FILE_LIST_DIFF, rc);
	ctrlListDiff->SetSize(rc);

	GetStatusBar()->GetFieldRect(STATUS_MATCH_QUEUE, rc);
	ctrlMatchQueue->SetSize(rc);

	GetStatusBar()->GetFieldRect(STATUS_FIND, rc);
	ctrlFind->SetSize(rc);

	GetStatusBar()->GetFieldRect(STATUS_NEXT, rc);
	ctrlFindNext->SetSize(rc);
}

void DirectoryListingFrame::initStatus() 
{
	tstring tmp = TSTRING(FILES) + _T(": ") + Util::toStringW(dl->getTotalFileCount(true));
	statusSizes[STATUS_TOTAL_FILES] = WinUtil::getTextWidth(tmp, GetStatusBar());
	GetStatusBar()->SetStatusText(tmp, STATUS_TOTAL_FILES);

	tmp = TSTRING(SIZE) + _T(": ") + Util::formatBytesW(dl->getTotalSize(true));
	statusSizes[STATUS_TOTAL_SIZE] = WinUtil::getTextWidth(tmp, GetStatusBar());
	GetStatusBar()->SetStatusText(tmp, STATUS_TOTAL_SIZE);

	tmp = TSTRING(SPEED) + _T(": ") + Util::formatBytesW(speed) + _T("/s");
	statusSizes[STATUS_SPEED] = WinUtil::getTextWidth(tmp, GetStatusBar());
	GetStatusBar()->SetStatusText(tmp, STATUS_SPEED);

	UpdateLayout();
}

void DirectoryListingFrame::updateStatus() 
{
	if(!searching && !updating) 
	{
		wxDataViewItemArray sel;
		int cnt = ctrlFiles->GetSelections(sel);
		int64_t total = 0;
		if(cnt == 0) 
		{
			cnt = ctrlFiles->getItemCount();
			total = ctrlFiles->forEachT(ItemInfo::TotalSize()).total;
		} 
		else 
		{
			total = ctrlFiles->forEachSelectedT(ItemInfo::TotalSize()).total;
		}

		tstring tmp = TSTRING(ITEMS) + _T(": ") + Util::toStringW(cnt);
		bool u = false;

		int w = WinUtil::getTextWidth(tmp, GetStatusBar());
		if(statusSizes[STATUS_SELECTED_FILES] < w)
		{
			statusSizes[STATUS_SELECTED_FILES] = w;
			u = true;
		}
		GetStatusBar()->SetStatusText(tmp, STATUS_SELECTED_FILES);

		tmp = TSTRING(SIZE) + _T(": ") + Util::formatBytesW(total);
		w = WinUtil::getTextWidth(tmp, GetStatusBar());
		if(statusSizes[STATUS_SELECTED_SIZE] < w) 
		{
			statusSizes[STATUS_SELECTED_SIZE] = w;
			u = true;
		}
		GetStatusBar()->SetStatusText(tmp, STATUS_SELECTED_SIZE);

		if(u)
			UpdateLayout();
	}
}

void DirectoryListingFrame::loadFile(const tstring& name, const tstring& dir)
{
	GetStatusBar()->SetStatusText(TSTRING(LOADING_FILE_LIST), STATUS_TEXT);

	//don't worry about cleanup, the object will delete itself once the thread has finished it's job
	ThreadedDirectoryListing* tdl = new ThreadedDirectoryListing(this, Text::fromT(name), Util::emptyString, dir);
	loading = true;
	tdl->start();
}

void DirectoryListingFrame::loadXML(const string& txt) 
{
	GetStatusBar()->SetStatusText(TSTRING(LOADING_FILE_LIST), STATUS_TEXT);

	//don't worry about cleanup, the object will delete itself once the thread has finished it's job
	ThreadedDirectoryListing* tdl = new ThreadedDirectoryListing(this, Util::emptyString, txt);
	loading = true;
	tdl->start();
}

void DirectoryListingFrame::refreshTree(const tstring& root)
{
	if(!loading) 
	{
		throw AbortException();
	}

	ctrlDirs->Freeze();

	// find root
	wxTreeItemId rootItem = findItem(ctrlDirs->GetRootItem(), root);
	if(!rootItem.IsOk())
		rootItem = ctrlDirs->GetRootItem();
	
	DirectoryListing::Directory* d = ((TreeData*)ctrlDirs->GetItemData(rootItem))->dir;

	// delete old tree
	ctrlDirs->DeleteChildren(rootItem);

	// create new tree
	updateTree(d, rootItem);

	ctrlDirs->Expand(rootItem);

	int index = d->getComplete() ? WinUtil::getDirIconIndex() : WinUtil::getDirMaskedIndex();
	ctrlDirs->SetItemImage(rootItem, index, wxTreeItemIcon_Normal);
	ctrlDirs->SetItemImage(rootItem, index, wxTreeItemIcon_Selected);

	ctrlDirs->EnsureVisible(rootItem);

	// raise TreeSelChanged event to update file list with dir content
	wxTreeEvent changedEvent(wxEVT_COMMAND_TREE_SEL_CHANGED, ctrlDirs, rootItem);
	ctrlDirs->GetEventHandler()->ProcessEvent(changedEvent);

	ctrlDirs->Thaw();
}

void DirectoryListingFrame::updateTree(DirectoryListing::Directory* tree, const wxTreeItemId& parent)
{
	for(DirectoryListing::Directory::Iter i = tree->directories.begin(); i != tree->directories.end(); ++i) 
	{
		if(!loading) 
		{
			throw AbortException();
		}

		int iconIndex = (*i)->getComplete() ? WinUtil::getDirIconIndex() : WinUtil::getDirMaskedIndex();
		wxTreeItemId item = ctrlDirs->AppendItem(parent, Text::toT((*i)->getName()), iconIndex, iconIndex, new TreeData(*i));

		if((*i)->getAdls())
			ctrlDirs->SetItemBold(item);

		updateTree(*i, item);
	}

	ctrlDirs->SortChildren(parent);
}

void DirectoryListingFrame::changeDir(DirectoryListing::Directory* d, bool enableRedraw)
{
	ctrlFiles->Freeze();

	updating = true;
	int j = ctrlFiles->getItemCount();
	for(int i = 0; i < j; ++i)
	{
		delete ctrlFiles->getItemData(i);
	}

	ctrlFiles->deleteAllItems();

	for(DirectoryListing::Directory::Iter i = d->directories.begin(); i != d->directories.end(); ++i) 
	{
		ctrlFiles->insertItem(ctrlFiles->getItemCount(), new ItemInfo(*i));
	}

	for(DirectoryListing::File::Iter j = d->files.begin(); j != d->files.end(); ++j) 
	{
		ItemInfo* ii = new ItemInfo(*j);
		ctrlFiles->insertItem(ctrlFiles->getItemCount(), ii);
	}
	
	ctrlFiles->resort();
	
	if(enableRedraw)
		ctrlFiles->Thaw();
	
	updating = false;
	updateStatus();

	if(!d->getComplete()) 
	{
		if(dl->getUser()->isOnline()) 
		{
			try 
			{
				QueueManager::getInstance()->addList(dl->getHintedUser(), QueueItem::FLAG_PARTIAL_LIST, dl->getPath(d));
				GetStatusBar()->SetStatusText(TSTRING(DOWNLOADING_LIST), STATUS_TEXT);
			} 
			catch(const QueueException& e) 
			{
				GetStatusBar()->SetStatusText(Text::toT(e.getError()), STATUS_TEXT);
			}
		} 
		else 
		{
			GetStatusBar()->SetStatusText(TSTRING(USER_OFFLINE), STATUS_TEXT);
		}
	}	
}

wxTreeItemId DirectoryListingFrame::findItem(const wxTreeItemId& ht, const tstring& name) const
{
	string::size_type i = name.find('\\');
	if(i == string::npos)
		return ht;
	
	wxTreeItemIdValue cookie;
	for(wxTreeItemId child = ctrlDirs->GetFirstChild(ht, cookie); child.IsOk(); child = ctrlDirs->GetNextSibling(child)) 
	{
		DirectoryListing::Directory* d = ((TreeData*)ctrlDirs->GetItemData(child))->dir;
		if(Text::toT(d->getName()) == name.substr(0, i)) 
		{
			return findItem(child, name.substr(i+1));
		}
	}

	return wxTreeItemId();
}

void DirectoryListingFrame::OnDblClick(wxDataViewEvent& event)
{
	wxTreeItemId t = ctrlDirs->GetSelection();
	if(t.IsOk() && event.GetItem().IsOk()) 
	{
		const ItemInfo* ii = ctrlFiles->getItemData(event.GetItem());

		if(ii->type == ItemInfo::FILE) 
		{
			try 
			{
				dl->download(ii->file, SETTING(DOWNLOAD_DIRECTORY) + Text::fromT(ii->getText(COLUMN_FILENAME)), false, wxGetKeyState(WXK_SHIFT), QueueItem::DEFAULT);
			} 
			catch(const Exception& e) 
			{
				GetStatusBar()->SetStatusText(Text::toT(e.getError()), STATUS_TEXT);
			}
		} 
		else 
		{
			wxTreeItemIdValue cookie;
			wxTreeItemId ht = ctrlDirs->GetFirstChild(t, cookie);
			while(ht.IsOk()) 
			{
				if(((TreeData*)ctrlDirs->GetItemData(ht))->dir == ii->dir) 
				{
					ctrlDirs->SelectItem(ht);
					break;
				}
				ht = ctrlDirs->GetNextSibling(ht);
			}
		}
	}
}

void DirectoryListingFrame::OnListDiff(wxCommandEvent& /*event*/)
{
	tstring file;
	if(WinUtil::browseFile(file, this, false, Text::toT(Util::getListPath()), _T("File Lists\0*.xml.bz2\0All Files\0*.*\0"))) 
	{
		DirectoryListing dirList(dl->getHintedUser());
		try 
		{
			dirList.loadFile(Text::fromT(file));
			dl->getRoot()->filterList(dirList);
			loading = true;
			refreshTree(Util::emptyStringT);
			loading = false;
			initStatus();
			updateStatus();
		} 
		catch(const Exception&) 
		{
			/// @todo report to user?
		}
	}
}

void DirectoryListingFrame::OnMatchQueue(wxCommandEvent& /*event*/)
{
	int x = QueueManager::getInstance()->matchListing(*dl);
	GetStatusBar()->SetStatusText(wxString::Format(CTSTRING(MATCHED_FILES), x), STATUS_TEXT);
}

void DirectoryListingFrame::OnFind(wxCommandEvent& /*event*/)
{
}

void DirectoryListingFrame::OnFindNext(wxCommandEvent& /*event*/)
{
}

void DirectoryListingFrame::OnDownload(wxCommandEvent& /*event*/)
{
	downloadList(Text::toT(SETTING(DOWNLOAD_DIRECTORY)));
}

void DirectoryListingFrame::OnDownloadTo(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	if(ctrlFiles->GetSelections(sel) == 1) 
	{
		const ItemInfo* ii = ctrlFiles->getItemData(sel[0]);

		try 
		{
			if(ii->type == ItemInfo::FILE) 
			{
				tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY)) + ii->getText(COLUMN_FILENAME);
				if(WinUtil::browseFile(target, this)) 
				{
					WinUtil::addLastDir(Util::getFilePath(target));
					dl->download(ii->file, Text::fromT(target), false, wxGetKeyState(WXK_SHIFT), QueueItem::DEFAULT);
				}
			} 
			else 
			{
				tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
				if(WinUtil::browseDirectory(target, this)) 
				{
					WinUtil::addLastDir(target);
					dl->download(ii->dir, Text::fromT(target), wxGetKeyState(WXK_SHIFT), QueueItem::DEFAULT);
				}
			} 
		} 
		catch(const Exception& e) 
		{
			GetStatusBar()->SetStatusText(Text::toT(e.getError()), STATUS_TEXT);
		}
	} 
	else 
	{
		tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
		if(WinUtil::browseDirectory(target, this)) 
		{
			WinUtil::addLastDir(target);			
			downloadList(target);
		}
	}

}

void DirectoryListingFrame::OnDownloadDir(wxCommandEvent& /*event*/)
{
	wxTreeItemId t = ctrlDirs->GetSelection();
	if(t.IsOk()) 
	{
		DirectoryListing::Directory* dir = ((TreeData*)ctrlDirs->GetItemData(t))->dir;
		try 
		{
			dl->download(dir, SETTING(DOWNLOAD_DIRECTORY), wxGetKeyState(WXK_SHIFT), QueueItem::DEFAULT);
		} 
		catch(const Exception& e) 
		{
			GetStatusBar()->SetStatusText(Text::toT(e.getError()), STATUS_TEXT);
		}
	}
}

void DirectoryListingFrame::OnDownloadDirTo(wxCommandEvent& /*event*/)
{
	wxTreeItemId t = ctrlDirs->GetSelection();
	if(t.IsOk()) 
	{
		DirectoryListing::Directory* dir = ((TreeData*)ctrlDirs->GetItemData(t))->dir;

		tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
		if(WinUtil::browseDirectory(target, this)) 
		{
			WinUtil::addLastDir(target);
			
			try 
			{
				dl->download(dir, Text::fromT(target), wxGetKeyState(WXK_SHIFT), QueueItem::DEFAULT);
			} 
			catch(const Exception& e) 
			{
				GetStatusBar()->SetStatusText(Text::toT(e.getError()), STATUS_TEXT);
			}
		}
	}
}

void DirectoryListingFrame::OnViewAsText(wxCommandEvent& /*event*/)
{
	downloadList(Text::toT(Util::getTempPath()), true);
}

void DirectoryListingFrame::OnSearchForAlternates(wxCommandEvent& /*event*/)
{
	const ItemInfo* ii = ctrlFiles->getItemData(ctrlFiles->GetSelection());
	if(ii != NULL && ii->type == ItemInfo::FILE) 
	{
		WinUtil::searchHash(ii->file->getTTH());
	} 
}

void DirectoryListingFrame::OnGoToDirectory(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	if(ctrlFiles->GetSelections(sel) != 1) 
		return;

	tstring fullPath;
	const ItemInfo* ii = ctrlFiles->getItemData(sel[0]);
	if(ii->type == ItemInfo::FILE) 
	{
		if(!ii->file->getAdls())
			return;
		
		DirectoryListing::Directory* pd = ii->file->getParent();
		while(pd != NULL && pd != dl->getRoot()) 
		{
			fullPath = Text::toT(pd->getName()) + _T("\\") + fullPath;
			pd = pd->getParent();
		}
	} 
	else if(ii->type == ItemInfo::DIRECTORY)
	{
		if(!(ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()))
			return;
		fullPath = Text::toT(((DirectoryListing::AdlDirectory*)ii->dir)->getFullPath());
	}

	// TODO selectItem(fullPath);
}

void DirectoryListingFrame::OnPriority(wxCommandEvent& event)
{
	QueueItem::Priority p;

	switch(event.GetId()) 
	{
	case IDC_PRIORITY_PAUSED: p = QueueItem::PAUSED; break;
	case IDC_PRIORITY_LOWEST: p = QueueItem::LOWEST; break;
	case IDC_PRIORITY_LOW: p = QueueItem::LOW; break;
	case IDC_PRIORITY_NORMAL: p = QueueItem::NORMAL; break;
	case IDC_PRIORITY_HIGH: p = QueueItem::HIGH; break;
	case IDC_PRIORITY_HIGHEST: p = QueueItem::HIGHEST; break;
	default: p = QueueItem::DEFAULT; break;
	}

	downloadList(Text::toT(SETTING(DOWNLOAD_DIRECTORY)), false, p);
}

void DirectoryListingFrame::OnPriorityDir(wxCommandEvent& event)
{
	wxTreeItemId t = ctrlDirs->GetSelection();
	if(t.IsOk()) 
	{
		DirectoryListing::Directory* dir = ((TreeData*)ctrlDirs->GetItemData(t))->dir;

		QueueItem::Priority p;
		switch(event.GetId() - 90) 
		{
			case IDC_PRIORITY_PAUSED: p = QueueItem::PAUSED; break;
			case IDC_PRIORITY_LOWEST: p = QueueItem::LOWEST; break;
			case IDC_PRIORITY_LOW: p = QueueItem::LOW; break;
			case IDC_PRIORITY_NORMAL: p = QueueItem::NORMAL; break;
			case IDC_PRIORITY_HIGH: p = QueueItem::HIGH; break;
			case IDC_PRIORITY_HIGHEST: p = QueueItem::HIGHEST; break;
			default: p = QueueItem::DEFAULT; break;
		}

		try 
		{
			dl->download(dir, SETTING(DOWNLOAD_DIRECTORY), wxGetKeyState(WXK_SHIFT), p);
		} 
		catch(const Exception& e) 
		{
			GetStatusBar()->SetStatusText(Text::toT(e.getError()), STATUS_TEXT);
		}
	}
}

void DirectoryListingFrame::downloadList(const tstring& aTarget, bool view, QueueItem::Priority prio)
{
	string target = aTarget.empty() ? SETTING(DOWNLOAD_DIRECTORY) : Text::fromT(aTarget);

	wxDataViewItemArray sel;
	ctrlFiles->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const ItemInfo* ii = ctrlFiles->getItemData(sel[i]);

		try 
		{
			if(ii->type == ItemInfo::FILE) 
			{
				if(view) 
				{
					File::deleteFile(target + Util::validateFileName(ii->file->getName()));
				}

				dl->download(ii->file, target + Text::fromT(ii->getText(COLUMN_FILENAME)), view, wxGetKeyState(WXK_SHIFT) || view, prio);
			} 
			else if(!view) 
			{
				dl->download(ii->dir, target, wxGetKeyState(WXK_SHIFT), prio);
			} 
		} 
		catch(const Exception& e) 
		{
			GetStatusBar()->SetStatusText(Text::toT(e.getError()), STATUS_TEXT);
		}
	}
}

void DirectoryListingFrame::runUserCommand(const UserCommand& uc) 
{
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	set<UserPtr> nicks;

	wxDataViewItemArray sel;
	ctrlFiles->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const ItemInfo* ii = ctrlFiles->getItemData(sel[i]);
		if(uc.once()) {
			if(nicks.find(dl->getUser()) != nicks.end())
				continue;
			nicks.insert(dl->getUser());
		}
		if(!dl->getUser()->isOnline())
			return;
		ucParams["fileTR"] = "NONE";
		if(ii->type == ItemInfo::FILE) {
			ucParams["type"] = "File";
			ucParams["fileFN"] = dl->getPath(ii->file) + ii->file->getName();
			ucParams["fileSI"] = Util::toString(ii->file->getSize());
			ucParams["fileSIshort"] = Util::formatBytes(ii->file->getSize());
			ucParams["fileTR"] = ii->file->getTTH().toBase32();
		} else {
			ucParams["type"] = "Directory";
			ucParams["fileFN"] = dl->getPath(ii->dir) + ii->dir->getName();
			ucParams["fileSI"] = Util::toString(ii->dir->getTotalSize());
			ucParams["fileSIshort"] = Util::formatBytes(ii->dir->getTotalSize());
		}

		// compatibility with 0.674 and earlier
		ucParams["file"] = ucParams["fileFN"];
		ucParams["filesize"] = ucParams["fileSI"];
		ucParams["filesizeshort"] = ucParams["fileSIshort"];
		ucParams["tth"] = ucParams["fileTR"];

		StringMap tmp = ucParams;
		UserPtr tmpPtr = dl->getUser();
		ClientManager::getInstance()->userCommand(dl->getHintedUser(), uc, tmp, true);
	}
}