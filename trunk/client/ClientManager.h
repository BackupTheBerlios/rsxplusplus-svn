/*
 * Copyright (C) 2001-2009 Jacek Sieka, arnetheduck on gmail point com
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
	//RSX++
	typedef unordered_multimap<CID*, OnlineUser*> OnlineMap;
	typedef OnlineMap::iterator OnlineIter;
	typedef OnlineMap::const_iterator OnlineIterC;

	~ClientManager() throw() {
		TimerManager::getInstance()->removeListener(this); 
	}

	void openHub(const string& url) {
		fire(ClientManagerListener::ClientOpen(), url);
	}

	void closeHub(const string& url) {
		fire(ClientManagerListener::ClientClose(), url);
	}
	//END

	Client* getClient(const string& aHubURL);
	void putClient(Client* aClient);

	StringList getHubs(const CID& cid) const;
	StringList getHubNames(const CID& cid) const;
	StringList getNicks(const CID& cid) const;
	string getConnection(const CID& cid) const;
	uint8_t getSlots(const CID& cid) const;

	bool isConnected(const string& aUrl) const;
	
	void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, void* aOwner = 0);
	uint64_t search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, void* aOwner = 0);
	
	void cancelSearch(void* aOwner);
		
	void infoUpdated();

	UserPtr getUser(const string& aNick, const string& aHubUrl) throw();
	UserPtr getUser(const CID& cid) throw();

	string findHub(const string& ipPort) const;
	const string& findHubEncoding(const string& aUrl) const;

	UserPtr findUser(const string& aNick, const string& aHubUrl) const throw() { return findUser(makeCid(aNick, aHubUrl)); }
	UserPtr findUser(const CID& cid) const throw();
	UserPtr findLegacyUser(const string& aNick) const throw();
	
	void updateNick(const UserPtr& user, const string& nick) throw();
	string getMyNick(const string& hubUrl) const;
	
	void setIPUser(const UserPtr& user, const string& IP, uint16_t udpPort = 0, bool resolveHost = false) {
		if(IP.empty())
			return;
		string aHost = resolveHost ? Socket::getRemoteHost(IP) : Util::emptyString;
		Lock l(cs);
		OnlinePairC p = onlineUsers.equal_range(const_cast<CID*>(&user->getCID()));
		for (OnlineIterC i = p.first; i != p.second; i++) {
			if(i->second->getUser()->isSet(User::DHT)) continue;
			i->second->getIdentity().setIp(IP);
			if(udpPort > 0)
				i->second->getIdentity().setUdpPort(Util::toString(udpPort));
			//RSX++ //IP check
			if(!aHost.empty())
				i->second->getIdentity().set("HT", aHost);
			if(i->second->getIdentity().get("IC").empty())
				i->second->getIdentity().checkIP(*i->second);
			//END
		}
	}
	
	void reportUser(const UserPtr& p, const string& hubHint) {
		string nick, report;
		Client* c = 0;
		{
			Lock l(cs);
			OnlineUser* u = findOnlineUser(p->getCID(), hubHint);
			if(!u)
				return;

			nick = u->getIdentity().getNick();
			report = u->getIdentity().getReport();
			c = &u->getClient();
		}
		if(c)
			c->cheatMessage("*** Info on " + nick + " ***" + "\r\n" + report + "\r\n");
	}
	//RSX++ // Clean User
	void cleanUser(const UserPtr& p) {
		Lock l(cs);
		OnlineUser* ou = findOnlineUser(p->getCID(), Util::emptyString);
		if(!ou)
			return;

		ou->getIdentity().cleanUser();
		ou->getClient().updated(ou);
	}
	//RSX++ //Hide Share
	bool getSharingHub(const UserPtr& p, const string& hubHint = Util::emptyString) {
		Client* c = 0;
		{
			Lock l(cs);
			OnlineUser* ou = findOnlineUser(p->getCID(), hubHint);
			if(!ou)
				return false;
			c = &ou->getClient();
		}
		return c ? c->getHideShare() : false;
	}
	//RSX++ //check slot count
	void checkSlots(const UserPtr& p, int slots) {
		Client* c = 0;
		string report = Util::emptyString;
		{
			Lock l(cs);
			OnlineUser* ou = findOnlineUser(p->getCID(), Util::emptyString);
			if(!ou)
				return;
			c = &ou->getClient();
			if(ou->getIdentity().get("SC").empty())
				report = ou->getIdentity().checkSlotsCount(*ou, slots);
		}
		if(c && !report.empty()) {
			c->cheatMessage(report);
		}
	}
	//END

	bool isOp(const UserPtr& aUser, const string& aHubUrl) const;
	bool isStealth(const string& aHubUrl) const;

	/** Constructs a synthetic, hopefully unique CID */
	CID makeCid(const string& nick, const string& hubUrl) const throw();

	void putOnline(OnlineUser* ou) throw();
	void putOffline(OnlineUser* ou, bool disconnect = false) throw();

	UserPtr& getMe();
	
	void connect(const UserPtr& p, const string& token, const string& hintUrl);
	void send(AdcCommand& c, const CID& to);
	void privateMessage(const UserPtr& p, const string& msg, bool thirdPerson, const string& hintUrl);

	void userCommand(const UserPtr& p, const UserCommand& uc, StringMap& params, bool compatibility);
	void sendRawCommand(const UserPtr& user, const string& aRaw, bool checkProtection = false);

	int getMode(const string& aHubUrl) const;
	bool isActive(const string& aHubUrl = Util::emptyString) const { return getMode(aHubUrl) != SettingsManager::INCOMING_FIREWALL_PASSIVE; }

	void lock() throw() { cs.enter(); }
	void unlock() throw() { cs.leave(); }

	//RSX++
	void sendAction(const UserPtr& p, const int aAction);
	void sendAction(OnlineUser& ou, const int aAction);
	void kickFromAutosearch(const UserPtr& p, int action, const string& cheat, const string& file, const string& size, const string& tth, bool display = false);
	void multiHubKick(const UserPtr& p, const string& aRaw);
	tstring getHubsLoadInfo() const;
	//END

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
	const OnlineMap& getOnlineUsers() const { Lock l(cs); return onlineUsers; }
	//END

	// NMDC functions only!!!
	void setPkLock(const UserPtr& p, const string& aPk, const string& aLock);
	void setSupports(const UserPtr& p, const string& aSupports);
	
	void setGenerator(const UserPtr& p, const string& aGenerator, const string& aCID, const string& aBase);
	void setUnknownCommand(const UserPtr& p, const string& aUnknownCommand);

private:
	//RSX++
	bool compareUsers(const OnlineUser& ou1, const OnlineUser& ou2) const;
	//END
	typedef unordered_map<CID*, UserPtr> UserMap;
	typedef UserMap::iterator UserIter;

	typedef unordered_map<CID*, std::string> NickMap;

	//typedef unordered_multimap<CID*, OnlineUser*> OnlineMap;
	//typedef OnlineMap::iterator OnlineIter;
	//typedef OnlineMap::const_iterator OnlineIterC;
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

//	~ClientManager() throw() {
//		TimerManager::getInstance()->removeListener(this); 
//	}

	void updateNick(const OnlineUser& user) throw();
		
	OnlineUser* findOnlineUser(const CID& cid, const string& hintUrl = Util::emptyString) throw();

	// ClientListener
	void on(Connected, const Client* c) throw();
	void on(UserUpdated, const Client*, const OnlineUserPtr& user) throw();
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
 * $Id: ClientManager.h 450 2009-07-05 15:02:34Z BigMuscle $
 */
