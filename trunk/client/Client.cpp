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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Client.h"

#include "BufferedSocket.h"

#include "FavoriteManager.h"
#include "TimerManager.h"
#include "ResourceManager.h"
#include "ClientManager.h"
//RSX++
#include "ChatMessage.h"
#include "PluginsManager.h"
#include "RawManager.h"
#include "version.h"
#include "Thread.h"
#include "LogManager.h"
#include "sdk/hub.h"
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
	if(hub) {
		if(updateNick) {
			setCurrentNick(checkNick(hub->getNick(true)));
		}		

		if(!hub->getUserDescription().empty()) {
			setCurrentDescription(hub->getUserDescription());
		} else {
			setCurrentDescription(SETTING(DESCRIPTION));
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
		if(!hub->getCurrentEmail().empty())
			setCurrentEmail(hub->getCurrentEmail());
		if(!hub->getProtectedUsers().empty())
			setProtectedUsers(hub->getProtectedUsers());
		setCheckOnConnect(hub->getCheckOnConnect());
		setCheckClients(hub->getCheckClients());
		setCheckFilelists(hub->getCheckFilelists());
		setCheckMyInfo(hub->getCheckMyInfo());
		setHideShare(hub->getHideShare());
		setUseFilter(hub->getUseFilter());
		setUseAutosearch(hub->getUseAutosearch());
		setUseHL(hub->getUseHL());
		setUsersLimit(hub->getUsersLimit());
		setChatExtraInfo(hub->getChatExtraInfo());
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
		setProtectedUsers(Util::emptyString);
		setCheckOnConnect(false);
		setCheckClients(false);
		setCheckFilelists(false);
		setCheckMyInfo(false);
		setHideShare(false);
		setUseFilter(false);
		setUseAutosearch(false);
		setUseHL(false);
		setUsersLimit(0);
		setChatExtraInfo(Util::emptyString);
		//END
	}
}

bool Client::isActive() const {
	return ClientManager::getInstance()->isActive(hubUrl);
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

	PluginsManager::getInstance()->getSpeaker().speak(DCPP_EVENT_HUB, DCPP_EVENT_HUB_CONNECTING, (dcpp_param)this, (dcpp_param)hubUrl.c_str()); //RSX++

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

void Client::send(const char* aMessage, size_t aLen, bool bypassPlug /*= false*/) {
	dcassert(sock);
	if(!sock)
		return;
	//RSX++
	if(!bypassPlug && plugHubLine(aMessage, aLen, false))
		return;
	//END
	updateActivity();
	sock->write(aMessage, aLen);
	COMMAND_DEBUG(aMessage, DebugManager::HUB_OUT, getIpPort());
}

void Client::on(Connected) throw() {
	updateActivity(); 
	ip = sock->getIp();
	localIp = sock->getLocalIp();

	PluginsManager::getInstance()->getSpeaker().speak(DCPP_EVENT_HUB, DCPP_EVENT_HUB_CONNECTED, (dcpp_param)this, (dcpp_param)hubUrl.c_str()); //RSX++

	fire(ClientListener::Connected(), this);
	state = STATE_PROTOCOL;
}

void Client::on(Failed, const string& aLine) throw() {
	PluginsManager::getInstance()->getSpeaker().speak(DCPP_EVENT_HUB, DCPP_EVENT_HUB_DISCONNECTED, (dcpp_param)this, (dcpp_param)hubUrl.c_str()); //RSX++

	state = STATE_DISCONNECTED;
	FavoriteManager::getInstance()->removeUserCommand(getHubUrl());
	sock->removeListener(this);
	fire(ClientListener::Failed(), this, aLine);
}

void Client::disconnect(bool graceLess) {
	if(sock) 
		sock->disconnect(graceLess);
}

bool Client::isSecure() const {
	return sock && sock->isSecure();
}

bool Client::isTrusted() const {
	return sock && sock->isTrusted();
}

std::string Client::getCipherName() const {
	return sock ? sock->getCipherName() : Util::emptyString;
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

void Client::redirect(const std::string& url) {
//	PluginsManager::getInstance()->onHubRedirect(this, url.c_str());
	disconnect(true); 
	fire(ClientListener::Redirect(), this, url);
}
//RSX++
string Client::getCurrentDescription() const {
#ifdef _WIN64
#define BUILD_MODE "x64"
#else
#define BUILD_MODE "x86"
#endif
#ifdef SVNBUILD
	return currentDescription + "<SVN:" BOOST_STRINGIZE(SVN_REVISION) " " BUILD_MODE ">";
#elif _DEBUG
	return currentDescription + "<DBG SVN:" BOOST_STRINGIZE(SVN_REVISION) " " BUILD_MODE ">";
#else
	return currentDescription;
#endif
#undef BUILD_MODE
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

dcpp_param Client::clientCallFunc(const char* type, dcpp_param p1, dcpp_param p2, dcpp_param p3, int* handled) {
	*handled = DCPP_TRUE;
	if(strncmp(type, "Hub/", 4) == 0) {
		const char* cmd = type+4;
		if(strncmp(cmd, "Open", 4) == 0) {
			if(p2)
				ClientManager::getInstance()->openHub(reinterpret_cast<const char*>(p1));
			else
				return (dcpp_param)ClientManager::getInstance()->getClient(reinterpret_cast<const char*>(p1));
			return DCPP_TRUE;
		} else if(strncmp(cmd, "Close", 5) == 0) {
			if(p2)
				ClientManager::getInstance()->closeHub(reinterpret_cast<const char*>(p1));
			else
				ClientManager::getInstance()->putClient(reinterpret_cast<Client*>(p1));
			return DCPP_TRUE;
		} else if(strncmp(cmd, "FormatChatMessage", 17) == 0) {
			ChatMessage cm;
			dcppChatMessage* m = reinterpret_cast<dcppChatMessage*>(p1);
			dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p2);
			if(!m || !buf) return DCPP_FALSE;
			cm.from = reinterpret_cast<OnlineUser*>(m->from);
			cm.text = m->message;
			cm.thirdPerson = m->thirdPerson != 0 ? true : false;
			cm.timestamp = static_cast<time_t>(m->timestamp);

			string msg = cm.format();
			size_t len = buf->size;
			if(msg.size() < len)
				len = msg.size();
			memcpy(buf->buf, &msg[0], len);
			return len;
		} else {
			Client* c = reinterpret_cast<Client*>(p1);
			if(!c) return DCPP_FALSE;

			if(strncmp(cmd, "Redirect", 8) == 0) {

			} else if(strncmp(cmd, "Identity/", 9) == 0) {
				Identity* id = 0;
				char* x = 0;
				if(strncmp(cmd+9, "My/", 3) == 0) {
					id = &c->getMyIdentity();
					x = (char*)cmd+12;
				} else if(strncmp(cmd+9, "Hub/", 4) == 0) {
					id = &c->getHubIdentity();
					x = (char*)cmd+13;
				}
				if(strncmp(x, "Get", 3) == 0) {
					return (dcpp_param)id->get(reinterpret_cast<const char*>(p2)).c_str();
				} else if(strncmp(x, "Set", 3) == 0) {
					id->set(reinterpret_cast<const char*>(p2), reinterpret_cast<const char*>(p3));
					return DCPP_TRUE;
				}
			} else if(strncmp(cmd, "SendChatMessage", 15) == 0) {
				c->hubMessage(reinterpret_cast<const char*>(p2), p3 != 0 ? true : false);
				return DCPP_TRUE;
			} else if(strncmp(cmd, "SendUserCommand", 15) == 0) {
#pragma message("@todo: fixme")
				c->send(reinterpret_cast<const char*>(p2));
				//c->sendUserCmd(reinterpret_cast<const char*>(p2));
				return DCPP_TRUE;
			} else if(strncmp(cmd, "LineWrite", 9) == 0) {
				c->send(reinterpret_cast<const char*>(p2), static_cast<size_t>(p3), true);
				return DCPP_TRUE;
			} else if(strncmp(cmd, "ChatWindowWrite", 15) == 0) {
				c->addHubLine(reinterpret_cast<const char*>(p2), static_cast<int>(p3));
				return DCPP_TRUE;
			} else if(strncmp(cmd, "DispatchLine", 12) == 0) {
				c->parseCommand(reinterpret_cast<const char*>(p2));
				return DCPP_TRUE;
			} else if(strncmp(cmd, "GetHubInfo", 10) == 0) {
				dcppHubInfo* nfo = reinterpret_cast<dcppHubInfo*>(p2);
				if(!nfo) return DCPP_FALSE;
				nfo->port = c->getPort();
				nfo->address = c->getAddress().c_str();
				nfo->url = c->getHubUrl().c_str();
				nfo->ip = c->getIp().c_str();
				nfo->isAdc = c->getType() == ClientBase::ADC ? 1 : 0;
				//@todo find a cause of access violation here at certain conditions...
				//nfo->isSecured = c->isSecure() ? 1 : 0;
				return DCPP_TRUE;
			}
		}
	}
	*handled = DCPP_FALSE;
	return DCPP_FALSE;
}

bool Client::plugChatMessage(const ChatMessage& cm) {
	dcppChatMessage m;
	memzero(&m, sizeof(m));
	m.from = reinterpret_cast<dcpp_param>(cm.from.get());
	m.to = reinterpret_cast<dcpp_param>(cm.to.get());
	m.hubPtr = reinterpret_cast<dcpp_param>(this);
	m.replyTo = reinterpret_cast<dcpp_param>(cm.replyTo.get());
	m.message = cm.text.c_str();
	m.thirdPerson = cm.thirdPerson ? 1 : 0;
	m.timestamp = cm.timestamp;
	int p = PluginsManager::getInstance()->getSpeaker().speak(DCPP_EVENT_HUB, DCPP_EVENT_HUB_CHAT_MESSAGE, reinterpret_cast<dcpp_param>(&m), (dcpp_param)hubUrl.c_str());
	return p == DCPP_TRUE;
}

bool Client::plugHubLine(const char* line, size_t len, bool incoming) {
	dcppHubLine m;
	memzero(&m, sizeof(m));
	m.hubPtr = reinterpret_cast<dcpp_param>(this);
	m.line = line;
	m.length = len;
	m.incoming = incoming ? 1 : 0;

	int p = PluginsManager::getInstance()->getSpeaker().speak(DCPP_EVENT_HUB, DCPP_EVENT_HUB_LINE, reinterpret_cast<dcpp_param>(&m), (dcpp_param)hubUrl.c_str());
	return p == DCPP_TRUE;
}

bool Client::plugChatSendLine(const std::string& line) {
	int p = PluginsManager::getInstance()->getSpeaker().speak(DCPP_EVENT_HUB, DCPP_EVENT_HUB_CHAT_SEND_LINE, reinterpret_cast<dcpp_param>(this), reinterpret_cast<dcpp_param>(line.c_str()));
	return p == DCPP_TRUE;
}

//END

} // namespace dcpp

/**
 * @file
 * $Id: Client.cpp 482 2010-02-13 10:49:30Z bigmuscle $
 */
