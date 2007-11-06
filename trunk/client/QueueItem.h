/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(QUEUE_ITEM_H)
#define QUEUE_ITEM_H

class QueueManager;
class Download;

#include "User.h"
#include "FastAlloc.h"
#include "MerkleTree.h"
#include "Flags.h"
#include "FileChunksInfo.h"
#include "HashManager.h"
#include "SettingsManager.h"

class QueueItem : public Flags, public FastAlloc<QueueItem>, public PointerBase {
public:
	typedef QueueItem* Ptr;
	typedef deque<Ptr> List;
	typedef List::const_iterator Iter;
	typedef unordered_map<string*, Ptr, noCaseStringHash, noCaseStringEq> StringMap;
	typedef StringMap::const_iterator StringIter;
	typedef unordered_map<UserPtr, Ptr, User::Hash> UserMap;
	typedef UserMap::const_iterator UserIter;
	typedef unordered_map<UserPtr, List, User::Hash> UserListMap;
	typedef UserListMap::const_iterator UserListIter;

	enum Priority {
		DEFAULT = -1,
		PAUSED = 0,
		LOWEST,
		LOW,
		NORMAL,
		HIGH,
		HIGHEST,
		LAST
	};

	enum FileFlags {
		/** Normal download, no flags set */
		FLAG_NORMAL				= 0x00, 
		/** This download should be resumed if possible */
		FLAG_RESUME				= 0x01,
		/** This is a user file listing download */
		FLAG_USER_LIST			= 0x02,
		/** The file list is downloaded to use for directory download (used with USER_LIST) */
		FLAG_DIRECTORY_DOWNLOAD = 0x04,
		/** The file is downloaded to be viewed in the gui */
		FLAG_CLIENT_VIEW		= 0x08,
		/** Flag to indicate that file should be viewed as a text file */
		FLAG_TEXT				= 0x20,
		/** This file exists on the hard disk and should be prioritised */
		FLAG_EXISTS				= 0x40,
		/** Match the queue against this list */
		FLAG_MATCH_QUEUE		= 0x80,
		/** The file list downloaded was actually an .xml.bz2 list */
		FLAG_XML_BZLIST			= 0x200,
		/** Test user for slotlocker */
		FLAG_TESTSUR			= 0x400,
		/** Test user's file list for fake share */
		FLAG_CHECK_FILE_LIST	= 0x800,
		/** Autodrop slow source is enabled for this file */
		FLAG_AUTODROP			= 0x1000
	};

	/**
	 * Source parts info
	 * Meaningful only when Source::FLAG_PARTIAL is set
	 */
	class PartialSource : public FastAlloc<PartialSource>, public PointerBase {
	public:
		PartialSource(const string& aMyNick, const string& aHubIpPort, const string& aIp, uint16_t udp) : 
		  myNick(aMyNick), hubIpPort(aHubIpPort), ip(aIp), udpPort(udp), nextQueryTime(0), pendingQueryCount(0) { }
		
		~PartialSource() { }

		typedef Pointer<PartialSource> Ptr;

		GETSET(PartsInfo, partialInfo, PartialInfo);
		GETSET(string, myNick, MyNick);
		GETSET(string, hubIpPort, HubIpPort);
		GETSET(string, ip, Ip);
		GETSET(uint64_t, nextQueryTime, NextQueryTime);
		GETSET(uint16_t, udpPort, UdpPort);
		GETSET(uint8_t, pendingQueryCount, PendingQueryCount);
	};

	class Source : public Flags {
	public:
		enum {
			FLAG_NONE				= 0x00,
			FLAG_FILE_NOT_AVAILABLE = 0x01,
			FLAG_PASSIVE			= 0x02,
			FLAG_REMOVED			= 0x04,
			FLAG_CRC_FAILED			= 0x08,
			FLAG_CRC_WARN			= 0x10,
			FLAG_NO_TTHF			= 0x20,
			FLAG_BAD_TREE			= 0x40,
			FLAG_SLOW				= 0x80,
			FLAG_NO_TREE			= 0x100,
			FLAG_NO_NEED_PARTS		= 0x200,
			FLAG_PARTIAL			= 0x400,
			FLAG_TTH_INCONSISTENCY	= 0x800,
			FLAG_PG_BLOCKED 		= 0x1000, //RSX++
			FLAG_MASK 				= FLAG_FILE_NOT_AVAILABLE
				| FLAG_PASSIVE | FLAG_REMOVED | FLAG_CRC_FAILED | FLAG_CRC_WARN | FLAG_BAD_TREE
				| FLAG_SLOW | FLAG_NO_TREE | FLAG_TTH_INCONSISTENCY /*RSX++*/ | FLAG_PG_BLOCKED
		};

		Source(const UserPtr& aUser) : user(aUser), partialSource(NULL) { }
		Source(const Source& aSource) : Flags(aSource), user(aSource.user), partialSource(aSource.partialSource) { }

		bool operator==(const UserPtr& aUser) const { return user == aUser; }
		UserPtr& getUser() { return user; }
		PartialSource::Ptr& getPartialSource() { return partialSource; }

		GETSET(UserPtr, user, User);
		GETSET(PartialSource::Ptr, partialSource, PartialSource);
	};

	typedef vector<Source> SourceList;
	typedef SourceList::iterator SourceIter;
	typedef SourceList::const_iterator SourceConstIter;

	QueueItem(const string& aTarget, int64_t aSize, 
		Priority aPriority, Flags::MaskType aFlag, time_t aAdded, const TTHValue& tth) :
	Flags(aFlag), target(aTarget), chunksInfo(NULL), maxSegments(1),
	size(aSize), priority(aPriority), added(aAdded),
	tthRoot(tth), autoPriority(false)
	{
		inc();
		setFlag(FLAG_AUTODROP);
	}

	QueueItem(const QueueItem& rhs) : 
	Flags(rhs), target(rhs.target), tempTarget(rhs.tempTarget),
		size(rhs.size), priority(rhs.priority), downloads(rhs.downloads), maxSegments(rhs.maxSegments),
		added(rhs.added), tthRoot(rhs.tthRoot), autoPriority(rhs.autoPriority)
	{
		inc();
		setChunksInfo(rhs.chunksInfo);
	}

	~QueueItem() {
		if(!isSet(FLAG_USER_LIST) && !isSet(FLAG_TESTSUR)) {
			chunksInfo->dec();
			chunksInfo = NULL;
			FileChunksInfo::Free(&getTTH());
		}
	}

	size_t countOnlineUsers() const;

	SourceList& getSources() { return sources; }
	const SourceList& getSources() const { return sources; }
	SourceList& getBadSources() { return badSources; }
	const SourceList& getBadSources() const { return badSources; }

	void getOnlineUsers(UserList& l) const {
		for(SourceConstIter i = sources.begin(); i != sources.end(); ++i)
			if(i->getUser()->isOnline())
				l.push_back(i->getUser());
	}

	string getTargetFileName() const { return Util::getFileName(getTarget()); }

	SourceIter getSource(const UserPtr& aUser) { return find(sources.begin(), sources.end(), aUser); }
	SourceIter getBadSource(const UserPtr& aUser) { return find(badSources.begin(), badSources.end(), aUser); }
	SourceConstIter getSource(const UserPtr& aUser) const { return find(sources.begin(), sources.end(), aUser); }
	SourceConstIter getBadSource(const UserPtr& aUser) const { return find(badSources.begin(), badSources.end(), aUser); }

	bool isSource(const UserPtr& aUser) const { return getSource(aUser) != sources.end(); }
	bool isBadSource(const UserPtr& aUser) const { return getBadSource(aUser) != badSources.end(); }
	bool isBadSourceExcept(const UserPtr& aUser, Flags::MaskType exceptions) const {
		SourceConstIter i = getBadSource(aUser);
		if(i != badSources.end())
			return i->isAnySet((Flags::MaskType)(exceptions^Source::FLAG_MASK));
		return false;
	}
	
	int64_t getDownloadedBytes() const;
	
	DownloadList& getDownloads() { return downloads; }
	
	FileChunksInfo::Ptr getChunksInfo() const { return chunksInfo; }
	void setChunksInfo(FileChunksInfo* aChunksInfo) { aChunksInfo->inc(); chunksInfo = aChunksInfo; }

	bool isRunning() const {
		return !isWaiting();
	}
	bool isWaiting() const {
		return downloads.empty();
	}

	string getListName() const {
		dcassert(isSet(QueueItem::FLAG_USER_LIST));
		if(isSet(QueueItem::FLAG_XML_BZLIST)) {
			return getTarget() + ".xml.bz2";
		} else {
			return getTarget() + ".xml";
		}
	}

	const string& getTempTarget();
	void setTempTarget(const string& aTempTarget) { tempTarget = aTempTarget; }

	GETSET(TTHValue, tthRoot, TTH);
	GETSET(DownloadList, downloads, Downloads);
	GETSET(string, target, Target);
	GETSET(int64_t, size, Size);
	GETSET(time_t, added, Added);
	GETSET(Priority, priority, Priority);
	GETSET(uint8_t, maxSegments, MaxSegments);
	GETSET(bool, autoPriority, AutoPriority);
	
	QueueItem::Priority calculateAutoPriority() const {
		if(autoPriority) {
			QueueItem::Priority p;
			int percent = static_cast<int>(getDownloadedBytes() * 10.0 / size);
			switch(percent){
					case 0:
					case 1:
					case 2:
						p = QueueItem::LOW;
						break;
					case 3:
					case 4:
					case 5:						
					default:
						p = QueueItem::NORMAL;
						break;
					case 6:
					case 7:
					case 8:
						p = QueueItem::HIGH;
						break;
					case 9:
					case 10:
						p = QueueItem::HIGHEST;			
						break;
			}
			return p;			
		}
		return priority;
	}

	bool hasFreeSegments() const {
		return ((downloads.size() < maxSegments) &&
				(!BOOLSETTING(DONT_BEGIN_SEGMENT) || ((size_t)(SETTING(DONT_BEGIN_SEGMENT_SPEED)*1024) > getAverageSpeed())));
	}

	int64_t getAverageSpeed() const {
		int64_t totalSpeed = 0;
		
		for(DownloadList::const_iterator i = downloads.begin(); i != downloads.end(); i++) {
			totalSpeed += static_cast<int64_t>((*i)->getAverageSpeed());
		}

		return totalSpeed;
	}

private:
	QueueItem& operator=(const QueueItem&);

	friend class QueueManager;
	SourceList sources;
	SourceList badSources;
	string tempTarget;

	FileChunksInfo* chunksInfo;

	void addSource(const UserPtr& aUser);
	void removeSource(const UserPtr& aUser, Flags::MaskType reason);
};

#endif // !defined(QUEUE_ITEM_H)

/**
* @file
* $Id: QueueItem.h 330 2007-10-13 22:26:06Z bigmuscle $
*/
