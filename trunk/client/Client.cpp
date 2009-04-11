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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Client.h"

#include "BufferedSocket.h"

#include "FavoriteManager.h"
#include "TimerManager.h"
#include "ResourceManager.h"
#include "ClientManager.h"
//RSX++
#include "cstring.hpp"
#include "PluginsManager.h"
#include "ScriptManager.h"
#include "RawManager.h"
#include "version.h"
#include "Thread.h"
#include "LogManager.h"
//END

namespace dcpp {

Client::Counts Client::counts;

Client::Client(const string& hubURL, char separator_, bool secure_) : 
	myIdentity(ClientManager::getInstance()->getMe(), 0),
	reconnDelay(120), lastActivity(GET_TICK()), registered(false), autoReconnect(false),
	encoding(const_cast<string*>(&Text::systemCharset)), state(STATE_DISCONNECTED), sock(0),
	hubUrl(hubURL), port(0), separator(separator_),
	secure(secure_), countType(COUNT_UNCOUNTED), availableBytes(0)
	, usersLimit(0), userCount(0) //RSX++
{
	string file;
	Util::decodeUrl(hubURL, address, port, file);

	TimerManager::getInstance()->addListener(this);
	//RSX++
	setCheckedAtConnect(false);
	cmdQueue.setClientPtr(this);
	//END
}

Client::~Client() throw() {
	dcassert(!sock);
	
	// In case we were deleted before we Failed
	FavoriteManager::getInstance()->removeUserCommand(getHubUrl());
	TimerManager::getInstance()->removeListener(this);
	updateCounts(true);
}

void Client::reconnect() {
	disconnect(true);
	setAutoReconnect(true);
	setReconnDelay(0);
}

void Client::shutdown() {
	//RSX++
	cmdQueue.clear();
	putDetectors();
	//END
	if(sock) {
		BufferedSocket::putSocket(sock);
		sock = 0;
	}
}

void Client::reloadSettings(bool updateNick) {
	const FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(getHubUrl());
	//RSX++
	string speedDescription = Util::emptyString;
	if(BOOLSETTING(SHOW_DESCRIPTION_SPEED))
		speedDescription = "["+SETTING(DOWN_SPEED)+"/"+SETTING(UP_SPEED)+"]";
	//END
	if(hub) {
		if(updateNick) {
			setCurrentNick(checkNick(hub->getNick(true)));
		}		

		if(!hub->getUserDescription().empty()) {
			setCurrentDescription(speedDescription + hub->getUserDescription());
		} else {
			setCurrentDescription(speedDescription + SETTING(DESCRIPTION));
		}
		if(!hub->getPassword().empty())
			setPassword(hub->getPassword());
		setStealth(hub->getStealth());
		setFavIp(hub->getIP());
		
		if(hub->getSearchInterval() < 10)
			setSearchInterval(SETTING(MINIMUM_SEARCH_INTERVAL) * 1000);
		else
			setSearchInterval(hub->getSearchInterval() * 1000);
		//RSX++
		updateSettings(hub->getSettings());
		setUsersLimit((uint32_t)hub->getUsersLimit());
		//END
	} else {
		if(updateNick) {
			setCurrentNick(checkNick(SETTING(NICK)));
		}
		setCurrentDescription(SETTING(DESCRIPTION));
		setStealth(true);
		setFavIp(Util::emptyString);
		setSearchInterval(SETTING(MINIMUM_SEARCH_INTERVAL) * 1000);
		//RSX++
		setCurrentEmail(SETTING(EMAIL));
		setUseFilter(false);
		setUseHL(false);
		setUsersLimit(0);
		//END
	}
}

bool Client::isActive() const {
	return ClientManager::getInstance()->getMode(hubUrl) != SettingsManager::INCOMING_FIREWALL_PASSIVE;
}

void Client::connect() {
	if(sock)
		BufferedSocket::putSocket(sock);

	availableBytes = 0;
	userCount = 0; //RSX++

	setAutoReconnect(true);
	setReconnDelay(120 + Util::rand(0, 60));
	reloadSettings(true);
	setRegistered(false);
	setMyIdentity(Identity(ClientManager::getInstance()->getMe(), 0));
	setHubIdentity(Identity());

	state = STATE_CONNECTING;

	try {
		sock = BufferedSocket::getSocket(separator);
		sock->addListener(this);
		sock->connect(address, port, secure, BOOLSETTING(ALLOW_UNTRUSTED_HUBS), true);
	} catch(const Exception& e) {
		if(sock) {
			BufferedSocket::putSocket(sock);
			sock = 0;
		}
		fire(ClientListener::Failed(), this, e.getError());
	}
	updateActivity();
}

void Client::send(const char* aMessage, size_t aLen) {
	dcassert(sock);
	if(!sock)
		return;
	updateActivity();
	sock->write(aMessage, aLen);
	COMMAND_DEBUG(aMessage, DebugManager::HUB_OUT, getIpPort());
}

void Client::on(Connected) throw() {
	updateActivity(); 
	ip = sock->getIp();
	localIp = sock->getLocalIp();
	fire(ClientListener::Connected(), this);
	state = STATE_PROTOCOL;
}

void Client::on(Failed, const string& aLine) throw() {
	state = STATE_DISCONNECTED;
	FavoriteManager::getInstance()->removeUserCommand(getHubUrl());
	sock->removeListener(this);
	fire(ClientListener::Failed(), this, aLine);
}

void Client::disconnect(bool graceLess) {
	if(sock) 
		sock->disconnect(graceLess);
}

void Client::updateCounts(bool aRemove) {
	// We always remove the count and then add the correct one if requested...
	if(countType == COUNT_NORMAL) {
		Thread::safeDec(counts.normal);
	} else if(countType == COUNT_REGISTERED) {
		Thread::safeDec(counts.registered);
	} else if(countType == COUNT_OP) {
		Thread::safeDec(counts.op);
	}

	countType = COUNT_UNCOUNTED;

	if(!aRemove) {
		if(getMyIdentity().isOp()) {
			Thread::safeInc(counts.op);
			countType = COUNT_OP;
		} else if(getMyIdentity().isRegistered()) {
			Thread::safeInc(counts.registered);
			countType = COUNT_REGISTERED;
		} else {
			Thread::safeInc(counts.normal);
			countType = COUNT_NORMAL;
		}
	}
}

string Client::getLocalIp() const {
	// Favorite hub Ip
	if(!getFavIp().empty())
		return Socket::resolve(getFavIp());

	// Best case - the server detected it
	if((!BOOLSETTING(NO_IP_OVERRIDE) || SETTING(EXTERNAL_IP).empty()) && !getMyIdentity().getIp().empty()) {
		return getMyIdentity().getIp();
	}

	if(!SETTING(EXTERNAL_IP).empty()) {
		return Socket::resolve(SETTING(EXTERNAL_IP));
	}

	if(localIp.empty()) {
		return Util::getLocalIp();
	}

	return localIp;
}

uint64_t Client::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, void* owner){
	dcdebug("Queue search %s\n", aString.c_str());

	if(searchQueue.interval){
		Search s;
		s.fileType = aFileType;
		s.size     = aSize;
		s.query    = aString;
		s.sizeType = aSizeMode;
		s.token    = aToken;
		s.owners.insert(owner);

		searchQueue.add(s);

		return searchQueue.getSearchTime(owner) - GET_TICK();
	}

	search(aSizeMode, aSize, aFileType , aString, aToken);
	return 0;

}
 
void Client::on(Line, const string& aLine) throw() {
	updateActivity();
	COMMAND_DEBUG(aLine, DebugManager::HUB_IN, getIpPort());
}

void Client::on(Second, uint64_t aTick) throw() {
	if(state == STATE_DISCONNECTED && getAutoReconnect() && (aTick > (getLastActivity() + getReconnDelay() * 1000)) ) {
		// Try to reconnect...
		connect();
	}

	if(!searchQueue.interval) return;

	if(isConnected()){
		Search s;
		
		if(searchQueue.pop(s)){
			search(s.sizeType, s.size, s.fileType , s.query, s.token);
		}
		cmdQueue.onSecond(aTick); //RSX+
	}
}
//RSX++
string Client::getCurrentDescription() const {
#ifdef SVNBUILD
	return currentDescription + "<SVN " BOOST_STRINGIZE(SVN_REVISION) ">";
#elif _DEBUG
	return currentDescription + "<DEBUG SVN " BOOST_STRINGIZE(SVN_REVISION) ">";
#else
	return currentDescription;
#endif
}

void Client::sendActionCommand(const OnlineUser& ou, int actionId) {
	if(!isConnected() || (userCount < getUsersLimit()))
		return;
	cmdQueue.addCommand(ou, actionId);
}

bool Client::isActionActive(const int aAction) const {
	FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(getHubUrl());
	return hub ? FavoriteManager::getInstance()->getEnabledAction(hub, aAction) : true;
}

bool Client::extOnMsgIn(const std::string& msg) {
	bool plugin = PluginsManager::getInstance()->plugEvent(DCPP_HUB_MSG_IN, (void*)this, (void*)msg.c_str(), 0) == DCPP_DROP_EVENT;
	bool script = ScriptManager::getInstance()->onHubMsgIn(this, msg);
	return plugin || script;
}

bool Client::extOnMsgOut(const std::string& msg) {
	bool plugin = PluginsManager::getInstance()->plugEvent(DCPP_HUB_MSG_OUT, (void*)this, (void*)msg.c_str(), 0) == DCPP_DROP_EVENT;
	bool script = ScriptManager::getInstance()->onHubMsgOut(this, msg);
	return plugin || script;
}

bool Client::extOnPmIn(OnlineUser* from, OnlineUser* to, OnlineUser* replyTo, const std::string& msg, bool thirdPerson) {
	if(ClientManager::getInstance()->getMe() == from->getUser()) return false;
	DCPP_DATA_PACK p1;
	DCPP_DATA_PACK p2;

	p1.p1 = (void*)from;
	p1.p2 = (void*)to;
	p1.p3 = (void*)replyTo;
	p2.p1 = (void*)this;
	p2.p2 = (void*)msg.c_str();
	p2.p3 = (void*)thirdPerson;

	bool plugin = PluginsManager::getInstance()->plugEvent(DCPP_HUB_PRIVATE_MSG_IN, &p1, &p2, 0) == DCPP_DROP_EVENT;
	bool script = ScriptManager::getInstance()->onPmMsgIn(this, from, to, replyTo, msg, thirdPerson);
	return plugin || script;
}

bool Client::extOnPmOut(OnlineUser* to, const std::string& msg, bool thirdPerson) {
	DCPP_DATA_PACK p;
	p.p1 = (void*)this;
	p.p2 = (void*)to;
	p.p3 = (void*)msg.c_str();

	bool plugin = PluginsManager::getInstance()->plugEvent(DCPP_HUB_PRIVATE_MSG_OUT, &p, (void*)thirdPerson, 0) == DCPP_DROP_EVENT;
	bool script = ScriptManager::getInstance()->onPmMsgOut(this, to, msg, thirdPerson);
	return plugin || script;
}
//END

} // namespace dcpp

/**
 * @file
 * $Id: Client.cpp 401 2008-07-07 17:02:03Z BigMuscle $
 */
