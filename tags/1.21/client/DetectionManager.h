/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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

#ifndef RSXPLUSPLUS_DETECTION_MANAGER_H
#define RSXPLUSPLUS_DETECTION_MANAGER_H

#include "Singleton.h"
#include "Thread.h"
#include "DetectionEntry.h"
#include "SimpleXML.h"

namespace dcpp {

class DetectionManager : public Singleton<DetectionManager> {
public:
	typedef vector<DetectionEntry> DetectionItems;

	DetectionManager() : profileVersion("N/A"), profileMessage("N/A"), profileUrl("N/A"), lastId(0),
		userInfoVersion("N/A"), userInfoMessage("N/A"), userInfoUrl("N/A"), ui_lastId(0) { };
	~DetectionManager() throw() { save(); det.clear(); };

	void ProfilesLoad();
	void ProfilesSave();

	void UserInfoLoad();
	void UserInfoSave();

	inline void load() {
		ProfilesLoad();
		UserInfoLoad();
	}
	inline void save() {
		ProfilesSave();
		UserInfoSave();
	}

	const DetectionItems& reload(bool isUserInfo = false);
	const DetectionItems& reloadFromHttp(bool bz2 = false, bool isUserInfo = false);

	void addDetectionItem(DetectionEntry& e, bool isUserInfo = false) throw(Exception);
	void updateDetectionItem(const uint32_t aOrigId, const DetectionEntry& e, bool isUserInfo = false) throw(Exception);
	void removeDetectionItem(const uint32_t id, bool isUserInfo = false) throw();

	bool getNextDetectionItem(const uint32_t aId, int pos, DetectionEntry& e, bool isUserInfo = false) throw();
	bool getDetectionItem(const uint32_t aId, DetectionEntry& e, bool isUserInfo = false) throw();
	bool moveDetectionItem(const uint32_t aId, int pos, bool isUserInfo = false);
	void setItemEnabled(const uint32_t aId, bool enabled, bool isUserInfo = false) throw();

	const DetectionItems& getProfiles(bool isUserInfo = false) throw() {
		Lock l(cs);
		return isUserInfo ? ui_det : det;
	}

	const DetectionItems& getProfiles(StringMap& p, bool isUserInfo = false) throw() {
		Lock l(cs);
		// don't override other params
		for(StringMapIter i = params.begin(); i != params.end(); ++i)
			p[i->first] = i->second;
		return isUserInfo ? ui_det : det;
	}

	StringMap& getParams() throw() {
		Lock l(cs);
		return params;
	}

	GETSET(string, profileVersion, ProfileVersion);
	GETSET(string, profileMessage, ProfileMessage);
	GETSET(string, profileUrl, ProfileUrl);

	GETSET(string, userInfoVersion, UserInfoVersion);
	GETSET(string, userInfoMessage, UserInfoMessage);
	GETSET(string, userInfoUrl, UserInfoUrl);

private:
	void loadCompressedProfiles();

	DetectionItems det;
	DetectionItems ui_det;

	StringMap params;
	uint32_t lastId;
	uint32_t ui_lastId;

	void validateItem(const DetectionEntry& e, bool checkIds, bool isUserInfo =false) throw(Exception);
	void importProfiles(SimpleXML& xml);

	friend class Singleton<DetectionManager>;
	CriticalSection cs;
};

}; // namespace dcpp

#endif // RSXPLUSPLUS_DETECTION_MANAGER_H

/**
 * @file
 * $Id: DetectionManager.h 230 2011-01-30 18:15:33Z adrian_007 $
 */
