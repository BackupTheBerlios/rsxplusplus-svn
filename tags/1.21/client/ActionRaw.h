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
struct Raw {
	Raw() : id(0), time(0), lua(false), enabled(false) { };
	Raw(int _id, const std::string& _name, const std::string& _raw, int _time, bool _enabled, bool _lua) 
		throw() : id(_id), name(_name), raw(_raw), time(_time), enabled(_enabled), lua(_lua) { };
	Raw(const Raw& rhs) : id(rhs.id), name(rhs.name), raw(rhs.raw), time(rhs.time), enabled(rhs.enabled), lua(rhs.lua) { }
	Raw& operator=(const Raw& rhs) { 
		id = rhs.id; name = rhs.name; raw = rhs.raw; time = rhs.time; enabled = rhs.enabled; lua = rhs.lua;
		return *this;
	}

	GETSET(int, id, Id);
	GETSET(std::string, name, Name);
	GETSET(std::string, raw, Raw);
	GETSET(int, time, Time);
	GETSET(bool, enabled, Enabled);
	GETSET(bool, lua, Lua);
};

struct Action {
	typedef vector<Raw> RawsList;
	typedef std::vector<Action*> ActionList;

	Action() : id(0) { };
	Action(int _id, const std::string& _name, bool _enabled) throw() : id(_id), name(_name), enabled(_enabled) { };
	~Action() { raw.clear(); };

	GETSET(int, id, Id);
	GETSET(std::string, name, Name);
	GETSET(bool, enabled, Enabled);

	RawsList raw;
};
}
#endif