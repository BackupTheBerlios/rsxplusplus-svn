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
#include "RegexUtil.h"

#include "../client/ClientManager.h"
#include "../client/LogManager.h"
#include "../client/ShareManager.h"
#include "../client/QueueManager.h"
#include "../client/StringTokenizer.h"
#include "../client/Pointer.h"
#include "../client/Thread.h"
#include "../client/SearchResult.h"

#define AUTOSEARCH_FILE "Autosearch.xml"

namespace dcpp {

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
	stopTaskThread();
	SearchManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	AutosearchSave();
	for_each(as.begin(), as.end(), DeleteFunction());
}

void AutoSearchManager::removeRegExpFromSearches() {
	//clear old data
	Lock l(cs);
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
	Lock l(cs);
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

void AutoSearchManager::on(TimerManagerListener::Minute, uint64_t /*aTick*/) throw() {
	if(RSXBOOLSETTING(AUTOSEARCH_ENABLED_TIME) && RSXBOOLSETTING(AUTOSEARCH_ENABLED) && !as.empty()) {

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
		if(vs.empty()) {
			return;
		}

		{
			Lock l(cs);
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
}

void AutoSearchManager::on(SearchManagerListener::SR, const SearchResultPtr& sr) throw() {
	if(RSXBOOLSETTING(AUTOSEARCH_ENABLED)) {
		if(!as.empty() && !allowedHubs.empty()) {
			UserPtr user = static_cast<UserPtr>(sr->getUser());
			if(users.find(user) == users.end()) {
				users.insert(user);
				Lock l(cs);
				for(Autosearch::List::const_iterator i = as.begin(); i != as.end(); ++i) {
					if(curSearch.compare((*i)->getSearchString()) == 0) {
						addResultToQueue(sr, (*i));
						return;
					}
				}
			}
		}
	}
}

//TaskThread
class TaskThread : public Thread {
public:
	TaskThread() : stop(true), as(NULL) { };
	~TaskThread() throw() { shutdown(); };

	void shutdown() { 
		stop = true;
		s.signal();
		as = NULL;
		join();
	}

	CriticalSection cs;
	Semaphore s;
	bool stop;

	typedef std::slist<SearchResultPtr> Results;
	Results results;
	Autosearch* as;

private:
	int run() {
		while(true) {
			if(stop || results.empty())
				break;
			{
				SearchResultPtr sr = NULL;
				Lock l(cs);
				sr = results.front();
				results.pop_front();

				if(sr != NULL) {
					Autosearch::List& list = AutoSearchManager::getInstance()->getAutosearch();
					for(Autosearch::List::const_iterator i = list.begin(); i != list.end(); ++i) {
						if((*i)->getFileType() == 9) {
							if(RegexUtil::match(sr->getFile(), (*i)->getSearchString())) {
								processAction(sr);
							}
						}
					}
					if(as->getFileType() == 8) {
						if(sr->getTTH().toBase32() == as->getSearchString()) {
							processAction(sr);
						}
					} else if(as->getFileType() == 7 && sr->getType() == SearchResult::TYPE_DIRECTORY) { //directory
						bool found = matchDirectory(sr->getFile(), as->getSearchString());
						if(found) {
							processAction(sr);
						}
					} else if(ShareManager::getInstance()->checkType(sr->getFile(), as->getFileType())) {
						if(!sr->getFile().empty()) {
							const string& iFile = Text::toLower(sr->getFile());
							StringTokenizer<string> tss(Text::toLower(as->getSearchString()), " ");
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
								processAction(sr);
							}
						}
					}
					//cleanup
					sr->dec();
				}
			}
			sleep(1000);
		}
		stop = true;
		return 0;
	}

	void processAction(const SearchResultPtr& s) {
		if(as->getAction() == 0) {
			ClientManager::getInstance()->kickFromAutosearch(s->getUser(), as->getRaw(), as->getCheat(), s->getFile(), Util::toString(s->getSize()), s->getTTH().toBase32(), as->getDisplayCheat());
		} else if(as->getAction() == 1) {
			addToQueue(s, false);
		} else if(as->getAction() == 2) {
			addToQueue(s, true);
		} else if(as->getAction() == 3) {
			ClientManager::getInstance()->addCheckToQueue(s->getUser(), true);
		}
	}

	void addToQueue(const SearchResultPtr& s, bool pausePrio) {
		const string& fullpath = SETTING(DOWNLOAD_DIRECTORY) + s->getFileName();
		if(!ShareManager::getInstance()->isTTHShared(s->getTTH())) {
			try {
				QueueManager::getInstance()->add(fullpath, s->getSize(), s->getTTH(), s->getUser());
				if(pausePrio)
					QueueManager::getInstance()->setPriority(fullpath, QueueItem::PAUSED);
			} catch(...) {
				//...
			}
		}
	}

	bool matchDirectory(const string& aFile, const string& aStrToMatch) {
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
}tasks;

void AutoSearchManager::addResultToQueue(const SearchResultPtr& sres, Autosearch* a) {
	sres->inc();
	{
		Lock l(tasks.cs);
		tasks.results.push_front(sres);
		tasks.as = a;
	}
	if(tasks.stop == true) {
		tasks.stop = false;
		tasks.start();
	}
	tasks.s.signal();
}

void AutoSearchManager::stopTaskThread() { tasks.shutdown(); }

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

}; // namespace dcpp