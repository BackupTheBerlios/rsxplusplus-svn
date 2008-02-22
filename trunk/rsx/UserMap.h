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
	UserMap() : clientEngine(NULL) { };
	~UserMap() {
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
		if(myInfoEngine.isRunning()) {
			myInfoEngine.cancel();
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
		ThreadedMyINFOCheck() : client(NULL), inThread(false) { };
		~ThreadedMyINFOCheck() { cancel(); }

		void cancel() { 
			inThread = false;
			join();
		}

		bool isRunning() { return inThread; }

		void startCheck(Client* _c) {
			client = _c;
			if(!client) {
				return;
			}
			if(!inThread) {
				start();
			}
		}

	private:
		int run() {
			dcassert(client != NULL);
			inThread = true;
			setThreadPriority(Thread::HIGH);
			if(client && client->isConnected() && !client->getCheckedAtConnect()) {
				//get users, release lock and then check them
				OnlineUser::List ul;
				{
					Lock l(client->cs);
					client->getUserList(ul);
				}
				for(OnlineUser::List::const_iterator i = ul.begin(); i != ul.end(); ++i) {
					if((*i)->isCheckable(false)) {
						(*i)->getIdentity().myInfoDetect(*(*i));
					}
					(*i)->dec();
					sleep(1);
				}
				client->setCheckedAtConnect(true);
			}
			return 0;
		}
		bool inThread;
		Client* client;
	}myInfoEngine;

	//clients check engine
	class ThreadedCheck : public Thread, public FastAlloc<ThreadedCheck> {
	public:
		ThreadedCheck(UserMap* _u, Client* _c) : client(_c), users(_u), 
			keepChecking(false), canCheckFilelist(false), inThread(false), checkOnConnect(false) { };
		~ThreadedCheck() {
			keepChecking = inThread = false;
			join();
			client = NULL;
		}

		bool isChecking() { 
			return inThread && keepChecking; 
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
			uint8_t secs = 0;
			OnlineUser* ou = NULL;

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
						dcassert(client != NULL);
						iterBreak = false;
						Lock l(client->cs);

						for(BaseMap::const_iterator i = users->begin(); i != users->end(); i++) {
							i->second->inc();
							ou = i->second;
							if(!ou || !inThread) { 
								i->second->dec();							
								break; 
							}
//							iterBreak = false;

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
											iterBreak = true;
											try {
												//dcdebug("ThreadedCheck: Adding TestSUR to queue %s\n", ou->getIdentity().getNick().c_str());
												QueueManager::getInstance()->addTestSUR(ou->getUser(), false);
												ou->getIdentity().setTestSURQueued("1");
											} catch(...) {
												//dcdebug("ThreadedCheck: Exception adding testsur %s\n", ou->getIdentity().getNick().c_str());
											}
											ou->dec();
											break;
										}
										//@todo find a reason of clearing TQ field while checks isnt't done
									} else if(ou->getIdentity().isTestSURQueued() && ou->getIdentity().isClientChecked()) {
										try {
											if(!QueueManager::getInstance()->isTestSURinQueue(ou->getUser())) {
												iterBreak = true;
												ou->getIdentity().setTestSURQueued(Util::emptyString);
											}
										} catch(...) {
											//dcdebug("ThreadedCheck: Exception removing testsur %s\n", ou->getIdentity().getNick().c_str());
										}
										ou->dec();
										break;
									}
								}
								if(getCheckFilelists()) {
									if(canCheckFilelist && f < RSXSETTING(MAX_FILELISTS)) {
										if(ou->shouldCheckFileList(!getCheckClients())) {
											if(!ou->getChecked(true)) {
												try {
													//dcdebug("Adding FileList check to queue %s\n", ou->getIdentity().getNick());
													QueueManager::getInstance()->addList(ou->getUser(), QueueItem::FLAG_CHECK_FILE_LIST);
													ou->getIdentity().setFileListQueued("1");
												} catch(...) {
													//dcdebug("ThreadedCheck: Exception adding filelist %s\n", ou->getIdentity().getNick().c_str());
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