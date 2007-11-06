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

#if !defined(ADC_HUB_H)
#define ADC_HUB_H

#include "Client.h"
#include "AdcCommand.h"
#include "TimerManager.h"
#include "User.h"

#include "../rsx/UserMap.h"
#include "ScriptManager.h" //RSX++ // Lua

class ClientManager;
//RSX++ // Lua
class AdcHub;

struct AdcScriptInstance : public ScriptInstance {
	bool onClientMessage(AdcHub* aClient, const string& aLine);
};
//END
class AdcHub : public Client, public CommandHandler<AdcHub>, /*//RSX++ //Lua*/public AdcScriptInstance /*END*/ {
public:
	using Client::send;
	using Client::connect;

	void connect(const OnlineUser& user, const string& token);
	void connect(const OnlineUser& user, string const& token, bool secure);
	
	void hubMessage(const string& aMessage);
	void privateMessage(const OnlineUser& user, const string& aMessage);
	void sendUserCmd(const string& aUserCmd) { send(aUserCmd); }
	void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void password(const string& pwd);
	void info(bool alwaysSend);
	
	size_t getUserCount() const { Lock l(cs); return users.size(); }

	string escape(string const& str) const { return AdcCommand::escape(str, false); }
	void send(const AdcCommand& cmd);

	string getMySID() { return AdcCommand::fromSID(sid); }

	/* these functions not implemented yet */
	void refreshUserList(bool) { }
	void cheatMessage(const string& aLine) {
		fire(ClientListener::CheatMessage(), this, aLine);
	}

private:
	friend class ClientManager;
	friend class CommandHandler<AdcHub>;

	AdcHub(const string& aHubURL, bool secure);

	AdcHub(const AdcHub&);
	AdcHub& operator=(const AdcHub&);
	~AdcHub() throw();

	/** Map session id to OnlineUser */
	//RSX++
	typedef unordered_map<uint32_t, OnlineUser*> SIDMap;
	typedef UserMap<true, SIDMap>::const_iterator SIDIter;
	UserMap<true, SIDMap> users;
	
	void startChecking() { users.startCheck(getCheckClients(), getCheckFilelists()); }
	void stopChecking()	{ users.stopCheck(); }
	bool isDetectorRunning() { return users.isDetectorRunning(); }
	void stopMyINFOCheck() { users.stopMyINFOCheck(); }
	void startCustomCheck(bool clients, bool filelists) { users.startCustomCheck(clients, filelists); }
	//END

	void getUserList(OnlineUser::List& list) const {
		Lock l(cs);
		for(SIDIter i = users.begin(); i != users.end(); i++) {
			i->second->inc();
			list.push_back(i->second);
		}
	}

	Socket udp;
	StringMap lastInfoMap;
	//mutable CriticalSection cs; //rsx++

	string salt;
	uint32_t sid;

	static const string CLIENT_PROTOCOL;
	static const string SECURE_CLIENT_PROTOCOL;
	static const string ADCS_FEATURE;
	static const string TCP4_FEATURE;
	static const string UDP4_FEATURE;

	string checkNick(const string& nick);

	OnlineUser& getUser(const uint32_t aSID, const CID& aCID);
	OnlineUser* findUser(const uint32_t sid) const;
	OnlineUser* findUser(const CID& cid) const;
	
	// just a workaround
	OnlineUser* AdcHub::findUser(const string& aNick) const { 
	   Lock l(cs); 
	   for(SIDMap::const_iterator i = users.begin(); i != users.end(); ++i) { 
		  if(i->second->getIdentity().getNick() == aNick) { 
			 return i->second; 
		  } 
	   } 
	   return NULL; 
	}

	void putUser(const uint32_t sid);

	void clearUsers();

	void handle(AdcCommand::SUP, AdcCommand& c) throw();
	void handle(AdcCommand::SID, AdcCommand& c) throw();
	void handle(AdcCommand::MSG, AdcCommand& c) throw();
	void handle(AdcCommand::INF, AdcCommand& c) throw();
	void handle(AdcCommand::GPA, AdcCommand& c) throw();
	void handle(AdcCommand::QUI, AdcCommand& c) throw();
	void handle(AdcCommand::CTM, AdcCommand& c) throw();
	void handle(AdcCommand::RCM, AdcCommand& c) throw();
	void handle(AdcCommand::STA, AdcCommand& c) throw();
	void handle(AdcCommand::SCH, AdcCommand& c) throw();
	void handle(AdcCommand::CMD, AdcCommand& c) throw();
	void handle(AdcCommand::RES, AdcCommand& c) throw();

	template<typename T> void handle(T, AdcCommand&) { }

	void sendUDP(const AdcCommand& cmd) throw();

	void on(Connecting) throw() { fire(ClientListener::Connecting(), this); }
	void on(Connected) throw();
	void on(Line, const string& aLine) throw();
	void on(Failed, const string& aLine) throw();

	void on(Second, uint64_t aTick) throw();

};

#endif // !defined(ADC_HUB_H)

/**
 * @file
 * $Id: AdcHub.h 326 2007-09-01 16:55:01Z bigmuscle $
 */
