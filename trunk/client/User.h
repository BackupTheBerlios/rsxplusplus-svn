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

#ifndef DCPLUSPLUS_CLIENT_USER_H
#define DCPLUSPLUS_CLIENT_USER_H

#include "Util.h"
#include "Pointer.h"
#include "CID.h"
#include "FastAlloc.h"
#include "CriticalSection.h"
#include "Flags.h"
#include "forward.h"
#include "TimerManager.h" //RSX++

/** A user connected to one or more hubs. */
class User : public FastAlloc<User>, public PointerBase, public Flags
{
public:
	/** Each flag is set if it's true in at least one hub */
	enum UserFlags {
		ONLINE		= 0x01,
		DCPLUSPLUS	= 0x02,
		PASSIVE		= 0x04,
		NMDC		= 0x08,
		BOT			= 0x10,
		TLS			= 0x20,	//< Client supports TLS
		OLD_CLIENT	= 0x40, //< Can't download - old client
		AWAY		= 0x80,
		SERVER		= 0x100,
		FIREBALL	= 0x200,
		PROTECTED 	= 0x400, //< User protected [rsx]
		PG_BLOCK 	= 0x800, //< Blocked by PeerGuardian
	};

	struct Hash {
		size_t operator()(const UserPtr& x) const { return ((size_t)(&(*x)))/sizeof(User); }
	};

	User(const CID& aCID) : cid(aCID), lastDownloadSpeed(0), soundActive(true) { }

	~User() throw() { }

	const CID& getCID() const { return cid; }
	operator const CID&() const { return cid; }

	bool isOnline() const { return isSet(ONLINE); }
	bool isNMDC() const { return isSet(NMDC); }

	GETSET(size_t, lastDownloadSpeed, LastDownloadSpeed);
	GETSET(bool, soundActive, SoundActive); //RSX++
private:
	User(const User&);
	User& operator=(const User&);

	CID cid;
};

/** One of possibly many identities of a user, mainly for UI purposes */
class Identity {
public:

	Identity() { }
	Identity(const UserPtr& ptr, uint32_t aSID) : user(ptr) { setSID(aSID); }
	Identity(const Identity& rhs) : user(rhs.user), info(rhs.info) { }
	Identity& operator=(const Identity& rhs) { Lock l1(cs); Lock l2(rhs.cs); user = rhs.user; info = rhs.info; return *this; }
	~Identity() { }

#define GS(n, x) string get##n() const { return get(x); } void set##n(const string& v) { set(x, v); }
	GS(Nick, "NI")
	GS(Description, "DE")
	GS(Ip, "I4")
	GS(UdpPort, "U4")
	GS(Email, "EM")
	GS(Status, "ST")
	//RSX++
	GS(MyInfoType, "MT")
	GS(ISP, "IS")
	GS(TestSURQueued, "TQ")
	GS(TestSURChecked, "TC")
	GS(FileListQueued, "FQ")
	GS(FileListChecked, "FC")
	//END

	void setBytesShared(const string& bs) { set("SS", bs); }
	int64_t getBytesShared() const { return Util::toInt64(get("SS")); }
	
	void setConnection(const string& name) { set("US", name); }
	string getConnection() const;

	void setOp(bool op) { set("OP", op ? "1" : Util::emptyString); }
	void setHub(bool hub) { set("HU", hub ? "1" : Util::emptyString); }
	void setBot(bool bot) { set("BO", bot ? "1" : Util::emptyString); }
	void setHidden(bool hidden) { set("HI", hidden ? "1" : Util::emptyString); }
	const string getTag() const;
	bool supports(const string& name) const;
	bool isHub() const { return !get("HU").empty(); }
	bool isOp() const { return !get("OP").empty(); }
	bool isRegistered() const { return !get("RG").empty(); }
	bool isHidden() const { return !get("HI").empty(); }
	bool isBot() const { return !get("BO").empty(); }
	bool isAway() const { return !get("AW").empty(); }
	bool isTcpActive() const { return (!user->isSet(User::NMDC) && !getIp().empty()) || !user->isSet(User::PASSIVE); }
	bool isUdpActive() const { return !getIp().empty() && !getUdpPort().empty(); }
	//RSX++
	bool isTestSURQueued() const { return !get("TQ").empty(); }
	bool isFileListQueued() const { return !get("FQ").empty(); }
	bool isClientChecked() const { return !get("TC").empty(); }
	bool isFileListChecked() const { return !get("FC").empty(); }
	//END
	const string get(const char* name) const;
	void set(const char* name, const string& val);
	string getSIDString() const { uint32_t sid = getSID(); return string((const char*)&sid, 4); }
	
	uint32_t getSID() const { return Util::toUInt32(get("SI")); }
	void setSID(uint32_t sid) { if(sid != 0) set("SI", Util::toString(sid)); }

	const string getReport() const;
	void getParams(StringMap& map, const string& prefix, bool compatibility) const;

	//RSX++
	void setCheatMsg(Client& c, const string& aCheatDescription, bool aBadClient, bool aBadFilelist = false, bool aDisplayCheat = true);
	bool canICheckUser(OnlineUser& ou, bool filelist = false);
	bool isMyInfoSpamming();
	bool isCtmSpamming();
	bool isPmSpamming();

	void myInfoDetect(OnlineUser& ou);
	bool updateClientType(OnlineUser& ou);
	void isFakeShare(OnlineUser& ou);
	void checkIP(OnlineUser& ou);
	void checkFilelistGenerator(OnlineUser& ou);
	void checkrmDC(OnlineUser& ou);
	void checkSlotsCount(OnlineUser& ou, int realSlots);

	string getVersion() const;
	void cleanUser();
	bool isProtectedUser(const Client& c) const;
	//END
	UserPtr& getUser() { return user; }
	GETSET(UserPtr, user, User);
	GETSET(uint64_t, loggedIn, LoggedIn); //RSX++
private:
	typedef map<short, string> InfMap;
	typedef InfMap::const_iterator InfIter;
	InfMap info;
	/** @todo there are probably more threading issues here ...*/
	mutable CriticalSection cs;

	//RSX++
	//Flood stuff
	uint16_t myinfoFloodCounter;	
	uint16_t cteFloodCounter;
	uint16_t pmFloodCounter;

	uint64_t lastMyInfo;
	uint64_t lastCte;	
	uint64_t lastPm;

	string isEmpty(const string& val) const { return val.empty() ? "N/A" : val; }
	string getFilelistGeneratorVer() const;
	bool isActionActive(Client& client, int id);
	void logDetect(bool successful);
	void checkTagState(OnlineUser& ou);
	bool shouldRecheck() { 
		if(get("R1").empty()) { 
			set("R1", "1"); 
			return true; 
		} 
		return false; 
	}
	//END
};

class NmdcHub;
#include "UserInfoBase.h"

class OnlineUser : public FastAlloc<OnlineUser>, public PointerBase, public UserInfoBase {
public:
	enum {
		COLUMN_FIRST,
		COLUMN_NICK = COLUMN_FIRST, 
		COLUMN_SHARED, 
		COLUMN_EXACT_SHARED, 
		COLUMN_DESCRIPTION, 
		COLUMN_TAG,
		COLUMN_CONNECTION, 
		COLUMN_EMAIL,
		COLUMN_CLIENTS, 
		COLUMN_MYINFOS,
		COLUMN_CHEATING_DESCRIPTION,
		COLUMN_VERSION, 
		COLUMN_MODE, 
		COLUMN_HUBS, 
		COLUMN_SLOTS,
		COLUMN_UPLOAD_SPEED,
		COLUMN_IP,
		COLUMN_HOST,
		COLUMN_ISP,
		COLUMN_PK, 
		COLUMN_LOCK,
		COLUMN_SUPPORT,
		COLUMN_STATUS,
		COLUMN_COMMENT,
		COLUMN_LAST
	};

	typedef vector<OnlineUser*> List;
	typedef List::const_iterator Iter;

	OnlineUser(const UserPtr& ptr, Client& client_, uint32_t sid_);
	~OnlineUser() { }

	operator UserPtr&() { return getUser(); }
	operator const UserPtr&() const { return getUser(); }

	inline UserPtr& getUser() { return getIdentity().getUser(); }
	inline const UserPtr& getUser() const { return getIdentity().getUser(); }
	inline Identity& getIdentity() { return identity; }
	Client& getClient() { return client; }
	const Client& getClient() const { return client; }

	/* UserInfo */
	bool update(int sortCol, const tstring& oldText = Util::emptyStringT);
	uint8_t imageIndex() const { return UserInfoBase::getImage(identity); }
	static int compareItems(const OnlineUser* a, const OnlineUser* b, uint8_t col);
	const string getNick() const { return identity.getNick(); }
	bool isHidden() const { return identity.isHidden(); }
	
	const tstring getText(uint8_t col) const;

	bool isInList;
	//RSX++
	bool isProtectedUser() const { return identity.isProtectedUser(getClient()); }
	bool getChecked(bool filelist = false);
	bool isCheckable(bool delay = true) const;
	bool shouldTestSUR() const { return !identity.isTestSURQueued() && !identity.isClientChecked(); }
	bool shouldCheckFileList(bool onlyFilelist = false) const;
	void initializeData() { identity.setLoggedIn(GET_TICK()); identity.set("LT", Util::formatTime("%d-%m %H:%M", GET_TIME())); }
	void setLoggedIn() { identity.setLoggedIn(GET_TICK()); }
	void setTestSURComplete() { identity.setTestSURChecked(Util::toString(GET_TIME())); }
	void setFileListComplete() { identity.setFileListChecked(Util::toString(GET_TIME())); }
	void updateUser();
	//END
	GETSET(Identity, identity, Identity);
private:
	friend class NmdcHub;

	OnlineUser(const OnlineUser&);
	OnlineUser& operator=(const OnlineUser&);

	Client& client;
};

#endif // !defined(USER_H)

/**
 * @file
 * $Id: User.h 334 2007-11-04 13:04:34Z bigmuscle $
 */
