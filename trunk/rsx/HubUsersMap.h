/*
 * Copyright (C) 2007-2008 adrian_007, adrian-007 on o2 point pl
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

#ifndef HUB_USERS_MAP_H
#define HUB_USERS_MAP_H

#include "../client/QueueManager.h"
#include "../client/Client.h"
#include "../client/Thread.h"
#include "../client/FastAlloc.h"

namespace dcpp {

template<bool isADC, typename BaseMap>
class HubUsersMap : public BaseMap, private boost::noncopyable {
public:
	HubUsersMap() : clientEngine(NULL) { };
	~HubUsersMap() {
		//stopMyINFOCheck();
		//stopCheck();
		QueueManager::getInstance()->removeOfflineChecks();
	};

	void startMyINFOCheck(Client* c) { 
		if(!myInfoEngine.isRunning()) {
			myInfoEngine.startCheck(c);
		}
	}

	void startCheck(Client* c, bool cc, bool cf, bool cOnConnect = false) {
		try {
			if(clientEngine == NULL) {
				clientEngine = new ThreadedCheck(this, c);
				clientEngine->setCheckClients(cc);
				clientEngine->setCheckFilelists(cf);
				if(cOnConnect) {
					clientEngine->setCheckOnConnect(true);
				} else {
					clientEngine->setKeepChecking(true);
				}
				clientEngine->startCheck();
			}
		} catch(...) { }
	}

	void stopMyINFOCheck() {
		if(myInfoEngine.isRunning()) {
			myInfoEngine.cancel();
		}
	}

	void stopCheck() {
		try {
			if(clientEngine != NULL) {
				delete clientEngine;
				clientEngine = NULL;
			}
		} catch(...) { }
	}

	bool isDetectorRunning() { 
		return (clientEngine != NULL && clientEngine->isChecking());
	}

private:

	//myinfo check engine
	class ThreadedMyINFOCheck : public Thread, public FastAlloc<ThreadedMyINFOCheck> {
	public:
		ThreadedMyINFOCheck() : client(NULL), stop(true) { };
		~ThreadedMyINFOCheck() { cancel(); }

		void cancel() { 
			stop = true;
			//join();
		}

		bool isRunning() const { return !stop; }

		void startCheck(Client* _c) {
			if(_c && stop) {
				client = _c;
				start();
			}
		}

	private:
		CriticalSection cs;
		int run() {
			stop = false;
			setThreadPriority(Thread::HIGH);
			if(client && client->isConnected()) {
				client->setCheckedAtConnect(true);
				// get users, release lock and then check them
				OnlineUser::List ul;
				{
					Lock l(client->cs);
					client->getUserList(ul);
				}
				Lock l(cs);
				for(OnlineUser::List::const_iterator i = ul.begin(); i != ul.end(); ++i) {
					if(stop) {
						// don't let to not execute it
						(*i)->dec();
						continue;
					}
					OnlineUser* ou = *i;
					if(ou->isCheckable()) {
						string report = ou->getIdentity().myInfoDetect(*ou);
						if(!report.empty()) {
							ou->getClient().cheatMessage(report);
							ou->getClient().updated(*ou);
						}
					}
					ou->dec();
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
	class ThreadedCheck : public Thread, public FastAlloc<ThreadedCheck> {
	public:
		ThreadedCheck(HubUsersMap* _u, Client* _c) : client(_c), users(_u), 
			keepChecking(false), canCheckFilelist(false), inThread(false), checkOnConnect(false) { };
		~ThreadedCheck() {
			keepChecking = inThread = false;
			client = NULL;
			join();
		}

		inline bool isChecking() { 
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

	private:
		inline bool preformUserCheck(OnlineUser* ou, const uint8_t clientItems, const uint8_t filelistItems) {
			if(!ou->isCheckable((uint16_t)RSXSETTING(CHECK_DELAY)))
				return false;
			if(isADC) {
				if((ou->getUser()->isSet(User::NO_ADC_1_0_PROTOCOL) || ou->getUser()->isSet(User::NO_ADC_0_10_PROTOCOL)) && ou->shouldCheckClient()) {
					//nasty...
					ou->setTestSURComplete();
					ou->setFileListComplete();
					string report = ou->setCheat("No ADC 1.0/0.10 support", true, false, false);
					client->updated(*ou);
					client->cheatMessage(report);
					sleep(5);
					return false;
				}
			}
			Identity& i = ou->getIdentity();
			if(getCheckClients() && clientItems < RSXSETTING(MAX_TESTSURS)) {
				if(ou->shouldCheckClient()) {
					if(!ou->getChecked(false, false)) {
						try {
							QueueManager::getInstance()->addTestSUR(ou->getUser(), false);
							i.setTestSURQueued("1");
							return true;
						} catch(const QueueException& eq) {
							dcdebug("ThreadedCheck::QueueException: Exception adding client check %s\n", eq.getError().c_str());
							return false;
						} catch(const FileException& ef) {
							dcdebug("ThreadedCheck::FileException: Exception adding client check %s\n", ef.getError().c_str());
							return false;
						}
					}
				} else if(i.isClientQueued()) {
					if(!QueueManager::getInstance()->isTestSURinQueue(ou->getUser())) {
						i.setTestSURQueued(Util::emptyString);
						// process
						return false;
					}
				}
			}
			if(getCheckFilelists() && filelistItems < RSXSETTING(MAX_FILELISTS)) {
				if(canCheckFilelist) {
					if(ou->shouldCheckFileList()) {
						if(!ou->getChecked(true, false)) {
							try {
								QueueManager::getInstance()->addFileListCheck(ou->getUser());
								i.setFileListQueued("1");
								return true;
							} catch(const QueueException& eq) {
								dcdebug("ThreadedCheck::QueueException: Exception adding filelist %s\n", eq.getError().c_str());
								return false;
							} catch(const FileException& ef) {
								dcdebug("ThreadedCheck::FileException: Exception adding filelist %s\n", ef.getError().c_str());
								return false;
							}
						}
					}
				}
			}
			return false;
		}

		int run() {
			inThread = true;
			setThreadPriority(Thread::LOW);
			//while(users->myInfoEngine.isRunning())
			//	sleep(10);
			if(checkOnConnect && !keepChecking) { 
				Thread::sleep(RSXSETTING(CHECK_DELAY));
				keepChecking = true;
				checkOnConnect = false;
				client->setCheckOnConnect(false);
			}

			if(!client->isConnected() || (!checkClients && !checkFilelists)) { 
				keepChecking = false; 
			}

			canCheckFilelist = !checkClients || !RSXBOOLSETTING(CHECK_ALL_CLIENTS_BEFORE_FILELISTS);
			bool iterBreak = false;
			const uint64_t	sleepTime =	static_cast<uint64_t>(RSXSETTING(SLEEP_TIME));

			while(keepChecking) {
				dcassert(client != NULL);
				if(client->isConnected()) {
					uint8_t t = 0;
					uint8_t f = 0;
					{
						const QueueItem::StringMap& q = QueueManager::getInstance()->lockQueue();
						for(QueueItem::StringIter i = q.begin(); i != q.end(); ++i) {
							if(i->second->isSet(QueueItem::FLAG_TESTSUR)) {
								t++;
							} else if(i->second->isSet(QueueItem::FLAG_CHECK_FILE_LIST)) {
								f++;
							}
						}
						QueueManager::getInstance()->unlockQueue();
					}

					{
						Lock l(client->cs);
						iterBreak = false;
						for(BaseMap::const_iterator i = users->begin(); i != users->end(); i++) {
							if(!inThread) {
								break;
							}
							i->second->inc();
							iterBreak = preformUserCheck(i->second, t, f);
							i->second->dec();
							if(iterBreak) {
								break;
							}
						}
					}
					if(!canCheckFilelist) {
						canCheckFilelist = !iterBreak;
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
#endif

/**
 * @file
 * $Id$
 */
