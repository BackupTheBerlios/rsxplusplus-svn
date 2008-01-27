/*
 * Copyright (C) 2007 cologic, cologic@parsoma.net
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

#if !defined(SCRIPTMANAGER_H)
#define SCRIPTMANAGER_H

#include "DCPlusPlus.h"
#include "Singleton.h"
#include "User.h"
#include "Socket.h"
#include "TimerManager.h"
#include "ClientManagerListener.h"
#include "CriticalSection.h"

#include <lunar.h>

class ScriptManagerListener {
public:
	typedef ScriptManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Types {
		DEBUG_MESSAGE,
	};

	virtual void onAction(Types, const string&) throw() = 0;
};

struct LuaManager {
	static const char className[];
	static Lunar<LuaManager>::RegType methods[];

	LuaManager(lua_State* /* L */) { }
	int SendClientMessage(lua_State* L);
	int SendHubMessage(lua_State* L);
	int GenerateDebugMessage(lua_State* L);
	int GetHubIpPort(lua_State* L);
	int GetHubUrl(lua_State* L);
	int GetClientIp(lua_State* L);
	int SendUDPPacket(lua_State* L);
	int InjectHubMessageNMDC(lua_State* L);
	int InjectHubMessageADC(lua_State* L);
	int HubWindowAttention(lua_State* L);
	int FindWindow(lua_State* L);
	int SendMessage(lua_State* L);
	int DropUserConnection(lua_State* L);
	//RSX++
	int GetRSXSetting(lua_State* L);
	int MessageBox(lua_State* L);
	int SendMessageEx(lua_State* L);
	int IsWindow(lua_State* L);
	int OpenLink(lua_State* L);
	int DecodeURI(lua_State* L);
	//END

	int CreateClient(lua_State* L);
	int DeleteClient(lua_State* L);

	int RunTimer(lua_State* L);

	int GetSetting(lua_State* L);
	int GetAppPath(lua_State* L);

	int ToUtf8(lua_State* L);
	int FromUtf8(lua_State* L);
};

class ScriptInstance {
	bool MakeCallRaw(const string& table, const string& method, int args, int ret) throw();
protected:
	virtual ~ScriptInstance() { }
	static lua_State* L;
	static CriticalSection cs;
	static FastCriticalSection fastCs;

	template <typename T>
	bool MakeCall(const string& table, const string& method, int ret, const T& t) throw() {
		Lock l(cs);
		dcassert(lua_gettop(L) == 0);
		LuaPush(t);
		return MakeCallRaw(table, method, 1, ret);
	}
	template <typename T, typename T2>
	bool MakeCall(const string& table, const string& method, int ret, const T& t, const T2& t2) throw() {
		Lock l(cs);
		dcassert(lua_gettop(L) == 0);
		LuaPush(t);
		LuaPush(t2);
		return MakeCallRaw(table, method, 2, ret);
	}
/*	template <typename T, typename T2, typename T3>
	bool MakeCall(const string& table, const string& method, int ret, const T& t, const T2& t2, const T3& t3) throw() {
		Lock l(cs);
		dcassert(lua_gettop(L) == 0);
		LuaPush(t);
		LuaPush(t2);
		LuaPush(t3);
		return MakeCallRaw(table, method, 3, ret);
	}*/
	template <typename T>
	void LuaPush(T* p) { lua_pushlightuserdata(L, (void*)p); }

	void LuaPush(int i);
	void LuaPush(const string& s);
	bool GetLuaBool();
	string GetClientType(const Client* aClient);
	string colorize(string& aLine);

public:
	void EvaluateFile(const string& fn);
	void EvaluateChunk(const string& chunk);
};

class ScriptManager : public ScriptInstance, public Singleton<ScriptManager>, public Speaker<ScriptManagerListener>,
		private ClientManagerListener, private TimerManagerListener {
public:
	void load();
	void SendDebugMessage(const string& s);
	void onRaw(const string& aRawName, const string& aRaw, const Client* aClient) throw();

	GETSET(bool, timerEnabled, TimerEnabled);
	bool isRunning;
private:
	Socket s;
	friend class Singleton<ScriptManager>;
	ScriptManager();
	~ScriptManager() { lua_close(L); if(timerEnabled) TimerManager::getInstance()->removeListener(this); }

	friend struct LuaManager;
	friend class ScriptInstance;

	void on(ClientConnected, const Client* aClient) throw();
	void on(ClientDisconnected, const Client* aClient) throw();
	void on(Second, uint64_t /* ticks */);
};

#endif // !defined(SCRIPTMANAGER_H)