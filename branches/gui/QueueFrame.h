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

#ifndef _QUEUEFRAME_H
#define _QUEUEFRAME_H

#include "StaticFrame.h"
#include "TypedDataViewCtrl.h"

#include "../client/QueueItem.h"
#include "../client/QueueManager.h"

#include <wx/splitter.h>
#include <wx/treectrl.h>

class QueueFrame :
	public StaticFrame<QueueFrame>, private QueueManagerListener
{
public:

	typedef StaticFrame<QueueFrame> BaseType;

	QueueFrame(void);
	~QueueFrame(void);

	static bool GetItemAttr(void* data, int64_t& position, int64_t& size, wxColour& backColour, wxColour& textColour);

private:

	enum 
	{
		COLUMN_FIRST,
		COLUMN_TARGET = COLUMN_FIRST,
		COLUMN_STATUS,
		COLUMN_SEGMENTS,
		COLUMN_SIZE,
		COLUMN_PROGRESS,
		COLUMN_DOWNLOADED,
		COLUMN_PRIORITY,
		COLUMN_USERS,
		COLUMN_PATH,
		COLUMN_EXACT_SIZE,
		COLUMN_ERRORS,
		COLUMN_ADDED,
		COLUMN_TTH,
		COLUMN_LAST
	};

	class QueueItemInfo : public FastAlloc<QueueItemInfo> 
	{
	public:

		QueueItemInfo(QueueItem* aQI) : qi(aQI)	
		{
			qi->inc();
		}

		~QueueItemInfo() 
		{
		 
			qi->dec();
		}

		void remove() { QueueManager::getInstance()->remove(getTarget()); }

		// TypedListViewCtrl functions
		wxString getText(int col) const;

		static int compareItems(const QueueItemInfo* a, const QueueItemInfo* b, int col) 
		{
			switch(col) 
			{
				case COLUMN_SIZE: case COLUMN_EXACT_SIZE: return compare(a->getSize(), b->getSize());
				case COLUMN_PRIORITY: return compare((int)a->getPriority(), (int)b->getPriority());
				case COLUMN_DOWNLOADED: return compare(a->getDownloadedBytes(), b->getDownloadedBytes());
				case COLUMN_ADDED: return compare(a->getAdded(), b->getAdded());
				default: return lstrcmpi(a->getText(col).c_str(), b->getText(col).c_str());
			}
		}

		int getImageIndex() const { return WinUtil::getIconIndex(Text::toT(getTarget()));	}

		const QueueItem* getQueueItem() const { return qi; }
		string getPath() const { return Util::getFilePath(getTarget()); }

		bool isSet(Flags::MaskType aFlag) const { return (qi->getFlags() & aFlag) == aFlag; }

		const string& getTarget() const { return qi->getTarget(); }

		int64_t getSize() const { return qi->getSize(); }
		int64_t getDownloadedBytes() const { return qi->getDownloadedBytes(); }

		time_t getAdded() const { return qi->getAdded(); }
		const TTHValue& getTTH() const { return qi->getTTH(); }

		QueueItem::Priority getPriority() const { return qi->getPriority(); }
		bool isWaiting() const { return qi->isWaiting(); }
		bool isFinished() const { return qi->isFinished(); }

		bool getAutoPriority() const { return qi->getAutoPriority(); }

	private:
		QueueItem* qi;

		QueueItemInfo(const QueueItemInfo&);
		QueueItemInfo& operator=(const QueueItemInfo&);
	};

	struct TreeData : public wxTreeItemData, public FastAlloc<TreeData>
	{
		TreeData(const string& _dir) : dir(_dir) { }
		string dir;
	};

	DECLARE_EVENT_TABLE();

	// events
	void OnClose(wxCloseEvent& event);
	void OnTreeSelChanged(wxTreeEvent& event);
	void OnFilesKeyDown(wxKeyEvent& event);
	void OnDirsKeyDown(wxKeyEvent& event);

	void OnAdded(QueueItemInfo* ii);
	void OnRemoved(const string& target);
	void OnUpdated(const string& target);

	void UpdateLayout();
	void addQueueList(const QueueItem::StringMap& l);
	void addQueueItem(QueueItemInfo* qi, bool noSort);
	QueueItemInfo* getItemInfo(const string& target) const;

	wxTreeItemId addDirectory(const string& dir, bool isFileList = false, wxTreeItemId startAt = wxTreeItemId());
	void removeDirectory(const string& dir, bool isFileList = false);
	void clearTree(const wxTreeItemId& item);
	void moveNode(const wxTreeItemId& item, const wxTreeItemId& parent);
	void removeDir(const wxTreeItemId& item);

	void removeSelected();
	void removeSelectedDir();

	bool isCurDir(const string& aDir) const { return stricmp(curDir, aDir) == 0; }

	void updateQueue();
	void updateStatus();

	const string& getSelectedDir() const 
	{
		wxTreeItemId ht = ctrlDirs->GetSelection();
		return !ht.IsOk() ? Util::emptyString : getDir(ctrlDirs->GetSelection());
	}
	
	const string& getDir(const wxTreeItemId& ht) const { dcassert(ht.IsOk()); return ((TreeData*)ctrlDirs->GetItemData(ht))->dir; }

	typedef unordered_multimap<string, QueueItemInfo*, noCaseStringHash, noCaseStringEq> DirectoryMap;
	typedef DirectoryMap::iterator DirectoryIter;
	typedef DirectoryMap::const_iterator DirectoryIterC;
	typedef pair<DirectoryIterC, DirectoryIterC> DirectoryPairC;
	DirectoryMap	directories;
	string			curDir;

	wxTreeItemId fileLists;

	wxSplitterWindow*					splitter;
	wxTreeCtrl*							ctrlDirs;
	TypedDataViewCtrl<QueueItemInfo>*	ctrlFiles;
	wxCheckBox*							ctrlShowTree;

	int		statusSizes[6];
	int64_t queueSize;
	int		queueItems;

	bool dirty;
	bool showTree;

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	// QueueManagerListener
	void on(QueueManagerListener::Added, QueueItem* aQI) throw();
	void on(QueueManagerListener::Moved, const QueueItem* aQI, const string& oldTarget) throw();
	void on(QueueManagerListener::Removed, const QueueItem* aQI) throw();
	void on(QueueManagerListener::SourcesUpdated, const QueueItem* aQI) throw();
	void on(QueueManagerListener::StatusUpdated, const QueueItem* aQI) throw() { on(QueueManagerListener::SourcesUpdated(), aQI); }



};

#endif	// _QUEUEFRAME_H