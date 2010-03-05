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

#ifndef DCPLUSPLUS_DCPP_HUBENTRY_H_
#define DCPLUSPLUS_DCPP_HUBENTRY_H_

#include "rsxppSettingsManager.h" //RSX++

namespace dcpp {

class HubEntry {
public:
	typedef vector<HubEntry> List;
	
	HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) throw() : 
	name(aName), server(aServer), description(aDescription), country(Util::emptyString), 
	rating(Util::emptyString), reliability(0.0), shared(0), minShare(0), users(Util::toInt(aUsers)), minSlots(0), maxHubs(0), maxUsers(0) { }

	HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers, const string& aCountry,
		const string& aShared, const string& aMinShare, const string& aMinSlots, const string& aMaxHubs, const string& aMaxUsers,
		const string& aReliability, const string& aRating) : name(aName), server(aServer), description(aDescription), country(aCountry), 
		rating(aRating), reliability((float)(Util::toFloat(aReliability) / 100.0)), shared(Util::toInt64(aShared)), minShare(Util::toInt64(aMinShare)),
		users(Util::toInt(aUsers)), minSlots(Util::toInt(aMinSlots)), maxHubs(Util::toInt(aMaxHubs)), maxUsers(Util::toInt(aMaxUsers)) 
	{

	}

	HubEntry() throw() { }
	HubEntry(const HubEntry& rhs) throw() : name(rhs.name), server(rhs.server), description(rhs.description), country(rhs.country), 
		rating(rhs.rating), reliability(rhs.reliability), shared(rhs.shared), minShare(rhs.minShare), users(rhs.users), minSlots(rhs.minSlots),
		maxHubs(rhs.maxHubs), maxUsers(rhs.maxUsers) { }

	~HubEntry() throw() { }

	GETSET(string, name, Name);
	GETSET(string, server, Server);
	GETSET(string, description, Description);
	GETSET(string, country, Country);
	GETSET(string, rating, Rating);
	GETSET(float, reliability, Reliability);
	GETSET(int64_t, shared, Shared);
	GETSET(int64_t, minShare, MinShare);
	GETSET(int, users, Users);
	GETSET(int, minSlots, MinSlots);
	GETSET(int, maxHubs, MaxHubs);
	GETSET(int, maxUsers, MaxUsers);
};

class FavoriteHubEntry {
public:
	typedef FavoriteHubEntry* Ptr;
	typedef vector<Ptr> List;
	typedef List::const_iterator Iter;

	FavoriteHubEntry() throw() : connect(false), encoding(Text::systemCharset), chatusersplit(0), stealth(false), userliststate(true), mode(0), ip(Util::emptyString), searchInterval(SETTING(MINIMUM_SEARCH_INTERVAL)),
		//RSX++
		currentEmail(Util::emptyString), protectedUsers(Util::emptyString), useFilter(RSXPP_BOOLSETTING(USE_FILTER_FAV)), 
		useHL(RSXPP_BOOLSETTING(USE_HL_FAV)), useAutosearch(false), checkClients(false), 
		checkFilelists(false), checkOnConnect(false), checkMyInfo(false), 
		hideShare(false), chatExtraInfo(Util::emptyString), usersLimit(0) 
		//END
	{ }
	FavoriteHubEntry(const HubEntry& rhs) throw() : name(rhs.getName()), server(rhs.getServer()), encoding(Text::systemCharset), searchInterval(SETTING(MINIMUM_SEARCH_INTERVAL)),
		description(rhs.getDescription()), connect(false), chatusersplit(0), stealth(false), userliststate(true), mode(0), ip(Util::emptyString),
		//RSX++
		currentEmail(Util::emptyString), protectedUsers(Util::emptyString), useFilter(RSXPP_BOOLSETTING(USE_FILTER_FAV)), 
		useHL(RSXPP_BOOLSETTING(USE_HL_FAV)), useAutosearch(false), checkClients(false), 
		checkFilelists(false), checkOnConnect(false), checkMyInfo(false), 
		hideShare(false), chatExtraInfo(Util::emptyString), usersLimit(0) 
		//END
	{ }
	FavoriteHubEntry(const FavoriteHubEntry& rhs) throw() : userdescription(rhs.userdescription), name(rhs.getName()), 
		server(rhs.getServer()), description(rhs.getDescription()), password(rhs.getPassword()), connect(rhs.getConnect()), 
		nick(rhs.nick), chatusersplit(rhs.chatusersplit), stealth(rhs.stealth), searchInterval(rhs.searchInterval),
		userliststate(rhs.userliststate), mode(rhs.mode), ip(rhs.ip), encoding(rhs.getEncoding()), group(rhs.getGroup()),
		//rawOne(rhs.rawOne), rawTwo(rhs.rawTwo), rawThree(rhs.rawThree), rawFour(rhs.rawFour), rawFive(rhs.rawFive) { }
		//RSX++
		currentEmail(rhs.currentEmail), protectedUsers(rhs.protectedUsers), useFilter(rhs.useFilter),
		useHL(rhs.useHL), useAutosearch(rhs.useAutosearch), checkClients(rhs.checkClients),
		checkFilelists(rhs.checkFilelists), checkOnConnect(rhs.checkOnConnect), checkMyInfo(rhs.checkMyInfo),
		hideShare(rhs.hideShare), chatExtraInfo(rhs.chatExtraInfo), usersLimit(rhs.usersLimit) 
		//END
	{ }
	~FavoriteHubEntry() throw() {
		//RSX++
		for(FavAction::List::const_iterator j = action.begin(); j != action.end(); ++j) {
			delete j->second;
		}
		//END
	}
	
	const string& getNick(bool useDefault = true) const { 
		return (!nick.empty() || !useDefault) ? nick : SETTING(NICK);
	}

	void setNick(const string& aNick) { nick = aNick; }

	GETSET(string, userdescription, UserDescription);
	GETSET(string, name, Name);
	GETSET(string, server, Server);
	GETSET(string, description, Description);
	GETSET(string, password, Password);
	GETSET(string, headerOrder, HeaderOrder);
	GETSET(string, headerWidths, HeaderWidths);
	GETSET(string, headerVisible, HeaderVisible);
	GETSET(string, encoding, Encoding);
	GETSET(string, ip, IP);
	GETSET(uint32_t, searchInterval, SearchInterval);
	GETSET(int, mode, Mode); // 0 = default, 1 = active, 2 = passive	
	GETSET(int, chatusersplit, ChatUserSplit);
	GETSET(bool, connect, Connect);	
	GETSET(bool, stealth, Stealth);
	GETSET(bool, userliststate, UserListState);
	GETSET(string, group, Group);
	//RSX++
	GETSET(string, awayMsg, AwayMsg);
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

	//Raw Manager
	struct FavAction {
		typedef unordered_map<int, FavAction*> List;

		FavAction(bool _enabled, string _raw = Util::emptyString, int id = 0) throw();

		GETSET(bool, enabled, Enabled);
		std::list<int> raws;
	};

	FavAction::List action;
	//END
private:
	string nick;
};

class RecentHubEntry {
public:
	typedef RecentHubEntry* Ptr;
	typedef vector<Ptr> List;
	typedef List::const_iterator Iter;

	~RecentHubEntry() throw() { }	
	
	GETSET(string, name, Name);
	GETSET(string, server, Server);
	GETSET(string, description, Description);
	GETSET(string, users, Users);
	GETSET(string, shared, Shared);	
};

}

#endif /*HUBENTRY_H_*/
