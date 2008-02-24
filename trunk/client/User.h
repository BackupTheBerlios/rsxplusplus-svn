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
//RSX++
#include "TimerManager.h"
#include "../rsx/PluginAPI/UserInterface.h"
//END

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
		NO_ADC_1_0_PROTOCOL = 0x400,	//< Doesn't support "ADC/1.0" (dc++ <=0.703)
		NO_ADC_0_10_PROTOCOL = 0x800,	//< Doesn't support "ADC/0.10"
		NO_ADCS_0_10_PROTOCOL = 0x1000,	//< Doesn't support "ADCS/0.10"
		//RSX++
		PROTECTED 	= 0x2000 //< User protected
		//END
	};

	struct Hash {
		size_t operator()(const UserPtr& x) const { return ((size_t)(&(*x)))/sizeof(User); }
	};

	User(const CID& aCID) : cid(aCID), lastDownloadSpeed(0), firstNick(Util::emptyString), soundActive(true) { }

	~User() throw() { }

	const CID& getCID() const { return cid; }
	operator const CID&() const { return cid; }

	bool isOnline() const { return isSet(ONLINE); }
	bool isNMDC() const { return isSet(NMDC); }
	const string& getNick(bool first = true) const;

	GETSET(string, firstNick, FirstNick);
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
	enum ClientType {
		CT_BOT = 1,
		CT_REGGED = 2,
		CT_OP = 4,
		CT_SU = 8,
		CT_OWNER = 16,
		CT_HUB = 32
	};

	Identity() { resetCounters(); }
	Identity(const UserPtr& ptr, uint32_t aSID) : user(ptr) { setSID(aSID); resetCounters(); }
	Identity(const Identity& rhs) { *this = rhs; } // Use operator= since we have to lock before reading...
	Identity& operator=(const Identity& rhs) { FastLock l(cs); user = rhs.user; info = rhs.info; return *this; }

#define GS(n, x) string get##n() const { return get(x); } void set##n(const string& v) { set(x, v); }
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
	GS(ClientType, "CL")
	GS(ClientComment, "CM")
	//END

	void setNick(const string& aNick) {
		if(!user || !user->isSet(User::NMDC)) {
			set("NI", aNick);
		}
	}

	const string& getNick() const {
		if(user && user->isSet(User::NMDC)) {
			return user->getFirstNick();
		} else {
			FastLock l(cs);
			InfMap::const_iterator i = info.find(*(short*)"NI");
			return i == info.end() ? Util::emptyString : i->second;
		}
	}

	void setBytesShared(const string& bs) { set("SS", bs); }
	int64_t getBytesShared() const { return Util::toInt64(get("SS")); }
	
	void setConnection(const string& name) { set("US", name); }
	string getConnection() const;

	void setOp(bool op) { set("OP", op ? "1" : Util::emptyString); }
	void setHub(bool hub) { set("HU", hub ? "1" : Util::emptyString); }
	void setBot(bool bot) { set("BO", bot ? "1" : Util::emptyString); }
	void setHidden(bool hidden) { set("HI", hidden ? "1" : Util::emptyString); }
	string getTag() const;
	bool supports(const string& name) const;
	bool isHub() const { return isClientType(CT_HUB) || isSet("HU"); }
	bool isOp() const { return isClientType(CT_OP) || isSet("OP"); }
	bool isRegistered() const { return isClientType(CT_REGGED) || isSet("RG"); }
	bool isHidden() const { return isSet("HI"); }
	bool isBot() const { return isClientType(CT_BOT) || isSet("BO"); }
	bool isAway() const { return isSet("AW"); }
	bool isTcpActive() const { return (!user->isSet(User::NMDC) && !getIp().empty()) || !user->isSet(User::PASSIVE); }
	bool isUdpActive() const { return !getIp().empty() && !getUdpPort().empty(); }
	//RSX++
	bool isTestSURQueued() const { return isSet("TQ"); }
	bool isFileListQueued() const { return isSet("FQ"); }
	bool isClientChecked() const { return isSet("TC"); }
	bool isFileListChecked() const { return isSet("FC"); }
	//END
	string get(const char* name) const;
	void set(const char* name, const string& val);
	bool isSet(const char* name) const;	
	string getSIDString() const { uint32_t sid = getSID(); return string((const char*)&sid, 4); }
	
	uint32_t getSID() const { return Util::toUInt32(get("SI")); }
	void setSID(uint32_t sid) { if(sid != 0) set("SI", Util::toString(sid)); }

	bool isClientType(ClientType ct) const;

	const string getReport() const;
	void getParams(StringMap& map, const string& prefix, bool compatibility) const;

	//RSX++
	string setCheat(const Client& c, const string& aCheatDescription, bool aBadClient, bool aBadFilelist = false, bool aDisplayCheat = true);
	bool canICheckUser(OnlineUser& ou, bool filelist = false);
	bool isMyInfoSpamming();
	bool isCtmSpamming();
	bool isPmSpamming();

	const string myInfoDetect(OnlineUser& ou);
	const string updateClientType(OnlineUser& ou);
	void checkIP(OnlineUser& ou);
	const string checkFilelistGenerator(OnlineUser& ou);
	const string checkrmDC(OnlineUser& ou);
	const string checkSlotsCount(OnlineUser& ou, int realSlots);

	string getVersion() const;
	void cleanUser();
	bool isProtectedUser(const Client& c, bool OpBotHubCheck) const;
	//END
	UserPtr& getUser() { return user; }
	GETSET(UserPtr, user, User);
	GETSET(uint64_t, loggedIn, LoggedIn); //RSX++
private:
	typedef std::tr1::unordered_map<short, string> InfMap;
	typedef InfMap::const_iterator InfIter;
	InfMap info;

	static FastCriticalSection cs;
	//RSX++
	//Flood stuff
	uint16_t myinfoFloodCounter;	
	uint16_t cteFloodCounter;
	uint16_t pmFloodCounter;

	uint64_t lastMyInfo;
	uint64_t lastCte;	
	uint64_t lastPm;

	void resetCounters() {
		set("MC", Util::emptyString);
		myinfoFloodCounter = cteFloodCounter = pmFloodCounter = 0;
		lastMyInfo = lastCte = lastPm = 0;
	}

	string isEmpty(const string& val) const { return val.empty() ? "N/A" : val; }
	string getFilelistGeneratorVer() const;
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

class OnlineUser : public FastAlloc<OnlineUser>, public PointerBase, public UserInfoBase, public iUser {
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
	inline const string setCheat(const string& aCheat, bool aBadClient, bool aBadFilelist = false, bool aDisplayCheat = true) {
		return identity.setCheat(getClient(), aCheat, aBadClient, aBadFilelist, aDisplayCheat);
	}
	inline bool isProtectedUser(bool checkOp = true) const { return identity.isProtectedUser(getClient(), checkOp); }
	bool getChecked(bool filelist = false);
	bool isCheckable(bool delay = true) const;
	bool shouldTestSUR() const { return (!identity.isTestSURQueued() && !identity.isClientChecked()); }
	bool shouldCheckFileList(bool onlyFilelist = false) const;
	void initializeData() { identity.setLoggedIn(GET_TICK()); identity.set("LT", Util::formatTime("%d-%m %H:%M", GET_TIME())); }
	void setTestSURComplete() { identity.setTestSURChecked(Util::toString(GET_TIME())); }
	void setFileListComplete() { identity.setFileListChecked(Util::toString(GET_TIME())); }
	void updateUser();
	
	/** User Interface **/
	string iGet(const char* name) const { 
		return getIdentity().get(name); 
	}
	// get field value from user identity
	string iGetNick() const { 
		return getNick(); 
	}
	// send private message to user
	void sendPM(const std::string& aMsg);
	//get user client
	iClient* getUserClient();
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
 * $Id: User.h 355 2008-01-05 14:43:39Z bigmuscle $
 */
