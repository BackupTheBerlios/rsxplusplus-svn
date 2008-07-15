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

#ifndef RAW_MANAGER_H
#define RAW_MANAGER_H

#include "CriticalSection.h"
#include "Singleton.h"
#include "ActionRaw.h"
#include "../rsx/rsx-settings/rsx-SettingsManager.h"

namespace dcpp {
class SimpleXML;
class RawManager : public Singleton<RawManager>, private RSXSettingsManagerListener {
public:
	typedef std::map<int, pair<int, bool> > ADLPoints;

	Action::List& getActionList() { Lock l(cs); return action; }
	Action::RawsList getRawList(int id);
	Action::RawsList getRawListActionId(int actionId);
	Action::Raw addRaw(int id, const string& name, const string& raw, int time, bool lua) throw(Exception);

	int addAction(int actionId, const string& name, bool actif) throw(Exception);
	int getValidAction(int actionId);
	int getActionId(int id);

	void renameAction(const string& oName, const string& nName) throw(Exception);
	void setActifAction(int id, bool actif);
	void removeAction(int id);
	void addRaw(int idAction, int rawId, const string& name, const string& raw, int time, bool actif, bool lua);
	void changeRaw(int id, const string& oName, const string& nName, const string& raw, int time, bool lua) throw(Exception);
	void getRawItem(int id, int idRaw, Action::Raw& ra, bool favHub = false);
	void setActifRaw(int id, int idRaw, bool actif);
	void removeRaw(int id, int idRaw);

	void loadActionRaws();
	void saveActionRaws();

	bool moveRaw(int id, int idRaw, int pos);
	bool getActifActionId(int actionId);
	tstring getNameActionId(int actionId);
	string getRawCommand(int pos, int rawPos);

	//custom points system
	void calcADLAction(int aPoints, int& a, bool& d);
	void remADLPoints(int) { }

	bool addADLPoints(int, int, bool) {/*
		if(aPoints >= 0)
			return false;
		Lock l(cs);
		ADLPoints::iterator i = points.find(aPoints);
		if(i != points.end()) {
			return false;
		}
		points.insert(make_pair(aPoints, make_pair(aAction, aDisp)));
		return true;*/
		return false;
	}

	ADLPoints& getADLPoints() { Lock l(cs); return points; }

private:
	friend class Singleton<RawManager>;

	RawManager();
	~RawManager();

	void loadActionRaws(SimpleXML& aXml);

	void on(RSXSettingsManagerListener::Load, SimpleXML& xml) throw();
	void on(RSXSettingsManagerListener::Save, SimpleXML& xml) throw();

	Action::List action;
	ADLPoints points;
	CriticalSection cs;
	uint16_t lastAction;
};

class RawSelector {
protected:
	typedef unordered_map<int, int> ActionList;
	ActionList idAction;

	void createList();
	int getId(int actionId);
	int getIdAction(int id);
};
} // namespace dcpp
#endif //RAW_MANAGER_H

/**
 * @file
 * $Id: RawManager.h 42 2007-10-31 18:27:40Z adrian_007 $
 */