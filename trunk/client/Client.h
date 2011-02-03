/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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
#include "rsxppSettingsManager.h"
#include "CommandQueue.h"
#include "sdk/interfaces/User.hpp"
#include "sdk/interfaces/Hub.hpp"
#include "ChatMessage.h"
//END

namespace dcpp {
class ClientBase
{
public:
	
	ClientBase() : type(DIRECT_CONNECT) { }

	enum P2PType { DIRECT_CONNECT, DHT, ADC };
	P2PType type;
	
	P2PType getType() const { return type; }

	virtual const string& getHubUrl() const = 0;
	virtual string getHubName() const = 0;
	virtual bool isOp() const = 0;
	virtual void connect(const OnlineUser& user, const string& token) = 0;
	virtual void privateMessage(const OnlineUserPtr& user, const string& aMessage, bool thirdPerson = false) = 0;
	
};

/** Yes, this should probably be called a Hub */
class Client : public ClientBase, public Speaker<ClientListener>, public BufferedSocketListener, protected TimerManagerListener,
	public interfaces::Hub
{
public:
	typedef unordered_map<string*, Client*, noCaseStringHash, noCaseStringEq> List;
	typedef List::const_iterator Iter;

	virtual void connect();
	virtual void disconnect(bool graceless);

	virtual void connect(const OnlineUser& user, const string& token) = 0;
	virtual void hubMessage(const string& aMessage, bool thirdPerson = false) = 0;
	virtual void privateMessage(const OnlineUserPtr& user, const string& aMessage, bool thirdPerson = false) = 0;
	virtual void sendUserCmd(const UserCommand& command, const StringMap& params) = 0;

	uint64_t search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, void* owner);
	void cancelSearch(void* aOwner) { searchQueue.cancelSearch(aOwner); }
	
	virtual void password(const string& pwd) = 0;
	virtual void info(bool force) = 0;

	size_t getUserCount() const { return userCount; }
	int64_t getAvailable() const { return availableBytes; };
	
	virtual void send(const AdcCommand& command) = 0;

	virtual string escape(string const& str) const { return str; }

	bool isConnected() const { return state != STATE_DISCONNECTED; }
	bool isReady() const { return state != STATE_CONNECTING && state != STATE_DISCONNECTED; }
	bool isSecure() const;
	bool isTrusted() const;
	std::string getCipherName() const;

	bool isOp() const { return getMyIdentity().isOp(); }

	virtual void refreshUserList(bool) = 0;
	virtual void getUserList(OnlineUserList& list) const = 0;
	virtual OnlineUserPtr findUser(const string& aNick) const = 0;
	//RSX++
	virtual OnlineUser* findUser(const CID& aCid) const = 0;
	virtual OnlineUser* findUser(const uint32_t sid) const = 0;
	//END

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

	void redirect(const std::string& url);// { disconnect(true); fire(ClientListener::Redirect(), this, url); }
	void addHubLine(const string& aMsg, int mType = 0) { fire(ClientListener::AddClientLine(), this, aMsg, mType); }
	void sendActionCommand(const OnlineUser& ou, int actionId);
	void putDetectors() { stopMyINFOCheck(); stopChecking(); setCheckedAtConnect(false); }
	bool isActionActive(const int aAction) const;
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
	void reportUser(const Identity& i) {
		fire(ClientListener::UserReport(), this, i);
	}
	
	void reconnect();
	void shutdown();
	bool isActive() const;

	void send(const string& aMessage) { send(aMessage.c_str(), aMessage.length()); }
	void send(const char* aMessage, size_t aLen, bool bypassPlug = false);

	string getMyNick() const { return getMyIdentity().getNick(); }
	string getHubName() const { return getHubIdentity().getNick().empty() ? getHubUrl() : getHubIdentity().getNick(); }
	string getHubDescription() const { return getHubIdentity().getDescription(); }

	Identity& getHubIdentity() { return hubIdentity; }
	Identity& getMyIdentity() { return myIdentity; } //RSX++

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

	GETSET(string, currentEmail, CurrentEmail);
	GETSET(string, protectedUsers, ProtectedUsers);
	GETSET(string, chatExtraInfo, ChatExtraInfo);
	GETSET(bool, useFilter, UseFilter);
	GETSET(bool, useHL, UseHL);
	GETSET(bool, useAutosearch, UseAutosearch);
	GETSET(bool, checkClients, CheckClients);
	GETSET(bool, checkFilelists, CheckFilelists);
	GETSET(bool, checkOnConnect, CheckOnConnect);
	GETSET(bool, checkMyInfo, CheckMyInfo);
	GETSET(bool, hideShare, HideShare);
	GETSET(uint32_t, usersLimit, UsersLimit);
	//END
	GETSET(string, favIp, FavIp);
	
	GETSET(uint64_t, lastActivity, LastActivity);
	GETSET(uint32_t, reconnDelay, ReconnDelay);
	
	GETSET(string*, encoding, Encoding);	
		
	GETSET(bool, registered, Registered);
	GETSET(bool, autoReconnect, AutoReconnect);
	GETSET(bool, stealth, Stealth);

	mutable CriticalSection cs; //RSX++

	//RSX++
	template<bool incoming>
	bool handleChatMessage(ChatMessage& cm) {
		bool handled = false;
		Lock l(plugins.cs);
		for(ProxyListener::Listeners::iterator i = plugins.ls.begin(); i != plugins.ls.end(); ++i) {
			if(incoming) {
				(*i)->onHub_IncomingMessage(static_cast<interfaces::Hub*>(this), static_cast<interfaces::ChatMessage*>(&cm), handled);
			} else {
				(*i)->onHub_OutgoingMessage(static_cast<interfaces::Hub*>(this), static_cast<interfaces::ChatMessage*>(&cm), handled);
			}
		}
		return handled;
	}

	template<bool incoming>
	bool handleHubLine(const char* line) {
		bool handled = false;
		Lock l(plugins.cs);
		for(ProxyListener::Listeners::iterator i = plugins.ls.begin(); i != plugins.ls.end(); ++i) {
			if(incoming) {
				(*i)->onHub_IncomingCommand(static_cast<interfaces::Hub*>(this), line, handled);
			} else {
				(*i)->onHub_OutgoingCommand(static_cast<interfaces::Hub*>(this), line, handled);
			}
		}
		return handled;
	}
	//END
protected:
	friend class ClientManager;
	Client(const string& hubURL, char separator, bool secure_);
	virtual ~Client() throw();

	friend class PluginsManager; //RSX++

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
	size_t userCount; //RSX++
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

	//RSX++
	void sendMessage(const char* msg, bool thirdPerson = false) {
		hubMessage(msg, thirdPerson);
	}
	void sendPrivateMessage(interfaces::OnlineUser* u, const char* msg, bool thirdPerson) {
		OnlineUser* ou = dynamic_cast<OnlineUser*>(u);
		if(ou) {
			privateMessage(ou, msg, thirdPerson);
		}
	}
	void sendData(const void* data, size_t len) {
		send(static_cast<const char*>(data), len);
	}
	const char* getHubAddress() { return address.c_str(); }
	const char* getHubURL() { return hubUrl.c_str(); }
	const char* getHubIP() { return ip.c_str(); }
	//const char* getLocalIP() = 0;

	bool isAccountRegistered() const { return this->getRegistered(); }
	const char* getAccountNick() { return currentNick.c_str(); }
	const char* getAccountPassword() { return currentDescription.c_str(); }

	interfaces::Identity* getAccountIdentity() { return static_cast<interfaces::Identity*>(&myIdentity); }
	interfaces::Identity* getIdentity() { return static_cast<interfaces::Identity*>(&hubIdentity); }

	dcpp::interfaces::OnlineUser* findOnlineUser(const char* cidOrNick, bool nick = true) {
		if(nick) {
			return this->findUser(cidOrNick).get();
		} else {
			return this->findUser(CID(cidOrNick));
		}
	}

	dcpp::interfaces::OnlineUser* findOnlineUser(uint32_t sid) { return this->findUser(sid); }

	void mutex(bool lock) {
		if(lock)
			cs.lock();
		else
			cs.unlock();
	}

	class ProxyListener : public ClientListener {
	public:
		void addListener(interfaces::HubListener* listener) {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				if(*i == listener)
					return;
			}
			ls.push_back(listener);
		}

		void remListener(interfaces::HubListener* listener) {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				if(*i == listener) {
					ls.erase(i);
					break;
				}
			}
		}

	private:
		friend class Client;

		typedef std::deque<interfaces::HubListener*> Listeners;
		Listeners ls;
		static CriticalSection cs;

		void on(ClientListener::Connecting, const Client* c) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i)
				(*i)->onHub_Connecting(const_cast<Client*>(c));
		}
		void on(ClientListener::Connected, const Client* c) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i)
				(*i)->onHub_Connected(const_cast<Client*>(c));
		}
		void on(ClientListener::UserUpdated, const Client* c, const OnlineUserPtr& u) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i)
				(*i)->onHub_UserUpdated(const_cast<Client*>(c), u.get());
		}
		void on(ClientListener::UsersUpdated, const Client* c, const OnlineUserList& ul) throw() { 
		
		}
		void on(ClientListener::UserRemoved, const Client* c, const OnlineUserPtr& u) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i)
				(*i)->onHub_UserRemoved(const_cast<Client*>(c), u.get());
		}
		void on(ClientListener::Redirect, const Client* c, const string& s) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i)
				(*i)->onHub_Redirect(const_cast<Client*>(c), s.c_str());
		}
		void on(ClientListener::Failed, const Client* c, const string& s) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i)
				(*i)->onHub_Failed(const_cast<Client*>(c), s.c_str());
		}
		void on(ClientListener::HubUpdated, const Client* c) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i)
				(*i)->onHub_HubUpdated(const_cast<Client*>(c));
		}
		void on(HubFull, const Client* c) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i)
				(*i)->onHub_HubFull(const_cast<Client*>(c));
		}
		void on(NickTaken, const Client* c) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i)
				(*i)->onHub_AccountNickTaken(const_cast<Client*>(c));
		}
	}plugins;

	void addEventListener(interfaces::HubListener* listener) {
		plugins.addListener(listener);
	}

	void remEventListener(interfaces::HubListener* listener) {
		plugins.remListener(listener);
	}
	//END
};

} // namespace dcpp

#endif // !defined(CLIENT_H)

/**
 * @file
 * $Id: Client.h 482 2010-02-13 10:49:30Z bigmuscle $
 */
