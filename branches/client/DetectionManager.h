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

#ifndef DETECTION_MANAGER_H
#define DETECTION_MANAGER_H

#include "Singleton.h"
#include "CriticalSection.h"
#include "DetectionEntry.h"

namespace dcpp {

class DetectionManager : public Singleton<DetectionManager> {
public:
	typedef vector<DetectionEntry> DetectionItems;

	DetectionManager() : profileVersion("N/A"), profileMessage("N/A"), profileUrl("N/A"), lastId(0) { };
	~DetectionManager() throw() { save(); };

	void load();
	void save();

	void addDetectionItem(DetectionEntry& e) throw(Exception);
	void updateDetectionItem(const uint32_t aOrigId, const DetectionEntry& e) throw(Exception);
	void removeDetectionItem(const uint32_t id) throw();

	bool getDetectionItem(const uint32_t aId, DetectionEntry& e) throw();
	bool moveDetectionItem(const uint32_t aId, int pos);
	void setItemEnabled(const uint32_t aId, bool enabled) throw();

	const DetectionItems& reload();
	const DetectionItems& reloadFromHttp(bool bz2 = false);

	const DetectionItems& getProfiles() throw() {
		Lock l(cs);
		return det;
	}

	const DetectionItems& getProfiles(StringMap& p) throw() {
		Lock l(cs);
		p = params;
		return det;
	}

	StringMap& getParams() throw() {
		Lock l(cs);
		return params;
	}

	GETSET(string, profileVersion, ProfileVersion);
	GETSET(string, profileMessage, ProfileMessage);
	GETSET(string, profileUrl, ProfileUrl);

private:
	DetectionItems det;
	StringMap params;
	uint32_t lastId;

	void validateItem(const DetectionEntry& e, bool checkIds) throw(Exception);

	friend class Singleton<DetectionManager>;
	CriticalSection cs;
};

}; // namespace dcpp

#endif // DETECTION_MANAGER_H

/**
 * @file
 * $Id: DetectionManager.h 61 2008-03-09 17:26:28Z adrian_007 $
 */
