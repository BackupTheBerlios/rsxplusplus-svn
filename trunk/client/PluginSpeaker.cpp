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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "CriticalSection.h"
#include "PluginSpeaker.hpp"

namespace dcpp {

bool PluginSpeaker::addCaller(const dcppCallFunc& fn) {
	Lock l(callerCs);
	Callers::iterator i = std::find(cp.begin(), cp.end(), fn);
	if(i == cp.end()) {
		cp.push_back(fn);
		return true;
	}
	return false;
}
		
bool PluginSpeaker::removeCaller(const dcppCallFunc& fn) {
	Lock l(callerCs);
	Callers::iterator i = std::find(cp.begin(), cp.end(), fn);
	if(i != cp.end()) {
		cp.erase(i);
		return true;
	}
	return false;
}

bool PluginSpeaker::addSpeaker(const std::string& type, bool isProtected /*= true*/) {
	Lock l(speakerCs);
	if(sp.find(type) == sp.end()) {
		sp.insert(make_pair(type, Listeners()));
		if(isProtected)
			protectedSpeakers.push_back(type);
		return true;
	}
	return false;
}

bool PluginSpeaker::removeSpeaker(const std::string& type) {
	Lock l(speakerCs);
	if(std::find(protectedSpeakers.begin(), protectedSpeakers.end(), type) != protectedSpeakers.end())
		return false;

	Speakers::iterator i = sp.find(type);
	if(i != sp.end()) {
		sp.erase(i);
		return true;
	}
	return false;
}

bool PluginSpeaker::isSpeaker(const std::string& type) {
	Lock l(speakerCs);
	return sp.find(type) != sp.end();
}

bool PluginSpeaker::addListener(const std::string& type, const dcppListenerFunc& fn) {
	Lock l(speakerCs);
	Speakers::iterator i = sp.find(type);
	if(i != sp.end()) {
		Listeners& l = i->second;
		l.push_back(fn);
		return true;
	}
	return false;
}

bool PluginSpeaker::removeListener(const std::string& type, const dcppListenerFunc& fn) {
	Lock l(speakerCs);
	Speakers::iterator i = sp.find(type);
	if(i != sp.end()) {
		Listeners& l = i->second;
		Listeners::iterator j = std::find(l.begin(), l.end(), fn);
		if(j != l.end()) {
			l.erase(j);
			return true;
		}
	}
	return false;
}

int PluginSpeaker::speak(const std::string& type, int callReason, dcpp_ptr_t param1, dcpp_ptr_t param2) {
	int returnCode = DCPP_FALSE;
	Lock l(speakerCs);
	Speakers::const_iterator i = sp.find(type);
	if(i != sp.end()) {
		const Listeners& l = i->second;
		int tmp;
		for(Listeners::const_iterator j = l.begin(); j != l.end(); ++j) {
			const dcppListenerFunc& fn = *j;
			if(!fn)
				continue;
			tmp = fn(callReason, param1, param2);
			if(tmp != DCPP_FALSE)
				returnCode = tmp;
		}
	}
	return returnCode;
}

dcpp_ptr_t PluginSpeaker::call(const char* type, dcpp_ptr_t p1, dcpp_ptr_t p2, dcpp_ptr_t p3) {
	dcpp_ptr_t tmp;
	int handled;
	Lock l(callerCs);
	for(Callers::const_iterator i = cp.begin(); i != cp.end(); ++i) {
		const dcppCallFunc& fn = *i;
		if(!fn) 
			continue;
		handled = DCPP_FALSE;
		tmp = fn(type, p1, p2, p3, &handled);
		if(handled == DCPP_TRUE)
			return tmp;
	}
	return DCPP_FALSE;
}

};

/**
 * @file
 * $Id$
 */