/*
 * Copyright (C) 2007-2010 adrian_007, adrian-007 on o2 point pl
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

#ifndef RSXPLUSPLUS_HUB_USERS_MAP_H
#define RSXPLUSPLUS_HUB_USERS_MAP_H

#include "../client/QueueManager.h"
#include "../client/Client.h"
#include "../client/Thread.h"
#include "../client/rsxppSettingsManager.h"

namespace dcpp {

template<bool isADC, typename BaseMap>
class HubUsersMap : public BaseMap {
public:
	HubUsersMap() : clientEngine(NULL) { };
	~HubUsersMap() throw() {
		stopCheck();
		stopMyINFOCheck();
	};

	void startMyINFOCheck(Client* c) { 
		if(!myInfoEngine.isRunning()) {
			myInfoEngine.startCheck(c);
		}
	}

	tstring startChecking(Client* c, const tstring& param) throw() {
		if(!c->isOp())
			return _T("You are not an Operator on this hub");
		if(clientEngine != NULL) {
			stopCheck();
			return _T("Checking stopped");
		}

		bool cc = false;
		bool cf = false;

		if(param.empty()) {
			cc = c->getCheckClients();
			cf = c->getCheckFilelists();
		} else {
			if(stricmp(param.c_str(), _T("clients")) == 0 || stricmp(param.c_str(), _T("c")) == 0)
				cc = true;
			else if(stricmp(param.c_str(), _T("filelists")) == 0 || stricmp(param.c_str(), _T("fl")) == 0 || stricmp(param.c_str(), _T("f")) == 0)
				cf = true;
			else if(stricmp(param.c_str(), _T("all")) == 0)
				cc = cf = true;
			else
				return _T("Incorrect parameters!");
		}
		if(!cc && !cf)
			return _T("There is nothing to check - select check type (Clients/FileLists) in Hub Properties");

		if(clientEngine == NULL) {
			clientEngine = new ThreadedCheck(this, c);
			clientEngine->setCheckClients(cc);
			clientEngine->setCheckFilelists(cf);
			if(c->getCheckOnConnect())
				clientEngine->setCheckOnConnect(true);
			clientEngine->startCheck();

			if(cc && !cf)
				return _T("Checking started (Clients)");
			else if(!cc && cf)
				return _T("Checking started (FileLists)");
			else if(cc && cf)
				return _T("Checking started (Clients & FileLists)");
		}
		return Util::emptyStringT;
	}

	void stopMyINFOCheck() {
		if(myInfoEngine.isRunning()) {
			myInfoEngine.stop = true;
		}
	}

	void stopCheck() throw() {
		if(clientEngine != NULL) {
			delete clientEngine;
			clientEngine = NULL;
		}
	}

	bool isDetectorRunning() const { 
		return (clientEngine != NULL && clientEngine->isChecking());
	}

private:
	//myinfo check engine
	class ThreadedMyINFOCheck : public Thread {
	public:
		ThreadedMyINFOCheck() : client(NULL), stop(true) { };

		bool isRunning() const { return !stop; }

		void startCheck(Client* _c) {
			if(_c && stop) {
				client = _c;
				start();
			}
		}

		CriticalSection cs;
		int run() {
			stop = false;
			setThreadPriority(Thread::HIGH);
			if(client && client->isConnected()) {
				client->setCheckedAtConnect(true);

				OnlineUserList ul;
				client->getUserList(ul);

				Lock l(cs);
				for(OnlineUserList::const_iterator i = ul.begin(); i != ul.end(); ++i) {
					if(stop) {
						break;
					}
					{
						OnlineUserPtr ou = *i;
						if(ou->isCheckable()) {
							string report = ou->getIdentity().myInfoDetect(*ou);
							if(!report.empty()) {
								ou->getClient().cheatMessage(report);
								ou->getClient().updated(ou);
							}
						}
					}
					sleep(1);
				}
			}
			stop = true;
			return 0;
		}
		bool stop;
		Client* client;
	}myInfoEngine;

	//clients check engine
	class ThreadedCheck : public Thread {
	public:
		ThreadedCheck(HubUsersMap* _u, Client* _c) : client(_c), users(_u), 
			keepChecking(false), canCheckFilelist(false), inThread(false), checkOnConnect(false) { };
		~ThreadedCheck() {
			keepChecking = inThread = false;

			StringList items;
			OnlineUserList ul;
			client->getUserList(ul);
			for(OnlineUserList::const_iterator i = ul.begin(); i != ul.end(); ++i) {
				const Identity& id = (*i)->getIdentity();
				if(id.isClientQueued()) {
					string path = Util::getPath(Util::PATH_USER_CONFIG) + "TestSURs\\" + id.getTestSURQueued();
					items.push_back(path);
				}
				if(id.isFileListQueued()) {
					string path = Util::getPath(Util::PATH_USER_CONFIG) + id.getFileListQueued();
					items.push_back(path);
				}
			}

			for(StringIter j = items.begin(); j != items.end(); ++j) {
				try {
					QueueManager::getInstance()->remove(*j);
				} catch(...) {
					//
				}
			}
			join();
			client = NULL;
		}

		bool isChecking() const { 
			return inThread && keepChecking; 
		}

		inline void startCheck() {
			if(!client || !users) {
				return;
			}
			if(!inThread) {
				start();
			}
		}

		CriticalSection cs;

		enum Actions {
			ADD_CLIENT_CHECK = 0x01,
			REMOVE_CLIENT_CHECK = 0x02,
			ADD_FILELIST_CHECK = 0x04,
			CONTINUE = 0x08,
			BREAK = 0x10
		};

		int preformUserCheck(OnlineUser* ou, const uint8_t clientItems, const uint8_t filelistItems) {
			if(!ou->isCheckable((uint16_t)RSXPP_SETTING(CHECK_DELAY)))
				return CONTINUE;
			if(isADC) {
				if((ou->getUser()->isSet(User::NO_ADC_1_0_PROTOCOL) || ou->getUser()->isSet(User::NO_ADC_0_10_PROTOCOL)) && 
					!(ou->getIdentity().isClientChecked() || ou->getIdentity().isFileListChecked())) {
					//nasty...
					ou->getIdentity().setTestSURChecked(Util::toString(GET_TIME()));
					ou->getIdentity().setFileListChecked(Util::toString(GET_TIME()));
					string report = ou->setCheat("No ADC 1.0/0.10 support", true, false, false);
					client->updated(ou);
					client->cheatMessage(report);
					return BREAK;
				}
			}
			Identity& i = ou->getIdentity();
			if(getCheckClients() && clientItems < RSXPP_SETTING(MAX_TESTSURS)) {
				if(ou->shouldCheckClient()) {
					if(!ou->getChecked(false, false)) {
						return ADD_CLIENT_CHECK | BREAK;
					}
				} else if(i.isClientQueued() && i.isClientChecked()) {
					return REMOVE_CLIENT_CHECK | BREAK;
				}
			}
			if(getCheckFilelists() && filelistItems < RSXPP_SETTING(MAX_FILELISTS)) {
				if(canCheckFilelist) {
					if(ou->shouldCheckFileList()) {
						if(!ou->getChecked(true, false)) {
							return ADD_FILELIST_CHECK | BREAK;
						}
					}
				}
			}
			return CONTINUE;
		}

		int run() {
			inThread = true;
			keepChecking = true;
			setThreadPriority(Thread::LOW);

			if(checkOnConnect) {
				if(checkClients && !checkFilelists)
					client->addHubLine("*** Checking started (Clients)", 3);
				else if(!checkClients && checkFilelists)
					client->addHubLine("*** Checking started (FileLists)", 3);
				else if(checkClients && checkFilelists)
					client->addHubLine("*** Checking started (Clients & FileLists)", 3);

				Thread::sleep(RSXPP_SETTING(CHECK_DELAY) + 1000);
				checkOnConnect = false;
				client->setCheckOnConnect(false);
			}

			if(!client->isConnected()) { 
				keepChecking = false; 
			}

			canCheckFilelist = !checkClients || !RSXPP_BOOLSETTING(CHECK_ALL_CLIENTS_BEFORE_FILELISTS);
			const uint64_t sleepTime = static_cast<uint64_t>(RSXPP_SETTING(SLEEP_TIME));

			while(keepChecking) {
				dcassert(client != NULL);
				if(client->isConnected()) {
					uint8_t t = 0;
					uint8_t f = 0;
					{
						Lock l(cs);
						const QueueItem::StringMap& q = QueueManager::getInstance()->lockQueue();
						for(QueueItem::StringIter i = q.begin(); i != q.end(); ++i) {
							if(i->second->countOnlineUsers() == 0)
								continue;
							if(i->second->isSet(QueueItem::FLAG_TESTSUR)) {
								t++;
							} else if(i->second->isSet(QueueItem::FLAG_CHECK_FILE_LIST)) {
								f++;
							}
						}
						QueueManager::getInstance()->unlockQueue();
					}

					OnlineUserPtr ou = NULL;
					int action = 0;
					{
						Lock l(client->cs);
						for(BaseMap::const_iterator i = users->begin(); i != users->end(); ++i) {
							if(!inThread)
								break;
							action = preformUserCheck(i->second, t, f);
							if(action & CONTINUE)
								continue;
							if(action & BREAK) {
								ou = i->second;
								break;
							}
						}
					}
					if(ou != NULL) {
						Lock l(cs);
						if(action & ADD_CLIENT_CHECK) {
							try {
								string fname = QueueManager::getInstance()->addClientCheck(HintedUser(ou->getUser(), client->getHubUrl()));
								if(!fname.empty())
									ou->getIdentity().setTestSURQueued(fname);
							} catch(...) {
								//
							}
						} else if(action & ADD_FILELIST_CHECK) {
							try {
								string fname = QueueManager::getInstance()->addFileListCheck(HintedUser(ou->getUser(), client->getHubUrl()));
								if(!fname.empty())
									ou->getIdentity().setFileListQueued(fname);
							} catch(...) {
								//
							}
						} else if(action & REMOVE_CLIENT_CHECK) {
							string path = Util::getPath(Util::PATH_USER_CONFIG) + "TestSURs\\" + ou->getIdentity().getTestSURQueued();
							if(!Util::fileExists(path)) {
								ou->getIdentity().setTestSURQueued(Util::emptyString);
							}
						}
					}

					if(!canCheckFilelist) {
						canCheckFilelist = !(action & BREAK);
					}
					sleep(sleepTime);
				}
			}
			inThread = false;
			return 0;
		}
		GETSET(bool, keepChecking, KeepChecking);
		GETSET(bool, checkOnConnect, CheckOnConnect);
		GETSET(bool, checkFilelists, CheckFilelists);
		GETSET(bool, checkClients, CheckClients);
		bool canCheckFilelist;
		bool inThread;

		Client* client;
		HubUsersMap* users;
	}*clientEngine;
};

} // namespace dcpp
#endif // RSXPLUSPLUS_HUB_USERS_MAP

/**
 * @file
 * $Id$
 */
