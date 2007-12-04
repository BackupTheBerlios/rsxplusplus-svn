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

#if !defined(CLIENT_H)
#define CLIENT_H

#include "User.h"
#include "BufferedSocket.h"
#include "SettingsManager.h"
#include "TimerManager.h"
#include "ClientListener.h"
#include "DebugManager.h"
//RSX++
#include "ScriptManager.h" // Lua
#include "../rsx/rsx-settings/rsx-SettingsManager.h"

struct ClientScriptInstance : public ScriptInstance {
	bool onHubFrameEnter(Client* aClient, const string& aLine);
};
//END
/** Yes, this should probably be called a Hub */
class Client : public Speaker<ClientListener>, public BufferedSocketListener, protected TimerManagerListener, /*//RSX++ // Lua*/ public ClientScriptInstance /*END*/ {
public:
	typedef Client* Ptr;
	typedef slist<Ptr> List;
	typedef List::const_iterator Iter;

	virtual void connect();
	virtual void disconnect(bool graceless);

	virtual void connect(const OnlineUser& user, const string& token) = 0;
	virtual void hubMessage(const string& aMessage) = 0;
	virtual void privateMessage(const OnlineUser& user, const string& aMessage) = 0;
	virtual void sendUserCmd(const string& aUserCmd) = 0;
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) = 0;
	virtual void password(const string& pwd) = 0;
	virtual void info(bool force) = 0;
	virtual void cheatMessage(const string& aLine) = 0;

	virtual size_t getUserCount() const = 0;
	int64_t getAvailable() const { return availableBytes; };
	
	virtual void send(const AdcCommand& command) = 0;

	virtual string escape(string const& str) const { return str; }

	bool isConnected() const throw() { return socket && socket->isConnected(); }
	bool isOp() const { return getMyIdentity().isOp(); }

	virtual void refreshUserList(bool) = 0;
	virtual void getUserList(OnlineUser::List& list) const = 0;
	virtual OnlineUser* findUser(const string& aNick) const = 0;

	uint16_t getPort() const { return port; }
	const string& getAddress() const { return address; }

	const string& getIp() const { return ip; }
	string getIpPort() const { return getIp() + ':' + Util::toString(port); }
	string getLocalIp() const;

	void updated(const OnlineUser& aUser) { fire(ClientListener::UserUpdated(), this, aUser); }
	//RSX++
	void attention() { fire(ClientListener::Attention(), this); }
	virtual void startChecking() = 0;
	virtual void stopChecking() = 0;
	virtual bool isDetectorRunning() = 0;
	virtual void stopMyINFOCheck() = 0;
	virtual void startCustomCheck(bool clients, bool filelists) = 0;
	void addHubLine(const string& aMsg, int mType) { fire(ClientListener::AddClientLine(), this, aMsg, mType); }
	void sendActionCommand(const OnlineUser& ou, int actionId);
	//void statusMessage(const string& aLine) { fire(ClientListener::StatusMessage(), this, aLine); }
	void putDetectors() { Lock l(cs); stopMyINFOCheck(); stopChecking(); setCheckedAtConnect(false); }
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

	void reconnect();
	void shutdown();
	bool isActive() const;

	void send(const string& aMessage) { send(aMessage.c_str(), aMessage.length()); }
	void send(const char* aMessage, size_t aLen) {
		if(!socket)
			return;
		updateActivity();
		socket->write(aMessage, aLen);
		COMMAND_DEBUG(aMessage, DebugManager::HUB_OUT, getIpPort());
	}

	string getMyNick() const { return getMyIdentity().getNick(); }
	string getHubName() const { return getHubIdentity().getNick().empty() ? getHubUrl() : getHubIdentity().getNick(); }
	string getHubDescription() const { return getHubIdentity().getDescription(); }

	Identity& getHubIdentity() { return hubIdentity; }

	const string& getHubUrl() const { return hubUrl; }

	GETSET(Identity, myIdentity, MyIdentity);
	GETSET(Identity, hubIdentity, HubIdentity);

	GETSET(string, defpassword, Password);
	
	GETSET(string, currentNick, CurrentNick);
	GETSET(string, currentDescription, CurrentDescription);
	//RSX++
	//we can use unused by protocol variables to stroe our settings
	//it will give a little amount of free memory... it's always something ;)
	//most common types are string and bool
#define GSS(n, x) string get##n() const { return hubIdentity.get(x); } \
	void set##n(const string& v) { hubIdentity.set(x, v); }
#define GSB(n, x) bool get##n() const { return !hubIdentity.get(x).empty(); } \
	void set##n(bool v) { hubIdentity.set(x, v ? "1" : Util::emptyString); }

	GSS(CurrentEmail, "11");
	GSS(UserProtected, "12");
	GSB(UseFilter, "13");
	GSB(UseAutosearch, "14");
	GSB(UseHL, "15");
	GSB(CheckClients, "51");
	GSB(CheckFilelists, "52");
	GSB(CheckOnConnect, "53");
	GSB(CheckMyInfo, "54");
	GSB(HideShare, "55");
	GSB(CheckFakeShare, "56");
	GSB(CheckedAtConnect, "57");
	//END
	GETSET(string, favIp, FavIp);
	
	GETSET(uint64_t, lastActivity, LastActivity);
	GETSET(uint32_t, reconnDelay, ReconnDelay);
	
	GETSET(string*, encoding, Encoding);	
		
	GETSET(bool, registered, Registered);
	GETSET(bool, autoReconnect, AutoReconnect);
	GETSET(bool, stealth, Stealth);

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

	BufferedSocket* socket;

	static Counts counts;
	Counts lastCounts;

	int64_t availableBytes;

	void updateCounts(bool aRemove);
	void updateActivity() { lastActivity = GET_TICK(); }

	/** Reload details from favmanager or settings */
	void reloadSettings(bool updateNick);

	virtual string checkNick(const string& nick) = 0;

	// TimerManagerListener
	virtual void on(Second, uint64_t aTick) throw();
	// BufferedSocketListener
	virtual void on(Connecting) throw() { fire(ClientListener::Connecting(), this); }
	virtual void on(Connected) throw();
	virtual void on(Line, const string& aLine) throw();
	virtual void on(Failed, const string&) throw();

	//RSX++
	GETSET(uint16_t, usersLimit, UsersLimit);
	void insertRaw(const string& aRawCmd);
	//END
	mutable CriticalSection cs;

private:
	void putSender(bool clear = false); //RSX++

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
	uint16_t port;
	char separator;
	bool secure;
};

#endif // !defined(CLIENT_H)

/**
 * @file
 * $Id: Client.h 317 2007-08-04 14:52:24Z bigmuscle $
 */
