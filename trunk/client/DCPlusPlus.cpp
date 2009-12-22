/* 
 * Copyright (C) 2001-2009 Jacek Sieka, arnetheduck on gmail point com
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

#include "ConnectionManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"
#include "ShareManager.h"
#include "SearchManager.h"
#include "QueueManager.h"
#include "ClientManager.h"
#include "HashManager.h"
#include "LogManager.h"
#include "FavoriteManager.h"
#include "SettingsManager.h"
#include "FinishedManager.h"
#include "ADLSearch.h"
//RSX++ ////////////////////////////////////////////////////
#include "rsxppSettingsManager.h"
#include "../rsx/UpdateManager.h"
#include "../rsx/IpManager.h"
#include "../rsx/RsxUtil.h"
#include "../rsx/AutoSearchManager.h"
#include "../rsx/HTTPDownloadManager.h"
#include "PluginsManager.h"
#include "RawManager.h"
#include "NetworkConfiguration.hpp"
////////////////////////////////////////////////////////////

#include "StringTokenizer.h"

#include "DebugManager.h"
#include "DetectionManager.h"
#include "WebServerManager.h"
#include "ThrottleManager.h"

#include "../dht/dht.h"
#include "../windows/PopupManager.h"

/*
#ifdef _STLP_DEBUG
void __stl_debug_terminate() {
	int* x = 0;
	*x = 0;
}
#endif
*/

namespace dcpp {

void startup(void (*f)(void*, const tstring&), void* p) {
	// "Dedicated to the near-memory of Nev. Let's start remembering people while they're still alive."
	// Nev's great contribution to dc++
	while(1) break;


#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	Util::initialize();

	ResourceManager::newInstance();
	SettingsManager::newInstance();
	rsxppSettingsManager::newInstance(); //RSX++

	LogManager::newInstance();
	TimerManager::newInstance();
	HashManager::newInstance();
	CryptoManager::newInstance();
	SearchManager::newInstance();
	ClientManager::newInstance();
	ConnectionManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	ThrottleManager::newInstance();
	ShareManager::newInstance();
	FavoriteManager::newInstance();
	RawManager::newInstance(); //RSX++
	QueueManager::newInstance();
	FinishedManager::newInstance();
	ADLSearchManager::newInstance();
	DebugManager::newInstance();
	DetectionManager::newInstance();	
	PopupManager::newInstance();
	//RSX++ //instances
	NetworkConfiguration::newInstance();
	HTTPDownloadManager::newInstance();
	UpdateManager::newInstance();
	AutoSearchManager::newInstance();
	IpManager::newInstance();
	PluginsManager::newInstance();

	NetworkConfiguration::getInstance()->setupPorts();
	//---
	SettingsManager::getInstance()->load();
	//+++
	AutoSearchManager::getInstance()->AutosearchLoad();
	IpManager::getInstance()->load();
	//END

	if(!SETTING(LANGUAGE_FILE).empty()) {
		ResourceManager::getInstance()->loadLanguage(SETTING(LANGUAGE_FILE));
	}

	FavoriteManager::getInstance()->load();
	CryptoManager::getInstance()->loadCertificates();
	DetectionManager::getInstance()->load();
	WebServerManager::newInstance();
	
	dht::DHT::newInstance();

	if(f != NULL)
		(*f)(p, TSTRING(HASH_DATABASE));
	HashManager::getInstance()->startup();
	if(f != NULL)
		(*f)(p, TSTRING(SHARED_FILES));
	ShareManager::getInstance()->refresh(true, false, true);
	if(f != NULL)
		(*f)(p, TSTRING(DOWNLOAD_QUEUE));
	QueueManager::getInstance()->loadQueue();

	//RSX++
	RsxUtil::init();
	if(RSXPP_BOOLSETTING(IPUPDATE))
		IpManager::getInstance()->UpdateExternalIp();
	PluginsManager::getInstance()->init(f, p);
	//END
}

void shutdown() {
	TimerManager::getInstance()->shutdown();
	HashManager::getInstance()->shutdown();
	ConnectionManager::getInstance()->shutdown();

	BufferedSocket::waitShutdown();
	
	QueueManager::getInstance()->saveQueue(true);
	SettingsManager::getInstance()->save();

	dht::DHT::deleteInstance();
	//RSX++
	NetworkConfiguration::getInstance()->closePorts();
	PluginsManager::getInstance()->close();

	RsxUtil::uinit();
	PluginsManager::deleteInstance();
	UpdateManager::deleteInstance();
	IpManager::deleteInstance();
	AutoSearchManager::deleteInstance();
	HTTPDownloadManager::deleteInstance();
	//END
	DebugManager::deleteInstance();
	WebServerManager::deleteInstance();
	DetectionManager::deleteInstance();	
	PopupManager::deleteInstance();
	ADLSearchManager::deleteInstance();
	RawManager::deleteInstance(); //RSX++
	FinishedManager::deleteInstance();
	ShareManager::deleteInstance();
	CryptoManager::deleteInstance();
	ThrottleManager::deleteInstance();
	DownloadManager::deleteInstance();
	UploadManager::deleteInstance();
	QueueManager::deleteInstance();
	ConnectionManager::deleteInstance();
	SearchManager::deleteInstance();
	FavoriteManager::deleteInstance();
	ClientManager::deleteInstance();
	HashManager::deleteInstance();
	LogManager::deleteInstance();
	NetworkConfiguration::deleteInstance();
	rsxppSettingsManager::deleteInstance(); //RSX++
	SettingsManager::deleteInstance();
	TimerManager::deleteInstance();
	ResourceManager::deleteInstance();

#ifdef _WIN32	
	::WSACleanup();
#endif
}

} // namespace dcpp

/**
 * @file
 * $Id: DCPlusPlus.cpp 454 2009-08-13 20:28:19Z BigMuscle $
 */
