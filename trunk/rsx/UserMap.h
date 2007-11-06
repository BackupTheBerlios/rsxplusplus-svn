/*
 * Copyright (C) 2007 adrian_007, adrian-007 on o2 point pl
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

template<bool isADC, typename BaseMap>
class UserMap : public BaseMap {
public:
	UserMap() : clientEngine(NULL), myInfoEngine(NULL) { };
	virtual ~UserMap() throw() { 
		uinitMyINFOCheck();
		uinitClientCheck();
		QueueManager::getInstance()->removeOfflineChecks();
	};

	void initMyINFOCheck(Client* client) {
		if(myInfoEngine == NULL) {
			myInfoEngine = new ThreadedMyINFOCheck(client, this);
		}
	}

	void initClientCheck(Client* client) {
		if(clientEngine == NULL) {
			clientEngine = new ThreadedCheck(client, this);
		}
	}

	void uinitClientCheck() {
		stopCheck();
		if(clientEngine != NULL) {
			delete clientEngine;
			clientEngine = NULL;
		}
	}

	void uinitMyINFOCheck() {
		stopMyINFOCheck();
		if(myInfoEngine != NULL) {
			delete myInfoEngine;
			myInfoEngine = NULL;
		}
	}

	//myinfo start/stop functions
	void startMyINFOCheck(bool fs, bool myinfo) { 
		try {
			if(myInfoEngine && !myInfoEngine->isChecking() && (fs || myinfo)) {
				myInfoEngine->startCheck(fs, myinfo);
			}
		} catch(/*const ThreadException& e*/...) {
			//
		}
	}
	void stopMyINFOCheck() { if(myInfoEngine) myInfoEngine->cancel(); }

	//clients check start/stop functions
	void startCheck(bool cc, bool cf) {
		if(clientEngine && !clientEngine->isChecking()) {
			clientEngine->setCheckClients(cc);
			clientEngine->setCheckFilelists(cf);
			clientEngine->setKeepChecking(true);
			clientEngine->startCheck();
		}
	}

	void startCustomCheck(bool checkClient, bool checkFileList) {
		if(clientEngine && !clientEngine->isChecking()) {
			clientEngine->setCheckClients(checkClient);
			clientEngine->setCheckFilelists(checkFileList);
			clientEngine->setKeepChecking(true);
			clientEngine->startCheck();
		}
	}

	void stopCheck() { if(clientEngine) clientEngine->cancel(); }

	bool isDetectorRunning() { return clientEngine && clientEngine->isChecking(); }

	void startCheckOnConnect(bool cc, bool cf) {
		if(clientEngine && !clientEngine->isChecking()) {
			clientEngine->setCheckClients(cc);
			clientEngine->setCheckFilelists(cf);
			clientEngine->setCheckOnConnect(true);
			clientEngine->startCheck();
		}
	}

private:
	CriticalSection cs;

	//myinfo check engine
	class ThreadedMyINFOCheck : public Thread {
	public:
		ThreadedMyINFOCheck(Client* cl, UserMap* um) : client(cl), users(um), inThread(false), checkFakeShare(false), checkMyInfo(false) { };
		~ThreadedMyINFOCheck() throw() { cancel(); join(); }
		bool isChecking() { 
			return inThread; 
		}
		void cancel() { 
			inThread = false;
		}

		void startCheck(bool cfs, bool myInfo) {
			checkFakeShare = cfs;
			checkMyInfo = myInfo;
			if(!inThread)
				start();
		}

		int run() throw() {
			inThread = true;
			setThreadPriority(Thread::LOW);
			//Thread::sleep(100);
			//Lock l(client->cs);
			if(client && client->isConnected()) {
				for(BaseMap::const_iterator i = users->begin(); i != users->end(); ++i) {
					if(!inThread) 
						break;
					i->second->inc();
					Lock l(i->second->getClient().cs);
					if(client && client->isConnected() && !i->second->isHidden()) {
						try {
							if(checkMyInfo) {
								i->second->getIdentity().myInfoDetect((*i->second));
							}
							if(checkFakeShare) {
								i->second->getIdentity().isFakeShare((*i->second));
							}
						} catch(...) {
							//...
						}
					}
					i->second->dec();
					Thread::sleep(1);
				}
				client->setCheckedAtConnect(true);
			}
			inThread = false;
			return 0;
		}
	private:
		bool inThread;
		bool checkFakeShare, checkMyInfo;
		Client* client;
		UserMap* users;
	} *myInfoEngine;

	//clients check engine
	class ThreadedCheck : public Thread {
	public:
		ThreadedCheck(Client* cl, UserMap* um) : client(cl), users(um), keepChecking(false), canCheckFilelist(false), inThread(false), checkOnConnect(false) { };
		~ThreadedCheck() throw() { cancel(); join(); }

		bool isChecking() { 
			return inThread && keepChecking; 
		}

		void cancel() { 
			keepChecking = inThread = false; 
		}

		void startCheck() { start(); }

		int run() {
			OnlineUser* ou = NULL;
			setThreadPriority(Thread::LOW);

			if(checkOnConnect && !keepChecking) { 
				sleep(RSXSETTING(CHECK_DELAY));
				keepChecking = true;
				checkOnConnect = false;
				client->setCheckOnConnect(false);
			}
			canCheckFilelist = !checkClients || !RSXBOOLSETTING(CHECK_ALL_CLIENTS_BEFORE_FILELISTS);

			bool iterBreak;
			const uint64_t	sleepTime =	static_cast<uint64_t>(RSXSETTING(SLEEP_TIME));
			uint8_t			secs =		0;

			inThread = true;
			if((client && !client->isConnected()) || (!checkClients && !checkFilelists)) { 
				keepChecking = false; 
			}

			while(keepChecking) {
				if(client && client->isConnected()) {
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
						Lock l(client->cs);
						iterBreak = false;

						for(BaseMap::const_iterator i = users->begin(); i != users->end(); ++i) {
							ou = i->second;
							if(!inThread || !ou) break;
							ou->inc();
							iterBreak = false;
							if(!ou->isHidden()) {
								if(checkClients) {
									if(ou->isCheckable()) {
										if(ou->shouldTestSUR()) {
											if(!ou->getChecked()) {
												iterBreak = true;
												try {
													if(isADC && RsxUtil::checkVersion(ou->getIdentity().getTag(), true)) {
														ou->setTestSURComplete();
														ou->setFileListComplete();
														ou->dec();
														break;
													}
													QueueManager::getInstance()->addTestSUR(ou->getUser());
													ou->getIdentity().setTestSURQueued("1");
													ou->dec();
													break;
												} catch(...) {
													//...
												}
											}
										} else if(!ou->getIdentity().getTestSURQueued().empty()) {
											try {
												if(!QueueManager::getInstance()->isTestSURinQueue(ou->getUser())) {
													iterBreak = true;
													ou->getIdentity().setTestSURQueued(Util::emptyString);
													ou->dec();
													break;
												}
											} catch(...) {
												//...
											}
										}
									}
								}
								if(checkFilelists) {
									if(canCheckFilelist && f < RSXSETTING(MAX_FILELISTS)) {
										if(ou->shouldCheckFileList(!checkClients)) {
											if(!ou->getChecked(true)) {
												try {
													if(isADC && RsxUtil::checkVersion(ou->getIdentity().getTag(), true)) {
														ou->setTestSURComplete();
														ou->setFileListComplete();
														ou->dec();
														break;
													}
													QueueManager::getInstance()->addList(ou->getUser(), QueueItem::FLAG_CHECK_FILE_LIST);
													ou->getIdentity().setFileListQueued("1");
													ou->dec();
													break;
												} catch(...) {
													//...
												}
											}
										}
									}
								}
							}
							ou->dec();
						}
						if(!canCheckFilelist)
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
				}
			}
			inThread = false;
			return 0;
		}
	private:
		GETSET(bool, keepChecking, KeepChecking);
		GETSET(bool, checkOnConnect, CheckOnConnect);
		GETSET(bool, checkFilelists, CheckFilelists);
		GETSET(bool, checkClients, CheckClients);
		bool canCheckFilelist;
		bool inThread;

		Client* client;
		UserMap* users;
	} *clientEngine;
};
#endif