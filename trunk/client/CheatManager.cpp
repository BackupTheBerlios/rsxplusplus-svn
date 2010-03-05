/**
 * This file is a part of client manager.
 * It has been divided but shouldn't be used anywhere else.
 */

void ClientManager::sendRawCommand(const UserPtr& user, const string& aRaw, bool checkProtection/* = false*/) {
	if(!aRaw.empty()) {
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&user->getCID()));
		if(i == onlineUsers.end()) return;
		OnlineUser* ou = i->second;

		if(!checkProtection || !i->second->isProtectedUser()) {
			StringMap ucParams;
			UserCommand uc = UserCommand(0, 0, 0, 0, "", aRaw, "", "");
			userCommand(HintedUser(user, ou->getClientBase().getHubUrl()), uc, ucParams, true);
			RSXPP_SET(TOTAL_RAW_COMMANDS_SENT, RSXPP_SETTING(TOTAL_RAW_COMMANDS_SENT)+1); //important stuff ;p
			if(SETTING(LOG_RAW_CMD)) {
				LOG(LogManager::RAW, ucParams);
			}
		}
	}
}

void ClientManager::setListLength(const UserPtr& p, const string& listLen) {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
	if(i != onlineUsers.end()) {
		i->second->getIdentity().set("LL", listLen);
	}
}

void ClientManager::fileListDisconnected(const UserPtr& p) {
	if(RSXPP_SETTING(MAX_DISCONNECTS) == 0)
		return;

	bool remove = false;
	string report = Util::emptyString;
	Client* c = NULL;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
		if(i == onlineUsers.end()) return;
		OnlineUser* ou = i->second;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

		int fileListDisconnects = Util::toInt(ou->getIdentity().get("FD")) + 1;
		ou->getIdentity().set("FD", Util::toString(fileListDisconnects));

		if(fileListDisconnects == RSXPP_SETTING(MAX_DISCONNECTS)) {
			c = &ou->getClient();
			report = ou->setCheat("Disconnected file list %[userFD] times", false, true, RSXPP_BOOLSETTING(SHOW_DISCONNECT_RAW));
			if(ou->getIdentity().isFileListQueued()) {
				ou->setFileListComplete();
				ou->getIdentity().setFileListQueued(Util::emptyString);
				remove = true;
			}
			sendAction(*ou, RSXPP_SETTING(DISCONNECT_RAW));
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
	if(RSXPP_SETTING(MAX_TIMEOUTS) == 0)
		return;

	uint8_t remove = 0;
	string report = Util::emptyString;
	Client* c = NULL;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
		if(i == onlineUsers.end()) return;
		OnlineUser* ou = i->second;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

		int connectionTimeouts = Util::toInt(ou->getIdentity().get("TO")) + 1;
		ou->getIdentity().set("TO", Util::toString(connectionTimeouts));
	
		if(connectionTimeouts == RSXPP_SETTING(MAX_TIMEOUTS)) {
			c = &ou->getClient();
			report = ou->setCheat("Connection timeout %[userTO] times", false, false, RSXPP_BOOLSETTING(SHOW_TIMEOUT_RAW));
				
			if(ou->getIdentity().isClientQueued()) {
				ou->setTestSURComplete();
				ou->getIdentity().setTestSURQueued(Util::emptyString);
				remove += 1;
			}
			if(ou->getIdentity().isFileListQueued()) {
				ou->setFileListComplete();
				ou->getIdentity().setFileListQueued(Util::emptyString);
				remove += 2;
			}
			sendAction(*ou, RSXPP_SETTING(TIMEOUT_RAW));
		}
	}
	if(remove > 0) {
		string hubHint = Util::emptyString;
		if(c)
			hubHint = c->getHubUrl();
		try {
			if(remove == 1 || remove == 3) {
				QueueManager::getInstance()->removeTestSUR(HintedUser(p, hubHint));
			}
			if(remove == 2 || remove == 3) {
				QueueManager::getInstance()->removeFileListCheck(HintedUser(p, hubHint));
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
	OnlineUser* ou = 0;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
		if(i == onlineUsers.end()) return;
		ou = i->second;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

		int64_t statedSize = ou->getIdentity().getBytesShared();
		int64_t realSize = dl->getTotalSize();

		double multiplier = ((100+(double)RSXPP_SETTING(PERCENT_FAKE_SHARE_TOLERATED))/100); 
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
			report = ou->setCheat(cheatStr, false, true, RSXPP_BOOLSETTING(SHOW_FAKESHARE_RAW));
			sendAction(*ou, RSXPP_SETTING(FAKESHARE_RAW));
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
					bool show = false;
					int rawToSend = 0;
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

void ClientManager::setCheating(const UserPtr& p, const string& _ccResponse, const string& _cheatString, int _actionId, bool _displayCheat,
		bool _badClient, bool _badFileList, bool _clientCheckComplete, bool _fileListCheckComplete) {
	OnlineUser* ou = 0;
	string report = Util::emptyString;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
		if(i == onlineUsers.end()) return;
		
		ou = i->second;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

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

void ClientManager::setPkLock(const UserPtr& p, const string& aPk, const string& aLock) {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
	if(i == onlineUsers.end()) return;
	
	i->second->getIdentity().set("PK", aPk);
	i->second->getIdentity().set("LO", aLock);
}

void ClientManager::setSupports(const UserPtr& p, const string& aSupports) {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
	if(i == onlineUsers.end()) return;
	
	i->second->getIdentity().set("SU", aSupports);
}

void ClientManager::setGenerator(const UserPtr& p, const string& aGenerator, const string& aCID, const string& aBase) {
	Client* c = 0;
	string report = Util::emptyString;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
		if(i == onlineUsers.end()) return;
		OnlineUser* ou = i->second;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

		ou->getIdentity().set("GE", aGenerator);
		ou->getIdentity().set("FI", aCID);
		ou->getIdentity().set("FB", aBase);
		report = ou->getIdentity().checkFilelistGenerator(*ou);
		c = &ou->getClient();
	}
	if(c && !report.empty()) {
		c->cheatMessage(report);
	}
}

void ClientManager::setUnknownCommand(const UserPtr& p, const string& aUnknownCommand) {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
	if(i == onlineUsers.end()) return;
	i->second->getIdentity().set("UC", aUnknownCommand);
}

void ClientManager::reportUser(const HintedUser& user) {
	bool priv = FavoriteManager::getInstance()->isPrivate(user.hint);
	string nick; string report;

	{
		Lock l(cs);
		OnlineUser* ou = findOnlineUser(user.user->getCID(), user.hint, priv);
		if(!ou) return;

		ou->getClient().reportUser(ou->getIdentity());
	}
}
//RSX++ //autosearch stuff
void ClientManager::kickFromAutosearch(const UserPtr& p, int action, const string& cheat, const string& file, const string& size, const string& tth, bool display/* = false*/) {
	OnlineUser* ou = NULL;
	string report = Util::emptyString;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
		if(i == onlineUsers.end()) return;
		ou = i->second;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

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
	OnlineUser* ou = 0;
	bool addCheck = false;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
		if(i == onlineUsers.end()) return;
		ou = i->second;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

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
				QueueManager::getInstance()->addFileListCheck(HintedUser(p, ou->getClient().getHubUrl()));
				ou->getIdentity().setFileListQueued("1");
			} else {
				QueueManager::getInstance()->addTestSUR(HintedUser(p, ou->getClient().getHubUrl()), false);
				ou->getIdentity().setTestSURQueued("1");
			}
		} catch(...) {
			//...
		}
		ou->dec();
	}
}

void ClientManager::setListSize(const UserPtr& p, int64_t aFileLength, bool adc) {
	OnlineUser* ou = NULL;
	string report = Util::emptyString;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
		if(i == onlineUsers.end()) return;
		ou = i->second;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

		ou->getIdentity().set("LS", Util::toString(aFileLength));

		if(ou->getIdentity().getBytesShared() > 0) {
			if((RSXPP_SETTING(MAXIMUM_FILELIST_SIZE) > 0) && (aFileLength > RSXPP_SETTING(MAXIMUM_FILELIST_SIZE)) && RSXPP_SETTING(FILELIST_TOO_SMALL_BIG)) {
				report = ou->setCheat("Too large filelist - %[userLSshort] for the specified share of %[userSSshort]", false, true, RSXPP_BOOLSETTING(FILELIST_TOO_SMALL_BIG));
				sendAction(*ou, RSXPP_SETTING(FILELIST_TOO_SMALL_BIG));
			} else if((aFileLength < RSXPP_SETTING(MINIMUM_FILELIST_SIZE) && RSXPP_SETTING(FILELIST_TOO_SMALL_BIG)) || (aFileLength < 100)) {
				report = ou->setCheat("Too small filelist - %[userLSshort] for the specified share of %[userSSshort]", false, true, RSXPP_BOOLSETTING(FILELIST_TOO_SMALL_BIG));
				sendAction(*ou, RSXPP_SETTING(FILELIST_TOO_SMALL_BIG));
			}
		} else if(adc == false) {
			int64_t listLength = (!ou->getIdentity().get("LL").empty()) ? Util::toInt64(ou->getIdentity().get("LL")) : -1;
			if(p->isSet(User::DCPLUSPLUS) && (listLength != -1) && (listLength * 3 < aFileLength) && (ou->getIdentity().getBytesShared() > 0)) {
				report = ou->setCheat("Fake file list - ListLen = %[userLL], FileLength = %[userLS]", false, true, RSXPP_BOOLSETTING(LISTLEN_MISMATCH));
				sendAction(*ou, RSXPP_SETTING(LISTLEN_MISMATCH));
			}
		}
	}
	if(ou) {
		ou->getClient().updated(ou);
		if(!report.empty())
			ou->getClient().cheatMessage(report);
	}
}
//RSX++ // Clean User
void ClientManager::cleanUser(const HintedUser& user) {
	bool priv = FavoriteManager::getInstance()->isPrivate(user.hint);
	{
		Lock l(cs);
		OnlineUser* ou = findOnlineUser(user.user->getCID(), user.hint, priv);
		if(!ou)
			return;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

		ou->getIdentity().cleanUser();
		ou->getClient().updated(ou);
	}
}
//RSX++ //Hide Share
bool ClientManager::getSharingHub(const HintedUser& user) {
	bool priv = FavoriteManager::getInstance()->isPrivate(user.hint);
	Client* c = 0;
	{
		Lock l(cs);
		OnlineUser* ou = findOnlineUser(user.user->getCID(), user.hint, priv);
		if(!ou)
			return false;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return false;

		c = &ou->getClient();
	}
	return c ? c->getHideShare() : false;
}
//RSX++ //check slot count
void ClientManager::checkSlots(const HintedUser& user, int slots) {
	bool priv = FavoriteManager::getInstance()->isPrivate(user.hint);
	Client* c = 0;
	string report = Util::emptyString;
	{
		Lock l(cs);
		OnlineUser* ou = findOnlineUser(user.user->getCID(), user.hint, priv);
		if(!ou)
			return;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

		c = &ou->getClient();
		if(ou->getIdentity().get("SC").empty())
			report = ou->getIdentity().checkSlotsCount(*ou, slots);
	}
	if(c && !report.empty()) {
		c->cheatMessage(report);
	}
}
//RSX++ //MultiHubKick
void ClientManager::multiHubKick(const UserPtr& p, const string& aRaw) {
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
		if(i == onlineUsers.end()) return;
		OnlineUser* ou = i->second;

		if(ou->getClientBase().getType() == ClientBase::DHT)
			return;

		for(OnlineMap::const_iterator j = onlineUsers.begin(); j != onlineUsers.end(); j++) {
			if(j->second->getUser()->isSet(User::DHT)) continue;
			if(compareUsers(*ou, *j->second)) {
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
	if(p->isSet(User::DHT) || aAction < 1)
		return;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(const_cast<CID*>(&p->getCID()));
		if(i == onlineUsers.end()) 
			return;

		if(i->second->getClient().isOp() && !i->second->isProtectedUser()) {
			i->second->getClient().sendActionCommand((*i->second), aAction);
		}
	}
}

void ClientManager::sendAction(OnlineUser& ou, const int aAction) {
	if(ou.getIdentity().getUser()->isSet(User::DHT) || aAction < 1)
		return;

	if(ou.getClient().isOp() && !ou.isProtectedUser()) {
		ou.getClient().sendActionCommand(ou, aAction);
	}
}
//END