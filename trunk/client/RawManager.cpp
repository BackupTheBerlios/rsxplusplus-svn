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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "RawManager.h"

#include "ResourceManager.h"
#include "SimpleXML.h"
#include "ClientManager.h"

#define RAW_FILE "Raws.xml"

RawManager::RawManager() : lastAction(0) {
	loadActionRaws(); 
	TimerManager::getInstance()->addListener(this);
	RSXSettingsManager::getInstance()->addListener(this);
}

RawManager::~RawManager() {
	saveActionRaws();
	TimerManager::getInstance()->removeListener(this);
	RSXSettingsManager::getInstance()->removeListener(this);
	for(Action::List::const_iterator i = action.begin(); i != action.end(); ++i) {
		delete i->second;
	}
}

void RawManager::saveActionRaws() {
	try {
		SimpleXML xml;
		xml.addTag("ActionRaws");
		xml.stepIn();

		for(Action::List::const_iterator l = action.begin(); l != action.end(); ++l) {
			xml.addTag("Action");
			xml.addChildAttrib("ID", Util::toString(l->second->getActionId()));
			xml.addChildAttrib("Name", l->second->getName());
			xml.addChildAttrib("Actif", Util::toString(l->second->getActif()));
			xml.stepIn();
			for(Action::RawsList::const_iterator j = l->second->raw.begin(); j != l->second->raw.end(); ++j) {
				xml.addTag("Raw");
				xml.addChildAttrib("ID", Util::toString(j->getRawId()));
				xml.addChildAttrib("Name", j->getName());
				xml.addChildAttrib("Raw", j->getRaw());
				xml.addChildAttrib("Time", Util::toString(j->getTime()));
				xml.addChildAttrib("Actif", Util::toString(j->getActif()));
				xml.addChildAttrib("UseLua", Util::toString(j->getLua()));
			}
			xml.stepOut();
		}
		xml.stepOut();

		string fname = Util::getConfigPath() + RAW_FILE;

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);

	} catch(const Exception& e) {
		dcdebug("RawManager::saveActionRaws: %s\n", e.getError().c_str());
	}
}

void RawManager::loadActionRaws(SimpleXML& aXml) {
	aXml.resetCurrentChild();
	int actionId = NULL;
	while(aXml.findChild("Action")) {
		actionId = aXml.getIntChildAttrib("ID");
		addAction(actionId, aXml.getChildAttrib("Name"), aXml.getBoolChildAttrib("Actif"));
		aXml.stepIn();
		while(aXml.findChild("Raw")) {
			addRaw(actionId, aXml.getIntChildAttrib("ID"), aXml.getChildAttrib("Name"), aXml.getChildAttrib("Raw"), 
				aXml.getIntChildAttrib("Time"), aXml.getBoolChildAttrib("Actif"), aXml.getBoolChildAttrib("UseLua"));
		}
		aXml.stepOut();
	}
}

void RawManager::loadActionRaws() {
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + RAW_FILE, File::READ, File::OPEN).read());
		
		if(xml.findChild("ActionRaws")) {
			xml.stepIn();
			loadActionRaws(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("RawManager::loadActionRaws: %s\n", e.getError().c_str());
	}
}

int RawManager::addAction(int actionId, const string& name, bool actif) throw(Exception) {
	if(name.empty())
		throw Exception(STRING(NO_NAME_SPECIFIED));

	Lock l(act);
	for(Action::List::const_iterator i = action.begin(); i != action.end(); ++i) {
		if(i->second->getName() == name)
			throw Exception(STRING(ACTION_EXISTS));
	}

	while(actionId == 0) {
		actionId = Util::rand(1, 2147483647);
		for(Action::List::const_iterator i = action.begin(); i != action.end(); ++i) {
			if(i->second->getActionId() == actionId) {
				actionId = 0;
			}
		}
	}

	lastAction++;
	action.insert(make_pair(lastAction, new Action(actionId, name, actif)));

	return lastAction;
}

void RawManager::renameAction(const string& oName, const string& nName) throw(Exception) {
	if(oName.empty() || nName.empty())
		throw Exception(STRING(NO_NAME_SPECIFIED));

	Lock l(act);
	for(Action::List::const_iterator i = action.begin(); i != action.end(); ++i) {
		if(i->second->getName() == nName)
			throw Exception(STRING(ACTION_EXISTS));
	}
	for(Action::List::iterator i = action.begin(); i != action.end(); ++i) {
		if(i->second->getName() == oName)
			i->second->setName(nName);
	}
}

void RawManager::setActifAction(int id, bool actif) {
	Lock l(act);
	Action::List::iterator i = action.find(id);
	if(i == action.end())
		return;
	i->second->setActif(actif);
}

bool RawManager::getActifActionId(int actionId) {
	Lock l(act);
	for(Action::List::const_iterator i = action.begin(); i != action.end(); ++i) {
		if(i->second->getActionId() == actionId) {
			return i->second->getActif();
		}
	}
	return false;
}

void RawManager::removeAction(int id) {
	Lock l(act);
	Action::List::iterator i = action.find(id);
	if(i == action.end())
		return;
	delete i->second;
	action.erase(i);
}

int RawManager::getValidAction(int actionId) {
	Lock l(act);
	for(Action::List::const_iterator i = action.begin(); i != action.end(); ++i) {
		if(i->second->getActionId() == actionId)
			return i->second->getActionId();
	}
	return 0;
}

int RawManager::getActionId(int id) {
	Lock l(act);
	Action::List::const_iterator i = action.find(id);
	if(i == action.end())
		return 0;
	return i->second->getActionId();
}

tstring RawManager::getNameActionId(int actionId) {
	Lock l(act);
	for(Action::List::const_iterator i = action.begin(); i != action.end(); ++i) {
		if(i->second->getActionId() == actionId)
			return Text::toT(i->second->getName());
	}
	return TSTRING(UN_ACTION);
}

string RawManager::getRawCommand(int pos, int rawPos) {
	Lock l(act);
	Action::List::const_iterator i = action.find(pos);
	if(i == action.end())
		return Util::emptyString;

	Action::RawsList::const_iterator j = i->second->raw.begin() + rawPos;
	if(j != i->second->raw.end())
		return j->getRaw();
	return Util::emptyString;
}

Action::RawsList RawManager::getRawList(int id) {
	Lock l(act);
	Action::RawsList list;
	Action::List::const_iterator i = action.find(id);
	if(i != action.end()) {
		list = i->second->raw;
	}
	return list;
}

Action::RawsList RawManager::getRawListActionId(int actionId) {
	Lock l(act);
	Action::RawsList list;
	for(Action::List::const_iterator i = action.begin(); i != action.end(); ++i) {
		if(i->second->getActionId() == actionId) {
			list = i->second->raw;
			break;
		}
	}
	return list;
}

void RawManager::addRaw(int actionId, int rawId, const string& name, const string& raw, int time, bool actif, bool lua) {
	if(name.empty())
		return;

	Lock l(act);
	for(Action::List::const_iterator i = action.begin(); i != action.end(); ++i) {
		if(i->second->getActionId() == actionId) {
			for(Action::RawsList::const_iterator j = i->second->raw.begin(); j != i->second->raw.end(); ++j) {
				if(j->getName() == name)
					return;
			}

			while(rawId == 0) {
				rawId = Util::rand(1, 2147483647);
				for(Action::RawsList::const_iterator j = i->second->raw.begin(); j != i->second->raw.end(); ++j) {
					while(j->getRawId() == rawId) {
						rawId = 0;
					}
				}
			}
			i->second->raw.push_back(Action::Raw(i->second->lastRaw++, rawId, name, raw, time, actif, lua));
			return;
		}
	}
}

Action::Raw RawManager::addRaw(int id, const string& name, const string& raw, int time, bool lua) throw(Exception) {
	if(name.empty())
		throw Exception(STRING(NO_NAME_SPECIFIED));

	Lock l(act);
	Action::List::const_iterator i = action.find(id);
	if(i != action.end()) {
		for(Action::RawsList::const_iterator j = i->second->raw.begin(); j != i->second->raw.end(); ++j) {
			if(j->getName() == name)
				throw Exception(STRING(RAW_EXISTS));
		}
		int rawId = 0;
		while(rawId == 0) {
			rawId = Util::rand(1, 2147483647);
			for(Action::RawsList::const_iterator j = i->second->raw.begin(); j != i->second->raw.end(); ++j) {
				while(j->getRawId() == rawId) {
					rawId = 0;
				}
			}
		}
		i->second->raw.push_back(Action::Raw(i->second->lastRaw++, rawId, name, raw, time, true, lua));
		return i->second->raw.back();
	}
	return Action::Raw(0, 0, Util::emptyString, Util::emptyString, 0, false, lua);
}

void RawManager::changeRaw(int id, const string& oName, const string& nName, const string& raw, int time, bool lua) throw(Exception) {
	if(oName.empty() || nName.empty())
		throw Exception(STRING(NO_NAME_SPECIFIED));

	Lock l(act);
	Action::List::const_iterator i = action.find(id);
	if(i != action.end()) {
		if(oName != nName) {
			for(Action::RawsList::const_iterator j = i->second->raw.begin(); j != i->second->raw.end(); ++j) {
				if(j->getName() == nName)
					throw Exception(STRING(RAW_EXISTS));
			}
		}

		for(Action::RawsList::iterator j = i->second->raw.begin(); j != i->second->raw.end(); ++j) {
			if(j->getName() == oName) {
				j->setName(nName);
				j->setRaw(raw);
				j->setTime(time);
				j->setLua(lua);
				break;
			}
		}
	}
}

void RawManager::getRawItem(int id, int idRaw, Action::Raw& ra, bool favHub/* = false*/) {
	Lock l(act);
	Action::List::const_iterator i = action.find(id);
	if(i == action.end())
		return;
	for(Action::RawsList::const_iterator j = i->second->raw.begin(); j != i->second->raw.end(); ++j) {
		if(favHub) {
			if(j->getRawId() == idRaw) {
				ra = *j;
				break;
			}
		} else {
			if(j->getId() == idRaw) {
				ra = *j;
				break;
			}
		}
	}
}

void RawManager::setActifRaw(int id, int idRaw, bool actif) {
	Lock l(act);
	Action::List::const_iterator i = action.find(id);
	if(i == action.end())
		return;
	for(Action::RawsList::iterator j = i->second->raw.begin(); j != i->second->raw.end(); ++j) {
		if(j->getId() == idRaw) {
			j->setActif(actif);
			break;
		}
	}
}

void RawManager::removeRaw(int id, int idRaw) {
	Lock l(act);
	Action::List::const_iterator i = action.find(id);
	if(i == action.end())
		return;
	for(Action::RawsList::iterator j = i->second->raw.begin(); j != i->second->raw.end(); ++j) {
		if(j->getId() == idRaw) {
			i->second->raw.erase(j);
			break;
		}
	}
}

bool RawManager::moveRaw(int id, int idRaw, int pos) {
	dcassert(pos == -1 || pos == 1);
	Lock l(act);
	Action::List::const_iterator i = action.find(id);
	if(i == action.end())
		return false;
	
	for(Action::RawsList::iterator j = i->second->raw.begin(); j != i->second->raw.end(); ++j) {
		if(j->getId() == idRaw) {
			swap(*j, *(j + pos));
			return true;
		}
	}
	return false;
}

void RawManager::on(TimerManagerListener::Second, uint64_t aTick) throw() {
	for(ListRaw::iterator i = raw.begin(); i != raw.end(); ++i) {
		if(aTick >= i->first) {
			if(i->second.client) {
				if(i->second.lua) {
					ScriptManager::getInstance()->onRaw(i->second.rawName, i->second.raw, i->second.client);
				} else {
					i->second.client->insertRaw(i->second.raw);
				}
			}
			raw.erase(i);
		}
	}
}

void RawManager::on(RSXSettingsManagerListener::Load, SimpleXML& xml) throw() {
	if(xml.findChild("ADLSPoints")) {
		xml.stepIn();
		while(xml.findChild("PointsSetting")) {
			addADLPoints(xml.getIntChildAttrib("Points"), xml.getIntChildAttrib("Action"), xml.getBoolChildAttrib("DisplayCheat"));
		}
		xml.stepOut();
	}
}

void RawManager::on(RSXSettingsManagerListener::Save, SimpleXML& xml) throw() {
	xml.addTag("ADLSPoints");
	xml.stepIn();
	for(ADLPoints::const_iterator i = points.begin(); i != points.end(); ++i) {
		xml.addTag("PointsSetting");
		xml.addChildAttrib("Points", i->first);
		xml.addChildAttrib("Action", i->second.first);
		xml.addChildAttrib("DisplayCheat", i->second.second);
	}
	xml.stepOut();
}
//Raw Selector class
void RawSelector::createList() {
	Action::List lst = RawManager::getInstance()->getActionList();

	int j = 0;
	idAction.insert(make_pair(j, j));
	for(Action::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		idAction.insert(make_pair(++j, i->second->getActionId()));
	}
}

int RawSelector::getId(int actionId) {
	for(ActionList::const_iterator i = idAction.begin(); i != idAction.end(); ++i) {
		if(i->second == actionId)
			return i->first;
	}
	return 0;
}

int RawSelector::getIdAction(int id) {
	ActionList::const_iterator i = idAction.find(id);
	if(i == idAction.end())
		return 0;
	return i->second;
}

/**
 * @file
 * $Id: RawManager.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */