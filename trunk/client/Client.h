/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_CLIENT_H
#define DCPLUSPLUS_DCPP_CLIENT_H

#include "forward.h"

#include "User.h"
#include "Speaker.h"
#include "BufferedSocketListener.h"
#include "TimerManager.h"
#include "ClientListener.h"
#include "DebugManager.h"
#include "SearchQueue.h"
//RSX++
#include "ScriptManager.h" // Lua
#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "PluginAPI/ClientInterface.h"
#include "CommandQueue.h"
#include "HubSettings.h"
//END
namespace dcpp {
//RSX++
struct ClientScriptInstance : public ScriptInstance {
	bool onHubFrameEnter(Client* aClient, const string& aLine);
	bool onClientMessage(Client* aClient, const string& prot, const string& aLine);
};
//END
/** Yes, this should probably be called a Hub */
class Client : public Speaker<ClientListener>, public iClient, public BufferedSocketListener, protected TimerManagerListener, 
	/*RSX++*/ public ClientScriptInstance, public HubSettings/*END*/ {
public:
	typedef list<Client*> List;
	typedef List::const_iterator Iter;

	virtual void connect();
	virtual void disconnect(bool graceless);

	virtual void connect(const OnlineUser& user, const string& token) = 0;
	virtual void hubMessage(const string& aMessage, bool thirdPerson = false) = 0;
	virtual void privateMessage(const OnlineUser& user, const string& aMessage, bool thirdPerson = false) = 0;
	virtual void sendUserCmd(const string& aUserCmd) = 0;

	uint64_t search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, void* owner);
	void cancelSearch(void* aOwner) { searchQueue.cancelSearch(aOwner); }
	
	virtual void password(const string& pwd) = 0;
	virtual void info(bool force) = 0;

	virtual size_t getUserCount() const = 0;
	int64_t getAvailable() const { return availableBytes; };
	
	virtual void send(const AdcCommand& command) = 0;

	virtual string escape(string const& str) const { return str; }

	bool isConnected() const { return state != STATE_DISCONNECTED; }
	bool isOp() const { return getMyIdentity().isOp(); }

	virtual void refreshUserList(bool) = 0;
	virtual void getUserList(OnlineUserList& list) const = 0;
	virtual OnlineUserPtr findUser(const string& aNick) const = 0;
	
	uint16_t getPort() const { return port; }
	const string& getAddress() const { return address; }

	const string& getIp() const { return ip; }
	string getIpPort() const { return getIp() + ':' + Util::toString(port); }
	string getLocalIp() const;

	void updated(const OnlineUserPtr& aUser) { fire(ClientListener::UserUpdated(), this, aUser); }
	//RSX++
	virtual tstring startChecking(const tstring& aParams) = 0;
	virtual void stopChecking() = 0;
	virtual void stopMyINFOCheck() = 0;

	void attention() { fire(ClientListener::Attention(), this); }
	void addHubLine(const string& aMsg, int mType) { fire(ClientListener::AddClientLine(), this, aMsg, mType); }
	void sendActionCommand(const OnlineUser& ou, int actionId);
	void putDetectors() { stopMyINFOCheck(); stopChecking(); setCheckedAtConnect(false); }
	bool isActionActive(const int aAction) const;

	/** iClient functions **/
	void __cdecl p_hubMessage(const rString& aMsg, bool thirdPerson = false) { hubMessage(aMsg.c_str(), thirdPerson); }
	void __cdecl p_addHubLine(const rString& aMsg, int mType = 0) { addHubLine(aMsg.c_str(), mType); }
	rString __cdecl p_getField(const char* name) { return getHubIdentity().get(name).c_str(); }
	void __cdecl p_setField(const char* name, const rString& value) { getHubIdentity().set(name, value.c_str()); }
	rString __cdecl p_getMyField(const char* name) { return getMyIdentity().get(name).c_str(); }
	void __cdecl p_setMyField(const char* name, const rString& value) { getMyIdentity().set(name, value.c_str()); }
	dcpp::rString __cdecl p_getHubSetting(const char* name) { return get(name).c_str(); }
	void __cdecl p_setHubSetting(const char* name, const dcpp::rString& value) { set(name, value.c_str()); }
	rString __cdecl p_getHubUrl() { return hubUrl.c_str(); }
	void __cdecl p_sendUserCmd(const rString& aUserCmd) { sendUserCmd(aUserCmd.c_str());  }
	iOnlineUser* __cdecl p_getUserByNick(const rString& aNick) { OnlineUserPtr ou = findUser(aNick.c_str()); return ou.get(); }
	void __cdecl p_lock() { cs.enter(); }
	void __cdecl p_unlock() { cs.leave(); }
	//END

	static int getTotalCounts() {
		return counts.normal + counts.registered + counts.op;
	}

	static string getCounts() {
		char buf[128];
		return string(buf, snprintf(buf, sizeof(buf), "%ld/%ld/%ld", counts.normal, counts.registered, counts.op));
	}

	StringMap& escapeParams(StringMap& sm) {
		for(StringMapIter i = sm.begin(); i != sm.end(); ++i) {
			i->second = escape(i->second);
		}
		return sm;
	}
	
	void setSearchInterval(uint32_t aInterval) {
		// min interval is 10 seconds
		searchQueue.interval = max(aInterval + 2000, (uint32_t)(10 * 1000));
	}

	uint32_t getSearchInterval() const {
		return searchQueue.interval;
	}	

	void cheatMessage(const string& msg) {
		fire(ClientListener::CheatMessage(), this, msg);
	}
	
	void reconnect();
	void shutdown();
	bool isActive() const;

	void send(const string& aMessage) { send(aMessage.c_str(), aMessage.length()); }
	void send(const char* aMessage, size_t aLen);

	string getMyNick() const { return getMyIdentity().getNick(); }
	string getHubName() const { return getHubIdentity().getNick().empty() ? getHubUrl() : getHubIdentity().getNick(); }
	string getHubDescription() const { return getHubIdentity().getDescription(); }

	Identity& getHubIdentity() { return hubIdentity; }
	Identity& getMyIdentity() { return myIdentity; }

	const string& getHubUrl() const { return hubUrl; }

	GETSET(Identity, myIdentity, MyIdentity);
	GETSET(Identity, hubIdentity, HubIdentity);

	GETSET(string, defpassword, Password);

	GETSET(string, currentNick, CurrentNick);
	//RSX++
	GETSET(bool, checkedAtConnect, CheckedAtConnect);
	void setCurrentDescription(const string& description) {
		currentDescription = description;
	}
	string getCurrentDescription() const;
	GS_STR(CurrentEmail,			"MAIL")
	GS_STR(UserProtected,			"PROT")
	GS_BOOL(UseFilter,				"UCHF")
	GS_BOOL(UseHL,					"UCHL")
	GS_BOOL(UseAutosearch,			"AUSR")
	GS_BOOL(CheckClients,			"DECC")
	GS_BOOL(CheckFilelists,			"DEFL")
	GS_BOOL(CheckOnConnect,			"DEOC")
	GS_BOOL(CheckMyInfo,			"DEUI")
	GS_BOOL(HideShare,				"HIDS")
	GS_BOOL(ShowIpOnChat,			"SWIP")
	GS_BOOL(ShowCountryCodeOnChat,	"SWCC")
	//END
	GETSET(string, favIp, FavIp);
	
	GETSET(uint64_t, lastActivity, LastActivity);
	GETSET(uint32_t, reconnDelay, ReconnDelay);
	
	GETSET(string*, encoding, Encoding);	
		
	GETSET(bool, registered, Registered);
	GETSET(bool, autoReconnect, AutoReconnect);
	GETSET(bool, stealth, Stealth);

	mutable CriticalSection cs; //RSX++
protected:
	friend class ClientManager;
	Client(const string& hubURL, char separator, bool secure_);
	virtual ~Client() throw();
	struct Counts {
		Counts(long n = 0, long r = 0, long o = 0) : normal(n), registered(r), op(o) { }
		volatile long normal;
		volatile long registered;
		volatile long op;
		bool operator !=(const Counts& rhs) { return normal != rhs.normal || registered != rhs.registered || op != rhs.op; }
	};

	enum States {
		STATE_CONNECTING,	///< Waiting for socket to connect
		STATE_PROTOCOL,		///< Protocol setup
		STATE_IDENTIFY,		///< Nick setup
		STATE_VERIFY,		///< Checking password
		STATE_NORMAL,		///< Running
		STATE_DISCONNECTED,	///< Nothing in particular
	} state;

	SearchQueue searchQueue;
	BufferedSocket* sock;

	static Counts counts;
	Counts lastCounts;

	int64_t availableBytes;

	void updateCounts(bool aRemove);
	void updateActivity() { lastActivity = GET_TICK(); }

	/** Reload details from favmanager or settings */
	void reloadSettings(bool updateNick);

	virtual string checkNick(const string& nick) = 0;
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) = 0;

	// TimerManagerListener
	virtual void on(Second, uint64_t aTick) throw();
	// BufferedSocketListener
	virtual void on(Connecting) throw() { fire(ClientListener::Connecting(), this); }
	virtual void on(Connected) throw();
	virtual void on(Line, const string& aLine) throw();
	virtual void on(Failed, const string&) throw();
	//RSX++
	GETSET(uint32_t, usersLimit, UsersLimit);
	uint32_t userCount;
	//END
private:
	//RSX++
	CommandQueue cmdQueue;
	string currentDescription;
	//END

	enum CountType {
		COUNT_UNCOUNTED,
		COUNT_NORMAL,
		COUNT_REGISTERED,
		COUNT_OP
	};

	Client(const Client&);
	Client& operator=(const Client&);

	CountType countType;
	string hubUrl;
	string address;
	string ip;
	string localIp;
	uint16_t port;
	char separator;
	bool secure;
};

} // namespace dcpp

#endif // !defined(CLIENT_H)

/**
 * @file
 * $Id: Client.h 405 2008-07-14 11:41:15Z BigMuscle $
 */
