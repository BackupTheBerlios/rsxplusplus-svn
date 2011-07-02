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

#ifndef _TRANSFERVIEW_H
#define _TRANSFERVIEW_H

#include "AsyncHandler.h"
#include "TypedTreeListCtrl.h"
#include "UCHandler.h"
#include "UserInfoBaseHandler.h"
#include "WinUtil.h"

#include "../client/ConnectionManagerListener.h"
#include "../client/Download.h"
#include "../client/DownloadManagerListener.h"
#include "../client/QueueItem.h"
#include "../client/QueueManagerListener.h"
#include "../client/TaskQueue.h"
#include "../client/Transfer.h"
#include "../client/Upload.h"
#include "../client/UploadManagerListener.h"

#include <wx/window.h>

class TransferView :
	public wxWindow, 
	public AsyncHandler<TransferView>,
	private DownloadManagerListener, 
	private UploadManagerListener, 
	private ConnectionManagerListener, 
	private QueueManagerListener, 
	public UserInfoBaseHandler<TransferView>, 
	public UCHandler<TransferView>,
	private SettingsManagerListener

{

private:

	struct UpdateInfo;
	class ItemInfo : public UserInfoBase 
	{
	public:

		enum Status 
		{
			STATUS_RUNNING,
			STATUS_WAITING
		};
		
		ItemInfo(const HintedUser& u, bool aDownload);

		bool download;
		bool transferFailed;
		bool collapsed;
		
		uint8_t flagIndex;
		int16_t running;
		int16_t hits;

		ItemInfo* parent;
		HintedUser user;
		Status status;
		Transfer::Type type;
		
		int64_t pos;
		int64_t size;
		int64_t actual;
		int64_t speed;
		int64_t timeLeft;
		
		tstring ip;
		tstring statusString;
		tstring target;
		tstring cipher;

		void update(const UpdateInfo& ui);

		const UserPtr& getUser() const { return user.user; }

		void disconnect();
		void removeAll();

		double getRatio() const { return (pos > 0) ? (double)actual / (double)pos : 1.0; }

		wxString getText(uint8_t col) const;
		static int compareItems(const ItemInfo* a, const ItemInfo* b, uint8_t col);

		uint8_t getImageIndex() const { return static_cast<uint8_t>(!download ? IMAGE_UPLOAD : (!parent ? IMAGE_DOWNLOAD : IMAGE_SEGMENT)); }

		ItemInfo* createParent() 
		{
	  		ItemInfo* ii = new ItemInfo(HintedUser(NULL, Util::emptyString), true);
			ii->running = 0;
			ii->hits = 0;
			ii->target = target;
			ii->statusString = TSTRING(CONNECTING);
			return ii;
		}

		inline const tstring& getGroupCond() const { return target; }
	};

	struct UpdateInfo : public Task 
	{
		enum {
			MASK_POS			= 0x01,
			MASK_SIZE			= 0x02,
			MASK_ACTUAL			= 0x04,
			MASK_SPEED			= 0x08,
			MASK_FILE			= 0x10,
			MASK_STATUS			= 0x20,
			MASK_TIMELEFT		= 0x40,
			MASK_IP				= 0x80,
			MASK_STATUS_STRING	= 0x100,
			MASK_SEGMENT		= 0x200,
			MASK_CIPHER			= 0x400
		};

		bool operator==(const ItemInfo& ii) const { return download == ii.download && user == ii.user; }

		UpdateInfo(const HintedUser& aUser, bool isDownload, bool isTransferFailed = false) : 
			updateMask(0), user(aUser), queueItem(NULL), download(isDownload), transferFailed(isTransferFailed), flagIndex(0), type(Transfer::TYPE_LAST)
		{ }
		
		UpdateInfo(QueueItem* qi, bool isDownload, bool isTransferFailed = false) : 
			updateMask(0), queueItem(qi), user(HintedUser(NULL, Util::emptyString)), download(isDownload), transferFailed(isTransferFailed), flagIndex(0), type(Transfer::TYPE_LAST) 
		{ qi->inc(); }

		~UpdateInfo() { if(queueItem) queueItem->dec(); }

		uint32_t updateMask;

		HintedUser user;

		bool download;
		bool transferFailed;
		uint8_t flagIndex;		
		void setRunning(int16_t aRunning) { running = aRunning; updateMask |= MASK_SEGMENT; }
		int16_t running;
		void setStatus(ItemInfo::Status aStatus) { status = aStatus; updateMask |= MASK_STATUS; }
		ItemInfo::Status status;
		void setPos(int64_t aPos) { pos = aPos; updateMask |= MASK_POS; }
		int64_t pos;
		void setSize(int64_t aSize) { size = aSize; updateMask |= MASK_SIZE; }
		int64_t size;
		void setActual(int64_t aActual) { actual = aActual; updateMask |= MASK_ACTUAL; }
		int64_t actual;
		void setSpeed(int64_t aSpeed) { speed = aSpeed; updateMask |= MASK_SPEED; }
		int64_t speed;
		void setTimeLeft(int64_t aTimeLeft) { timeLeft = aTimeLeft; updateMask |= MASK_TIMELEFT; }
		int64_t timeLeft;
		void setStatusString(const tstring& aStatusString) { statusString = aStatusString; updateMask |= MASK_STATUS_STRING; }
		tstring statusString;
		void setTarget(const tstring& aTarget) { target = aTarget; updateMask |= MASK_FILE; }
		tstring target;
		void setIP(const tstring& aIP, uint8_t aFlagIndex) { IP = aIP; flagIndex = aFlagIndex, updateMask |= MASK_IP; }
		tstring IP;
		void setCipher(const tstring& aCipher) { cipher = aCipher; updateMask |= MASK_CIPHER; }
		tstring cipher;
		void setType(const Transfer::Type& aType) { type = aType; }
		Transfer::Type type;	

	private:
		QueueItem* queueItem;
	};

public:

	TransferView(wxWindow* parent);
	~TransferView(void);

	void prepareClose();

	typedef TypedTreeListCtrl<ItemInfo, tstring, noCaseStringHash, noCaseStringEq> ItemInfoList;
	ItemInfoList& getUserList() { return *ctrlTransfers; }

	static bool GetItemAttr(void* data, int64_t& position, int64_t& size, wxColour& backColour, wxColour& textColour);

	void runUserCommand(const UserCommand& uc);

private:

	enum 
	{
		ADD_ITEM,
		REMOVE_ITEM,
		UPDATE_ITEM,
		UPDATE_PARENT
	};

	enum 
	{
		COLUMN_FIRST,
		COLUMN_USER = COLUMN_FIRST,
		COLUMN_HUB,
		COLUMN_STATUS,
		COLUMN_TIMELEFT,
		COLUMN_SPEED,
		COLUMN_FILE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_CIPHER,
		COLUMN_IP,
		COLUMN_RATIO,
		COLUMN_LAST
	};

	enum 
	{
		IMAGE_DOWNLOAD = 0,
		IMAGE_UPLOAD,
		IMAGE_SEGMENT
	};

	DECLARE_EVENT_TABLE();

	// events
	void OnSize(wxSizeEvent& event);
	void OnTasks();
	void OnDblClick(wxDataViewEvent& event);
	void OnContextMenu(wxDataViewEvent& event);
	void OnChar(wxKeyEvent& event);

	void OnForce(wxCommandEvent& event);
	void OnSearchForAlternates(wxCommandEvent& event);
	void OnSlowDisconnect(wxCommandEvent& event);
	void OnDisconnectAll(wxCommandEvent& event);
	void OnExpandAll(wxCommandEvent& event);
	void OnCollapseAll(wxCommandEvent& event);
	void OnRemove(wxCommandEvent& /*event*/)	{ ctrlTransfers->forEachSelected(&ItemInfo::disconnect); }
	void OnRemoveAll(wxCommandEvent& /*event*/)	{ ctrlTransfers->forEachSelected(&ItemInfo::removeAll); }

	// attributes
	ItemInfoList* ctrlTransfers;
	static int columnIndexes[];
	static int columnSizes[];

	wxImageList arrows;

	TaskQueue tasks;
	StringMap ucLineParams;

	// methods
	void speak(uint8_t type, UpdateInfo* ui) { tasks.add(type, ui); callAsync(std::bind(&TransferView::OnTasks, this)); }
	ItemInfo* findItem(const UpdateInfo& ui, int& pos) const;
	void updateItem(int ii, uint32_t updateMask);

	void starting(UpdateInfo* ui, const Transfer* t);
	void onTransferComplete(const Transfer* aTransfer, bool isUpload, const string& aFileName, bool isTree);

	void on(ConnectionManagerListener::Added, const ConnectionQueueItem* aCqi) throw();
	void on(ConnectionManagerListener::Failed, const ConnectionQueueItem* aCqi, const string& aReason) throw();
	void on(ConnectionManagerListener::Removed, const ConnectionQueueItem* aCqi) throw();
	void on(ConnectionManagerListener::StatusChanged, const ConnectionQueueItem* aCqi) throw();

	void on(DownloadManagerListener::Requesting, const Download* aDownload) throw();	
	void on(DownloadManagerListener::Complete, const Download* aDownload, bool isTree) throw() { onTransferComplete(aDownload, false, Util::getFileName(aDownload->getPath()), isTree);}
	void on(DownloadManagerListener::Failed, const Download* aDownload, const string& aReason) throw();
	void on(DownloadManagerListener::Starting, const Download* aDownload) throw();
	void on(DownloadManagerListener::Tick, const DownloadList& aDownload) throw();
	void on(DownloadManagerListener::Status, const UserConnection*, const string&) throw();

	void on(UploadManagerListener::Starting, const Upload* aUpload) throw();
	void on(UploadManagerListener::Tick, const UploadList& aUpload) throw();
	void on(UploadManagerListener::Complete, const Upload* aUpload) throw() { onTransferComplete(aUpload, true, aUpload->getPath(), false); }

	void on(QueueManagerListener::StatusUpdated, const QueueItem*) throw();
	void on(QueueManagerListener::Removed, const QueueItem*) throw();
	void on(QueueManagerListener::Finished, const QueueItem*, const string&, const Download*) throw();

	void on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw();

};


#endif	// _TRANSFERVIEW_H