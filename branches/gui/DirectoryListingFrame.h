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

#ifndef _DIRECTORYLISTINGFRAME_H
#define _DIRECTORYLISTINGFRAME_H

#include "MDIChildFrame.h"
#include "TypedDataViewCtrl.h"
#include "UCHandler.h"

#include "../client/ADLSearch.h"
#include "../client/ClientManager.h"
#include "../client/DirectoryListing.h"

#include <wx/splitter.h>
#include <wx/treectrl.h>

class DirectoryListingFrame :
	public MDIChildFrame,
	public UCHandler<DirectoryListingFrame>
{
public:

	static void openWindow(const tstring& aFile, const tstring& aDir, const HintedUser& aUser, int64_t aSpeed);
	static void openWindow(const HintedUser& aUser, const string& txt, int64_t aSpeed);

	void runUserCommand(const UserCommand& uc); 

private:

	enum 
	{
		COLUMN_FILENAME,
		COLUMN_TYPE,
		COLUMN_EXACTSIZE,
		COLUMN_SIZE,
		COLUMN_TTH,
		COLUMN_LAST
	};
		
	enum 
	{
		STATUS_TEXT,
		STATUS_SPEED,
		STATUS_TOTAL_FILES,
		STATUS_TOTAL_SIZE,
		STATUS_SELECTED_FILES,
		STATUS_SELECTED_SIZE,
		STATUS_FILE_LIST_DIFF,
		STATUS_MATCH_QUEUE,
		STATUS_FIND,
		STATUS_NEXT,
		STATUS_LAST
	};

	class ItemInfo : public FastAlloc<ItemInfo> 
	{
	public:
		
		enum ItemType 
		{
			FILE,
			DIRECTORY
		} type;
		
		union 
		{
			DirectoryListing::File* file;
			DirectoryListing::Directory* dir;
		};

		ItemInfo(DirectoryListing::File* f) : type(FILE), file(f) { }
		ItemInfo(DirectoryListing::Directory* d) : type(DIRECTORY), dir(d) { }

		wxString getText(uint8_t col) const 
		{
			switch(col) 
			{
				case COLUMN_FILENAME: return type == DIRECTORY ? Text::toT(dir->getName()) : Text::toT(file->getName());
				case COLUMN_TYPE: 
					if(type == FILE) 
					{
						tstring type = Util::getFileExt(Text::toT(file->getName()));
						if(type.size() > 0 && type[0] == '.')
							type.erase(0, 1);
						return type;
					} 
					else 
					{
						return Util::emptyStringT;
					}
				case COLUMN_EXACTSIZE: return type == DIRECTORY ? Util::formatExactSize(dir->getTotalSize()) : Util::formatExactSize(file->getSize());
				case COLUMN_SIZE: return  type == DIRECTORY ? Util::formatBytesW(dir->getTotalSize()) : Util::formatBytesW(file->getSize());
				case COLUMN_TTH: return type == FILE ? Text::toT(file->getTTH().toBase32()) : Util::emptyStringT;
				default: return Util::emptyStringT;
			}
		}
		
		struct TotalSize 
		{
			TotalSize() : total(0) { }
			void operator()(ItemInfo* a) { total += a->type == DIRECTORY ? a->dir->getTotalSize() : a->file->getSize(); }
			int64_t total;
		};

		static int compareItems(const ItemInfo* a, const ItemInfo* b, uint8_t col) 
		{
			if(a->type == DIRECTORY) 
			{
				if(b->type == DIRECTORY) 
				{
					switch(col) 
					{
						case COLUMN_EXACTSIZE: return compare(a->dir->getTotalSize(), b->dir->getTotalSize());
						case COLUMN_SIZE: return compare(a->dir->getTotalSize(), b->dir->getTotalSize());
						default: return lstrcmpi(a->getText(col).c_str(), b->getText(col).c_str());
					}
				} 
				else 
				{
					return -1;
				}
			} 
			else if(b->type == DIRECTORY) 
			{
				return 1;
			} 
			else 
			{
				switch(col) 
				{
					case COLUMN_EXACTSIZE: return compare(a->file->getSize(), b->file->getSize());
					case COLUMN_SIZE: return compare(a->file->getSize(), b->file->getSize());
					default: return lstrcmp(a->getText(col).c_str(), b->getText(col).c_str());
				}
			}
		}

		int getImageIndex() const 
		{
			if(type == DIRECTORY)
				return dir->getComplete() ? WinUtil::getDirIconIndex() : WinUtil::getDirMaskedIndex();
			else
				return WinUtil::getIconIndex(getText(COLUMN_FILENAME));
		}
	};

	DECLARE_EVENT_TABLE();

	// events
	void OnClose(wxCloseEvent& event);
	void OnTreeSelChanged(wxTreeEvent& event);
	void OnTreeContextMenu(wxTreeEvent& event);
	void OnFilesContextMenu(wxDataViewEvent& event);
	void OnDblClick(wxDataViewEvent& event);
	void OnListDiff(wxCommandEvent& event);
	void OnMatchQueue(wxCommandEvent& event);
	void OnFind(wxCommandEvent& event);
	void OnFindNext(wxCommandEvent& event);
	void OnDownload(wxCommandEvent& event);
	void OnDownloadTo(wxCommandEvent& event);
	void OnDownloadDir(wxCommandEvent& event);
	void OnDownloadDirTo(wxCommandEvent& event);
	void OnViewAsText(wxCommandEvent& event);
	void OnSearchForAlternates(wxCommandEvent& event);
	void OnGoToDirectory(wxCommandEvent& event);
	void OnPriority(wxCommandEvent& event);
	void OnPriorityDir(wxCommandEvent& event);

	void OnFinished();
	void OnAborted();

	// attributes
	typedef unordered_map<UserPtr, DirectoryListingFrame*, User::Hash> UserMap;
	typedef UserMap::const_iterator UserIter;
	static UserMap lists;

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	bool loading;
	bool searching;
	bool updating;

	wxSplitterWindow*				splitter;
	wxTreeCtrl*						ctrlDirs;
	TypedDataViewCtrl<ItemInfo>*	ctrlFiles;
	wxButton*						ctrlListDiff;
	wxButton*						ctrlMatchQueue;
	wxButton*						ctrlFind;
	wxButton*						ctrlFindNext;

	unique_ptr<DirectoryListing> dl;

	int64_t speed;		/**< Speed at which this file list was downloaded */
	tstring error;

	int statusSizes[10];

	StringList	targets;
	StringMap	ucLineParams;
	
	// methods
	DirectoryListingFrame(const HintedUser& aUser, int64_t aSpeed);
	~DirectoryListingFrame(void);
	
	void UpdateLayout();

	void initStatus();
	void updateStatus();

	void loadFile(const tstring& name, const tstring& dir);
	void loadXML(const string& txt);

	void refreshTree(const tstring& root);
	void updateTree(DirectoryListing::Directory* tree, const wxTreeItemId& parent);
	void changeDir(DirectoryListing::Directory* d, bool enableRedraw);
	
	wxTreeItemId findItem(const wxTreeItemId& ht, const tstring& name) const;

	void downloadList(const tstring& aTarget, bool view = false, QueueItem::Priority prio = QueueItem::DEFAULT);

	struct TreeData : public wxTreeItemData, public FastAlloc<TreeData>
	{
		TreeData(DirectoryListing::Directory* _dir) : dir(_dir) { }
		DirectoryListing::Directory* dir;
	};

	class ThreadedDirectoryListing : public Thread
	{
	public:
		ThreadedDirectoryListing(DirectoryListingFrame* pWindow, 
			const string& pFile, const string& pTxt, const tstring& aDir = Util::emptyStringT) : mWindow(pWindow),
			mFile(pFile), mTxt(pTxt), mDir(aDir)
		{ }

	protected:
		DirectoryListingFrame* mWindow;
		string mFile;
		string mTxt;
		tstring mDir;

	private:
		int run() 
		{
			try 
			{
				if(!mFile.empty()) 
				{
					mWindow->dl->loadFile(mFile);
					ADLSearchManager::getInstance()->matchListing(*mWindow->dl);
					mWindow->refreshTree(mDir);
				} 
				else 
				{
					mWindow->refreshTree(Text::toT(Util::toNmdcFile(mWindow->dl->updateXML(mTxt))));
				}

				mWindow->callAsync(std::bind(&DirectoryListingFrame::OnFinished, mWindow));
			} 
			catch(const AbortException) 
			{
				mWindow->callAsync(std::bind(&DirectoryListingFrame::OnAborted, mWindow));
			} 
			catch(const Exception& e) 
			{
				mWindow->error = Text::toT(ClientManager::getInstance()->getNicks(mWindow->dl->getUser()->getCID(), mWindow->dl->getHintedUser().hint)[0] + ": " + e.getError());
				mWindow->callAsync(std::bind(&DirectoryListingFrame::OnAborted, mWindow));
			}

			//cleanup the thread object
			delete this;

			return 0;
		}
	};

};

#endif	// _DIRECTORYLISTINGFRAME_H