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

#ifndef DCPLUSPLUS_DCPP_CLIENT_MANAGER_H
#define DCPLUSPLUS_DCPP_CLIENT_MANAGER_H

#include "TimerManager.h"

#include "Client.h"
#include "Singleton.h"
#include "SettingsManager.h"
#include "User.h"
#include "Socket.h"
#include "DirectoryListing.h"

#include "ClientManagerListener.h"
//RSX++
#include "../rsx/IpManager.h"
//END

namespace dcpp {

class UserCommand;

class ClientManager : public Speaker<ClientManagerListener>, 
	private ClientListener, public Singleton<ClientManager>, 
	private TimerManagerListener
{
public:
	Client* getClient(const string& aHubURL);
	void putClient(Client* aClient);

	size_t getUserCount() const;
	int64_t getAvailable() const;
	StringList getHubs(const CID& cid) const;
	StringList getHubNames(const CID& cid) const;
	StringList getNicks(const CID& cid) const;
	string getConnection(const CID& cid) const;

	bool isConnected(const string& aUrl) const;
	
	void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void infoUpdated();

	UserPtr getUser(const string& aNick, const string& aHubUrl) throw();
	UserPtr getUser(const CID& cid) throw();

	string findHub(const string& ipPort) const;
	const string& findHubEncoding(const string& aUrl) const;

	UserPtr findUser(const string& aNick, const string& aHubUrl) const throw() { return findUser(makeCid(aNick, aHubUrl)); }
	UserPtr findUser(const CID& cid) const throw();
	UserPtr findLegacyUser(const string& aNick) const throw();

	void updateNick(const UserPtr& user, const string& nick) throw();

	bool isOnline(const UserPtr& aUser) const {
		Lock l(cs);
		return onlineUsers.find(aUser->getCID()) != onlineUsers.end();
	}
	
	void setIPUser(const string& IP, const UserPtr& user, bool resolveHost = false) {
		string aHost = resolveHost ? Socket::getRemoteHost(IP) : Util::emptyString;
		Lock l(cs);
		OnlinePairC p = onlineUsers.equal_range(user->getCID());
		for(OnlineIterC i = p.first; i != p.second; i++) {
			OnlineUser* ou = i->second;
			//RSX++ //IP check
			ou->getIdentity().setIp(IP);
			if(!aHost.empty())
				ou->getIdentity().set("HT", aHost);
			if(ou->getIdentity().get("IC").empty())
				ou->getIdentity().checkIP(*ou);
			//END
		}
	}
	
	string getMyNMDCNick(const UserPtr& p) const {
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i != onlineUsers.end()) {
			return i->second->getClient().getMyNick();
		}
		return Util::emptyString;
	}

	void reportUser(const UserPtr& p) {
		string nick; string report;
		Client* c;
		{
			Lock l(cs);
			OnlineIterC i = onlineUsers.find(p->getCID());
			if(i == onlineUsers.end()) return;

			nick = i->second->getIdentity().getNick();
			report = i->second->getIdentity().getReport();
			c = &i->second->getClient();
		}
		c->cheatMessage("*** Info on " + nick + " ***" + "\r\n" + report + "\r\n");
	}
	//RSX++ // Clean User
	void cleanUser(const UserPtr& p) {
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end()) return;

		i->second->getIdentity().cleanUser();
		i->second->getClient().updated(*i->second);
	}
	//RSX++ //Hide Share
	bool getSharingHub(const UserPtr& p) {
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i != onlineUsers.end()) {
			return i->second->getClient().getHideShare();
		}
		return false;
	}
	//RSX++ //check slot count
	void checkSlots(const UserPtr& p, int slots) {
		Client* c = NULL;
		string report = Util::emptyString;
		{
			Lock l(cs);
			OnlineIterC i = onlineUsers.find(p->getCID());
			if(i == onlineUsers.end()) return;
			c = &i->second->getClient();
			if(i->second->getIdentity().get("SC").empty())
				report = i->second->getIdentity().checkSlotsCount((*i->second), slots);
		}
		if(c != NULL && !report.empty()) {
			c->cheatMessage(report);
		}
	}
	//END
	void updateUser(const UserPtr& p) {
		OnlineUser* ou;
		{
			Lock l(cs);
			OnlineIterC i = onlineUsers.find(p->getCID());
			if(i == onlineUsers.end()) return;

			ou = i->second;
		}
		ou->getClient().updated(*ou);
	}

	void setPkLock(const UserPtr& p, const string& aPk, const string& aLock) {
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end()) return;
		i->second->getIdentity().set("PK", aPk);
		i->second->getIdentity().set("LO", aLock);
	}

	void setSupports(const UserPtr& p, const string& aSupports) {
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end()) return;
		i->second->getIdentity().set("SU", aSupports);
	}

	void setGenerator(const UserPtr& p, const string& aGenerator, const string& aCID, const string& aBase) {
		Client* c = NULL;
		string report = Util::emptyString;
		{
			Lock l(cs);
			OnlineIterC i = onlineUsers.find(p->getCID());
			if(i == onlineUsers.end()) return;
			i->second->getIdentity().set("GE", aGenerator);
			i->second->getIdentity().set("FI", aCID);
			i->second->getIdentity().set("FB", aBase);
			report = i->second->getIdentity().checkFilelistGenerator((*i->second));
		}
		if(c != NULL && !report.empty()) {
			c->cheatMessage(report);
		}
	}

	void setUnknownCommand(const UserPtr& p, const string& aUnknownCommand) {
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end()) return;
		i->second->getIdentity().set("UC", aUnknownCommand);
	}

	bool isOp(const UserPtr& aUser, const string& aHubUrl) const;
	bool isStealth(const string& aHubUrl) const;

	/** Constructs a synthetic, hopefully unique CID */
	CID makeCid(const string& nick, const string& hubUrl) const throw();

	void putOnline(OnlineUser* ou) throw();
	void putOffline(OnlineUser* ou, bool disconnect = false) throw();

	UserPtr& getMe();
	
	void connect(const UserPtr& p, const string& token);
	void send(AdcCommand& c, const CID& to);
	void privateMessage(const UserPtr& p, const string& msg, bool thirdPerson);

	void userCommand(const UserPtr& p, const UserCommand& uc, StringMap& params, bool compatibility);
	void sendRawCommand(const UserPtr& user, const string& aRaw, bool checkProtection = false);

	int getMode(const string& aHubUrl) const;
	bool isActive(const string& aHubUrl) const { return getMode(aHubUrl) != SettingsManager::INCOMING_FIREWALL_PASSIVE; }

	void lock() throw() { cs.enter(); }
	void unlock() throw() { cs.leave(); }

	//RSX++
	void sendAction(const UserPtr& p, const int aAction);
	void sendAction(OnlineUser& ou, const int aAction);
	void kickFromAutosearch(const UserPtr& p, int action, const string& cheat, const string& file, const string& size, const string& tth, bool display = false);
	void multiHubKick(const UserPtr& p, const string& aRaw);
	tstring getHubsLoadInfo() const;
	//END
	string getHubUrl(const UserPtr& aUser) const;

	const Client::List& getClients() const { return clients; }

	CID getMyCID();
	const CID& getMyPID();
	
	// fake detection methods
	void setListLength(const UserPtr& p, const string& listLen);
	void setListSize(const UserPtr& p, int64_t aFileLength, bool adc);
	void fileListDisconnected(const UserPtr& p);
	void connectionTimeout(const UserPtr& p);
	void checkCheating(const UserPtr& p, DirectoryListing* dl);
	void setCheating(const UserPtr& p, const string& _ccResponse, const string& _cheatString, int _actionId, bool _displayCheat,
		bool _badClient, bool _badFileList, bool _clientCheckComplete, bool _fileListCheckComplete);
	//RSX++
	void addCheckToQueue(const UserPtr& p, bool filelist);
	static bool ucExecuteLua(const string& cmd, StringMap& params);
	//END

private:
	//RSX++
	bool compareUsers(const OnlineUser& ou1, const OnlineUser& ou2);
	//END
	typedef unordered_map<CID, UserPtr> UserMap;
	typedef UserMap::iterator UserIter;

	typedef unordered_map<CID, std::string> NickMap;

	typedef unordered_multimap<CID, OnlineUser*> OnlineMap;
	typedef OnlineMap::iterator OnlineIter;
	typedef OnlineMap::const_iterator OnlineIterC;
	typedef pair<OnlineIter, OnlineIter> OnlinePair;
	typedef pair<OnlineIterC, OnlineIterC> OnlinePairC;

	Client::List clients;
	mutable CriticalSection cs;
	
	UserMap users;
	OnlineMap onlineUsers;
	NickMap nicks;

	UserPtr me;
	
	CID pid;	

	friend class Singleton<ClientManager>;

	ClientManager() {
		TimerManager::getInstance()->addListener(this); 
	}

	~ClientManager() throw() {
		TimerManager::getInstance()->removeListener(this); 
	}

	void updateNick(const OnlineUser& user) throw();
	
	// ClientListener
	void on(Connected, const Client* c) throw();
	void on(UserUpdated, const Client*, const OnlineUser& user) throw();
	void on(UsersUpdated, const Client* c, const OnlineUserList&) throw();
	void on(Failed, const Client*, const string&) throw();
	void on(HubUpdated, const Client* c) throw();
	void on(HubUserCommand, const Client*, int, int, const string&, const string&) throw();
	void on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize,
		int aFileType, const string& aString, bool) throw();
	void on(AdcSearch, const Client* c, const AdcCommand& adc, const CID& from) throw();
	// TimerManagerListener
	void on(TimerManagerListener::Minute, uint64_t aTick) throw();
};

} // namespace dcpp

#endif // !defined(CLIENT_MANAGER_H)

/**
 * @file
 * $Id: ClientManager.h 387 2008-05-16 10:41:48Z BigMuscle $
 */
