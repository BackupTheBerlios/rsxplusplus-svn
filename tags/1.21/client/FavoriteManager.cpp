/* 
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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

#include "FavoriteManager.h"

#include "ClientManager.h"
#include "ResourceManager.h"
#include "CryptoManager.h"

#include "HttpConnection.h"
#include "StringTokenizer.h"
#include "SimpleXML.h"
#include "UserCommand.h"
#include "BZUtils.h"
#include "FilteredFile.h"
#include "RawManager.h" //RSX++
#include "StringTokenizer.h" //RSX++

namespace dcpp {
//RSX++
FavoriteHubEntry::FavAction::FavAction(bool _enabled, string _raw /*= Util::emptyString*/, int id /*=0*/) throw() : enabled(_enabled) {
	if(_raw.empty()) return;
	const Action* a = RawManager::getInstance()->findAction(id);
	if(a != NULL) {
		StringTokenizer<string> tok(_raw, ',');
		for(StringIter j = tok.getTokens().begin(); j != tok.getTokens().end(); ++j) {
			int rId = Util::toInt(*j);
			for(Action::RawsList::const_iterator i = a->raw.begin(); i != a->raw.end(); ++i) {
				if(rId == i->getId()) {
					raws.push_back(rId);
					break;
				}
			}
		}
	}
};
//END

FavoriteManager::FavoriteManager() : lastId(0), useHttp(false), running(false), c(NULL), lastServer(0), listType(TYPE_NORMAL), dontSave(false) {
	SettingsManager::getInstance()->addListener(this);
	ClientManager::getInstance()->addListener(this);

	File::ensureDirectory(Util::getHubListsPath());
}

FavoriteManager::~FavoriteManager() throw() {
	ClientManager::getInstance()->removeListener(this);
	SettingsManager::getInstance()->removeListener(this);
	if(c) {
		c->removeListener(this);
		delete c;
		c = NULL;
	}

	for_each(favoriteHubs.begin(), favoriteHubs.end(), DeleteFunction());
	for_each(recentHubs.begin(), recentHubs.end(), DeleteFunction());
	for_each(previewApplications.begin(), previewApplications.end(), DeleteFunction());
	//RSX++
	for_each(dirsEx.begin(), dirsEx.end(), DeleteFunction());
	for_each(filters.begin(), filters.end(), DeleteFunction());
	for_each(hls.begin(), hls.end(), DeleteFunction());
	//END
}

UserCommand FavoriteManager::addUserCommand(int type, int ctx, Flags::MaskType flags, const string& name, const string& command, const string& to, const string& hub) {
	// No dupes, add it...
	Lock l(cs);
	userCommands.push_back(UserCommand(lastId++, type, ctx, flags, name, command, to, hub));
	UserCommand& uc = userCommands.back();
	if(!uc.isSet(UserCommand::FLAG_NOSAVE)) 
		save();
	return userCommands.back();
}

bool FavoriteManager::getUserCommand(int cid, UserCommand& uc) {
	Lock l(cs);
	for(UserCommand::List::const_iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == cid) {
			uc = *i;
			return true;
		}
	}
	return false;
}

bool FavoriteManager::moveUserCommand(int cid, int pos) {
	dcassert(pos == -1 || pos == 1);
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == cid) {
			swap(*i, *(i + pos));
			return true;
		}
	}
	return false;
}

void FavoriteManager::updateUserCommand(const UserCommand& uc) {
	bool nosave = true;
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == uc.getId()) {
			*i = uc;
			nosave = uc.isSet(UserCommand::FLAG_NOSAVE);
			break;
		}
	}
	if(!nosave)
		save();
}

int FavoriteManager::findUserCommand(const string& aName, const string& aUrl) {
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getName() == aName && i->getHub() == aUrl) {
			return i->getId();
		}
	}
	return -1;
}

void FavoriteManager::removeUserCommand(int cid) {
	bool nosave = true;
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == cid) {
			nosave = i->isSet(UserCommand::FLAG_NOSAVE);
			userCommands.erase(i);
			break;
		}
	}
	if(!nosave)
		save();
}
void FavoriteManager::removeUserCommand(const string& srv) {
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ) {
		if((i->getHub() == srv) && i->isSet(UserCommand::FLAG_NOSAVE)) {
			i = userCommands.erase(i);
		} else {
			++i;
		}
	}
}

void FavoriteManager::removeHubUserCommands(int ctx, const string& hub) {
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ) {
		if(i->getHub() == hub && i->isSet(UserCommand::FLAG_NOSAVE) && i->getCtx() & ctx) {
			i = userCommands.erase(i);
		} else {
			++i;
		}
	}
}

void FavoriteManager::addFavoriteUser(const UserPtr& aUser) { 
	Lock l(cs);
	if(users.find(aUser->getCID()) == users.end()) {
		StringList urls = ClientManager::getInstance()->getHubs(aUser->getCID(), Util::emptyString);
		StringList nicks = ClientManager::getInstance()->getNicks(aUser->getCID(), Util::emptyString);
        
		/// @todo make this an error probably...
		if(urls.empty())
			urls.push_back(Util::emptyString);
		if(nicks.empty())
			nicks.push_back(Util::emptyString);

		FavoriteMap::const_iterator i = users.insert(make_pair(aUser->getCID(), FavoriteUser(aUser, nicks[0], urls[0]))).first;
		fire(FavoriteManagerListener::UserAdded(), i->second);
		save();
	}
}

void FavoriteManager::removeFavoriteUser(const UserPtr& aUser) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(aUser->getCID());
	if(i != users.end()) {
		fire(FavoriteManagerListener::UserRemoved(), i->second);
		users.erase(i);
		save();
	}
}

std::string FavoriteManager::getUserURL(const UserPtr& aUser) const {
	Lock l(cs);
	FavoriteMap::const_iterator i = users.find(aUser->getCID());
	if(i != users.end()) {
		const FavoriteUser& fu = i->second;
		return fu.getUrl();
	}
	return Util::emptyString;
}

void FavoriteManager::addFavorite(const FavoriteHubEntry& aEntry) {
	FavoriteHubEntry* f;

	FavoriteHubEntryList::const_iterator i = getFavoriteHub(aEntry.getServer());
	if(i != favoriteHubs.end()) {
		return;
	}
	f = new FavoriteHubEntry(aEntry);
	favoriteHubs.push_back(f);
	fire(FavoriteManagerListener::FavoriteAdded(), f);
	save();
}

void FavoriteManager::removeFavorite(const FavoriteHubEntry* entry) {
	FavoriteHubEntryList::iterator i = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(i == favoriteHubs.end()) {
		return;
	}

	fire(FavoriteManagerListener::FavoriteRemoved(), entry);
	favoriteHubs.erase(i);
	delete entry;
	save();
}

bool FavoriteManager::isFavoriteHub(const std::string& url) {
	FavoriteHubEntryList::const_iterator i = getFavoriteHub(url);
	if(i != favoriteHubs.end()) {
		return true;
	}
	return false;
}

bool FavoriteManager::addFavoriteDir(const string& aDirectory, const string & aName){
	string path = aDirectory;

	if( path[ path.length() -1 ] != PATH_SEPARATOR )
		path += PATH_SEPARATOR;

	for(StringPairIter i = favoriteDirs.begin(); i != favoriteDirs.end(); ++i) {
		if((strnicmp(path, i->first, i->first.length()) == 0) && (strnicmp(path, i->first, path.length()) == 0)) {
			return false;
		}
		if(stricmp(aName, i->second) == 0) {
			return false;
		}
	}
	favoriteDirs.push_back(make_pair(aDirectory, aName));
	save();
	return true;
}

bool FavoriteManager::removeFavoriteDir(const string& aName) {
	string d(aName);

	if(d[d.length() - 1] != PATH_SEPARATOR)
		d += PATH_SEPARATOR;

	for(StringPairIter j = favoriteDirs.begin(); j != favoriteDirs.end(); ++j) {
		if(stricmp(j->first.c_str(), d.c_str()) == 0) {
			favoriteDirs.erase(j);
			save();
			return true;
		}
	}
	return false;
}

bool FavoriteManager::renameFavoriteDir(const string& aName, const string& anotherName) {

	for(StringPairIter j = favoriteDirs.begin(); j != favoriteDirs.end(); ++j) {
		if(stricmp(j->second.c_str(), aName.c_str()) == 0) {
			j->second = anotherName;
			save();
			return true;
		}
	}
	return false;
}

void FavoriteManager::addRecent(const RecentHubEntry& aEntry) {
	RecentHubEntry::Iter i = getRecentHub(aEntry.getServer());
	if(i != recentHubs.end()) {
		return;
	}
	RecentHubEntry* f = new RecentHubEntry(aEntry);
	recentHubs.push_back(f);
	fire(FavoriteManagerListener::RecentAdded(), f);
	recentsave();
}

void FavoriteManager::removeRecent(const RecentHubEntry* entry) {
	RecentHubEntry::List::iterator i = find(recentHubs.begin(), recentHubs.end(), entry);
	if(i == recentHubs.end()) {
		return;
	}
		
	fire(FavoriteManagerListener::RecentRemoved(), entry);
	recentHubs.erase(i);
	delete entry;
	recentsave();
}

void FavoriteManager::updateRecent(const RecentHubEntry* entry) {
	RecentHubEntry::Iter i = find(recentHubs.begin(), recentHubs.end(), entry);
	if(i == recentHubs.end()) {
		return;
	}
		
	fire(FavoriteManagerListener::RecentUpdated(), entry);
	recentsave();
}

class XmlListLoader : public SimpleXMLReader::CallBack {
public:
	XmlListLoader(HubEntryList& lst) : publicHubs(lst) { }
	~XmlListLoader() { }
	void startTag(const string& name, StringPairList& attribs, bool) {
		if(name == "Hub") {
			const string& name = getAttrib(attribs, "Name", 0);
			const string& server = getAttrib(attribs, "Address", 1);
			const string& description = getAttrib(attribs, "Description", 2);
			const string& users = getAttrib(attribs, "Users", 3);
			const string& country = getAttrib(attribs, "Country", 4);
			const string& shared = getAttrib(attribs, "Shared", 5);
			const string& minShare = getAttrib(attribs, "Minshare", 5);
			const string& minSlots = getAttrib(attribs, "Minslots", 5);
			const string& maxHubs = getAttrib(attribs, "Maxhubs", 5);
			const string& maxUsers = getAttrib(attribs, "Maxusers", 5);
			const string& reliability = getAttrib(attribs, "Reliability", 5);
			const string& rating = getAttrib(attribs, "Rating", 5);
			publicHubs.push_back(HubEntry(name, server, description, users, country, shared, minShare, minSlots, maxHubs, maxUsers, reliability, rating));
		}
	}
	void endTag(const string&, const string&) {

	}
private:
	HubEntryList& publicHubs;
};

bool FavoriteManager::onHttpFinished(bool fromHttp) throw() {
	MemoryInputStream mis(downloadBuf);
	bool success = true;

	Lock l(cs);
	HubEntryList& list = publicListMatrix[publicListServer];
	list.clear();

	try {
		XmlListLoader loader(list);

		if((listType == TYPE_BZIP2) && (!downloadBuf.empty())) {
			FilteredInputStream<UnBZFilter, false> f(&mis);
			SimpleXMLReader(&loader).parse(f);
		} else {
			SimpleXMLReader(&loader).parse(mis);
		}
	} catch(const Exception&) {
		success = false;
		fire(FavoriteManagerListener::Corrupted(), fromHttp ? publicListServer : Util::emptyString);
	}

	if(fromHttp) {
		try {
			File f(Util::getHubListsPath() + Util::validateFileName(publicListServer), File::WRITE, File::CREATE | File::TRUNCATE);
			f.write(downloadBuf);
			f.close();
		} catch(const FileException&) { }
	}

	downloadBuf = Util::emptyString;
	
	return success;
}

void FavoriteManager::save() {
	if(dontSave)
		return;

	Lock l(cs);
	try {
		SimpleXML xml;

		xml.addTag("Favorites");
		xml.stepIn();

		xml.addTag("Hubs");
		xml.stepIn();

		for(FavHubGroups::const_iterator i = favHubGroups.begin(), iend = favHubGroups.end(); i != iend; ++i) {
			xml.addTag("Group");
			xml.addChildAttrib("Name", i->first);
			xml.addChildAttrib("Private", i->second.priv);
		}

		for(FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(), iend = favoriteHubs.end(); i != iend; ++i) {
			xml.addTag("Hub");
			xml.addChildAttrib("Name", (*i)->getName());
			xml.addChildAttrib("Connect", (*i)->getConnect());
			xml.addChildAttrib("Description", (*i)->getDescription());
			xml.addChildAttrib("Nick", (*i)->getNick(false));
			xml.addChildAttrib("Password", (*i)->getPassword());
			xml.addChildAttrib("Server", (*i)->getServer());
			xml.addChildAttrib("UserDescription", (*i)->getUserDescription());
			xml.addChildAttrib("Encoding", (*i)->getEncoding());
			xml.addChildAttrib("ChatUserSplit", (*i)->getChatUserSplit());
			xml.addChildAttrib("StealthMode", (*i)->getStealth());
			xml.addChildAttrib("UserListState", (*i)->getUserListState());
			xml.addChildAttrib("HubFrameOrder",	(*i)->getHeaderOrder());
			xml.addChildAttrib("HubFrameWidths", (*i)->getHeaderWidths());
			xml.addChildAttrib("HubFrameVisible", (*i)->getHeaderVisible());
			xml.addChildAttrib("Mode", Util::toString((*i)->getMode()));
			xml.addChildAttrib("IP", (*i)->getIP());
			xml.addChildAttrib("SearchInterval", Util::toString((*i)->getSearchInterval()));
			xml.addChildAttrib("Group", (*i)->getGroup());
			//RSX++
			xml.addChildAttrib("Email", (*i)->getCurrentEmail());
			xml.addChildAttrib("AwayMsg", (*i)->getAwayMsg());
			xml.addChildAttrib("UserProtected", (*i)->getProtectedUsers());
			xml.addChildAttrib("CheckOnConnect", (*i)->getCheckOnConnect());
			xml.addChildAttrib("CheckClients", (*i)->getCheckClients());
			xml.addChildAttrib("CheckFilelists", (*i)->getCheckFilelists());
			xml.addChildAttrib("CheckUserInfo", (*i)->getCheckMyInfo());
			xml.addChildAttrib("HideShare", (*i)->getHideShare());
			xml.addChildAttrib("UseFilter", (*i)->getUseFilter());
			xml.addChildAttrib("UseAutosearch", (*i)->getUseAutosearch());
			xml.addChildAttrib("UseHighLight", (*i)->getUseHL());
			xml.addChildAttrib("ChatExtraInfo", (*i)->getChatExtraInfo());
			xml.addChildAttrib("UsersLimitToUseActions", (*i)->getUsersLimit());
			//RSX++
			xml.stepIn();
			for(FavoriteHubEntry::FavAction::List::const_iterator a = (*i)->action.begin(); a != (*i)->action.end(); ++a) {
				if(RawManager::getInstance()->getValidAction(a->first)) {
					string raw = Util::emptyString;
					for(std::list<int>::const_iterator j = a->second->raws.begin(); j != a->second->raws.end(); ++j) {
						if(!raw.empty())
							raw += ",";
						raw += Util::toString(*j);
					}
					if(!raw.empty() || a->second->getEnabled()) {
						xml.addTag("Action");
						xml.addChildAttrib("ID", a->first);
						xml.addChildAttrib("Active", Util::toString(a->second->getEnabled()));
						xml.addChildAttrib("Raw", raw);
					}
				}
			}
			xml.stepOut();
			//END
		}

		xml.stepOut();

		xml.addTag("Users");
		xml.stepIn();
		for(FavoriteMap::const_iterator i = users.begin(), iend = users.end(); i != iend; ++i) {
			xml.addTag("User");
			xml.addChildAttrib("LastSeen", i->second.getLastSeen());
			xml.addChildAttrib("GrantSlot", i->second.isSet(FavoriteUser::FLAG_GRANTSLOT));
			xml.addChildAttrib("UserDescription", i->second.getDescription());
			xml.addChildAttrib("Nick", i->second.getNick());
			xml.addChildAttrib("URL", i->second.getUrl());
			xml.addChildAttrib("CID", i->first.toBase32());
		}
		xml.stepOut();

		xml.addTag("IgnoredUsers");
		xml.stepIn();
		for(unordered_set<CID>::const_iterator i = ignoredUsers.begin(), iend = ignoredUsers.end(); i != iend; ++i) {
			xml.addTag("User");
			xml.addChildAttrib("CID", i->toBase32());
		}
		xml.stepOut();

		xml.addTag("UserCommands");
		xml.stepIn();
		for(UserCommand::List::const_iterator i = userCommands.begin(), iend = userCommands.end(); i != iend; ++i) {
			if(!i->isSet(UserCommand::FLAG_NOSAVE)) {
				xml.addTag("UserCommand");
				xml.addChildAttrib("Type", i->getType());
				xml.addChildAttrib("Context", i->getCtx());
				xml.addChildAttrib("Name", i->getName());
				xml.addChildAttrib("Command", i->getCommand());
				xml.addChildAttrib("To", i->getTo());
				xml.addChildAttrib("Hub", i->getHub());
			}
		}
		xml.stepOut();

		//Favorite download to dirs
		xml.addTag("FavoriteDirs");
		xml.stepIn();
		StringPairList spl = getFavoriteDirs();
		for(StringPairIter i = spl.begin(), iend = spl.end(); i != iend; ++i) {
			xml.addTag("Directory", i->first);
			xml.addChildAttrib("Name", i->second);
		}
		xml.stepOut();

		xml.stepOut();

		string fname = getConfigFile();

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);

	} catch(const Exception& e) {
		dcdebug("FavoriteManager::save: %s\n", e.getError().c_str());
	}
}

void FavoriteManager::recentsave() {
	try {
		SimpleXML xml;

		xml.addTag("Recents");
		xml.stepIn();

		xml.addTag("Hubs");
		xml.stepIn();

		for(RecentHubEntry::Iter i = recentHubs.begin(); i != recentHubs.end(); ++i) {
			xml.addTag("Hub");
			xml.addChildAttrib("Name", (*i)->getName());
			xml.addChildAttrib("Description", (*i)->getDescription());
			xml.addChildAttrib("Users", (*i)->getUsers());
			xml.addChildAttrib("Shared", (*i)->getShared());
			xml.addChildAttrib("Server", (*i)->getServer());
		}

		xml.stepOut();
		xml.stepOut();
		
		string fname = Util::getPath(Util::PATH_USER_CONFIG) + "Recents.xml";

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::recentsave: %s\n", e.getError().c_str());
	}
}
//RSX++ //Filters
void FavoriteManager::filtersSave() {
	try {
		SimpleXML xml;

		xml.addTag("Filters");
		xml.stepIn();
			xml.addTag("Filters");
			xml.stepIn();

			for(Filters::Iter i = filters.begin(); i != filters.end(); ++i) {
				xml.addTag("Filters");
				xml.addChildAttrib("Fstring", (*i)->getFstring());
				xml.addChildAttrib("UseColor", (*i)->getUseColor());
				xml.addChildAttrib("HasFontColor", (*i)->getHasFontColor());
				xml.addChildAttrib("HasBgColor", (*i)->getHasBgColor());
				xml.addChildAttrib("FontColor", (*i)->getFontColor());
				xml.addChildAttrib("BackColor", (*i)->getBackColor());
				xml.addChildAttrib("BoldFont", (*i)->getBoldFont());
				xml.addChildAttrib("ItalicFont", (*i)->getItalicFont());
				xml.addChildAttrib("UnderlineFont", (*i)->getUnderlineFont());
				xml.addChildAttrib("StrikeoutFont", (*i)->getStrikeoutFont());
			}

			xml.stepOut();

			xml.addTag("HighLights");
			xml.stepIn();
			for(HighLight::Iter i = hls.begin(); i != hls.end(); ++i) {
				xml.addTag("HighLights");
				xml.addChildAttrib("HLstring", (*i)->getHstring());
				xml.addChildAttrib("HasFontColor", (*i)->getHasFontColor());
				xml.addChildAttrib("HasBgColor", (*i)->getHasBgColor());
				xml.addChildAttrib("FontColor", (*i)->getFontColor());
				xml.addChildAttrib("BackColor", (*i)->getBackColor());
				xml.addChildAttrib("BoldFont", (*i)->getBoldFont());
				xml.addChildAttrib("ItalicFont", (*i)->getItalicFont());
				xml.addChildAttrib("UnderlineFont", (*i)->getUnderlineFont());
				xml.addChildAttrib("StrikeoutFont", (*i)->getStrikeoutFont());
				xml.addChildAttrib("DisplayPopup",	(*i)->getDisplayPopup());
				xml.addChildAttrib("FlashWindow",	(*i)->getFlashWindow());
				xml.addChildAttrib("PlaySound",		(*i)->getPlaySound());
				xml.addChildAttrib("SoundFile",		(*i)->getSoundFilePath());
			}
			xml.stepOut();
		xml.stepOut();

		string fname = Util::getPath(Util::PATH_USER_CONFIG) + "Filters.xml";

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::recentsave: %s\n", e.getError().c_str());
	}
}

void FavoriteManager::loadFilters(SimpleXML& aXml){
	dontSave = true;
	aXml.resetCurrentChild();
	if(aXml.findChild("Filters")) {
		aXml.stepIn();
		while(aXml.findChild("Filters")) {					
			addFilter(aXml.getChildAttrib("Fstring"), aXml.getBoolChildAttrib("UseColor"), aXml.getBoolChildAttrib("HasFontColor"),
				aXml.getBoolChildAttrib("HasBgColor"), aXml.getIntChildAttrib("FontColor"), aXml.getIntChildAttrib("BackColor"), 
				aXml.getBoolChildAttrib("BoldFont"), aXml.getBoolChildAttrib("ItalicFont"), aXml.getBoolChildAttrib("UnderlineFont"), 
				aXml.getBoolChildAttrib("StrikeoutFont"));			
		}
		aXml.stepOut();
	}
	dontSave = false;
}

void FavoriteManager::loadFilters(){
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getPath(Util::PATH_USER_CONFIG) + "Filters.xml", File::READ, File::OPEN).read());
		if(xml.findChild("Filters")) {
			xml.stepIn();
			loadFilters(xml);
			loadHighLight(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::recentload: %s\n", e.getError().c_str());
	}	
}

void FavoriteManager::loadHighLight(SimpleXML& aXml){
	dontSave = true;
	aXml.resetCurrentChild();
	if(aXml.findChild("HighLights")) {
		aXml.stepIn();
		while(aXml.findChild("HighLights")) {					
			addHighLight(
				aXml.getChildAttrib("HLstring"), 
				aXml.getBoolChildAttrib("HasFontColor"), 
				aXml.getBoolChildAttrib("HasBgColor"),
				aXml.getIntChildAttrib("FontColor"), 
				aXml.getIntChildAttrib("BackColor"), 
				aXml.getBoolChildAttrib("BoldFont"), 
				aXml.getBoolChildAttrib("ItalicFont"), 
				aXml.getBoolChildAttrib("UnderlineFont"), 
				aXml.getBoolChildAttrib("StrikeoutFont"),
				aXml.getBoolChildAttrib("DisplayPopup"),
				aXml.getBoolChildAttrib("FlashWindow"),
				aXml.getBoolChildAttrib("PlaySound"),
				aXml.getChildAttrib("SoundFile")
				);
		}
		aXml.stepOut();
	}
	dontSave = false;
}
//END

void FavoriteManager::load() {
	loadFilters(); //RSX++

	try {
		SimpleXML xml;
		Util::migrate(getConfigFile());
		xml.fromXML(File(getConfigFile(), File::READ, File::OPEN).read());
		
		if(xml.findChild("Favorites")) {
			xml.stepIn();
			load(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::load: %s\n", e.getError().c_str());
	}

	try {
		Util::migrate(Util::getPath(Util::PATH_USER_CONFIG) + "Recents.xml");
		
		SimpleXML xml;
		xml.fromXML(File(Util::getPath(Util::PATH_USER_CONFIG) + "Recents.xml", File::READ, File::OPEN).read());
		
		if(xml.findChild("Recents")) {
			xml.stepIn();
			recentload(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::recentload: %s\n", e.getError().c_str());
	}
}

void FavoriteManager::load(SimpleXML& aXml) {
	dontSave = true;
	bool needSave = false;

	aXml.resetCurrentChild();
	if(aXml.findChild("Hubs")) {
		aXml.stepIn();

		while(aXml.findChild("Group")) {
			string name = aXml.getChildAttrib("Name");
			if(name.empty())
				continue;
			FavHubGroupProperties props = { aXml.getBoolChildAttrib("Private") };
			favHubGroups[name] = props;
		}

		aXml.resetCurrentChild();
		while(aXml.findChild("Hub")) {
			FavoriteHubEntry* e = new FavoriteHubEntry();
			e->setName(aXml.getChildAttrib("Name"));
			e->setConnect(aXml.getBoolChildAttrib("Connect"));
			e->setDescription(aXml.getChildAttrib("Description"));
			e->setNick(aXml.getChildAttrib("Nick"));
			e->setPassword(aXml.getChildAttrib("Password"));
			e->setServer(aXml.getChildAttrib("Server"));
			e->setUserDescription(aXml.getChildAttrib("UserDescription"));
			e->setEncoding(aXml.getChildAttrib("Encoding"));
			e->setChatUserSplit(aXml.getIntChildAttrib("ChatUserSplit"));
			e->setStealth(aXml.getBoolChildAttrib("StealthMode"));
			e->setUserListState(aXml.getBoolChildAttrib("UserListState"));
			e->setHeaderOrder(aXml.getChildAttrib("HubFrameOrder", SETTING(HUBFRAME_ORDER)));
			e->setHeaderWidths(aXml.getChildAttrib("HubFrameWidths", SETTING(HUBFRAME_WIDTHS)));
			e->setHeaderVisible(aXml.getChildAttrib("HubFrameVisible", SETTING(HUBFRAME_VISIBLE)));
			//RSX++
			e->setCurrentEmail(aXml.getChildAttrib("Email"));
			e->setAwayMsg(aXml.getChildAttrib("AwayMsg"));
			e->setProtectedUsers(aXml.getChildAttrib("UserProtected"));		
			e->setCheckOnConnect(aXml.getBoolChildAttrib("CheckOnConnect"));
			e->setCheckClients(aXml.getBoolChildAttrib("CheckClients"));
			e->setCheckFilelists(aXml.getBoolChildAttrib("CheckFilelists"));
			e->setCheckMyInfo(aXml.getBoolChildAttrib("CheckUserInfo"));
			e->setHideShare(aXml.getBoolChildAttrib("HideShare"));
			e->setUseFilter(aXml.getBoolChildAttrib("UseFilter"));
			e->setUseAutosearch(aXml.getBoolChildAttrib("UseAutosearch"));
			e->setUseHL(aXml.getBoolChildAttrib("UseHighLight"));
			e->setChatExtraInfo(aXml.getChildAttrib("ChatExtraInfo"));
			e->setUsersLimit(aXml.getIntChildAttrib("UsersLimitToUseActions"));
			//END
			e->setMode(Util::toInt(aXml.getChildAttrib("Mode")));
			e->setIP(aXml.getChildAttrib("IP"));
			e->setSearchInterval(Util::toUInt32(aXml.getChildAttrib("SearchInterval")));
			e->setGroup(aXml.getChildAttrib("Group"));
			//RSX++
			aXml.stepIn();
			while(aXml.findChild("Action")) {
				int actionId = aXml.getIntChildAttrib("ID");
				bool enabled = aXml.getBoolChildAttrib("Active");
				const string& raw = aXml.getChildAttrib("Raw");
				if(RawManager::getInstance()->getValidAction(actionId))
					e->action.insert(make_pair(actionId, new FavoriteHubEntry::FavAction(enabled, raw, actionId)));
			}
			aXml.stepOut();
			//END
			favoriteHubs.push_back(e);
		}

		aXml.stepOut();
	}
	aXml.resetCurrentChild();
	if(aXml.findChild("Users")) {
		aXml.stepIn();
		while(aXml.findChild("User")) {
			UserPtr u;
			const string& cid = aXml.getChildAttrib("CID");
			const string& nick = aXml.getChildAttrib("Nick");
			const string& hubUrl = aXml.getChildAttrib("URL");

			if(cid.length() != 39) {
				if(nick.empty() || hubUrl.empty())
					continue;
				u = ClientManager::getInstance()->getUser(nick, hubUrl);
			} else {
				u = ClientManager::getInstance()->getUser(CID(cid));
			}
			FavoriteMap::iterator i = users.insert(make_pair(u->getCID(), FavoriteUser(u, nick, hubUrl))).first;

			if(aXml.getBoolChildAttrib("GrantSlot"))
				i->second.setFlag(FavoriteUser::FLAG_GRANTSLOT);

			i->second.setLastSeen((uint32_t)aXml.getIntChildAttrib("LastSeen"));
			i->second.setDescription(aXml.getChildAttrib("UserDescription"));

		}
		aXml.stepOut();
	}

	aXml.resetCurrentChild();
	if(aXml.findChild("IgnoredUsers")) {
		aXml.stepIn();
		while(aXml.findChild("User")) {
			ignoredUsers.insert(CID(aXml.getChildAttrib("CID")));
		}
		aXml.stepOut();
	}

	aXml.resetCurrentChild();
	if(aXml.findChild("UserCommands")) {
		aXml.stepIn();
		while(aXml.findChild("UserCommand")) {
			addUserCommand(aXml.getIntChildAttrib("Type"), aXml.getIntChildAttrib("Context"), 0, aXml.getChildAttrib("Name"),
				aXml.getChildAttrib("Command"), aXml.getChildAttrib("To"), aXml.getChildAttrib("Hub"));
		}
		aXml.stepOut();
	}

	//Favorite download to dirs
	aXml.resetCurrentChild();
	if(aXml.findChild("FavoriteDirs")) {
		aXml.stepIn();
		while(aXml.findChild("Directory")) {
			string virt = aXml.getChildAttrib("Name");
			string d(aXml.getChildData());
			FavoriteManager::getInstance()->addFavoriteDir(d, virt);
		}
		aXml.stepOut();
	}

	dontSave = false;
	if(needSave)
		save();
}

void FavoriteManager::userUpdated(const OnlineUser& info) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(info.getUser()->getCID());
	if(i != users.end()) {
		FavoriteUser& fu = i->second;
		fu.update(info);
		save();
	}
}

FavoriteHubEntry* FavoriteManager::getFavoriteHubEntry(const string& aServer) const {
	for(FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(), iend = favoriteHubs.end(); i != iend; ++i) {
		FavoriteHubEntry* hub = *i;
		if(stricmp(hub->getServer(), aServer) == 0) {
			return hub;
		}
	}
	return NULL;
}
	
FavoriteHubEntryList FavoriteManager::getFavoriteHubs(const string& group) const {
	FavoriteHubEntryList ret;
	for(FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(), iend = favoriteHubs.end(); i != iend; ++i)
		if((*i)->getGroup() == group)
			ret.push_back(*i);
	return ret;
}

bool FavoriteManager::isPrivate(const string& url) const {
	if(url.empty())
		return false;

	FavoriteHubEntry* fav = getFavoriteHubEntry(url);
	if(fav) {
		const string& name = fav->getGroup();
		if(!name.empty()) {
			FavHubGroups::const_iterator group = favHubGroups.find(name);
			if(group != favHubGroups.end())
				return group->second.priv;
		}
	}
	return false;
}

bool FavoriteManager::hasSlot(const UserPtr& aUser) const { 
	Lock l(cs);
	FavoriteMap::const_iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return false;
	return i->second.isSet(FavoriteUser::FLAG_GRANTSLOT);
}

time_t FavoriteManager::getLastSeen(const UserPtr& aUser) const { 
	Lock l(cs);
	FavoriteMap::const_iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return 0;
	return i->second.getLastSeen();
}

void FavoriteManager::setAutoGrant(const UserPtr& aUser, bool grant) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return;
	if(grant)
		i->second.setFlag(FavoriteUser::FLAG_GRANTSLOT);
	else
		i->second.unsetFlag(FavoriteUser::FLAG_GRANTSLOT);
	save();
}
void FavoriteManager::setUserDescription(const UserPtr& aUser, const string& description) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return;
	i->second.setDescription(description);
	save();
}

void FavoriteManager::recentload(SimpleXML& aXml) {
	aXml.resetCurrentChild();
	if(aXml.findChild("Hubs")) {
		aXml.stepIn();
		while(aXml.findChild("Hub")) {
			RecentHubEntry* e = new RecentHubEntry();
			e->setName(aXml.getChildAttrib("Name"));
			e->setDescription(aXml.getChildAttrib("Description"));
			e->setUsers(aXml.getChildAttrib("Users"));
			e->setShared(aXml.getChildAttrib("Shared"));
			e->setServer(aXml.getChildAttrib("Server"));
			recentHubs.push_back(e);
		}
		aXml.stepOut();
	}
}

StringList FavoriteManager::getHubLists() {
	StringTokenizer<string> lists(SETTING(HUBLIST_SERVERS), ';');
	return lists.getTokens();
}

FavoriteHubEntryList::const_iterator FavoriteManager::getFavoriteHub(const string& aServer) const {
	for(FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i) {
		if(stricmp((*i)->getServer(), aServer) == 0) {
			return i;
		}
	}
	return favoriteHubs.end();
}

RecentHubEntry::Iter FavoriteManager::getRecentHub(const string& aServer) const {
	for(RecentHubEntry::Iter i = recentHubs.begin(); i != recentHubs.end(); ++i) {
		if(stricmp((*i)->getServer(), aServer) == 0) {
			return i;
		}
	}
	return recentHubs.end();
}

void FavoriteManager::setHubList(int aHubList) {
	lastServer = aHubList;
	refresh();
}

void FavoriteManager::refresh(bool forceDownload /* = false */) {
	StringList sl = getHubLists();
	if(sl.empty())
		return;
	publicListServer = sl[(lastServer) % sl.size()];
	if(strnicmp(publicListServer.c_str(), "http://", 7) != 0) {
		lastServer++;
		return;
	}

	if(!forceDownload) {
		string path = Util::getHubListsPath() + Util::validateFileName(publicListServer);
		if(File::getSize(path) > 0) {
			useHttp = false;
			string fileDate;
			{
				Lock l(cs);
				publicListMatrix[publicListServer].clear();
			}
			listType = (stricmp(path.substr(path.size() - 4), ".bz2") == 0) ? TYPE_BZIP2 : TYPE_NORMAL;
			try {
				File cached(path, File::READ, File::OPEN);
				downloadBuf = cached.read();
				char buf[20];
				time_t fd = cached.getLastModified();
				if (strftime(buf, 20, "%x", localtime(&fd))) {
					fileDate = string(buf);
				}
			} catch(const FileException&) {
				downloadBuf = Util::emptyString;
			}
			if(!downloadBuf.empty()) {
				if (onHttpFinished(false)) {
					fire(FavoriteManagerListener::LoadedFromCache(), publicListServer, fileDate);
				}		
				return;
			}
		}
	}

	if(!running) {
		useHttp = true;
		{
			Lock l(cs);
			publicListMatrix[publicListServer].clear();
		}
		fire(FavoriteManagerListener::DownloadStarting(), publicListServer);
		if(c == NULL)
			c = new HttpConnection();
		c->addListener(this);
		c->downloadFile(publicListServer);
		running = true;
	}
}

UserCommand::List FavoriteManager::getUserCommands(int ctx, const StringList& hubs, bool& op) {
	vector<bool> isOp(hubs.size());

	for(size_t i = 0; i < hubs.size(); ++i) {
		if(ClientManager::getInstance()->isOp(ClientManager::getInstance()->getMe(), hubs[i])) {
			isOp[i] = true;
			op = true; // ugly hack
		}
	}

	Lock l(cs);
	UserCommand::List lst;
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		UserCommand& uc = *i;
		if(!(uc.getCtx() & ctx)) {
			continue;
		}

		for(size_t j = 0; j < hubs.size(); ++j) {
			const string& hub = hubs[j];
			bool hubAdc = hub.compare(0, 6, "adc://") == 0 || hub.compare(0, 7, "adcs://") == 0;
			bool commandAdc = uc.getHub().compare(0, 6, "adc://") == 0 || uc.getHub().compare(0, 7, "adcs://") == 0;
			if(hubAdc && commandAdc) {
				if((uc.getHub() == "adc://" || uc.getHub() == "adcs://") ||
					((uc.getHub() == "adc://op" || uc.getHub() == "adcs://op") && isOp[j]) ||
					(uc.getHub() == hub) )
				{
					lst.push_back(*i);
					break;
				}
			} else if((!hubAdc && !commandAdc) || uc.isChat()) {
				if((uc.getHub().length() == 0) || 
					(uc.getHub() == "op" && isOp[j]) ||
					(uc.getHub() == hub) )
				{
					lst.push_back(*i);
					break;
				}
			}
		}
	}
	return lst;
}

// HttpConnectionListener
void FavoriteManager::on(Data, HttpConnection*, const uint8_t* buf, size_t len) throw() { 
	if(useHttp)
		downloadBuf.append((const char*)buf, len);
}

void FavoriteManager::on(Failed, HttpConnection*, const string& aLine) throw() { 
	c->removeListener(this);
	lastServer++;
	running = false;
	if(useHttp){
		downloadBuf = Util::emptyString;
		fire(FavoriteManagerListener::DownloadFailed(), aLine);
	}
}
void FavoriteManager::on(Complete, HttpConnection*, const string& aLine, bool fromCoral) throw() {
	bool parseSuccess = false;

	c->removeListener(this);
	if(useHttp) {
		parseSuccess = onHttpFinished(true);
	}	
	running = false;
	if(useHttp && parseSuccess) {
		fire(FavoriteManagerListener::DownloadFinished(), aLine, fromCoral);
	}
}
void FavoriteManager::on(Redirected, HttpConnection*, const string& aLine) throw() { 
	if(useHttp)
		fire(FavoriteManagerListener::DownloadStarting(), aLine);
}
void FavoriteManager::on(TypeNormal, HttpConnection*) throw() { 
	if(useHttp)
		listType = TYPE_NORMAL; 
}
void FavoriteManager::on(TypeBZ2, HttpConnection*) throw() { 
	if(useHttp)
		listType = TYPE_BZIP2; 
}

void FavoriteManager::on(UserUpdated, const OnlineUser& user) throw() {
	userUpdated(user);
}
void FavoriteManager::on(UserDisconnected, const UserPtr& user) throw() {
	bool isFav = false;
	{
		Lock l(cs);
		FavoriteMap::iterator i = users.find(user->getCID());
		if(i != users.end()) {
			isFav = true;
			i->second.setLastSeen(GET_TIME());
			save();
		}
	}
	if(isFav)
		fire(FavoriteManagerListener::StatusChanged(), user);
}

void FavoriteManager::on(UserConnected, const UserPtr& user) throw() {
	bool isFav = false;
	{
		Lock l(cs);
		FavoriteMap::const_iterator i = users.find(user->getCID());
		if(i != users.end()) {
			isFav = true;
		}
	}
	if(isFav)
		fire(FavoriteManagerListener::StatusChanged(), user);
}

void FavoriteManager::previewload(SimpleXML& aXml){
	aXml.resetCurrentChild();
	if(aXml.findChild("PreviewApps")) {
		aXml.stepIn();
		while(aXml.findChild("Application")) {					
			addPreviewApp(aXml.getChildAttrib("Name"), aXml.getChildAttrib("Application"), 
				aXml.getChildAttrib("Arguments"), aXml.getChildAttrib("Extension"));			
		}
		aXml.stepOut();
	}	
}

void FavoriteManager::previewsave(SimpleXML& aXml){
	aXml.addTag("PreviewApps");
	aXml.stepIn();
	for(PreviewApplication::Iter i = previewApplications.begin(); i != previewApplications.end(); ++i) {
		aXml.addTag("Application");
		aXml.addChildAttrib("Name", (*i)->getName());
		aXml.addChildAttrib("Application", (*i)->getApplication());
		aXml.addChildAttrib("Arguments", (*i)->getArguments());
		aXml.addChildAttrib("Extension", (*i)->getExtension());
	}
	aXml.stepOut();
}
//RSX++ //Download to depends on extension/name
void FavoriteManager::dirsExLoad(SimpleXML& aXml){
	aXml.resetCurrentChild();
	if(aXml.findChild("DirectoriesEx")) {
		aXml.stepIn();
		while(aXml.findChild("Dir")) {					
			addDirEx(
				aXml.getChildAttrib("Name"), 
				aXml.getChildAttrib("Path"), 
				aXml.getChildAttrib("Extension")
				);			
		}
		aXml.stepOut();
	}	
}

void FavoriteManager::dirsExSave(SimpleXML& aXml){
	aXml.addTag("DirectoriesEx");
	aXml.stepIn();
	for(DirectoriesEx::Iter i = dirsEx.begin(); i != dirsEx.end(); ++i) {
		aXml.addTag("Dir");
		aXml.addChildAttrib("Name", (*i)->getName());
		aXml.addChildAttrib("Path", (*i)->getPath());
		aXml.addChildAttrib("Extension", (*i)->getExtension());
	}
	aXml.stepOut();
}
//RSX++ //Raw Manager
bool FavoriteManager::getEnabledAction(FavoriteHubEntry* entry, int actionId) {
	FavoriteHubEntry::Iter h = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(h == favoriteHubs.end())
		return false;

	FavoriteHubEntry::FavAction::List::const_iterator i = (*h)->action.find(actionId);
	if(i != (*h)->action.end()) {
		return i->second->getEnabled();
	} else {
		(*h)->action.insert(make_pair(actionId, new FavoriteHubEntry::FavAction(false)));
		return false;
	}
}

void FavoriteManager::setEnabledAction(FavoriteHubEntry* entry, int actionId, bool enabled) {
	FavoriteHubEntry::Iter h = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(h == favoriteHubs.end())
		return;

	FavoriteHubEntry::FavAction::List::iterator i = (*h)->action.find(actionId);
	if(i != (*h)->action.end()) {
		i->second->setEnabled(enabled);
		return;
	}
	if(enabled)
		(*h)->action.insert(make_pair(actionId, new FavoriteHubEntry::FavAction(true)));
}

bool FavoriteManager::getEnabledRaw(FavoriteHubEntry* entry, int actionId, int rawId) {
	FavoriteHubEntry::Iter h = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(h == favoriteHubs.end())
		return false;

	FavoriteHubEntry::FavAction::List::const_iterator i = (*h)->action.find(actionId);
	if(i == (*h)->action.end())
		return false;
	for(std::list<int>::const_iterator j = i->second->raws.begin(); j != i->second->raws.end(); ++j) {
		if(*j == rawId) {
			return true;
		}
	}
	return false;
}

void FavoriteManager::setEnabledRaw(FavoriteHubEntry* entry, int actionId, int rawId, bool enabled) {
	FavoriteHubEntry::Iter h = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(h == favoriteHubs.end())
		return;

	FavoriteHubEntry::FavAction::List::const_iterator i = (*h)->action.find(actionId);
	if(i != (*h)->action.end()) {
		for(std::list<int>::iterator j = i->second->raws.begin(); j != i->second->raws.end(); ++j) {
			if(*j == rawId) {
				if(!enabled)
					i->second->raws.erase(j);
				return;
			}
		}
		if(enabled)
			i->second->raws.push_back(rawId);
		return;
	}

	if(enabled) {
		FavoriteHubEntry::FavAction* act = (*h)->action.insert(make_pair(actionId, new FavoriteHubEntry::FavAction(true))).first->second;
		act->raws.push_back(rawId);
	}
}
//RSX++
string FavoriteManager::getAwayMessage(const string& aServer) {
	FavoriteHubEntry* hub = getFavoriteHubEntry(aServer);
	if(hub) {
		return hub->getAwayMsg();
	}
	return Util::emptyString;
}
//END

} // namespace dcpp

/**
 * @file
 * $Id: FavoriteManager.cpp 482 2010-02-13 10:49:30Z bigmuscle $
 */
