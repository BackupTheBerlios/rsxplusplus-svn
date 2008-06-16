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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "SimpleXML.h"
#include "File.h"

#include "DetectionManager.h"

namespace dcpp {

void DetectionManager::load() {
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + "Profiles.xml", File::READ, File::OPEN).read());

		if(xml.findChild("Profiles")) {
			xml.stepIn();
			if(xml.findChild("ClientProfilesV3")) {
				xml.stepIn();
				while(xml.findChild("DetectionProfile")) {
					xml.stepIn();
					if(xml.findChild("DetectionEntry")) {
						lastId = Util::toUInt32(xml.getChildAttrib("ProfileID", Util::toString(++lastId)));
						if(lastId < 1) continue;
						xml.stepIn();

						DetectionEntry item;
						item.Id = lastId;

						if(xml.findChild("Name")) {
							item.name = xml.getChildData();
							xml.resetCurrentChild();
						}
						if(xml.findChild("Cheat")) {
							item.cheat = xml.getChildData();
							xml.resetCurrentChild();
						}
						if(xml.findChild("Comment")) {
							item.comment = xml.getChildData();
							xml.resetCurrentChild();
						}
						if(xml.findChild("RawToSend")) {
							item.rawToSend = Util::toUInt32(xml.getChildData());
							xml.resetCurrentChild();
						}
						if(xml.findChild("ClientFlag")) {
							item.clientFlag = Util::toUInt32(xml.getChildData());
							xml.resetCurrentChild();
						}
						if(xml.findChild("IsEnabled")) {
							item.isEnabled = (Util::toInt(xml.getChildData()) > 0);
							xml.resetCurrentChild();
						}

						if(xml.findChild("InfFields")) {
							xml.stepIn();
							while(xml.findChild("InfField")) {
								const string& field = xml.getChildAttrib("Field");
								const string& pattern = xml.getChildAttrib("Pattern");
								if(field.empty() || pattern.empty())
									continue;
								item.infMap.push_back(make_pair(field, pattern));
							}
							xml.stepOut();
							xml.resetCurrentChild();
						}
						try {
							addDetectionItem(item);
						} catch(const Exception&) {
							//...
						}
						xml.stepOut();
					}
					xml.stepOut();
				}
				xml.stepOut();
			}
			xml.resetCurrentChild();
			if(xml.findChild("Params")) {
				xml.stepIn();
				while(xml.findChild("Param")) {
					const string& name = xml.getChildAttrib("Name");
					const string& pattern = xml.getChildAttrib("Pattern");
					if(!name.empty() && !pattern.empty())
						params.insert(make_pair(name, pattern));
				}
				xml.stepOut();
			}
			xml.resetCurrentChild();
			if(xml.findChild("ProfileInfo")) {
				xml.stepIn();
				if(xml.findChild("DetectionProfile")) {
					xml.stepIn();
					if(xml.findChild("Version")) {
						setProfileVersion(xml.getChildData());
						xml.resetCurrentChild();
					}
					if(xml.findChild("Message")) {
						setProfileMessage(xml.getChildData());
						xml.resetCurrentChild();
					}
					if(xml.findChild("URL")) {
						setProfileUrl(xml.getChildData());
						xml.resetCurrentChild();
					}
					xml.stepOut();
				}
				xml.stepOut();
			}
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("DetectionManager::load: %s\n", e.getError().c_str());
	}
}

const DetectionManager::DetectionItems& DetectionManager::reload() {
	Lock l(cs);
	det.clear();
	params.clear();
	load();

	return det;
}

const DetectionManager::DetectionItems& DetectionManager::reloadFromHttp(bool /*bz2 = false*/) {
	Lock l(cs);
	DetectionManager::DetectionItems oldDet = det;
	det.clear();
	params.clear();
	load();
	for(DetectionManager::DetectionItems::iterator j = det.begin(); j != det.end(); ++j) {
		for(DetectionManager::DetectionItems::const_iterator k = oldDet.begin(); k != oldDet.end(); ++k) {
			if(k->Id == j->Id) {
				j->rawToSend = k->rawToSend;
				j->cheat = k->cheat;
				j->clientFlag = k->clientFlag;
				j->isEnabled = k->isEnabled;
			}
		}
	}

	return det;
}

void DetectionManager::save() {
	try {
		SimpleXML xml;
		xml.addTag("Profiles");
		xml.stepIn();

		xml.addTag("ClientProfilesV3");
		xml.stepIn();

		Lock l(cs);
		for(DetectionItems::const_iterator i = det.begin(); i != det.end(); ++i) {
			xml.addTag("DetectionProfile");
			xml.stepIn();
			{
				xml.addTag("DetectionEntry");
				xml.addChildAttrib("ProfileID", i->Id);
				xml.stepIn();
				{
					xml.addTag("Name", i->name);
					xml.addTag("Cheat", i->cheat);
					xml.addTag("Comment", i->comment);
					xml.addTag("RawToSend", Util::toString(i->rawToSend));
					xml.addTag("ClientFlag", Util::toString(i->clientFlag));
					xml.addTag("IsEnabled", i->isEnabled);

					xml.addTag("InfFields");
					xml.stepIn();
					{
						const DetectionEntry::StringMapV& InfMap = i->infMap;
						for(DetectionEntry::StringMapV::const_iterator j = InfMap.begin(); j != InfMap.end(); ++j) {
							xml.addTag("InfField");
							xml.addChildAttrib("Field", j->first);
							xml.addChildAttrib("Pattern", j->second);
						}
					}
					xml.stepOut();
				}
				xml.stepOut();
			}
			xml.stepOut();
		}
		xml.stepOut();
		xml.addTag("Params");
		xml.stepIn();
		{
			for(StringMap::const_iterator j = params.begin(); j != params.end(); ++j) {
				xml.addTag("Param");
				xml.addChildAttrib("Name", j->first);
				xml.addChildAttrib("Pattern", j->second);
			}
		}
		xml.stepOut();
		xml.addTag("ProfileInfo");
		xml.stepIn();
		{
			xml.addTag("DetectionProfile");
			xml.stepIn();
			{
				xml.addTag("Version", getProfileVersion());
				xml.addTag("Message", getProfileMessage());
				xml.addTag("URL", getProfileUrl());
			}
			xml.stepOut();
		}
		xml.stepOut();
		xml.stepOut();

		string fname = Util::getConfigPath() + "Profiles.xml";

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);

	} catch(const Exception& e) {
		dcdebug("DetectionManager::save: %s\n", e.getError().c_str());
	}
}

void DetectionManager::addDetectionItem(DetectionEntry& e) throw(Exception) {
	Lock l(cs);
	if(det.size() >= 2147483647)
		throw Exception("No more items can be added!");

	validateItem(e, true);

	if(e.Id == 0) {
		e.Id = ++lastId;

		// This should only happen if lastId (aka. unsigned int) goes over it's capacity ie. virtually never :P
		while(e.Id == 0) {
			e.Id = Util::rand(1, 2147483647);
			for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
				if(i->Id == e.Id) {
					e.Id = 0;
				}
			}
		}
	}

	det.push_back(e);
}

void DetectionManager::validateItem(const DetectionEntry& e, bool checkIds) throw(Exception) {
	Lock l(cs);
	if(checkIds && e.Id > 0) {
		for(DetectionItems::const_iterator i = det.begin(); i != det.end(); ++i) {
			if(i->Id == e.Id || e.Id <= 0) {
				throw Exception("Item with this ID already exist!");
			}
		}
	}

	{
		const DetectionEntry::StringMapV& inf = e.infMap;
		if(inf.empty())
			throw Exception("INF Map can't be empty!");
		for(DetectionEntry::StringMapV::const_iterator i = inf.begin(); i != inf.end(); ++i) {
			if(i->first == Util::emptyString)
				throw Exception("INF entry name can't be empty!");
			else if(i->second == Util::emptyString)
				throw Exception("INF entry pattern can't be empty!");
		}
	}

	if(e.name.empty()) throw Exception("Item's name can't be empty!");
}

void DetectionManager::removeDetectionItem(const uint32_t id) throw() {
	Lock l(cs);
	for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == id) {
			det.erase(i);
			return;
		}
	}
}

void DetectionManager::updateDetectionItem(const uint32_t aOrigId, const DetectionEntry& e) throw(Exception) {
	Lock l(cs);
	validateItem(e, e.Id != aOrigId);
	for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == aOrigId) {
			*i = e;
			break;
		}
	}
}

bool DetectionManager::getDetectionItem(const uint32_t aId, DetectionEntry& e) throw() {
	Lock l(cs);
	for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == aId) {
			e = *i;
			return true;
		}
	}
	return false;
}

bool DetectionManager::getNextDetectionItem(const uint32_t aId, int pos, DetectionEntry& e) throw() {
	Lock l(cs);
	for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == aId) {
			i += pos;
			if(i < det.end() && i >= det.begin()) {
				e = *i;
				return true;
			}
			return false;
		}
	}
	return false;
}

bool DetectionManager::moveDetectionItem(const uint32_t aId, int pos) {
	Lock l(cs);
	for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == aId) {
			swap(*i, *(i + pos));
			return true;
		}
	}
	return false;
}

void DetectionManager::setItemEnabled(const uint32_t aId, bool enabled) throw() {
	Lock l(cs);
	for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == aId) {
			i->isEnabled = enabled;
			break;
		}
	}
}

}; // namespace dcpp

/**
 * @file
 * $Id: DetectionManager.h 61 2008-03-09 17:26:28Z adrian_007 $
 */
