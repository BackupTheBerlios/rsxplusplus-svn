/* 
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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
#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "../rsx/UpdateManager.h"
#include "../rsx/IpManager.h"
#include "../rsx/RsxUtil.h"
#include "../rsx/AutoSearchManager.h"
#include "../rsx/HTTPDownloadManager.h"
#include "PluginsManager.h"
#include "RawManager.h"
#include "ScriptManager.h"
////////////////////////////////////////////////////////////
#include "IgnoreManager.h"

#include "StringTokenizer.h"

#include "DebugManager.h"
#include "DetectionManager.h"
#include "ClientProfileManager.h"
#include "WebServerManager.h"
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
	RSXSettingsManager::newInstance(); //RSX++

	LogManager::newInstance();
	TimerManager::newInstance();
	HashManager::newInstance();
	CryptoManager::newInstance();
	SearchManager::newInstance();
	ClientManager::newInstance();
	ConnectionManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	ShareManager::newInstance();
	FavoriteManager::newInstance();
	RawManager::newInstance(); //RSX++
	QueueManager::newInstance();
	FinishedManager::newInstance();
	ADLSearchManager::newInstance();
	DebugManager::newInstance();
	DetectionManager::newInstance();	
	ClientProfileManager::newInstance();	
	PopupManager::newInstance();
	//RSX++ //instances
	HTTPDownloadManager::newInstance();
	IgnoreManager::newInstance();
	ScriptManager::newInstance();
	UpdateManager::newInstance();
	AutoSearchManager::newInstance();
	IpManager::newInstance();
	PluginsManager::newInstance();
	//END

	SettingsManager::getInstance()->load();
	//RSX++ //loaders
	RSXSettingsManager::getInstance()->load();
	ScriptManager::getInstance()->load();
	AutoSearchManager::getInstance()->AutosearchLoad();
	IpManager::getInstance()->load();
	//END

	if(!SETTING(LANGUAGE_FILE).empty()) {
		ResourceManager::getInstance()->loadLanguage(SETTING(LANGUAGE_FILE));
	}

	FavoriteManager::getInstance()->load();
	CryptoManager::getInstance()->loadCertificates();
	ClientProfileManager::getInstance()->load();
	DetectionManager::getInstance()->load();
	WebServerManager::newInstance();

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
	if(RSXBOOLSETTING(IPUPDATE))
		IpManager::getInstance()->UpdateExternalIp();
	//END
}

void shutdown() {
	TimerManager::getInstance()->shutdown();
	HashManager::getInstance()->shutdown();
	ConnectionManager::getInstance()->shutdown();

	BufferedSocket::waitShutdown();
	
	QueueManager::getInstance()->saveQueue();
	SettingsManager::getInstance()->save();
	RSXSettingsManager::getInstance()->save(); //RSX++

	//RSX++
	RsxUtil::uinit();
	PluginsManager::deleteInstance();
	ScriptManager::deleteInstance();
	UpdateManager::deleteInstance();
	IpManager::deleteInstance();
	IgnoreManager::deleteInstance();
	AutoSearchManager::deleteInstance();
	HTTPDownloadManager::deleteInstance();
	//END
	DebugManager::deleteInstance();
	WebServerManager::deleteInstance();
	DetectionManager::deleteInstance();	
	ClientProfileManager::deleteInstance();	
	PopupManager::deleteInstance();
	ADLSearchManager::deleteInstance();
	RawManager::deleteInstance(); //RSX++
	FinishedManager::deleteInstance();
	ShareManager::deleteInstance();
	CryptoManager::deleteInstance();
	DownloadManager::deleteInstance();
	UploadManager::deleteInstance();
	QueueManager::deleteInstance();
	ConnectionManager::deleteInstance();
	SearchManager::deleteInstance();
	FavoriteManager::deleteInstance();
	ClientManager::deleteInstance();
	HashManager::deleteInstance();
	LogManager::deleteInstance();
	SettingsManager::deleteInstance();
	RSXSettingsManager::deleteInstance(); //RSX++
	TimerManager::deleteInstance();
	ResourceManager::deleteInstance();

#ifdef _WIN32	
	::WSACleanup();
#endif
}

} // namespace dcpp

/**
 * @file
 * $Id: DCPlusPlus.cpp 399 2008-07-06 19:48:02Z BigMuscle $
 */
