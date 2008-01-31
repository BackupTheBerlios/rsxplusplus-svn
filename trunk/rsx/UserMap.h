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

#ifndef USER_MAP_H
#define USER_MAP_H

#include "../client/QueueManager.h"
#include "../client/Client.h"
#include "../client/Thread.h"
#include "../client/FastAlloc.h"

template<bool isADC, typename BaseMap>
class UserMap : public BaseMap {
public:
	UserMap() : clientEngine(NULL), myInfoEngine(NULL) { };
	~UserMap() {
		stopMyINFOCheck();
		stopCheck();
		try {
			QueueManager::getInstance()->removeOfflineChecks();
		} catch(...) {
			//...
		}
	};

	void startMyINFOCheck(Client* c, bool fs, bool myinfo) { 
		if(myInfoEngine == NULL && (fs || myinfo)) {
			myInfoEngine = new ThreadedMyINFOCheck(this, c);
			myInfoEngine->startCheck(fs, myinfo);
		}
	}

	void startCheck(Client* c, bool cc, bool cf, bool cOnConnect = false) {
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
	}

	void stopMyINFOCheck() {
		if(myInfoEngine != NULL) {
			delete myInfoEngine;
			myInfoEngine = NULL;
		}
	}


	void stopCheck() {
		if(clientEngine != NULL) {
			delete clientEngine;
			clientEngine = NULL;
		}
	}

	bool isDetectorRunning() { 
		return (clientEngine != NULL && clientEngine->isChecking());
	}

private:

	//myinfo check engine
	class ThreadedMyINFOCheck : public Thread, public FastAlloc<ThreadedMyINFOCheck> {
	public:
		ThreadedMyINFOCheck(UserMap* _u, Client* _c) : client(_c), users(_u), 
			inThread(false), checkFakeShare(false), checkMyInfo(false) { };
		~ThreadedMyINFOCheck() { cancel(); }

		bool isChecking() { 
			return inThread; 
		}

		void cancel() { 
			inThread = false;
			// called always after delete operator
			users->myInfoEngine = NULL;
			join();
		}

		void startCheck(bool cfs, bool myInfo) {
			if(!client || !users) {
				return;
			}
			checkFakeShare = cfs;
			checkMyInfo = myInfo;
			if(!inThread)
				start();
		}
	private:
		int run() {
			inThread = true;
			setThreadPriority(Thread::HIGH);
			if(client && client->isConnected() && !client->getCheckedAtConnect()) {
				Lock l(client->cs);
				for(BaseMap::const_iterator i = users->begin(); i != users->end(); i++) {
					OnlineUser* ou = i->second;
					if(!inThread || !ou || !(client && client->isConnected())) 
						break;
					ou->inc();
					if(ou->isCheckable(false)) {
						if(checkMyInfo) {
							ou->getIdentity().myInfoDetect(*ou);
						}
						if(checkFakeShare) {
							ou->getIdentity().isFakeShare(*ou);
						}
						sleep(1);
					}
					ou->dec();
				}
				client->setCheckedAtConnect(true);
			}
			inThread = false;
			// make some cleanup
			delete this;
			return 0;
		}

		bool inThread;
		bool checkFakeShare, checkMyInfo;
		Client* client;
		UserMap* users;
	}*myInfoEngine;

	//clients check engine
	class ThreadedCheck : public Thread, public FastAlloc<ThreadedCheck> {
	public:
		ThreadedCheck(UserMap* _u, Client* _c) : client(_c), users(_u), 
			keepChecking(false), canCheckFilelist(false), inThread(false), checkOnConnect(false) { };
		~ThreadedCheck() { cancel(); }

		bool isChecking() { 
			return inThread && keepChecking; 
		}

		void cancel() { 
			keepChecking = inThread = false;
			join();
			client = NULL;

			try {
				QueueManager::getInstance()->removeOfflineChecks();
			} catch(...) {
				//...
			}
		}

		void startCheck() {
			if(!client || !users) {
				return;
			}
			if(!inThread) {
				start();
			}
		}

	private:
		int run() {
			inThread = true;
			setThreadPriority(Thread::LOW);
			if(checkOnConnect && !keepChecking) { 
				sleep(RSXSETTING(CHECK_DELAY));
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
			uint8_t	secs = 0;

			while(keepChecking) {
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

					if(t < RSXSETTING(MAX_TESTSURS)) {
						iterBreak = false;
						Lock l(client->cs);

						for(BaseMap::const_iterator i = users->begin(); i != users->end(); ++i) {
							OnlineUser* ou = i->second;
							if(!ou || !inThread) { break; }
							ou->inc();
							iterBreak = false;
							if(ou->isCheckable()) {
								if(isADC) {
									if((ou->getUser()->isSet(User::NO_ADC_1_0_PROTOCOL) || ou->getUser()->isSet(User::NO_ADC_0_10_PROTOCOL)) && ou->shouldTestSUR()) {
										//nasty...
										ou->setTestSURComplete();
										ou->setFileListComplete();
										ou->getIdentity().setCheatMsg(ou->getClient(), "No ADC 1.0/0.10 support", true, false, false);
										ou->updateUser();
										ou->dec();
										//prevent spam but don't break, it'd be a time loss
										sleep(5);
										continue;
									}
								}

								if(getCheckClients()) {
									if(ou->shouldTestSUR()) {
										if(!ou->getChecked()) {
											try {
												dcdebug("Adding TestSUR to queue %s\n", ou->getIdentity().getNick());
												QueueManager::getInstance()->addTestSUR(ou->getUser());
												ou->getIdentity().setTestSURQueued("1");
											} catch(...) {
												dcdebug("Exception adding testsur %s\n", ou->getIdentity().getNick());
											}
											iterBreak = true;
										}
									} else if(ou->getIdentity().isTestSURQueued()) {
										try {
											if(!QueueManager::getInstance()->isTestSURinQueue(ou->getUser())) {
												ou->getIdentity().setTestSURQueued(Util::emptyString);
												iterBreak = true;
											}
										} catch(...) {
											dcdebug("Exception removing testsur %s\n", ou->getIdentity().getNick());
										}
									}
									if(iterBreak) {
										ou->dec();
										break;
									}
								}
								if(getCheckFilelists()) {
									if(canCheckFilelist && f < RSXSETTING(MAX_FILELISTS)) {
										if(ou->shouldCheckFileList(!getCheckClients())) {
											if(!ou->getChecked(true)) {
												try {
													dcdebug("Adding FileList check to queue %s\n", ou->getIdentity().getNick());
													QueueManager::getInstance()->addList(ou->getUser(), QueueItem::FLAG_CHECK_FILE_LIST);
													ou->getIdentity().setFileListQueued("1");
												} catch(...) {
													dcdebug("Exception adding filelist %s\n", ou->getIdentity().getNick());
												}
												ou->dec();
												break;
											}
										}
									}
								}
							}
							ou->dec();
						}
					}
					if(!canCheckFilelist) {
						canCheckFilelist = !iterBreak;
					}

					if(secs >= 30) {
						try {
							QueueManager::getInstance()->removeOfflineChecks();
							secs = 0;
						} catch(...) {
							// oh well, try again in 10 secs.
							secs = 20;
						}
					} else {
						secs++;
					}
					sleep(sleepTime);
				} else {
					sleep(1000);
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
		UserMap* users;
	}*clientEngine;
};
#endif