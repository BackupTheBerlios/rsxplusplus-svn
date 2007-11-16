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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Client.h"

#include "BufferedSocket.h"

#include "FavoriteManager.h"
#include "TimerManager.h"
#include "ResourceManager.h"
#include "ClientManager.h"
//RSX++
#include "RawManager.h"
#include "pme.h"
#include "PGManager.h"
#include "version.h"
#include "Thread.h"
#include "LogManager.h"
//END
Client::Counts Client::counts;

Client::Client(const string& hubURL, char separator_, bool secure_) : 
	myIdentity(ClientManager::getInstance()->getMe(), 0),
	reconnDelay(120), lastActivity(GET_TICK()), registered(false), autoReconnect(false),
	encoding(const_cast<string*>(&Text::systemCharset)), state(STATE_DISCONNECTED), socket(0),
	hubUrl(hubURL), port(0), separator(separator_),
	secure(secure_), countType(COUNT_UNCOUNTED), availableBytes(0), usersLimit(0)
{
	string file;
	Util::decodeUrl(hubURL, address, port, file);

	TimerManager::getInstance()->addListener(this);
	setCheckedAtConnect(false); //RSX++
}

Client::~Client() throw() {
	dcassert(!socket);

	// In case we were deleted before we Failed
	FavoriteManager::getInstance()->removeUserCommand(getHubUrl());
	TimerManager::getInstance()->removeListener(this);
	updateCounts(true);
	//RSX++	
	putSender(true);
	//END
}

void Client::reconnect() {
	disconnect(true);
	setAutoReconnect(true);
	setReconnDelay(0);
}

void Client::shutdown() {
	putDetectors(); //RSX++
	if(socket) {
		BufferedSocket::putSocket(socket);
		socket = 0;
	}
}

void Client::reloadSettings(bool updateNick) {
	const FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(getHubUrl());

	string speedDescription = Util::emptyString;
	if(BOOLSETTING(SHOW_DESCRIPTION_SPEED))
		speedDescription = "["+SETTING(DOWN_SPEED)+"/"+SETTING(UP_SPEED)+"]";

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

		if(!hub->getFavEmail().empty()) {
			setCurrentEmail(hub->getFavEmail());
		} else {
			setCurrentEmail(SETTING(EMAIL));
		}

		setStealth(hub->getStealth());
		setFavIp(hub->getIP());
		hubUrl = hub->getServer();

		if(!hub->getUserProtected().empty())
			setUserProtected(hub->getUserProtected());

		setCheckOnConnect(hub->getCheckOnConnect());
		setCheckClients(hub->getCheckClients());
		setCheckFilelists(hub->getCheckFilelists());
		setCheckMyInfo(hub->getCheckMyInfo());
		setHideShare(hub->getHideShare());
		setCheckFakeShare(hub->getCheckFakeShare());
		setUseFilter(hub->getUseFilter());
		setUseAutosearch(hub->getAutosearch());
		setUseHL(hub->getUseHL());
		setUsersLimit((uint16_t)hub->getUsersLimit());
		//END
	} else {
		if(updateNick) {
			setCurrentNick(checkNick(SETTING(NICK)));
		}
		setCurrentDescription(speedDescription + SETTING(DESCRIPTION));
		setCurrentEmail(SETTING(EMAIL));
		setStealth(true);
		setFavIp(Util::emptyString);
		//RSX++
		setUserProtected(Util::emptyString);
		setCheckOnConnect(false);
		setCheckClients(false);
		setCheckFilelists(false);
		setCheckMyInfo(false);
		setHideShare(false);
		setCheckFakeShare(false);
		setUseFilter(false);
		setUseAutosearch(false);
		setUseHL(false);
		setUsersLimit(0);
		//END
	}
	#ifdef SVN_REVISION_STR
	currentDescription += "<SVN " SVN_REVISION_STR ">";
	#endif
	if(getStealth())
		currentDescription += "<RSX++ " VERSIONSTRING ">";
}

bool Client::isActive() const {
	return ClientManager::getInstance()->getMode(hubUrl) != SettingsManager::INCOMING_FIREWALL_PASSIVE;
}

void Client::connect() {
	if(socket)
		BufferedSocket::putSocket(socket);

	availableBytes = 0;

	setAutoReconnect(true);
	setReconnDelay(120 + Util::rand(0, 60));
	reloadSettings(true);
	setRegistered(false);
	setMyIdentity(Identity(ClientManager::getInstance()->getMe(), 0));
	setHubIdentity(Identity());

	try {
		//RSX++
		if(SETTING(PG_ENABLE)) {
			// suerly people don't want to connect to hubs which is in their blocklist, well
			// and if they do that's just too bad (this is done this way to prevent abuse)
			string company = PGManager::getInstance()->getIPBlock(Socket::resolve(address));
			if(!company.empty()) {
				if(SETTING(PG_LOG)) {
					PGManager::getInstance()->log("Blocked outgoing hub connection to: " + address + " (" + company + ")");
				}
				throw Exception(STRING(PG_BLOCKED) + ": " + company);
			}
		}
		//END
		socket = BufferedSocket::getSocket(separator);
		socket->addListener(this);
		socket->connect(address, port, secure, BOOLSETTING(ALLOW_UNTRUSTED_HUBS), true);
	} catch(const Exception& e) {
		if(socket) {
			BufferedSocket::putSocket(socket);
			socket = 0;
		}
		fire(ClientListener::Failed(), this, e.getError());
	}
	updateActivity();
	state = STATE_CONNECTING;
}

void Client::on(Connected) throw() {
	updateActivity(); 
	ip = socket->getIp(); 
	fire(ClientListener::Connected(), this);
	state = STATE_PROTOCOL;
}

void Client::on(Failed, const string& aLine) throw() {
	//RSX++ //just in case...
	//putSender(true);
	putDetectors();
	//END
	updateActivity(); 
	state = STATE_DISCONNECTED;
	FavoriteManager::getInstance()->removeUserCommand(getHubUrl());
	socket->removeListener(this);
	fire(ClientListener::Failed(), this, aLine);
}

void Client::disconnect(bool graceLess) {
	//RSX++
	//putSender();
	putDetectors();
	//END
	if(socket) 
		socket->disconnect(graceLess);
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
		return getFavIp();

	// Best case - the server detected it
	if((!BOOLSETTING(NO_IP_OVERRIDE) || SETTING(EXTERNAL_IP).empty()) && !getMyIdentity().getIp().empty()) {
		return getMyIdentity().getIp();
	}

	if(!SETTING(EXTERNAL_IP).empty()) {
		return Socket::resolve(SETTING(EXTERNAL_IP));
	}

	string lip;
	if(socket)
		lip = socket->getLocalIp();

	if(lip.empty())
		return Util::getLocalIp();
	return lip;
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
}
//RSX++ // Lua
bool ClientScriptInstance::onHubFrameEnter(Client* aClient, const string& aLine) {
	Lock l(scs);
	MakeCall("dcpp", "OnCommandEnter", 1, aClient, aLine);
	return GetLuaBool();
}
//RSX++ //Threaded Raw Sender
class RawSender : public Thread {
public:
	bool stop;
	CriticalSection cs;
	Semaphore s;
	list<string> rawQueue;

	RawSender() : c(NULL), stop(true) { };
	~RawSender() throw() {
		shutdown();
	}

	void shutdown() { 
		stop = true;
		s.signal();
		c = NULL;
		join();
	}
	int run() {
		setThreadPriority(Thread::LOW);
		const uint64_t sleepTime = static_cast<uint64_t>(RSXSETTING(RAW_SENDER_SLEEP_TIME));
		while(true) {
			//s.wait(1000);
			if(stop || rawQueue.empty())
				break;
			{
				Lock l(cs);
				string& cmd = rawQueue.front();
				if(!cmd.empty() && c->isConnected())
					c->sendUserCmd(cmd);
				rawQueue.pop_front();
				sleep(sleepTime);
				RSXS_SET(TOTAL_RAW_COMMANDS_SENT, RSXSETTING(TOTAL_RAW_COMMANDS_SENT)+1);
			}
		}
		stop = true;
		return 0;
	}
	Client* c;
}sender;

void Client::insertRaw(const string& aRawCmd) {
	{
		Lock l(sender.cs);
		sender.rawQueue.push_back(aRawCmd);
	}
	if(sender.stop == true) {
		sender.stop = false;
		sender.c = this;
		sender.start();
	}
	sender.s.signal();
}

void Client::putSender(bool clear /*= false*/) {
	if(clear)
		sender.rawQueue.clear();
	sender.shutdown();
}

void Client::sendActionCommand(const OnlineUser& ou, int actionId) {
	Lock l(cs);
	if(!isConnected() || (getUserCount() < getUsersLimit()))
		return;

	FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(getHubUrl());
	if(hub) {
		if(RawManager::getInstance()->getActifActionId(actionId)) {
			if(FavoriteManager::getInstance()->getActifAction(hub, actionId)) {
				Action::RawsList lst = RawManager::getInstance()->getRawListActionId(actionId);
				uint64_t delayTime = GET_TICK();
				for(Action::RawsList::const_iterator i = lst.begin(); i != lst.end(); ++i) {
					if(i->getActif() && !(i->getRaw().empty())) {
						if(FavoriteManager::getInstance()->getActifRaw(hub, actionId, i->getRawId())) {
							StringMap params;
							const ::UserCommand uc = ::UserCommand(0, 0, 0, 0, "", i->getRaw(), "");
							ou.getIdentity().getParams(params, "user", true);
							getHubIdentity().getParams(params, "hub", false);
							getMyIdentity().getParams(params, "my", true);
							escapeParams(params);
							const string& formattedCmd = Util::formatParams(uc.getCommand(), params, false);
							if(RSXBOOLSETTING(USE_SEND_DELAYED_RAW)) {
								delayTime += (i->getTime() * 1000) + 1;
								RawManager::getInstance()->addRaw(delayTime, formattedCmd, this, i->getName(), i->getLua());
							} else {
								//@todo maybe move also it to thread...
								if(i->getLua()) {
									ScriptManager::getInstance()->onRaw(i->getName(), formattedCmd, this);
								} else {
									insertRaw(formattedCmd);
								}
							}
							if(SETTING(RAW_CMD_LOG)) {
								params["rawCommand"] = formattedCmd;
								LOG(LogManager::RAW, params);
							}
						}
					}
				}
			}
		}
	}
}
//END

/**
 * @file
 * $Id: Client.cpp 317 2007-08-04 14:52:24Z bigmuscle $
 */
