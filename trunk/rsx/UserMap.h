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
	UserMap() : client(NULL) { };
	~UserMap() { 
		stopMyINFOCheck();
		stopCheck();
		QueueManager::getInstance()->removeOfflineChecks();
		client = NULL;
	};

	void setClient(Client* cl) { 
		client = cl; 
	}

	//myinfo start/stop functions
	void startMyINFOCheck(bool fs, bool myinfo) { 
		if(!myInfoEngine.isChecking() && (fs || myinfo)) {
			myInfoEngine.startCheck(this, fs, myinfo);
		}

	}
	void stopMyINFOCheck() { 
		myInfoEngine.cancel();
	}

	//clients check start/stop functions
	void startCheck(bool cc, bool cf, bool cOnConnect = false) {
		if(!clientEngine.isChecking()) {
			clientEngine.setCheckClients(cc);
			clientEngine.setCheckFilelists(cf);
			if(cOnConnect)
				clientEngine.setCheckOnConnect(true);
			else
				clientEngine.setKeepChecking(true);
			clientEngine.startCheck(this);
		}
	}

	void stopCheck() { 
		clientEngine.cancel(); 
	}

	bool isDetectorRunning() { 
		return clientEngine.isChecking();
	}

private:
	Client* client;
	Client* getClient() { return client; }

	/*
	 * probably reference would be more safe than pointers
	 * specially when I don't care if pointer is destroyed with delete operator
	 * client (nmdchub/adchub) class do it for me :)
	 */

	//myinfo check engine
	class ThreadedMyINFOCheck : public Thread {
	public:
		ThreadedMyINFOCheck() : client(NULL), users(NULL), inThread(false), checkFakeShare(false), checkMyInfo(false) { };
		~ThreadedMyINFOCheck() { cancel(); }

		bool isChecking() { 
			return inThread; 
		}

		void cancel() { 
			inThread = false;
			join();
		}

		void startCheck(UserMap* map, bool cfs, bool myInfo) {
			users = map;
			client = map->getClient();
			if(!client) {
				users = NULL;
				return;
			}
			checkFakeShare = cfs;
			checkMyInfo = myInfo;
			if(!inThread)
				start();
		}

		int run() {
			inThread = true;
			setThreadPriority(Thread::LOW);
			//Thread::sleep(100);
			client->setCheckedAtConnect(true);
			for(BaseMap::iterator i = users->begin(); i != users->end(); ++i) {
				if(!inThread || !(client && client->isConnected())) 
					break;
				try {
					if(!i->second->isHidden()) {
						i->second->inc();
						if(checkMyInfo) {
							i->second->getIdentity().myInfoDetect((*i->second));
						}
						if(checkFakeShare) {
							i->second->getIdentity().isFakeShare((*i->second));
						}
						i->second->dec();
						Thread::sleep(1);
					}
				} catch(...) {
					//...
				}
			}
			inThread = false;
			return 0;
		}
	private:
		bool inThread;
		bool checkFakeShare, checkMyInfo;
		Client* client;
		UserMap* users;
	}myInfoEngine;

	//clients check engine
	class ThreadedCheck : public Thread {
	public:
		ThreadedCheck() : client(NULL), users(NULL), keepChecking(false), canCheckFilelist(false), 
			inThread(false), checkOnConnect(false) { };
		~ThreadedCheck() { cancel(); }

		bool isChecking() { 
			return inThread && keepChecking; 
		}

		void cancel() { 
			keepChecking = inThread = false;
			join();
		}

		void startCheck(UserMap* um) {
			users = um;
			client = users->getClient();
			if(!client) {
				users = NULL;
				return;
			}
			inThread = true; 
			start(); 
		}

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
			uint8_t	secs = 0;

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

						for(BaseMap::iterator i = users->begin(); i != users->end(); ++i) {
							if(!inThread)
								break;
							i->second->inc();
							ou = i->second;
							iterBreak = false;
							if(!ou->isHidden()) {
								if(checkClients && ou->isCheckable()) {
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
								if(checkFilelists) {
									if(canCheckFilelist && f < RSXSETTING(MAX_FILELISTS)) {
										if(ou->shouldCheckFileList(!checkClients) && !ou->getChecked(true)) {
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
							ou->dec();
						}
						ou = NULL;
						if(!canCheckFilelist) {
							canCheckFilelist = !iterBreak;
						}
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
			inThread = keepChecking = false;
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
	}clientEngine;
};
#endif