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

#include "forward.h"
#include "CriticalSection.h"
#include "Singleton.h"
#include "TimerManager.h"
#include "Util.h"
#include "Client.h"

class Action {
public:
	typedef Action* Ptr;
	typedef unordered_map<int, Ptr> List;

	Action() : lastRaw(0), actionId(0) { };
	Action(int aActionId, const string& aName, bool aActif) throw() : actionId(aActionId), name(aName), actif(aActif) { };
	~Action() { };

	GETSET(int, actionId, ActionId);
	GETSET(string, name, Name);
	GETSET(bool, actif, Actif);

	class Raw {
	public:
		Raw() : id(0), rawId(0), time(0) { };

		Raw(int aId, int aRawId, const string& aName, const string& aRaw, int aTime, bool aActif) 
			throw() : id(aId), rawId(aRawId), name(aName), raw(aRaw), time(aTime), actif(aActif) { };
		Raw(const Raw& rhs) : id(rhs.id), rawId(rhs.rawId), name(rhs.name), raw(rhs.raw), time(rhs.time), actif(rhs.actif) { }
		Raw& operator=(const Raw& rhs) { id = rhs.id; rawId = rhs.rawId; name = rhs.name; raw = rhs.raw;
			time = rhs.time; actif = rhs.actif;
			return *this;
		}

		GETSET(int, id, Id);
		GETSET(int, rawId, RawId);
		GETSET(string, name, Name);
		GETSET(string, raw, Raw);
		GETSET(int, time, Time);
		GETSET(bool, actif, Actif);
	};

	typedef vector<Raw> RawsList;
	RawsList raw;
	uint16_t lastRaw;
};

class SimpleXML;
class RawManager : public Singleton<RawManager>, public TimerManagerListener {
public:
	Action::List& getActionList() { Lock l(act); return action; }
	Action::RawsList getRawList(int id);
	Action::RawsList getRawListActionId(int actionId);
	Action::Raw addRaw(int id, const string& name, const string& raw, int time) throw(Exception);

	int addAction(int actionId, const string& name, bool actif) throw(Exception);
	int getValidAction(int actionId);
	int getActionId(int id);

	void renameAction(const string& oName, const string& nName) throw(Exception);
	void setActifAction(int id, bool actif);
	void removeAction(int id);
	void addRaw(int idAction, int rawId, const string& name, const string& raw, int time, bool actif);
	void changeRaw(int id, const string& oName, const string& nName, const string& raw, int time) throw(Exception);
	void getRawItem(int id, int idRaw, Action::Raw& ra, bool favHub = false);
	void setActifRaw(int id, int idRaw, bool actif);
	void removeRaw(int id, int idRaw);

	void loadActionRaws();
	void saveActionRaws();

	void addRaw(uint64_t time, string aRaw, Client* c) { 
		raw.insert(make_pair(time, make_pair(aRaw, c))); 
	}

	bool moveRaw(int id, int idRaw, int pos);
	bool getActifActionId(int actionId);
	tstring getNameActionId(int actionId);
	string getRawCommand(int pos, int rawPos);

private:
	RawManager();
	~RawManager();

	friend class Singleton<RawManager>;
	void loadActionRaws(SimpleXML& aXml);

	Action::List action;
	CriticalSection act;
	uint16_t lastAction;

	typedef map<uint64_t, pair<string, Client::Ptr>> ListRaw;
	ListRaw raw;

	// TimerManagerListener
	void on(TimerManagerListener::Second, uint64_t aTick) throw();
};

class RawSelector {
protected:
	typedef unordered_map<int, int> ActionList;
	ActionList idAction;

	void createList();
	int getId(int actionId);
	int getIdAction(int id);
};
#endif //RAW_MANAGER_H

/**
 * @file
 * $Id: RawManager.h 42 2007-10-31 18:27:40Z adrian_007 $
 */