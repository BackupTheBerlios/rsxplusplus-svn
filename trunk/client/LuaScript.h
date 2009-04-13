#ifndef RSXPLUSPLUS_LUA_SCRIPT
#define RSXPLUSPLUS_LUA_SCRIPT

#include "Util.h"

namespace dcpp {
class LuaScript {
public:
	LuaScript() { };

	string path;
	inline string getFileName() {
		return Util::getFileName(path);
	}

	bool enabled;
};
}; //namespace dcpp
#endif