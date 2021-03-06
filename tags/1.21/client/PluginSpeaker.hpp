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

#ifndef RSXPLUSPLUS_PLUGIN_SPEAKER
#define RSXPLUSPLUS_PLUGIN_SPEAKER

#include "sdk/dcpp.h"
#include "Thread.h"

namespace dcpp {

class PluginSpeaker {
public:
	bool addCaller(const dcppCallFunc& fn);
	bool removeCaller(const dcppCallFunc& fn);

	bool addSpeaker(const std::string& type, bool isProtected = true);
	bool removeSpeaker(const std::string& type);
	bool isSpeaker(const std::string& type);

	bool addListener(const std::string& type, const dcppListenerFunc& fn, void* userData);
	bool removeListener(const std::string& type, const dcppListenerFunc& fn);

	int speak(const std::string& type, int callReason, dcpp_param param1, dcpp_param param2);
	dcpp_param call(const char* type, dcpp_param p1, dcpp_param p2, dcpp_param p3);

	void cleanup() {
		sp.clear();
		cp.clear();
		protectedSpeakers.clear();
	}
private:
	typedef std::list<dcppCallFunc> Callers;
	typedef std::list<std::pair<dcppListenerFunc, void*> > Listeners;
	typedef unordered_map<std::string, Listeners> Speakers;

	CriticalSection speakerCs, callerCs;
	Speakers sp;
	Callers cp;
	StringList protectedSpeakers;
};
}

#endif

/**
 * @file
 * $Id: PluginSpeaker.hpp 211 2010-04-14 00:33:36Z adrian_007 $
 */