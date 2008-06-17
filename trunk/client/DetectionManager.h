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
	typedef std::vector<DetectionEntry> DetectionItems;

	DetectionManager();
	~DetectionManager();

	void load();
	void reload();
	void reloadFromHttp(bool bz2 = false);
	void save();

	void addDetectionItem(const DetectionEntry& e) throw(Exception);
	void updateDetectionItem(const int aId /* old id */, const DetectionEntry& e) throw(Exception);
	void validateItem(const DetectionEntry& e) throw(Exception);
	void removeDetectionItem(const int id) throw();

	bool getDetectionItem(const int aId, DetectionEntry& e) throw();
	bool moveDetectionItem(const int aId, int pos);
	void setItemEnabled(const int aId, bool enabled) throw();

	const DetectionItems& getProfiles() throw() {
		Lock l(cs);
		return det;
	}

	const DetectionItems& getProfiles(StringMap& p) throw() {
		Lock l(cs);
		p = params;
		return det;
	}

	const StringMap& getParams() {
		Lock l(cs);
		return params;
	}

	void addParam(const string& aName, const string& aPattern) throw(Exception);
	void changeParam(const string& aOldName, const string& aName, const string& aPattern) throw(Exception);
	void removeParam(const string& aName);

	GETSET(string, profileVersion, ProfileVersion);
	GETSET(string, profileMessage, ProfileMessage);
	GETSET(string, profileUrl, ProfileUrl);

private:
	//void validateEntry(DetectionEntry e, bool checkId = true) throw(Exception);

	DetectionItems det;
	StringMap params;

	friend class Singleton<DetectionManager>;
	CriticalSection cs;
};
}; // namespace dcpp
#endif

/**
 * @file
 * $Id$
 */
