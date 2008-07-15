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

#ifndef RSXPLUSPLUS_ACTION_RAW
#define RSXPLUSPLUS_ACTION_RAW

#include "Util.h"
namespace dcpp {
struct Action {
	typedef Action* Ptr;
	typedef unordered_map<int, Ptr> List;

	Action() : lastRaw(0), actionId(0) { };
	Action(int aActionId, const string& aName, bool aActif) throw() : actionId(aActionId), name(aName), actif(aActif) { };
	~Action() { };

	GETSET(int, actionId, ActionId);
	GETSET(string, name, Name);
	GETSET(bool, actif, Actif);

	struct Raw {
		Raw() : id(0), rawId(0), time(0), lua(false) { };

		Raw(int aId, int aRawId, const string& aName, const string& aRaw, int aTime, bool aActif, bool aLua) 
			throw() : id(aId), rawId(aRawId), name(aName), raw(aRaw), time(aTime), actif(aActif), lua(aLua) { };
		Raw(const Raw& rhs) : id(rhs.id), rawId(rhs.rawId), name(rhs.name), raw(rhs.raw), time(rhs.time), actif(rhs.actif), lua(rhs.lua) { }
		Raw& operator=(const Raw& rhs) { id = rhs.id; rawId = rhs.rawId; name = rhs.name; raw = rhs.raw;
		time = rhs.time; actif = rhs.actif; lua = rhs.lua;
			return *this;
		}

		GETSET(int, id, Id);
		GETSET(int, rawId, RawId);
		GETSET(string, name, Name);
		GETSET(string, raw, Raw);
		GETSET(int, time, Time);
		GETSET(bool, actif, Actif);
		GETSET(bool, lua, Lua);
	};

	typedef vector<Raw> RawsList;
	RawsList raw;
	uint16_t lastRaw;
};
}
#endif