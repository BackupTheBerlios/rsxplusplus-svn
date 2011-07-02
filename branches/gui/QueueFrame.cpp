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
#include "QueueFrame.h"

#include "ProgressBarRenderer.h"

#include "../client/DCPlusPlus.h"
#include "../client/version.h"

#include <wx/splitter.h>

BEGIN_EVENT_TABLE(QueueFrame, BaseType)
	EVT_CLOSE(QueueFrame::OnClose)
	EVT_TREE_SEL_CHANGED(wxID_ANY, QueueFrame::OnTreeSelChanged)
END_EVENT_TABLE()

#define FILE_LIST_NAME _T("File Lists")

int QueueFrame::columnIndexes[] = { COLUMN_TARGET, COLUMN_STATUS, COLUMN_SEGMENTS, COLUMN_SIZE, COLUMN_PROGRESS, COLUMN_DOWNLOADED, COLUMN_PRIORITY,
	COLUMN_USERS, COLUMN_PATH, COLUMN_EXACT_SIZE, COLUMN_ERRORS, COLUMN_ADDED, COLUMN_TTH };

int QueueFrame::columnSizes[] = { 200, 300, 70, 75, 100, 120, 75, 200, 200, 75, 200, 100, 125 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::STATUS, ResourceManager::SEGMENTS, ResourceManager::SIZE, 
	ResourceManager::DOWNLOADED_PARTS, ResourceManager::DOWNLOADED, ResourceManager::PRIORITY, ResourceManager::USERS, ResourceManager::PATH, 
	ResourceManager::EXACT_SIZE, ResourceManager::ERRORS, ResourceManager::ADDED, ResourceManager::TTH_ROOT };

QueueFrame::QueueFrame(void) : BaseType(ID_CMD_QUEUE, CTSTRING(DOWNLOAD_QUEUE), wxColour(0, 0, 0), wxT("IDI_QUEUE")),
	queueSize(0), queueItems(0), dirty(false), showTree(BOOLSETTING(QUEUEFRAME_SHOW_TREE))
{
	CreateStatusBar(6, wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE);

	memset(statusSizes, 0, sizeof(statusSizes));
	statusSizes[0] = 16;
	statusSizes[1] = -1;

	ctrlShowTree = new wxCheckBox(GetStatusBar(), ID_SHOWLIST_CHECKBOX, wxEmptyString, wxPoint(6, 5), wxSize(16, 16), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlShowTree->SetValue(showTree);

	splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxSP_3D | wxSP_LIVE_UPDATE);

	ctrlDirs = new wxTreeCtrl(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxTR_DEFAULT_STYLE);
	ctrlDirs->SetFont(WinUtil::font);
	ctrlDirs->SetBackgroundColour(WinUtil::bgColor);
	ctrlDirs->SetForegroundColour(WinUtil::textColor);
	ctrlDirs->SetImageList(&WinUtil::fileImages);
	ctrlDirs->Connect(wxEVT_CHAR, wxKeyEventHandler(QueueFrame::OnDirsKeyDown), NULL, this);

	ctrlFiles = new TypedDataViewCtrl<QueueItemInfo>(splitter, ID_LIST_CTRL, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxHSCROLL);
	ctrlFiles->SetFont(WinUtil::font);
	ctrlFiles->SetBackgroundColour(WinUtil::bgColor);
	ctrlFiles->SetForegroundColour(WinUtil::textColor);
	ctrlFiles->setImageList(&WinUtil::fileImages);
	ctrlFiles->Connect(wxEVT_CHAR, wxKeyEventHandler(QueueFrame::OnFilesKeyDown), NULL, this);

	for(uint8_t j = 0; j < COLUMN_LAST; j++) 
	{
		wxAlignment fmt = (j == COLUMN_SIZE || j == COLUMN_DOWNLOADED || j == COLUMN_EXACT_SIZE || j == COLUMN_SEGMENTS) ? wxALIGN_RIGHT : wxALIGN_LEFT;
		wxDataViewRenderer* rndr = (j == COLUMN_PROGRESS) ? new ProgressBarRenderer<QueueFrame>() : NULL;
		ctrlFiles->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], rndr);
	}

	ctrlFiles->setSortColumn(COLUMN_TARGET);

	splitter->SplitVertically(ctrlDirs, ctrlFiles, GetClientRect().GetWidth() / 4);
	splitter->SetMinimumPaneSize(1);
	splitter->SetSashGravity(0.25);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(splitter, 1, wxEXPAND);
	SetSizerAndFit(sizer);

	addQueueList(QueueManager::getInstance()->lockQueue());
	QueueManager::getInstance()->unlockQueue();
	QueueManager::getInstance()->addListener(this);

	updateStatus();
}

QueueFrame::~QueueFrame(void)
{
}

void QueueFrame::OnClose(wxCloseEvent& event)
{
	WinUtil::setButtonPressed(GetId(), false);

	QueueManager::getInstance()->removeListener(this);

	wxTreeItemId ht = ctrlDirs->GetRootItem();
	while(ht.IsOk()) 
	{
		clearTree(ht);
		ht = ctrlDirs->GetNextSibling(ht);
	}

	SettingsManager::getInstance()->set(SettingsManager::QUEUEFRAME_SHOW_TREE, ctrlShowTree->IsChecked());
	for(DirectoryIter i = directories.begin(); i != directories.end(); ++i) 
	{
		delete i->second;
	}
	directories.clear();
	ctrlFiles->deleteAllItems();


	event.Skip();
}

void QueueFrame::OnFilesKeyDown(wxKeyEvent& event)
{
	switch(event.GetKeyCode())
	{
		case WXK_DELETE:
			removeSelected();
			break;
		case WXK_ADD:
			break;
		case WXK_SUBTRACT:
			break;
	}
}

void QueueFrame::OnDirsKeyDown(wxKeyEvent& event)
{
	if(event.GetKeyCode() == WXK_DELETE)
	{
		removeSelectedDir();
	}
}

void QueueFrame::OnAdded(QueueItemInfo* ii)
{
	dcassert(ctrlFiles->findItem(ii) == -1);
	addQueueItem(ii, false);
	updateStatus();
}

void QueueFrame::OnRemoved(const string& target)
{
	const QueueItemInfo* ii = getItemInfo(target);
	if(!ii) {
		dcassert(ii);
		return;
	}
			
	if(!showTree || isCurDir(ii->getPath()) )
	{
		dcassert(ctrlFiles->findItem(ii) != -1);
		ctrlFiles->deleteItem(ii);
	}
			
	bool userList = ii->isSet(QueueItem::FLAG_USER_LIST);
			
	if(!userList) 
	{
		queueSize -= ii->getSize();
		dcassert(queueSize >= 0);
	}
	queueItems--;
	dcassert(queueItems >= 0);
			
	pair<DirectoryIter, DirectoryIter> i = directories.equal_range(ii->getPath());
	DirectoryIter j;
	for(j = i.first; j != i.second; ++j) 
	{
		if(j->second == ii)
			break;
	}
	dcassert(j != i.second);
	directories.erase(j);
	if(directories.count(ii->getPath()) == 0) 
	{
		removeDirectory(ii->getPath(), ii->isSet(QueueItem::FLAG_USER_LIST));
		if(isCurDir(ii->getPath()))
			curDir.clear();
	}
			
	delete ii;
	updateStatus();
	if (!userList && BOOLSETTING(BOLD_QUEUE)) 
	{
		setDirty();
	}
	dirty = true;
}

void QueueFrame::OnUpdated(const string& target)
{
	QueueItemInfo* ii = getItemInfo(target);
	if(!ii)
		return;

	if(!showTree || isCurDir(ii->getPath())) 
	{
		int pos = ctrlFiles->findItem(ii);
		if(pos != -1)
		{
			/* FIXME: ctrlFiles->updateItem(pos, COLUMN_SEGMENTS);
			ctrlFiles->updateItem(pos, COLUMN_PROGRESS);
			ctrlFiles->updateItem(pos, COLUMN_PRIORITY);
			ctrlFiles->updateItem(pos, COLUMN_USERS);
			ctrlFiles->updateItem(pos, COLUMN_ERRORS);
			ctrlFiles->updateItem(pos, COLUMN_STATUS);
			ctrlFiles->updateItem(pos, COLUMN_DOWNLOADED);*/
			ctrlFiles->updateItem(pos);
		}
	}
}

void QueueFrame::OnTreeSelChanged(wxTreeEvent& event)
{
	updateQueue();
}

void QueueFrame::updateQueue() 
{
	ctrlFiles->deleteAllItems();
	pair<DirectoryIter, DirectoryIter> i;
	if(showTree) 
	{
		i = directories.equal_range(getSelectedDir());
	} 
	else 
	{
		i.first = directories.begin();
		i.second = directories.end();
	}

	ctrlFiles->Freeze();
	for(DirectoryIter j = i.first; j != i.second; ++j) {
		QueueItemInfo* ii = j->second;
		ctrlFiles->insertItem(ii);
	}
	ctrlFiles->resort();
	ctrlFiles->Thaw();
	curDir = getSelectedDir();
	updateStatus();
}

void QueueFrame::addQueueList(const QueueItem::StringMap& li) 
{
	ctrlFiles->Freeze();
	ctrlDirs->Freeze();
	for(QueueItem::StringMap::const_iterator j = li.begin(); j != li.end(); ++j) 
	{
		QueueItem* aQI = j->second;
		QueueItemInfo* ii = new QueueItemInfo(aQI);
		addQueueItem(ii, true);
	}
	ctrlFiles->resort();
	ctrlFiles->Thaw();
	ctrlDirs->Thaw();
}

void QueueFrame::addQueueItem(QueueItemInfo* ii, bool noSort) 
{
	if(!ii->isSet(QueueItem::FLAG_USER_LIST)) 
	{
		queueSize += ii->getSize();
	}
	queueItems++;
	dirty = true;
	
	const string& dir = ii->getPath();
	
	bool updateDir = (directories.find(dir) == directories.end());
	directories.insert(make_pair(dir, ii));
	
	if(updateDir) 
	{
		addDirectory(dir, ii->isSet(QueueItem::FLAG_USER_LIST));
	} 
	
	if(!showTree || isCurDir(dir)) 
	{
		if(noSort)
			ctrlFiles->insertItem(ctrlFiles->getItemCount(), ii);
		else
			ctrlFiles->insertItem(ii);
	}
}

QueueFrame::QueueItemInfo* QueueFrame::getItemInfo(const string& target) const 
{
	string path = Util::getFilePath(target);
	DirectoryPairC items = directories.equal_range(path);
	for(DirectoryIterC i = items.first; i != items.second; ++i) 
	{
		if(i->second->getTarget() == target) 
		{
			return i->second;
		}
	}
	return 0;
}

wxTreeItemId QueueFrame::addDirectory(const string& dir, bool isFileList /* = false */, wxTreeItemId startAt /* = NULL */) 
{
	int image = WinUtil::getDirIconIndex();
	wxTreeItemIdValue cookie;

/*	TVINSERTSTRUCT tvi;
	tvi.hInsertAfter = TVI_SORT;
	tvi.item.mask = TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	tvi.item.iImage = tvi.item.iSelectedImage = WinUtil::getDirIconIndex();

	if(BOOLSETTING(EXPAND_QUEUE)) {
		tvi.itemex.mask |= TVIF_STATE;
		tvi.itemex.state = TVIS_EXPANDED;
		tvi.itemex.stateMask = TVIS_EXPANDED;
	}
*/
	if(isFileList) 
	{
		// We assume we haven't added it yet, and that all filelists go to the same
		// directory...
		dcassert(!fileLists.IsOk());
		fileLists = ctrlDirs->AppendItem(wxTreeItemId(), FILE_LIST_NAME, image, image, new TreeData(dir));
		return fileLists;
	} 

	// More complicated, we have to find the last available tree item and then see...
	string::size_type i = 0;
	string::size_type j;

	wxTreeItemId next;
	wxTreeItemId parent;

	if(!startAt.IsOk()) 
	{
		// First find the correct drive letter
		dcassert(dir[1] == ':');
		dcassert(dir[2] == '\\');

		next = ctrlDirs->GetRootItem();

		while(next.IsOk()) 
		{
			if(next != fileLists) 
			{
				string stmp = ((TreeData*)ctrlDirs->GetItemData(next))->dir;
				if(strnicmp(stmp, dir, 3) == 0)
					break;
			}
			next = ctrlDirs->GetNextSibling(next);
		}

		if(!next.IsOk()) 
		{
			// First addition, set commonStart to the dir minus the last part...
			i = dir.rfind('\\', dir.length()-2);
			if(i != string::npos) 
			{
				tstring name = Text::toT(dir.substr(0, i));
				next = ctrlDirs->AppendItem(wxTreeItemId(), name, image, image, new TreeData(dir.substr(0, i + 1)));
			} 
			else 
			{
				dcassert(dir.length() == 3);
				tstring name = Text::toT(dir);
				next = ctrlDirs->AppendItem(wxTreeItemId(), name, image, image, new TreeData(dir));
			}
		} 

		// Ok, next now points to x:\... find how much is common

		TreeData* rootStr = (TreeData*)ctrlDirs->GetItemData(next);

		i = 0;

		for(;;) {
			j = dir.find('\\', i);
			if(j == string::npos)
				break;
			if(strnicmp(dir.c_str() + i, rootStr->dir.c_str() + i, j - i + 1) != 0)
				break;
			i = j + 1;
		}

		if(i < rootStr->dir.length()) 
		{
			wxTreeItemId oldRoot = next;

			// Create a new root
			tstring name = Text::toT(rootStr->dir.substr(0, i-1));
			wxTreeItemId newRoot = ctrlDirs->AppendItem(wxTreeItemId(), name, image, image, new TreeData(rootStr->dir.substr(0, i)));

			parent = addDirectory(rootStr->dir, false, newRoot);

			next = ctrlDirs->GetFirstChild(oldRoot, cookie);
			while(next.IsOk()) 
			{
				moveNode(next, parent);
				next = ctrlDirs->GetFirstChild(oldRoot, cookie);
			}
			delete rootStr;
			ctrlDirs->Delete(oldRoot);
			parent = newRoot;
		} 
		else 
		{
			// Use this root as parent
			parent = next;
			next = ctrlDirs->GetFirstChild(parent, cookie);
		}
	} 
	else 
	{
		parent = startAt;
		next = ctrlDirs->GetFirstChild(parent, cookie);
		i = getDir(parent).length();
		dcassert(strnicmp(getDir(parent), dir, getDir(parent).length()) == 0);
	}

	while( i < dir.length() ) 
	{
		while(next.IsOk()) 
		{
			if(next != fileLists) 
			{
				const string& n = getDir(next);
				if(strnicmp(n.c_str()+i, dir.c_str()+i, n.length()-i) == 0) 
				{
					// Found a part, we assume it's the best one we can find...
					i = n.length();

					parent = next;
					next = ctrlDirs->GetFirstChild(next, cookie);
					break;
				}
			}
			next = ctrlDirs->GetNextSibling(next);
		}

		if(!next.IsOk()) 
		{
			// We didn't find it, add...
			j = dir.find('\\', i);
			dcassert(j != string::npos);
			tstring name = Text::toT(dir.substr(i, j-i));
			parent = ctrlDirs->AppendItem(parent, name, image, image, new TreeData(dir.substr(0, j + 1)));

			i = j + 1;
		}
	}

	return parent;
}

void QueueFrame::clearTree(const wxTreeItemId& item) 
{
	wxTreeItemIdValue cookie;
	wxTreeItemId next = ctrlDirs->GetFirstChild(item, cookie);
	while(next.IsOk()) 
	{
		clearTree(next);
		next = ctrlDirs->GetNextSibling(next);
	}
	//delete ctrlDirs->GetItemData(item);
}

// Put it here to avoid a copy for each recursion...
static TCHAR tmpBuf[1024];
void QueueFrame::moveNode(const wxTreeItemId& item, const wxTreeItemId& parent) 
{
	wxTreeItemId ht = ctrlDirs->AppendItem(parent, ctrlDirs->GetItemText(item), ctrlDirs->GetItemImage(item), ctrlDirs->GetItemImage(item, wxTreeItemIcon_Selected), ctrlDirs->GetItemData(item));
	
	wxTreeItemIdValue cookie;
	wxTreeItemId next = ctrlDirs->GetFirstChild(item, cookie);
	while(next.IsOk()) 
	{
		moveNode(next, ht);
		next = ctrlDirs->GetFirstChild(item, cookie);
	}
	ctrlDirs->Delete(item);
}

void QueueFrame::removeDir(const wxTreeItemId& item)
{
	if(!item.IsOk())
		return;

	wxTreeItemIdValue cookie;
	wxTreeItemId child = ctrlDirs->GetFirstChild(item, cookie);
	while(child.IsOk()) 
	{
		removeDir(child);
		child = ctrlDirs->GetNextSibling(child);
	}
	const string& name = getDir(item);
	DirectoryPairC dp = directories.equal_range(name);
	for(DirectoryIterC i = dp.first; i != dp.second; ++i) 
	{
		QueueManager::getInstance()->remove(i->second->getTarget());
	}
}

void QueueFrame::removeDirectory(const string& dir, bool isFileList /* = false */) 
{
	// First, find the last name available
	string::size_type i = 0;

	wxTreeItemId next = ctrlDirs->GetRootItem();
	wxTreeItemId parent;
	
	if(isFileList) 
	{
		dcassert(fileLists.IsOk());
		//delete (string*)ctrlDirs.GetItemData(fileLists);
		ctrlDirs->Delete(fileLists);
		fileLists.Unset();
		return;
	} 
	else 
	{
		while(i < dir.length()) 
		{
			while(next.IsOk()) 
			{
				if(next != fileLists) 
				{
				const string& n = getDir(next);
				if(strnicmp(n.c_str()+i, dir.c_str()+i, n.length()-i) == 0) 
				{
					// Match!
					parent = next;
					wxTreeItemIdValue cookie;
					next = ctrlDirs->GetFirstChild(next, cookie);
					i = n.length();
					break;
				}
				}
				next = ctrlDirs->GetNextSibling(next);
			}
			if(!next.IsOk())
				break;
		}
	}

	next = parent;

	wxTreeItemIdValue cookie;
	while((!ctrlDirs->GetFirstChild(next, cookie).IsOk()) && (directories.find(getDir(next)) == directories.end())) 
	{
		//delete (string*)ctrlDirs.GetItemData(next);
		parent = ctrlDirs->GetItemParent(next);
		
		ctrlDirs->Delete(next);
		if(!parent.IsOk())
			break;
		next = parent;
	}
}

void QueueFrame::updateStatus() 
{
	int64_t total = 0;
	
	wxDataViewItemArray sel;
	int cnt = ctrlFiles->GetSelections(sel);
	if(cnt == 0) {
		cnt = ctrlFiles->getItemCount();
		if(showTree) {
			for(int i = 0; i < cnt; ++i) {
				const QueueItemInfo* ii = ctrlFiles->getItemData(i);
				total += (ii->getSize() > 0) ? ii->getSize() : 0;
			}
		} else {
			total = queueSize;
		}
	} else {
		int i = -1;
		for(unsigned int i = 0; i < sel.size(); ++i)
		{
			const QueueItemInfo* ii = ctrlFiles->getItemData(sel[i]);
			total += (ii->getSize() > 0) ? ii->getSize() : 0;
		}

	}

	tstring tmp1 = TSTRING(ITEMS) + _T(": ") + Util::toStringW(cnt);
	tstring tmp2 = TSTRING(SIZE) + _T(": ") + Util::formatBytesW(total);
	bool u = false;

	int w = WinUtil::getTextWidth(tmp1, GetStatusBar());
	if(statusSizes[2] < w) {
		statusSizes[2] = w;
		u = true;
	}
	GetStatusBar()->SetStatusText(tmp1, 2);
	w = WinUtil::getTextWidth(tmp2, GetStatusBar());
	if(statusSizes[3] < w) {
		statusSizes[3] = w;
		u = true;
	}
	GetStatusBar()->SetStatusText(tmp2, 3);

	if(dirty) {
		tmp1 = TSTRING(FILES) + _T(": ") + Util::toStringW(queueItems);
		tmp2 = TSTRING(SIZE) + _T(": ") + Util::formatBytesW(queueSize);

		w = WinUtil::getTextWidth(tmp2, GetStatusBar());
		if(statusSizes[4] < w) {
			statusSizes[4] = w;
			u = true;
		}
		GetStatusBar()->SetStatusText(tmp1, 4);

		w = WinUtil::getTextWidth(tmp2, GetStatusBar());
		if(statusSizes[5] < w) {
			statusSizes[5] = w;
			u = true;
		}
		GetStatusBar()->SetStatusText(tmp2, 5);

		dirty = false;
	}

	if(u)
		UpdateLayout();
}

void QueueFrame::UpdateLayout()
{
	GetStatusBar()->SetFieldsCount(6, statusSizes);

	if(showTree) {
		if(!splitter->IsSplit()) {
			splitter->SplitVertically(ctrlDirs, ctrlFiles, GetClientRect().GetWidth() / 4);
			updateQueue();
		}
	} else {
		if(splitter->IsSplit()) {
			splitter->Unsplit(ctrlDirs);
			updateQueue();
		}
	}
}

wxString QueueFrame::QueueItemInfo::getText(int col) const 
{
	switch(col) 
	{
		case COLUMN_TARGET: return Text::toT(Util::getFileName(getTarget()));
		case COLUMN_STATUS: 
		{
			int online = 0;
			QueueItem::SourceList sources = QueueManager::getInstance()->getSources(qi);
			for(QueueItem::SourceConstIter j = sources.begin(); j != sources.end(); ++j) 
			{
				if(j->getUser().user->isOnline())
					online++;
			}

			if(isFinished()) 
			{
				return TSTRING(DOWNLOAD_FINISHED_IDLE);
			} 
			else if(isWaiting()) 
			{
				if(online > 0) 
				{
					size_t size = QueueManager::getInstance()->getSourcesCount(qi);
					if(size == 1) 
					{
						return TSTRING(WAITING_USER_ONLINE);
					} 
					else 
					{
						TCHAR buf[64];
						_stprintf(buf, CTSTRING(WAITING_USERS_ONLINE), online, size);
						return buf;
					}
				} 
				else 
				{
					size_t size = QueueManager::getInstance()->getSourcesCount(qi);
					if(size == 0) 
					{
						return TSTRING(NO_USERS_TO_DOWNLOAD_FROM);
					} 
					else if(size == 1) 
					{
						return TSTRING(USER_OFFLINE);
					} 
					else if(size == 2) 
					{
						return TSTRING(BOTH_USERS_OFFLINE);
					} 
					else if(size == 3) 
					{
						return TSTRING(ALL_3_USERS_OFFLINE);
					} 
					else if(size == 4) 
					{
						return TSTRING(ALL_4_USERS_OFFLINE);
					} 
					else 
					{
						TCHAR buf[64];
						_stprintf(buf, CTSTRING(ALL_USERS_OFFLINE), size);
						return buf;
					}
				}
			} else {
				size_t size = QueueManager::getInstance()->getSourcesCount(qi);
				if(size == 1) 
				{
					return TSTRING(USER_ONLINE);
				} 
				else 
				{
					TCHAR buf[64];
					_stprintf(buf, CTSTRING(USERS_ONLINE), online, size);
					return buf;
				}
			}
		}
		case COLUMN_SEGMENTS: 
		{
			const QueueItem* qi = QueueManager::getInstance()->fileQueue.find(getTarget());
			return Util::toStringW(qi ? qi->getDownloads().size() : 0) + _T("/") + Util::toStringW(qi ? qi->getMaxSegments() : 0);
		}
		case COLUMN_SIZE: 
			return (getSize() == -1) ? TSTRING(UNKNOWN) : Util::formatBytesW(getSize());
		case COLUMN_DOWNLOADED: 
		{
			int64_t downloadedBytes = getDownloadedBytes();
			return (getSize() > 0) ? Util::formatBytesW(downloadedBytes) + _T(" (") + Util::toStringW((double)downloadedBytes*100.0/(double)getSize()) + _T("%)") : Util::emptyStringT;
		}
		case COLUMN_PRIORITY: 
		{
			tstring priority;
			switch(getPriority()) 
			{
				case QueueItem::PAUSED: priority = TSTRING(PAUSED); break;
				case QueueItem::LOWEST: priority = TSTRING(LOWEST); break;
				case QueueItem::LOW: priority = TSTRING(LOW); break;
				case QueueItem::NORMAL: priority = TSTRING(NORMAL); break;
				case QueueItem::HIGH: priority = TSTRING(HIGH); break;
				case QueueItem::HIGHEST: priority = TSTRING(HIGHEST); break;
				default: dcassert(0); break;
			}
			if(getAutoPriority()) 
			{
				priority += _T(" (") + TSTRING(AUTO) + _T(")");
			}
			return priority;
		}
		case COLUMN_USERS: 
		{
			tstring tmp;

			QueueItem::SourceList sources = QueueManager::getInstance()->getSources(qi);
			for(QueueItem::SourceConstIter j = sources.begin(); j != sources.end(); ++j) 
			{
				if(tmp.size() > 0)
					tmp += _T(", ");

				tmp += WinUtil::getNicks(j->getUser());
			}
			return tmp.empty() ? TSTRING(NO_USERS) : tmp;
		}
		case COLUMN_PATH: return Text::toT(getPath());
		case COLUMN_EXACT_SIZE: return (getSize() == -1) ? TSTRING(UNKNOWN) : Util::formatExactSize(getSize());
		case COLUMN_ERRORS: 
		{
			tstring tmp;
			QueueItem::SourceList badSources = QueueManager::getInstance()->getBadSources(qi);
			for(QueueItem::SourceConstIter j = badSources.begin(); j != badSources.end(); ++j) {
				if(!j->isSet(QueueItem::Source::FLAG_REMOVED)) {
				if(tmp.size() > 0)
					tmp += _T(", ");
					tmp += WinUtil::getNicks(j->getUser());
					tmp += _T(" (");
					if(j->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE)) {
						tmp += TSTRING(FILE_NOT_AVAILABLE);
					} else if(j->isSet(QueueItem::Source::FLAG_PASSIVE)) {
						tmp += TSTRING(PASSIVE_USER);
					} else if(j->isSet(QueueItem::Source::FLAG_BAD_TREE)) {
						tmp += TSTRING(INVALID_TREE);
					} else if(j->isSet(QueueItem::Source::FLAG_SLOW_SOURCE)) {
						tmp += TSTRING(SLOW_USER);
					} else if(j->isSet(QueueItem::Source::FLAG_NO_TTHF)) {
						tmp += TSTRING(SOURCE_TOO_OLD);						
					} else if(j->isSet(QueueItem::Source::FLAG_NO_NEED_PARTS)) {
						tmp += TSTRING(NO_NEEDED_PART);
					} else if(j->isSet(QueueItem::Source::FLAG_UNTRUSTED)) {
						tmp += TSTRING(CERTIFICATE_NOT_TRUSTED);
					}
					tmp += _T(')');
				}
			}
			return tmp.empty() ? TSTRING(NO_ERRORS) : tmp;
		}
		case COLUMN_ADDED: return Text::toT(Util::formatTime("%Y-%m-%d %H:%M", getAdded()));
		case COLUMN_TTH: 
			return qi->isSet(QueueItem::FLAG_USER_LIST) ? Util::emptyStringT : Text::toT(getTTH().toBase32());

		default: return Util::emptyStringT;
	}
}

void QueueFrame::removeSelected()
{
	if(!BOOLSETTING(CONFIRM_DELETE) || wxMessageBox(CTSTRING(REALLY_REMOVE), wxT(APPNAME) wxT(" ") wxT(VERSIONSTRING), wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT) == wxYES)
		ctrlFiles->forEachSelected(&QueueItemInfo::remove);
}

void QueueFrame::removeSelectedDir()
{
	if(!BOOLSETTING(CONFIRM_DELETE) || wxMessageBox(CTSTRING(REALLY_REMOVE), wxT(APPNAME) _T(" ") wxT(VERSIONSTRING), wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT) == wxYES)
		removeDir(ctrlDirs->GetSelection()); 
}

// QueueManagerListener
void QueueFrame::on(QueueManagerListener::Added, QueueItem* aQI) throw()
{
	callAsync(std::bind(&QueueFrame::OnAdded, this, new QueueItemInfo(aQI)));
}

void QueueFrame::on(QueueManagerListener::Moved, const QueueItem* aQI, const string& oldTarget) throw()
{
	// TODO
}

void QueueFrame::on(QueueManagerListener::Removed, const QueueItem* aQI) throw()
{
	callAsync(std::bind(&QueueFrame::OnRemoved, this, aQI->getTarget()));
}

void QueueFrame::on(QueueManagerListener::SourcesUpdated, const QueueItem* aQI) throw()
{
	callAsync(std::bind(&QueueFrame::OnUpdated, this, aQI->getTarget()));
}

bool QueueFrame::GetItemAttr(void* data, int64_t& position, int64_t& size, wxColour& backColour, wxColour& textColour)
{
	QueueItemInfo* ii = (QueueItemInfo*)data;
	
	return false;
}
