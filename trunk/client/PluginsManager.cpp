/*
 * Copyright (C) 2007-2009 adrian_007, adrian-007 on o2 point pl
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

#include "Client.h"
#include "User.h"

#include "PluginsManager.h"
#include "Plugin.h"

#include "LogManager.h"
#include "ClientManager.h"
#include "sdk/events.h"

namespace dcpp {

PluginsManager::PluginsManager() : dcpp_func(0) {
	dcpp_func = new DCPP_FUNCTIONS;
	memzero(dcpp_func, sizeof(DCPP_FUNCTIONS));

	// memory managment functions
	dcpp_func->malloc = &malloc;
	dcpp_func->calloc = &calloc;
	dcpp_func->realloc = &realloc;
	dcpp_func->free = &free;

	dcpp_func->call = &PluginsManager::callFunc;
	dcpp_func->debug = &dcdebug;
}

void* PluginsManager::callFunc(int type, void* p1, void* p2, void* p3) {
	switch(type) {
		case DCPP_ACT_LOG_MSG: {
			LogManager::getInstance()->message(string(static_cast<char*>(p1)));
			break;
		}
		case DCPP_ACT_FORMAT_PARAMS: {
			StringMap params;
			DCPP_STRING_MAP* first = reinterpret_cast<DCPP_STRING_MAP*>(p1);
			for(; first; first = first->next)
				params[first->first] = first->second;
			return get_c_string(Util::formatParams(static_cast<char*>(p2), params, false));
		}
		case DCPP_USER_FIELD_IS_SET: {
			OnlineUser* ou = reinterpret_cast<OnlineUser*>((dcpp_ptr_t)p1);
			if(ou)
				return ou->getIdentity().isSet(static_cast<char*>(p2)) ? (void*)1 : (void*)0;
			break;
		}
		case DCPP_USER_FIELD_GET: {
			OnlineUser* ou = reinterpret_cast<OnlineUser*>((dcpp_ptr_t)p1);
			if(ou)
				return get_c_string(ou->getIdentity().get(static_cast<char*>(p2)));
			break;
		}
		case DCPP_USER_FIELD_SET: {
			OnlineUser* ou = reinterpret_cast<OnlineUser*>((dcpp_ptr_t)p1);
			if(ou)
				ou->getIdentity().set(static_cast<char*>(p2), string(static_cast<char*>(p3)));
			break;
		}
		case DCPP_HUB_FIELD_IS_SET: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c)
				return c->getHubIdentity().isSet(static_cast<char*>(p2)) ? (void*)1 : (void*)0;
			break;
		}
		case DCPP_HUB_FIELD_GET: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c)
				return get_c_string(c->getHubIdentity().get(static_cast<char*>(p2)));
			break;
		}
		case DCPP_HUB_FIELD_SET: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c)
				c->getHubIdentity().set(static_cast<char*>(p2), string(static_cast<char*>(p3)));
			break;
		}
 		case DCPP_HUB_FIELD_MY_IS_SET: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c)
				return c->getMyIdentity().isSet(static_cast<char*>(p2)) ? (void*)1 : (void*)0;
			break;
		}
		case DCPP_HUB_FIELD_MY_GET: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c)
				return get_c_string(c->getMyIdentity().get(static_cast<char*>(p2)));
			break;
		}
		case DCPP_HUB_FIELD_MY_SET: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c)
				c->getMyIdentity().set(static_cast<char*>(p2), string(static_cast<char*>(p3)));
			break;
		}
		case DCPP_HUB_SEND_SOCKET: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c)
				c->send(static_cast<char*>(p2));
			break;
		}
		case DCPP_HUB_SEND_MESSAGE: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c)
				c->hubMessage(string(static_cast<char*>(p2)), p3 ? true : false);
			break;
		}
		case DCPP_HUB_SEND_USER_CMD: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c)
				c->sendUserCmd(static_cast<char*>(p2));
			break;
		}
		case DCPP_HUB_CLOSE: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c) {
				if(!p2)
					c->closeHub();
				else
					ClientManager::getInstance()->putClient(c);
			}
			break;
		}
 		case DCPP_HUB_REDIRECT: {
			Client* c = reinterpret_cast<Client*>((dcpp_ptr_t)p1);
			if(c)
				c->redirect(static_cast<char*>(p2));
			break;
		}
		case DCPP_HUB_OPEN: {
			Client* c = ClientManager::getInstance()->getClient(static_cast<char*>(p1));
			if(p2) {
				//@todo with frame
			}
			return (void*)c;
		}
		default: break;
	}
	return 0;
}

void PluginsManager::init(void (*f)(void*, const tstring&), void* pv) {
	typedef DCPP_PLUG_INFO* (__cdecl *plugInfo)(int, int);

	StringList libs = File::findFiles(Util::getDataPath() + "Plugins" PATH_SEPARATOR_STR, "*.dll");

	Lock l(cs);
	for(StringIter i = libs.begin(); i != libs.end(); ++i) {
		tstring fname = Text::toT(*i);
		HMODULE dll = LoadLibrary(fname.c_str());
		if(dll != NULL) {
			Plugin::PluginProc p = reinterpret_cast<Plugin::PluginProc>(GetProcAddress(dll, "pluginProc"));
			plugInfo pInfo = reinterpret_cast<plugInfo>(GetProcAddress(dll, "pluginInfo"));

			if(p && pInfo) {
				if(f != NULL)
					(*f)(pv, Util::getFileName(fname));
				Plugin* plugin = new Plugin(dll);
				plugin->pluginProc = p;
				plugin->info = pInfo(0,0);
				plugins.push_back(plugin);
			} else {
				FreeLibrary(dll);
			}
		}
	}
}

void PluginsManager::load() {
	plugEvent(DCPP_INIT_OPEN, dcpp_func, 0, 0);
}

void PluginsManager::unload() {
	plugEvent(DCPP_INIT_CLOSE, 0, 0, 0);
}

void PluginsManager::getPluginsInfo(std::list<DCPP_PLUG_INFO*>& p) {
	Lock l(cs);
	for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
		p.push_back((*i)->info);
}

int PluginsManager::plugEventAll(int type, void* p1, void* p2, void* p3) {
	Lock l(cs);
	int ret = DCPP_PROCESS_EVENT;
	for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		int r = (*i)->pluginProc(type, p1, p2, p3);
		if(r != DCPP_PROCESS_EVENT)
			ret = r;
	}
	return ret;
}

int PluginsManager::plugEventBreakAtFirst(int type, void* p1, void* p2, void* p3) {
	Lock l(cs);
	for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		int r = (*i)->pluginProc(type, p1, p2, p3);
		if(r != DCPP_PROCESS_EVENT)
			return r;
	}
	return DCPP_PROCESS_EVENT;
}

} // namespace dcpp

/**
 * @file
 * $Id$
 */