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

void DetectionManager::load() {
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + "Profiles2.xml", File::READ, File::OPEN).read());

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
							item.setFlag((Flags::MaskType)Util::toInt(xml.getChildData()));
							xml.resetCurrentChild();
						}
						if(xml.findChild("IsEnabled")) {
							item.isEnabled = (Util::toInt(xml.getChildData()) > 0);
							xml.resetCurrentChild();
						}
						
						StringMap infFields;
						if(xml.findChild("InfFields")) {
							xml.stepIn();
							while(xml.findChild("InfField")) {
								const string& field = xml.getChildAttrib("Field");
								const string& pattern = xml.getChildAttrib("Pattern");
								if(field.empty() || pattern.empty())
									continue;
								infFields.insert(make_pair(field, pattern));
							}
							xml.stepOut();
							xml.resetCurrentChild();
						}
						if(!infFields.empty()) {
							Lock l(cs);
							item.infMap = infFields;
							det.push_back(item);
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

void DetectionManager::reload() {
	Lock l(cs);
	det.clear();
	params.clear();
	load();
}

void DetectionManager::reloadFromHttp(bool /*bz2 = false*/) {
	Lock l(cs);
	DetectionManager::DetectionItems oldDet = det;
	det.clear();
	params.clear();
	load();
	for(DetectionManager::DetectionItems::iterator j = det.begin(); j != det.end(); ++j) {
		for(DetectionManager::DetectionItems::const_iterator k = oldDet.begin(); k != oldDet.end(); ++k) {
			if(k->name.compare(j->name) == 0) {
				j->rawToSend = k->rawToSend;
				j->cheat = k->cheat;
			}
		}
	}
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
					xml.addTag("ClientFlag", i->getFlags());
					xml.addTag("IsEnabled", i->isEnabled);

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

void DetectionManager::addDetectionItem(int id, bool isEnabled, const StringMap& aMap, const string& name, const string& aCD, const string& aComment, Flags::MaskType flags) throw(Exception) {
	Lock l(cs);

	for(DetectionItems::const_iterator i = det.begin(); i != det.end(); ++i) {
		if(i->Id == id) {
			throw("Profile with same ID already exist!");
			return;
		}
	}

	if(aMap.empty()) {
		for(StringMap::const_iterator j = aMap.begin(); j != aMap.end(); j++) {
			if(j->second.empty()) {
				const string& err = "Pattern for field " + j->first + " is empty!";
				throw(err.c_str());
				return;
			}
		}
	} else {
		throw("INF Params List must not be empty!");
		return;
	}

	DetectionEntry entry;
	entry.Id = id;
	entry.isEnabled = isEnabled;
	entry.infMap = aMap;
	entry.name = name;
	entry.cheat = aCD;
	entry.comment = aComment;
	entry.setFlag(flags);

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

/**
 * @file
 * $Id: DetectionManager.h 61 2008-03-09 17:26:28Z adrian_007 $
 */
