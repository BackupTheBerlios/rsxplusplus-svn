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

DetectionManager::DetectionManager() {
	load();
}

DetectionManager::~DetectionManager() {
	save();
}

void DetectionManager::load(bool fromHttp) {
	const string& aPath = Util::getConfigPath() + (fromHttp ? "Profiles2.xml.new" : "Profiles2.xml");
	try {
		SimpleXML xml;
		xml.fromXML(File(aPath, File::READ, File::OPEN).read());

		if(xml.findChild("Profiles")) {
			xml.stepIn();
			if(xml.findChild("ClientProfilesV3")) {
				xml.stepIn();
				while(xml.findChild("DetectionProfile")) {
					xml.stepIn();
					if(xml.findChild("DetectionEntry")) {
						int id = xml.getIntChildAttrib("ProfileID");
						if(id < 1) continue;
						xml.stepIn();

						DetectionEntry item;
						item.Id = id;

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
							item.rawToSend = Util::toInt(xml.getChildData());
							xml.resetCurrentChild();
						}
						if(xml.findChild("ClientFlag")) {
							item.flag = Util::toInt(xml.getChildData());
							xml.resetCurrentChild();
						}

						if(xml.findChild("InfFields")) {
							xml.stepIn();
							StringMap infFields;
							while(xml.findChild("InfField")) {
								const string& field = xml.getChildAttrib("Field");
								const string& pattern = xml.getChildAttrib("Pattern");
								if(field.empty() || pattern.empty())
									continue;
								infFields.insert(make_pair(field, pattern));
							}
							{
								Lock l(cs);
								item.infMap = infFields;
								det.push_back(item);
							}
							xml.stepOut();
							xml.resetCurrentChild();
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
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("DetectionManager::load: %s\n", e.getError().c_str());
	}
}

void DetectionManager::reload(bool /*fromHttp*/) {

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
					xml.addTag("RawToSend", i->rawToSend);
					xml.addTag("ClientFlag", i->flag);

					xml.addTag("InfFields");
					xml.stepIn();
					{
						const StringMap& InfMap = i->infMap;
						for(StringMap::const_iterator j = InfMap.begin(); j != InfMap.end(); ++j) {
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
		xml.stepOut();

		const string& fname = Util::getConfigPath() + "Profiles2.xml";

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

void DetectionManager::addDetectionItem(int id, const StringMap& aMap, const string& name, const string& aCD, const string& aComment) throw(Exception) {
	Lock l(cs);
	{
		for(DetectionItems::const_iterator i = det.begin(); i != det.end(); ++i) {
			if(i->Id == id) {
				throw("Profile with same ID already exist!");
				return;
			}
		}
	}

	{
		for(StringMap::const_iterator i = aMap.begin(); i != aMap.end(); ++i) {
			if(i->first.length() != 2) {
				const string& err = i->first + " is not a valid ADC field";
				throw(err.c_str());
				return;
			}
			if(i->second.empty()) {
				const string& err = "Pattern for field " + i->first + " is empty!";
				throw(err.c_str());
				return;
			}
		}
	}

	DetectionEntry entry;
	entry.Id = id;
	entry.infMap = aMap;
	entry.name = name;
	entry.cheat = aCD;
	entry.comment = aComment;

	det.push_back(entry);
}

void DetectionManager::removeDetectionItem(const int id) {
	Lock l(cs);
	for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == id) {
			det.erase(i);
			return;
		}
	}
}

void DetectionManager::updateDetectionItem(const DetectionEntry& e) {
	Lock l(cs);
	for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == e.Id) {
			*i = e;
			return;
		}
	}
}

void DetectionManager::getDetectionItem(const int aId, DetectionEntry& e) {
	Lock l(cs);
	for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == aId) {
			e = *i;
			return;
		}
	}
}

bool DetectionManager::moveDetectionItem(const int aId, int pos) {
	Lock l(cs);
	for(DetectionItems::iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == aId) {
			swap(*i, *(i + pos));
			return true;
		}
	}
	return false;
}

void DetectionManager::addParam(const string& aName, const string& aPattern) throw(Exception) {
	Lock l(cs);
	if(aName.empty()) {
		throw("Name must not be empty!");
		return;
	}
	if(aPattern.empty()) {
		throw("Pattern must not be empty!");
		return;
	}
	StringMap::iterator i = params.find(aName);
	if(i != params.end()) {
		throw("Param already exist!");
		return;
	}
	params.insert(make_pair(aName, aPattern));
}

void DetectionManager::changeParam(const string& aOldName, const string& aName, const string& aPattern) throw(Exception) {
	Lock l(cs);
	if(aPattern.empty()) {
		throw("Pattern must not be empty!");
		return;
	}
	if(aName.empty()) {
		throw("Name must not be empty!");
		return;
	}

	StringMap::iterator i = params.find(aName);
	if(i != params.end()) {
		throw("Param with this name already exist!");
		return;
	}
	i = params.find(aOldName);
	if(i != params.end()) {
		params.erase(i);
		params.insert(make_pair(aName, aPattern));
	}
}

void DetectionManager::removeParam(const string& aName) {
	Lock l(cs);
	StringMap::iterator i = params.find(aName);
	if(i != params.end()) {
		params.erase(i);
	}
}

/**
 * @file
 * $Id: DetectionManager.h 61 2008-03-09 17:26:28Z adrian_007 $
 */
