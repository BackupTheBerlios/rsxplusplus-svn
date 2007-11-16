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
#include "../client/version.h"

#include "../client/ClientProfileManager.h"
#include "../client/File.h"

#include "UpdateManager.h"
#include "RsxUtil.h"
#include "IpManager.h"

UpdateManager::UpdateManager() : currentlyUpdating(-1) { }
UpdateManager::~UpdateManager() { }

void UpdateManager::updateFiles(UpdateMap& aUrls) {
	profileList = aUrls; 
	startDownload();
}

void UpdateManager::startDownload() {
	currentlyUpdating = 0;
	downBuf = Util::emptyString;

	UpdateMap::const_iterator j = profileList.begin();
	if(j != profileList.end()) {
		c.addListener(this);
		c.downloadFile(j->second + RsxUtil::getUpdateFileNames((int)j->first));
		currentlyUpdating = (int)j->first;
		profileList.erase(j->first);
		dcdebug("startDownload %s\n", RsxUtil::getUpdateFileNames(currentlyUpdating).c_str());
	}
}
//load new profiles and try to merge actions from old to new list
void UpdateManager::reloadFile(int file) {
	switch(file) {
		case CLIENT: 
			ClientProfileManager::getInstance()->reloadClientProfilesFromHttp();	
			break;
		case MYINFO:
			ClientProfileManager::getInstance()->reloadMyinfoProfilesFromHttp();
			break;
		case IPWATCH: 
			IpManager::getInstance()->reloadIpWatch();
			break;
		default: return;
	}
	fire(UpdateManagerListener::Complete(), file);
}
//load old files without touching
void UpdateManager::restoreOld(int file) {
	switch(file) {
		case CLIENT: 
			ClientProfileManager::getInstance()->reloadClientProfiles();	
			break;
		case MYINFO:
			ClientProfileManager::getInstance()->reloadMyinfoProfiles();
			break;
		case IPWATCH: 
			IpManager::getInstance()->reloadIpWatch();
			break;
		default: return;
	}
	fire(UpdateManagerListener::Complete(), file);
}

void UpdateManager::on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
	downBuf.append((char*)buf, len);
}

void UpdateManager::on(HttpConnectionListener::Complete, HttpConnection* /*conn*/, const string& /*aLine*/) throw() {
	c.removeListener(this);
	if(!downBuf.empty()) {
		string fname = Util::getConfigPath() + RsxUtil::getUpdateFileNames(currentlyUpdating);
		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(downBuf);
		f.close();
		File::copyFile(fname + ".tmp", fname + ".new");

		try {
			File::deleteFile(fname + ".old");
			File::renameFile(fname, fname + ".old");
			File::renameFile(fname + ".tmp", fname);
		} catch(...) {
			File::renameFile(fname + ".tmp", fname);
			dcdebug("UpdateManager::onAction, no old file found but who cares?");
		}
		reloadFile(currentlyUpdating);
		startDownload();
		dcdebug("Completet %s\n", RsxUtil::getUpdateFileNames(currentlyUpdating).c_str());
	}
}

void UpdateManager::on(HttpConnectionListener::Failed, HttpConnection* /*conn*/, const string& aLine) throw() {
	c.removeListener(this);
	{
		fire(UpdateManagerListener::Failed(), currentlyUpdating, aLine);
		startDownload();
		dcdebug("Failed %s %s\n", RsxUtil::getUpdateFileNames(currentlyUpdating).c_str(), aLine);
	}
}