/*
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

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"

#include "AutoSearchManager.h"

#include "../client/ClientManager.h"
#include "../client/LogManager.h"
#include "../client/ShareManager.h"
#include "../client/QueueManager.h"
#include "../client/StringTokenizer.h"
#include "../client/pme.h"
#include "../client/Pointer.h"

#define AUTOSEARCH_FILE "Autosearch.xml"

AutoSearchManager::AutoSearchManager() : version("1.00") {
	TimerManager::getInstance()->addListener(this);
	SearchManager::getInstance()->addListener(this);
	removeRegExpFromSearches();

	curPos = 0;
	endOfList = false;
	recheckTime = 0;
	curSearch = Util::emptyString;

	setTime((uint16_t)RSXSETTING(AUTOSEARCH_EVERY) -1); //1 minute delay
}

AutoSearchManager::~AutoSearchManager() {
	SearchManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	AutosearchSave();
	for_each(as.begin(), as.end(), DeleteFunction());
}

void AutoSearchManager::removeRegExpFromSearches() {
	//clear old data
	vs.clear();
	for(Autosearch::List::const_iterator j = as.begin(); j != as.end(); j++) {
		if((*j)->getEnabled()) {
			if(((*j)->getFileType() != 9)) {
				vs.push_back((*j));			//valid searches - we can search for it
			}
		}
	}
}

void AutoSearchManager::getAllowedHubs() {
	allowedHubs.clear();
	ClientManager* cm = ClientManager::getInstance();

	const Client::List& clients = cm->getClients();
	cm->lock();
	for(Client::List::const_iterator i = clients.begin(); i != clients.end(); ++i) {
		Client* client = *i;
		if(!client || !client->isConnected())
			continue;

		if(client->getUseAutosearch())
			allowedHubs.push_back(client->getHubUrl());
	}
	cm->unlock();
}

bool AutoSearchManager::matchDirectory(const string& aFile, const string& aStrToMatch) {
	string lastDir = Util::getLastDir(aFile);
	string dir = Text::toLower(lastDir);
	string strToMatch = Text::toLower(aStrToMatch);

	//path separator at string's end
	if(strToMatch.rfind(PATH_SEPARATOR) == string::npos)
		strToMatch += PATH_SEPARATOR;
	if(dir.rfind(PATH_SEPARATOR) == string::npos)
		dir += PATH_SEPARATOR;

	if(dir.compare(strToMatch) == 0) {
		return true;
	} else {
		return false;
	}
}

void AutoSearchManager::on(TimerManagerListener::Minute, uint64_t /*aTick*/) throw() {
	if(RSXBOOLSETTING(AUTOSEARCH_ENABLED_TIME) && RSXBOOLSETTING(AUTOSEARCH_ENABLED) && !as.empty()) {
		Lock l(cs);

		if(endOfList) {
			recheckTime++;
			if(recheckTime < RSXSETTING(AUTOSEARCH_RECHECK_TIME)) {
				return;
			} else {
				endOfList = false; //time's up, search for items again ;]
			}
		}

		time++;
		if(time < RSXSETTING(AUTOSEARCH_EVERY))
			return;

		getAllowedHubs();
		removeRegExpFromSearches();

		//no hubs? no fun...
		if(!allowedHubs.size()) {
			return;
		}
		//empty valid autosearch list? too bad
		if(!vs.size()) {
			return;
		}
		Autosearch::List::const_iterator pos = vs.begin() + curPos;
		users.clear();

		if(pos < vs.end()) {
			if((*pos) == NULL)
				return;
			SearchManager::getInstance()->search(allowedHubs, (*pos)->getSearchString(), 0, (SearchManager::TypeModes)(*pos)->getFileType(), SearchManager::SIZE_DONTCARE, "auto");
			curSearch = (*pos)->getSearchString();
			curPos++;
			setTime(0);
			LogManager::getInstance()->message("[A][S:" + Util::toString(curPos) + "]Searching for: " + (*pos)->getSearchString());
		} else {
			LogManager::getInstance()->message("[A]Next search after " + Util::toString(RSXSETTING(AUTOSEARCH_RECHECK_TIME))+ " minutes.");
			setTime(0);
			curPos = 0;
			endOfList = true;
			recheckTime = 0;
			curSearch = Util::emptyString;
		}
	}
}

void AutoSearchManager::onSearchResult(const SearchResult* sr) throw() {
	if(!as.empty() && RSXBOOLSETTING(AUTOSEARCH_ENABLED) && !allowedHubs.empty()) {
		{
			//Lock l(cs);
			UserPtr user = static_cast<UserPtr>(sr->getUser());
			if(users.find(user) == users.end()) {
				users.insert(user);
				Autosearch* item = NULL;

				for(Autosearch::List::const_iterator i = as.begin(); i != as.end(); ++i) {
					if((*i)->getFileType() == 9) { //regexp
						PME reg((*i)->getSearchString(), "gims");
						if(reg.IsValid() && reg.match(sr->getFile())) {
							processAction((*i), sr->getFile(), sr->getTTH(), sr->getSize(), user);
						}
					} else if(curSearch.compare((*i)->getSearchString()) == 0) { //match only to current search
						item = (*i);
						break;
					}
				}
				if(!item)
					return;
				if(item->getFileType() == 8) { //TTH
					if(sr->getTTH().toBase32() == item->getSearchString()) {
						processAction(item, sr->getFile(), sr->getTTH(), sr->getSize(), user);
					}
				} else if(item->getFileType() == 7 && sr->getType() == SearchResult::TYPE_DIRECTORY) { //directory
					bool found = matchDirectory(sr->getFile(), item->getSearchString());
					if(found) {
						processAction(item, sr->getFile(), sr->getTTH(), sr->getSize(), user);
					}
				} else if(ShareManager::getInstance()->checkType(sr->getFile(), item->getFileType())) {
					if(!sr->getFile().empty()) {
						const string& iFile = Text::toLower(sr->getFile());
						StringTokenizer<string> tss(Text::toLower(item->getSearchString()), " ");
						StringList& slSrch = tss.getTokens();
						bool matched = true;
						for(StringList::const_iterator j = slSrch.begin(); j != slSrch.end(); ++j) {
							if(j->empty()) continue;
							if(iFile.find(*j) == string::npos) {
								matched = false;
								break;
							}
						}
						slSrch.clear();
						if(matched) {
							processAction(item, sr->getFile(), sr->getTTH(), sr->getSize(), user);
						}
					}
				}
			}
		}
	}
}

void AutoSearchManager::processAction(const Autosearch* a, const string fl, const TTHValue tth, int64_t s, const UserPtr u) {
	if(a->getAction() == 0) {
		ClientManager::getInstance()->kickFromAutosearch(u, a->getRaw(), a->getCheat(), fl, Util::toString(s), tth.toBase32(), a->getDisplayCheat());
	} else if(a->getAction() == 1) {
		addToQueue(fl, tth, s, u);
	} else if(a->getAction() == 2) {
		addToQueue(fl, tth, s, u, true);
	} else if(a->getAction() == 3) {
		ClientManager::getInstance()->addCheckToQueue(u, true);
	}
}

void AutoSearchManager::addToQueue(const string& fl, const TTHValue& tth, int64_t s, const UserPtr& u, bool pausePrio/* = false*/) {
	Lock l(cs);
	const string& fullpath = SETTING(DOWNLOAD_DIRECTORY) + Util::getFileName(fl);
	if(!ShareManager::getInstance()->isTTHShared(tth)) {
		try {
			QueueManager::getInstance()->add(fullpath, s, tth, u);
			if(pausePrio)
				QueueManager::getInstance()->setPriority(fullpath, QueueItem::PAUSED);
		} catch(...) {
			//...
		}
	}
}

void AutoSearchManager::AutosearchSave() {
	Lock l(cs);
	try {
		SimpleXML xml;

		xml.addTag("Autosearch");
		xml.stepIn();
		xml.addTag("Autosearch");
		xml.stepIn();

		for(Autosearch::List::const_iterator i = as.begin(); i != as.end(); ++i) {
			xml.addTag("Autosearch");
			xml.addChildAttrib("Enabled", (*i)->getEnabled());
			xml.addChildAttrib("SearchString", (*i)->getSearchString());
			xml.addChildAttrib("FileType", (*i)->getFileType());
			xml.addChildAttrib("Action", (*i)->getAction());
			xml.addChildAttrib("Raw", (*i)->getRaw());
			xml.addChildAttrib("DisplayCheat", (*i)->getDisplayCheat());
			xml.addChildAttrib("Cheat", (*i)->getCheat());
		}

		xml.stepOut();
		xml.stepOut();

		xml.stepIn();
		xml.addTag("VersionInfo");
		xml.stepIn();
		xml.addTag("Version", getVersion());
		xml.stepOut();

		xml.stepOut();
		
		string fname = Util::getConfigPath() + AUTOSEARCH_FILE;

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);
	} catch(const Exception& e) {
		dcdebug("AutoSearchManager::AutosearchSave: %s\n", e.getError().c_str());
	}
}

void AutoSearchManager::loadAutosearch(SimpleXML& aXml) {
	Lock l(cs);
	as.clear();
	aXml.resetCurrentChild();
	if(aXml.findChild("Autosearch")) {
		aXml.stepIn();
		while(aXml.findChild("Autosearch")) {					
			addAutosearch(aXml.getBoolChildAttrib("Enabled"),
				aXml.getChildAttrib("SearchString"), 
				aXml.getIntChildAttrib("FileType"), 
				aXml.getIntChildAttrib("Action"), 
				aXml.getIntChildAttrib("Raw"),
				aXml.getBoolChildAttrib("DisplayCheat"), 
				aXml.getChildAttrib("Cheat"));			
		}
		aXml.stepOut();
	}

	aXml.resetCurrentChild();
	if(aXml.findChild("VersionInfo")) {
		aXml.stepIn();
		if(aXml.findChild("Version")) {
			setVersion(aXml.getChildData());
		}
		aXml.stepOut();
	}
}

void AutoSearchManager::AutosearchLoad() {
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + AUTOSEARCH_FILE, File::READ, File::OPEN).read());
		if(xml.findChild("Autosearch")) {
			xml.stepIn();
			loadAutosearch(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("AutoSearchManager::AutosearchLoad: %s\n", e.getError().c_str());
	}	
}