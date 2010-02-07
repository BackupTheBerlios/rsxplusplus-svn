/*
 * Copyright (C) 2007-2010 adrian_007, adrian-007 on o2 point pl
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

#ifndef RSXPLUSPLUS_PLUGINS_MANAGER
#define RSXPLUSPLUS_PLUGINS_MANAGER

#include "Singleton.h"
#include "TimerManager.h"
#include "PluginSpeaker.hpp"
#include "Thread.h"

#include "sdk/dcpp.h"

namespace dcpp {
class Plugin;
class Exception;

class PluginsManager : public Singleton<PluginsManager>, public TimerManagerListener {
public:
	PluginsManager();
	~PluginsManager();

	void init(void (*f)(void*, const tstring&), void* p);
	inline void init() {
		init(0, 0);
	}

	void load();
	void close();

	void getPluginsInfo(std::list<dcppPluginInformation*>& p);

	PluginSpeaker& getSpeaker() { return speaker; }
private:
	typedef std::list<Plugin*> Plugins;

	void loadPlugin(Plugin*& p, HMODULE dll) throw(Exception);

	dcppFunctions* dcpp_func;
	CriticalSection cs;
	PluginSpeaker speaker;
	Plugins plugins;

	static dcpp_ptr_t DCPP_CALL_CONV coreCallFunc(const char* type, dcpp_ptr_t p1, dcpp_ptr_t p2, dcpp_ptr_t p3, int* handled);

	// functions to manage caller/speaker/listener system
	static dcpp_ptr_t DCPP_CALL_CONV callFunc(const char* type, dcpp_ptr_t p1, dcpp_ptr_t p2, dcpp_ptr_t p3) {
		return PluginsManager::getInstance()->getSpeaker().call(type, p1, p2, p3);
	}

	static int DCPP_CALL_CONV addCaller(dcppCallFunc fn) {
		return PluginsManager::getInstance()->getSpeaker().addCaller(fn) ? DCPP_TRUE : DCPP_FALSE;
	}
	static int DCPP_CALL_CONV removeCaller(dcppCallFunc fn) {
		return PluginsManager::getInstance()->getSpeaker().removeCaller(fn) ? DCPP_TRUE : DCPP_FALSE;
	}
	static dcpp_ptr_t DCPP_CALL_CONV call(const char* type, dcpp_ptr_t p1, dcpp_ptr_t p2, dcpp_ptr_t p3) {
		return PluginsManager::getInstance()->getSpeaker().call(type, p1, p2, p3);
	}

	static int DCPP_CALL_CONV addSpeaker(const char* type) {
		return PluginsManager::getInstance()->getSpeaker().addSpeaker(string(type), false) ? DCPP_TRUE : DCPP_FALSE;
	}

	static int DCPP_CALL_CONV removeSpeaker(const char* type) {
		return PluginsManager::getInstance()->getSpeaker().removeSpeaker(string(type)) ? DCPP_TRUE : DCPP_FALSE;
	}

	static int DCPP_CALL_CONV isSpeaker(const char* type) {
		return PluginsManager::getInstance()->getSpeaker().isSpeaker(string(type)) ? DCPP_TRUE : DCPP_FALSE;
	}

	static int DCPP_CALL_CONV addListener(const char* type, dcppListenerFunc fn) {
		return PluginsManager::getInstance()->getSpeaker().addListener(string(type), fn) ? DCPP_TRUE : DCPP_FALSE;
	}

	static int DCPP_CALL_CONV removeListener(const char* type, dcppListenerFunc fn) {
		return PluginsManager::getInstance()->getSpeaker().removeListener(string(type), fn) ? DCPP_TRUE : DCPP_FALSE;
	}

	static int DCPP_CALL_CONV speak(const char* type, int callReason, dcpp_ptr_t param1, dcpp_ptr_t param2) {
		return PluginsManager::getInstance()->getSpeaker().speak(string(type), callReason, param1, param2);
	}

	void on(TimerManagerListener::Second, uint64_t /*tick*/) throw();
	void on(TimerManagerListener::Minute, uint64_t /*tick*/) throw();
};
} // namespace dcpp

#endif

/**
 * @file
 * $Id$
 */
