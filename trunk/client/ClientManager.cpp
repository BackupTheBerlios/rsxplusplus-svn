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

#include "ClientManager.h"

#include "ShareManager.h"
#include "SearchManager.h"
#include "ConnectionManager.h"
#include "CryptoManager.h"
#include "FavoriteManager.h"
#include "SimpleXML.h"
#include "UserCommand.h"
#include "ResourceManager.h"
#include "LogManager.h"
#include "SearchResult.h"

#include "AdcHub.h"
#include "NmdcHub.h"

#include "QueueManager.h"
#include "FinishedManager.h"
//RSX++
#include "RawManager.h"
#include "ADLSearch.h"
//END

namespace dcpp {

Client* ClientManager::getClient(const string& aHubURL) {
	Client* c;
	if(strnicmp("adc://", aHubURL.c_str(), 6) == 0) {
		c = new AdcHub(aHubURL, false);
	} else if(strnicmp("adcs://", aHubURL.c_str(), 7) == 0) {
		c = new AdcHub(aHubURL, true);
	} else {
		c = new NmdcHub(aHubURL);
	}

	{
		Lock l(cs);
		clients.push_front(c);
	}

	c->addListener(this);

	return c;
}

void ClientManager::putClient(Client* aClient) {
	fire(ClientManagerListener::ClientDisconnected(), aClient);
	aClient->removeListeners();

	{
		Lock l(cs);
		clients.remove(aClient);
	}
	aClient->shutdown();
	delete aClient;
}

size_t ClientManager::getUserCount() const {
	Lock l(cs);
	return onlineUsers.size();
}

StringList ClientManager::getHubs(const CID& cid) const {
	Lock l(cs);
	StringList lst;
	OnlinePairC op = onlineUsers.equal_range(cid);
	for(OnlineIterC i = op.first; i != op.second; ++i) {
		lst.push_back(i->second->getClient().getHubUrl());
	}
	return lst;
}

StringList ClientManager::getHubNames(const CID& cid) const {
	Lock l(cs);
	StringList lst;
	OnlinePairC op = onlineUsers.equal_range(cid);
	for(OnlineIterC i = op.first; i != op.second; ++i) {
		lst.push_back(i->second->getClient().getHubName());
	}
	return lst;
}

StringList ClientManager::getNicks(const CID& cid) const {
	Lock l(cs);
	StringSet ret;
	OnlinePairC op = onlineUsers.equal_range(cid);
	for(OnlineIterC i = op.first; i != op.second; ++i) {
		ret.insert(i->second->getIdentity().getNick());
	}
	if(ret.empty()) {
		NickMap::const_iterator i = nicks.find(cid);
		if(i != nicks.end()) {
			ret.insert(i->second);
		} else {
			// Offline perhaps?
			ret.insert('{' + cid.toBase32() + '}');
		}
	}
	return StringList(ret.begin(), ret.end());
}

string ClientManager::getConnection(const CID& cid) const {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(cid);
	if(i != onlineUsers.end()) {
		return i->second->getIdentity().getConnection();
	}
	return STRING(OFFLINE);
}

int64_t ClientManager::getAvailable() const {
	Lock l(cs);
	int64_t bytes = 0;
	for(OnlineIterC i = onlineUsers.begin(); i != onlineUsers.end(); ++i) {
		bytes += i->second->getIdentity().getBytesShared();
	}

	return bytes;
}

bool ClientManager::isConnected(const string& aUrl) const {
	Lock l(cs);

	for(Client::List::const_iterator i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->getHubUrl() == aUrl) {
			return true;
		}
	}
	return false;
}

string ClientManager::findHub(const string& ipPort) const {
	Lock l(cs);

	string ip;
	uint16_t port = 411;
	string::size_type i = ipPort.find(':');
	if(i == string::npos) {
		ip = ipPort;
	} else {
		ip = ipPort.substr(0, i);
		port = static_cast<uint16_t>(Util::toInt(ipPort.substr(i+1)));
	}

	string url;
	for(Client::List::const_iterator i = clients.begin(); i != clients.end(); ++i) {
		const Client* c = *i;
		if(c->getIp() == ip) {
			// If exact match is found, return it
			if(c->getPort() == port)
				return c->getHubUrl();

			// Port is not always correct, so use this as a best guess...
			url = c->getHubUrl();
		}
	}

	return url;
}

const string& ClientManager::findHubEncoding(const string& aUrl) const {
	Lock l(cs);

	for(Client::List::const_iterator i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->getHubUrl() == aUrl) {
			return *((*i)->getEncoding());
		}
	}
	return Text::systemCharset;
}

UserPtr ClientManager::findLegacyUser(const string& aNick) const throw() {
	Lock l(cs);
	dcassert(aNick.size() > 0);

	for(OnlineMap::const_iterator i = onlineUsers.begin(); i != onlineUsers.end(); ++i) {
		const OnlineUser* ou = i->second;
		if(ou->getUser()->isSet(User::NMDC) && stricmp(ou->getIdentity().getNick(), aNick) == 0)
			return ou->getUser();
	}
	return UserPtr();
}

UserPtr ClientManager::getUser(const string& aNick, const string& aHubUrl) throw() {
	CID cid = makeCid(aNick, aHubUrl);
	Lock l(cs);

	UserIter ui = users.find(cid);
	if(ui != users.end()) {
		ui->second->setFlag(User::NMDC);
		return ui->second;
	}

	UserPtr p(new User(cid));
	p->setFlag(User::NMDC);
	users.insert(make_pair(cid, p));

	return p;
}

UserPtr ClientManager::getUser(const CID& cid) throw() {
	Lock l(cs);
	UserMap::const_iterator ui = users.find(cid);
	if(ui != users.end()) {
		return ui->second;
	}

	UserPtr p(new User(cid));
	users.insert(make_pair(cid, p));
	return p;
}

UserPtr ClientManager::findUser(const CID& cid) const throw() {
	Lock l(cs);
	UserMap::const_iterator ui = users.find(cid);
	if(ui != users.end()) {
		return ui->second;
	}
	return 0;
}

bool ClientManager::isOp(const UserPtr& user, const string& aHubUrl) const {
	Lock l(cs);
	OnlinePairC p = onlineUsers.equal_range(user->getCID());
	for(OnlineIterC i = p.first; i != p.second; ++i) {
		if(i->second->getClient().getHubUrl() == aHubUrl) {
			return i->second->getIdentity().isOp();
		}
	}
	return false;
}

bool ClientManager::isStealth(const string& aHubUrl) const {
	Lock l(cs);
	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		const Client* c = *i;
		if(c->getHubUrl() == aHubUrl) {
			return c->getStealth();
		}
	}
	return false;
}

CID ClientManager::makeCid(const string& aNick, const string& aHubUrl) const throw() {
	string n = Text::toLower(aNick);
	TigerHash th;
	th.update(n.c_str(), n.length());
	th.update(Text::toLower(aHubUrl).c_str(), aHubUrl.length());
	// Construct hybrid CID from the bits of the tiger hash - should be
	// fairly random, and hopefully low-collision
	return CID(th.finalize());
}

void ClientManager::putOnline(OnlineUser* ou) throw() {
	{
		Lock l(cs);
		onlineUsers.insert(make_pair(ou->getUser()->getCID(), ou));
	}

	if(!ou->getUser()->isOnline()) {
		ou->getUser()->setFlag(User::ONLINE);
		ou->initializeData(); //RSX++
		fire(ClientManagerListener::UserConnected(), ou->getUser());
	}
}

void ClientManager::putOffline(OnlineUser* ou, bool disconnect) throw() {
	bool lastUser = false;
	{
		Lock l(cs);
		OnlinePair op = onlineUsers.equal_range(ou->getUser()->getCID());
		dcassert(op.first != op.second);
		for(OnlineIter i = op.first; i != op.second; ++i) {
			OnlineUser* ou2 = i->second;
			if(ou == ou2) {
				lastUser = (distance(op.first, op.second) == 1);
				onlineUsers.erase(i);
				break;
			}
		}
	}

	if(lastUser) {
		UserPtr& u = ou->getUser();
		u->unsetFlag(User::ONLINE);
		u->unsetFlag(User::PROTECTED); //RSX++

		if(disconnect)
			ConnectionManager::getInstance()->disconnect(u);
		fire(ClientManagerListener::UserDisconnected(), u);
	}
}

void ClientManager::connect(const UserPtr& p, const string& token) {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(p->getCID());
	if(i != onlineUsers.end()) {
		OnlineUser* u = i->second;
		u->getClient().connect(*u, token);
	}
}

void ClientManager::privateMessage(const UserPtr& p, const string& msg, bool thirdPerson) {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(p->getCID());
	if(i != onlineUsers.end()) {
		OnlineUser* u = i->second;
		u->getClient().privateMessage(*u, msg, thirdPerson);
	}
}

void ClientManager::send(AdcCommand& cmd, const CID& cid) {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(cid);
	if(i != onlineUsers.end()) {
		OnlineUser& u = *i->second;
		if(cmd.getType() == AdcCommand::TYPE_UDP && !u.getIdentity().isUdpActive()) {
			cmd.setType(AdcCommand::TYPE_DIRECT);
			cmd.setTo(u.getIdentity().getSID());
			u.getClient().send(cmd);
		} else {
			try {
				Socket udp;
				udp.writeTo(u.getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(u.getIdentity().getUdpPort())), cmd.toString(getMe()->getCID()));
			} catch(const SocketException&) {
				dcdebug("Socket exception sending ADC UDP command\n");
			}
		}
	}
}

void ClientManager::infoUpdated() {
	Lock l(cs);
	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->info(false);
		}
	}
}

void ClientManager::on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize, 
									int aFileType, const string& aString, bool isPassive) throw() 
{
	Speaker<ClientManagerListener>::fire(ClientManagerListener::IncomingSearch(), aString);

	// We don't wan't to answer passive searches if we're in passive mode...
	if(isPassive && !ClientManager::getInstance()->isActive(aClient->getHubUrl())) {
		return;
	}

	SearchResultList l;
	ShareManager::getInstance()->search(l, aString, aSearchType, aSize, aFileType, aClient, isPassive ? 5 : 10);
	if(l.size() > 0) {
		if(isPassive) {
			string name = aSeeker.substr(4);
			// Good, we have a passive seeker, those are easier...
			string str;
			for(SearchResultList::const_iterator i = l.begin(); i != l.end(); ++i) {
				const SearchResultPtr& sr = *i;
				str += sr->toSR(*aClient);
				str[str.length()-1] = 5;
				str += Text::fromUtf8(name, *(aClient->getEncoding()));
				str += '|';
			}
			
			if(str.size() > 0)
				aClient->send(str);
			
		} else {
			try {
				Socket udp;
				string ip, file;
				uint16_t port = 0;
				Util::decodeUrl(aSeeker, ip, port, file);
				ip = Socket::resolve(ip);
				
				if(port == 0) 
					port = 412;
				for(SearchResultList::const_iterator i = l.begin(); i != l.end(); ++i) {
					const SearchResultPtr& sr = *i;
					udp.writeTo(ip, port, sr->toSR(*aClient));
				}
			} catch(...) {
				dcdebug("Search caught error\n");
			}
		}
	} else if(!isPassive && (aFileType == SearchManager::TYPE_TTH) && (aString.compare(0, 4, "TTH:") == 0)) {
		PartsInfo partialInfo;
		TTHValue aTTH(aString.substr(4));
		if(!QueueManager::getInstance()->handlePartialSearch(aTTH, partialInfo)) {
			// if not found, try to find in finished list
			if(!FinishedManager::getInstance()->handlePartialRequest(aTTH, partialInfo)) {
				return;
			}
		}
		
		string ip, file;
		uint16_t port = 0;
		Util::decodeUrl(aSeeker, ip, port, file);
		
		try {
			AdcCommand cmd = SearchManager::getInstance()->toPSR(true, aClient->getMyNick(), aClient->getIpPort(), aTTH.toBase32(), partialInfo);
			Socket s;
			s.writeTo(Socket::resolve(ip), port, cmd.toString(ClientManager::getInstance()->getMyCID()));
		} catch(...) {
			dcdebug("Partial search caught error\n");		
		}
	}
}

void ClientManager::userCommand(const UserPtr& p, const UserCommand& uc, StringMap& params, bool compatibility) {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(p->getCID());
	if(i == onlineUsers.end())
		return;

	OnlineUser& ou = *i->second;
	ou.getIdentity().getParams(params, "user", compatibility);
	ou.getClient().getHubIdentity().getParams(params, "hub", false);
	ou.getClient().getMyIdentity().getParams(params, "my", compatibility);
	ou.getClient().escapeParams(params);
	if(!ucExecuteLua(uc.getCommand(), params)) 	//RSX++ // Lua
		ou.getClient().sendUserCmd(Util::formatParams(uc.getCommand(), params, false));
}

void ClientManager::sendRawCommand(const UserPtr& user, const string& aRaw, bool checkProtection/* = false*/) {
	if(!aRaw.empty()) {
		bool skipRaw = false;
		Lock l(cs);
		if(checkProtection) {
			OnlineIterC i = onlineUsers.find(user->getCID());
			if(i == onlineUsers.end())
				return;
			skipRaw = i->second->isProtectedUser();
		}
		if(!skipRaw || !checkProtection) {
			StringMap ucParams;
			UserCommand uc = UserCommand(0, 0, 0, 0, "", aRaw, "");
			userCommand(user, uc, ucParams, true);
			RSXS_SET(TOTAL_RAW_COMMANDS_SENT, RSXSETTING(TOTAL_RAW_COMMANDS_SENT)+1); //important stuff ;p
			if(SETTING(LOG_RAW_CMD)) {
				LOG(LogManager::RAW, ucParams);
			}
		}
	}
}

void ClientManager::on(AdcSearch, const Client*, const AdcCommand& adc, const CID& from) throw() {
	SearchManager::getInstance()->respond(adc, from);
}

string ClientManager::getHubUrl(const UserPtr& aUser) const {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(aUser->getCID());
	if(i != onlineUsers.end()) {
		return i->second->getClient().getHubUrl();
	}
	return Util::emptyString;
}

void ClientManager::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, void* aOwner) {
	Lock l(cs);

	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->search(aSizeMode, aSize, aFileType, aString, aToken, aOwner);
		}
	}
}

uint64_t ClientManager::search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, void* aOwner) {
	Lock l(cs);

	uint64_t estimateSearchSpan = 0;
	
	for(StringIter it = who.begin(); it != who.end(); ++it) {
		string& client = *it;
		for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
			Client* c = *j;
			if(c->isConnected() && c->getHubUrl() == client) {
				uint64_t ret = c->search(aSizeMode, aSize, aFileType, aString, aToken, aOwner);
				estimateSearchSpan = max(estimateSearchSpan, ret);
			}
		}
	}
	return estimateSearchSpan;
}

void ClientManager::on(TimerManagerListener::Minute, uint64_t /*aTick*/) throw() {
	Lock l(cs);

	// Collect some garbage...
	UserIter i = users.begin();
	while(i != users.end()) {
		if(i->second->unique()) {
			users.erase(i++);
		} else {
			++i;
		}
	}

	for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
		(*j)->info(false);
	}
}

UserPtr& ClientManager::getMe() {
	if(!me) {
		Lock l(cs);
		if(!me) {
			me = new User(getMyCID());
			users.insert(make_pair(me->getCID(), me));
		}
	}
	return me;
}

const CID& ClientManager::getMyPID() {
	if(pid.isZero())
		pid = CID(SETTING(PRIVATE_ID));
	return pid;
}

CID ClientManager::getMyCID() {
	TigerHash tiger;
	tiger.update(getMyPID().data(), CID::SIZE);
	return CID(tiger.finalize());
}

void ClientManager::updateNick(const OnlineUser& user) throw() {
	Lock l(cs);
	if(nicks.find(user.getUser()->getCID()) != nicks.end()) {
		return;
	}
	
	if(!user.getIdentity().getNick().empty()) {
		nicks.insert(std::make_pair(user.getUser()->getCID(), user.getIdentity().getNick()));
	}
}

void ClientManager::updateNick(const UserPtr& user, const string& nick) throw() {
	Lock l(cs);
	if(nicks.find(user->getCID()) != nicks.end()) {
		return;
	}
	
	if(!nick.empty()) {
		nicks.insert(std::make_pair(user->getCID(), nick));
	}
}

void ClientManager::on(Connected, const Client* c) throw() {
	fire(ClientManagerListener::ClientConnected(), c);
}

void ClientManager::on(UserUpdated, const Client*, const OnlineUserPtr& user) throw() {
	updateNick(*user);
	fire(ClientManagerListener::UserUpdated(), *user);
}

void ClientManager::on(UsersUpdated, const Client*, const OnlineUserList& l) throw() {
	for(OnlineUserList::const_iterator i = l.begin(), iend = l.end(); i != iend; ++i) {
		updateNick(*(*i));
		fire(ClientManagerListener::UserUpdated(), *(*i)); 
	}
}

void ClientManager::on(HubUpdated, const Client* c) throw() {
	fire(ClientManagerListener::ClientUpdated(), c);
}

void ClientManager::on(Failed, const Client* client, const string&) throw() { 
	fire(ClientManagerListener::ClientDisconnected(), client);
}

void ClientManager::on(HubUserCommand, const Client* client, int aType, int ctx, const string& name, const string& command) throw() { 
	if(BOOLSETTING(HUB_USER_COMMANDS)) {
		if(aType == UserCommand::TYPE_REMOVE) {
			int cmd = FavoriteManager::getInstance()->findUserCommand(name, client->getHubUrl());
			if(cmd != -1)
				FavoriteManager::getInstance()->removeUserCommand(cmd);
		} else if(aType == UserCommand::TYPE_CLEAR) {
 			FavoriteManager::getInstance()->removeHubUserCommands(ctx, client->getHubUrl());
 		} else {
			FavoriteManager::getInstance()->addUserCommand(aType, ctx, UserCommand::FLAG_NOSAVE, name, command, client->getHubUrl());
		}
	}
}

void ClientManager::setListLength(const UserPtr& p, const string& listLen) {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(p->getCID());
	if(i != onlineUsers.end()) {
		i->second->getIdentity().set("LL", listLen);
	}
}

void ClientManager::fileListDisconnected(const UserPtr& p) {
	bool remove = false;
	string report = Util::emptyString;
	Client* c = NULL;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i != onlineUsers.end()) {
			OnlineUser& ou = *i->second;
	
			int fileListDisconnects = Util::toInt(ou.getIdentity().get("FD")) + 1;
			ou.getIdentity().set("FD", Util::toString(fileListDisconnects));

			if(RSXSETTING(MAX_DISCONNECTS) == 0)
				return;

			if(fileListDisconnects == RSXSETTING(MAX_DISCONNECTS)) {
				c = &ou.getClient();
				report = ou.setCheat("Disconnected file list %[userFD] times", false, true, RSXBOOLSETTING(SHOW_DISCONNECT_RAW));
				if(ou.getIdentity().isFileListQueued()) {
					ou.setFileListComplete();
					ou.getIdentity().setFileListQueued(Util::emptyString);
					remove = true;
				}
				sendAction(ou, RSXSETTING(DISCONNECT_RAW));
			}
		}
	}

	if(remove) {
		try {
			QueueManager::getInstance()->removeFileListCheck(p);
		} catch (...) {
			//...
		}
	}
	if(c && !report.empty()) {
		c->cheatMessage(report);
	}
}

void ClientManager::connectionTimeout(const UserPtr& p) {
	uint8_t remove = 0;
	string report = Util::emptyString;
	Client* c = NULL;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i != onlineUsers.end()) {
			OnlineUser& ou = *i->second;
	
			int connectionTimeouts = Util::toInt(ou.getIdentity().get("TO")) + 1;
			ou.getIdentity().set("TO", Util::toString(connectionTimeouts));
	
			if(RSXSETTING(MAX_TIMEOUTS) == 0)
				return;
	
			if(connectionTimeouts == RSXSETTING(MAX_TIMEOUTS)) {
				c = &ou.getClient();
				report = ou.setCheat("Connection timeout %[userTO] times", false, false, RSXBOOLSETTING(SHOW_TIMEOUT_RAW));
				
				if(ou.getIdentity().isClientQueued()) {
					ou.setTestSURComplete();
					ou.getIdentity().setTestSURQueued(Util::emptyString);
					remove += 1;
				}
				if(ou.getIdentity().isFileListQueued()) {
					ou.setFileListComplete();
					ou.getIdentity().setFileListQueued(Util::emptyString);
					remove += 2;
				}
				sendAction(ou, RSXSETTING(TIMEOUT_RAW));
			}
		}
	}
	if(remove > 0) {
		try {
			if(remove == 1 || remove == 3) {
				QueueManager::getInstance()->removeTestSUR(p);
			}
			if(remove == 2 || remove == 3) {
				QueueManager::getInstance()->removeFileListCheck(p);
			}
		} catch(...) {
			//...
		}
	}
	if(c && !report.empty()) {
		c->cheatMessage(report);
	}
}

void ClientManager::checkCheating(const UserPtr& p, DirectoryListing* dl) {
	string report = Util::emptyString;
	OnlineUser* ou = NULL;
	{
		Lock l(cs);

		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end())
			return;

		ou = i->second;

		int64_t statedSize = ou->getIdentity().getBytesShared();
		int64_t realSize = dl->getTotalSize();

		double multiplier = ((100+(double)RSXSETTING(PERCENT_FAKE_SHARE_TOLERATED))/100); 
		int64_t sizeTolerated = (int64_t)(realSize*multiplier);

		ou->getIdentity().set("RS", Util::toString(realSize));
		ou->getIdentity().set("SF", Util::toString(dl->getTotalFileCount(true)));
	
		bool isFakeSharing = false;
		if(statedSize > sizeTolerated) {
			isFakeSharing = true;
		}

		if(isFakeSharing) {
			string cheatStr;
			if(realSize == 0) {
				cheatStr = "Mismatched share size - zero bytes real size";
			} else {
				double qwe = (double)((double)statedSize / (double)realSize);
				cheatStr = str(boost::format("Mismatched share size - filelist was inflated %1% times, stated size = %[userSSshort], real size = %[userRSshort]")
					% qwe);
			}
			report = ou->setCheat(cheatStr, false, true, RSXBOOLSETTING(SHOW_FAKESHARE_RAW));
			sendAction(*ou, RSXSETTING(FAKESHARE_RAW));
		} else {
			//RSX++ //ADLS Forbidden files
			const DirectoryListing::File::List forbiddenList = dl->getForbiddenFiles();
			const DirectoryListing::Directory::List forbiddenDirList = dl->getForbiddenDirs();

			if(forbiddenList.size() > 0 || forbiddenDirList.size() > 0) {
				int64_t fs = 0;
				string s, c, sz, tth, stringForKick, forbiddenFilesList;

				int actionCommand = 0, totalPoints = 0, point = 0;
				bool forFromFavs = false;
				bool forOverRide = false;

				if(forbiddenList.size() > 0) {
					for(DirectoryListing::File::Iter i = forbiddenList.begin() ; i != forbiddenList.end() ; i++) {
						fs += (*i)->getSize();
						totalPoints += (*i)->getPoints();
						if(((*i)->getPoints() >= point) || (*i)->getOverRidePoints()) {
							point = (*i)->getPoints();
							s = (*i)->getFullFileName();
							c = (*i)->getAdlsComment();
							tth = (*i)->getTTH().toBase32();
							sz = Util::toString((*i)->getSize());
							if((*i)->getOverRidePoints()) {
								forOverRide = true;
								if ((*i)->getFromFavs()) {
									actionCommand = (*i)->getAdlsRaw();
									forFromFavs = true;
								} else {
									stringForKick = (*i)->getKickString();
									forFromFavs = false;
								}
							}
						}
					}
				}
				if(forbiddenDirList.size() > 0) {
					for(DirectoryListing::Directory::Iter j = forbiddenDirList.begin() ; j != forbiddenDirList.end() ; j++) {
						fs += (*j)->getTotalSize();
						totalPoints += (*j)->getPoints();
						if(((*j)->getPoints() >= point) || (*j)->getOverRidePoints()) {
							point = (*j)->getPoints();
							s = (*j)->getName();
							c = (*j)->getAdlsComment();
							sz = Util::toString((*j)->getTotalSize());
							if((*j)->getOverRidePoints()) {
								forOverRide = true;
								if ((*j)->getFromFavs()) {
									actionCommand = (*j)->getAdlsRaw();
									forFromFavs = true;
								} else {
									stringForKick = (*j)->getKickString();
									forFromFavs = false;
								}
							}
						}
					}
				}
				ou->getIdentity().set("A1", s);								//file
				ou->getIdentity().set("A2", c);								//comment
				ou->getIdentity().set("A3", sz);							//file size
				ou->getIdentity().set("A4", tth);							//tth
				ou->getIdentity().set("A5", Util::toString(fs));			//forbidden size
				ou->getIdentity().set("A6", Util::toString(totalPoints));	//total points
				ou->getIdentity().set("A7", Util::toString((int)forbiddenList.size() + (int)forbiddenDirList.size())); //no. of forbidden files&dirs

				s = "[%[userA6]] Is sharing %[userA7] forbidden files/directories including: %[userA1]";

				if(forOverRide) {
					report = ou->setCheat(s, false, true, true);
					if(forFromFavs) {
						sendAction(*ou, actionCommand);
					} else {
						sendRawCommand(ou->getUser(), stringForKick);
					}
				} else if(totalPoints > 0) {
					bool show = RSXBOOLSETTING(SHOW_ADL_OUT_OF_RANGE);
					int rawToSend = RSXSETTING(ADL_OUT_OF_RANGE);
					RawManager::getInstance()->calcADLAction(totalPoints, rawToSend, show);
					report = ou->setCheat(s, false, true, show);
					sendAction(*ou, rawToSend);
				} else {
					const string& isRMDC = ou->getIdentity().checkrmDC(*ou);
					if(!isRMDC.empty()) {
						report = isRMDC;
					}
				}
			}
		}
		//END
	}

	if(ou) {
		ou->setFileListComplete();
		ou->getIdentity().setFileListQueued(Util::emptyString);
		ou->getClient().updated(ou);
		if(!report.empty()) {
			ou->getClient().cheatMessage(report);
		}
	}
}
//RSX++ //autosearch stuff
void ClientManager::kickFromAutosearch(const UserPtr& p, int action, const string& cheat, const string& file, const string& size, const string& tth, bool display/* = false*/) {
	OnlineUser* ou = NULL;
	string report = Util::emptyString;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end()) 
			return;
		ou = i->second;
		int noOfFiles = Util::toInt(ou->getIdentity().get("A7")) + 1;

		ou->getIdentity().set("A1", file);
		ou->getIdentity().set("A2", cheat);
		ou->getIdentity().set("A3", size);
		ou->getIdentity().set("A4", tth);
		ou->getIdentity().set("A7", Util::toString(noOfFiles));
		report = ou->setCheat(cheat, false, true, display && !cheat.empty());
		sendAction(*ou, action);
	}
	if(ou) {
		ou->getClient().updated(ou);
		if(!report.empty())
			ou->getClient().cheatMessage(report);
	}
}

void ClientManager::addCheckToQueue(const UserPtr& p, bool filelist) {
	OnlineUser* ou = NULL;
	bool addCheck = false;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end()) return;
		ou = i->second;

		if(ou->isCheckable() && ou->getClient().isOp()) {
			if(!ou->getChecked(filelist)) {
				if((filelist && ou->shouldCheckFileList()) || (!filelist && ou->shouldCheckClient())) {
					addCheck = true;
					ou->inc();
				}
			}
		}
	}

	if(addCheck) {
		try {
			if(filelist) {
				QueueManager::getInstance()->addFileListCheck(p);
				ou->getIdentity().setFileListQueued("1");
			} else {
				QueueManager::getInstance()->addTestSUR(p);
				ou->getIdentity().setTestSURQueued("1");
			}
		} catch(...) {
			//...
		}
		ou->dec();
	}
}
//hub stats
tstring ClientManager::getHubsLoadInfo() const {
	string hubsInfo = Util::emptyString;
	int64_t overallShare = 0;
	uint32_t overallUsers = 0;

	{
		Lock l(cs);
		for(Client::List::const_iterator i = clients.begin(); i != clients.end(); ++i) {
			overallShare += (*i)->getAvailable();
			overallUsers += (*i)->getUserCount();
		}
	}
	hubsInfo = "Hubs stats:";
	hubsInfo += "\r\n-]> Connected hubs:\t" + Util::toString(Client::getTotalCounts()) + " (" + Client::getCounts() + ")";
	hubsInfo += "\r\n-]> Available bytes:\t\t" + Util::formatBytes(overallShare);
	hubsInfo += "\r\n-]> Users count:\t\t" + Util::toString(overallUsers);
	return Text::toT(hubsInfo);
}
//END
void ClientManager::setCheating(const UserPtr& p, const string& _ccResponse, const string& _cheatString, int _actionId, bool _displayCheat,
		bool _badClient, bool _badFileList, bool _clientCheckComplete, bool _fileListCheckComplete) {
	OnlineUser* ou = NULL;
	string report = Util::emptyString;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end()) return;

		ou = i->second;

		if(!_ccResponse.empty()) {
			ou->setTestSURComplete();
			ou->getIdentity().set("TS", _ccResponse);
			report = ou->getIdentity().updateClientType(*ou);
		}
		if(_clientCheckComplete)
			ou->setTestSURComplete();
		if(_fileListCheckComplete)
			ou->setFileListComplete();
		if(!_cheatString.empty())
			report = ou->setCheat(_cheatString, _badClient, _badFileList, _displayCheat);
		sendAction(*ou, _actionId);
	}
	if(ou) {
		ou->getClient().updated(ou);
		if(!report.empty())
			ou->getClient().cheatMessage(report);
	}
}
//RSX++
void ClientManager::setListSize(const UserPtr& p, int64_t aFileLength, bool adc) {
	OnlineUser* ou = NULL;
	string report = Util::emptyString;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end()) return;
		
		ou = i->second;
		ou->getIdentity().set("LS", Util::toString(aFileLength));

		if(ou->getIdentity().getBytesShared() > 0) {
			if((RSXSETTING(MAXIMUM_FILELIST_SIZE) > 0) && (aFileLength > RSXSETTING(MAXIMUM_FILELIST_SIZE)) && RSXSETTING(FILELIST_TOO_SMALL_BIG)) {
				report = ou->setCheat("Too large filelist - %[userLSshort] for the specified share of %[userSSshort]", false, true, RSXBOOLSETTING(FILELIST_TOO_SMALL_BIG));
				sendAction(*ou, RSXSETTING(FILELIST_TOO_SMALL_BIG));
			} else if((aFileLength < RSXSETTING(MINIMUM_FILELIST_SIZE) && RSXSETTING(FILELIST_TOO_SMALL_BIG)) || (aFileLength < 100)) {
				report = ou->setCheat("Too small filelist - %[userLSshort] for the specified share of %[userSSshort]", false, true, RSXBOOLSETTING(FILELIST_TOO_SMALL_BIG));
				sendAction(*ou, RSXSETTING(FILELIST_TOO_SMALL_BIG));
			}
		} else if(adc == false) {
			int64_t listLength = (!ou->getIdentity().get("LL").empty()) ? Util::toInt64(ou->getIdentity().get("LL")) : -1;
			if(p->isSet(User::DCPLUSPLUS) && (listLength != -1) && (listLength * 3 < aFileLength) && (ou->getIdentity().getBytesShared() > 0)) {
				report = ou->setCheat("Fake file list - ListLen = %[userLL], FileLength = %[userLS]", false, true, RSXBOOLSETTING(LISTLEN_MISMATCH));
				sendAction(*ou, RSXSETTING(LISTLEN_MISMATCH));
			}
		}
	}
	if(ou) {
		ou->getClient().updated(ou);
		if(!report.empty())
			ou->getClient().cheatMessage(report);
	}
}
//END
int ClientManager::getMode(const string& aHubUrl) const {
	if(aHubUrl.empty()) return SETTING(INCOMING_CONNECTIONS);

	const FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(aHubUrl);
	if(hub) {
		switch(hub->getMode()) {
			case 1 : return SettingsManager::INCOMING_DIRECT;
			case 2 : return SettingsManager::INCOMING_FIREWALL_PASSIVE;
			default: return SETTING(INCOMING_CONNECTIONS);
		}
	}
	return SETTING(INCOMING_CONNECTIONS);
}

void ClientManager::cancelSearch(void* aOwner) {
	Lock l(cs);

	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		(*i)->cancelSearch(aOwner);
	}
}
//RSX++ //Lua
bool ClientManager::ucExecuteLua(const string& ucCommand, StringMap& params) throw() {
	bool executedlua = false;
	string::size_type i, j, k;
	i = j = k = 0;
	string tmp = ucCommand;
	while( (i = tmp.find("%[lua:", i)) != string::npos) {
		i += 6;
		j = tmp.find(']', i);
		if(j == string::npos)
			break;
		string chunk = tmp.substr(i, j-i);
		// for making possible using %[nick] and similar parameters too
		// !%!{nick!}
		k = 0;
		while( (k = chunk.find("!%")) != string::npos) {
			chunk.erase(k, 2);
			chunk.insert(k, "%");
		}
		k = 0;
		while( (k = chunk.find("!{")) != string::npos) {
			chunk.erase(k, 2);
			chunk.insert(k, "[");
		}
		k = 0;
		while( (k = chunk.find("!}")) != string::npos) {
			chunk.erase(k, 2);
			chunk.insert(k, "]");
		}
		//@todo: use filter? I opted for no here, but this means Lua has to be careful about
		//filtering if it cares.
		ScriptManager::getInstance()->EvaluateChunk(Util::formatParams(chunk, params, false));
		executedlua = true;
		i = j + 1;
	}
	return executedlua;
}
//RSX++ //MultiHubKick
void ClientManager::multiHubKick(const UserPtr& p, const string& aRaw) {
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end()) 
			return;
		for(OnlineMap::const_iterator j = onlineUsers.begin(); j != onlineUsers.end(); j++) {
			if(compareUsers((*i->second), (*j->second))) {
				sendRawCommand(j->second->getUser(), aRaw);
			}
		}
	}
}

bool ClientManager::compareUsers(const OnlineUser& ou1, const OnlineUser& ou2) const {
	//first, check if im op on hub ;]
	if(!ou1.getClient().isOp() || !ou2.getClient().isOp())
		return false;
	//check ip
	if(ou1.getIdentity().getIp().compare(ou2.getIdentity().getIp()) != 0)
		return false;
	//check sharesize
	if(ou1.getIdentity().getBytesShared() != ou2.getIdentity().getBytesShared())
		return false;
	//check connection
	if(ou1.getIdentity().getConnection().compare(ou2.getIdentity().getConnection()) != 0)
		return false;
	//check hub counts
	if(ou1.getIdentity().get("AH") == "0" || ou2.getIdentity().get("AH") == "0" || ou1.getIdentity().get("AH").empty() || ou2.getIdentity().get("AH").empty())
		return false;
	if(ou1.getIdentity().get("AH").compare(ou2.getIdentity().get("AH")) != 0)
		return false;
	//good, seems to be the same user
	return true;
}

void ClientManager::sendAction(const UserPtr& p, const int aAction) {
	if(aAction < 1)
		return;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end()) 
			return;

		if(i->second->getClient().isOp() && !i->second->isProtectedUser()) {
			i->second->getClient().sendActionCommand((*i->second), aAction);
		}
	}
}

void ClientManager::sendAction(OnlineUser& ou, const int aAction) {
	if(aAction < 1)
		return;

	if(ou.getClient().isOp() && !ou.isProtectedUser()) {
		ou.getClient().sendActionCommand(ou, aAction);
	}
}
//END

} // namespace dcpp

/**
 * @file
 * $Id: ClientManager.cpp 405 2008-07-14 11:41:15Z BigMuscle $
 */
