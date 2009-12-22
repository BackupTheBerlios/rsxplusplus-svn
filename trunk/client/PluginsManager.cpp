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

#include "PluginsManager.h"
#include "Plugin.h"

#include "Client.h"
#include "User.h"
#include "UserConnection.h"

#include "sdk/sdk.h"

#include "LogManager.h"
#include "version.h"

namespace dcpp {

PluginsManager::PluginsManager() : dcpp_func(new dcppFunctions) {
	memzero(dcpp_func, sizeof(dcppFunctions));

	dcpp_func->call = &PluginsManager::callFunc;

	dcpp_func->addCaller = &PluginsManager::addCaller;
	dcpp_func->removeCaller = &PluginsManager::removeCaller;

	dcpp_func->addSpeaker = &PluginsManager::addSpeaker;
	dcpp_func->removeSpeaker = &PluginsManager::removeSpeaker;
	dcpp_func->isSpeaker = &PluginsManager::isSpeaker;
	dcpp_func->fireSpeaker = &PluginsManager::speak;

	dcpp_func->addListener = &PluginsManager::addListener;
	dcpp_func->removeListener = &PluginsManager::removeListener;

	TimerManager::getInstance()->addListener(this);

	// prepare speakers (protected as default)
	getSpeaker().addSpeaker("Core/");
	getSpeaker().addSpeaker("User/");
	getSpeaker().addSpeaker("Hub/");
	getSpeaker().addSpeaker("UserConnection/");
	getSpeaker().addSpeaker("Timer/");

	getSpeaker().addCaller(&PluginsManager::coreCallFunc);
	getSpeaker().addCaller(&Client::clientCallFunc);
}

PluginsManager::~PluginsManager() {
	TimerManager::getInstance()->removeListener(this);
	close();
	delete dcpp_func;
}

void PluginsManager::loadPlugin(Plugin*& p, HMODULE dll) throw(Exception) {
	typedef dcppPluginInformation* (__stdcall *plugInfo)(unsigned long long, int);
	p = new Plugin(dll);

	Plugin::PluginLoad pLoad = reinterpret_cast<Plugin::PluginLoad>(GetProcAddress(dll, "pluginLoad"));
	Plugin::PluginUnload pUnload = reinterpret_cast<Plugin::PluginUnload>(GetProcAddress(dll, "pluginUnload"));
	plugInfo pInfo = reinterpret_cast<plugInfo>(GetProcAddress(dll, "pluginInfo"));

	if(!pLoad)
		throw Exception("Missing pluginLoad function");
	if(!pUnload)
		throw Exception("Missing pluginUnload function");
	if(!pInfo)
		throw Exception("Missing pluginInfo function");

	p->pluginLoad = pLoad;
	p->pluginUnload = pUnload;

	dcppPluginInformation* nfo = pInfo(SDK_VERSION, SVN_REVISION);
	if(!nfo)
		throw Exception("Missing plugin information");

	p->info = nfo;

	if(nfo->guid != 0) {
		for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
			if((*i)->info->guid != 0 && strcmp(nfo->guid, (*i)->info->guid) == 0)
				throw Exception("Only one instance of plugin is allowed");
		}
	}

	if(VER_MAJOR(nfo->sdkVersion) != VER_MAJOR(SDK_VERSION) || 
		VER_MINOR(nfo->sdkVersion) != VER_MINOR(SDK_VERSION) ||
		VER_REVISION(nfo->sdkVersion) != VER_REVISION(SDK_VERSION))
		throw Exception("Plugin is compiled with old version of PluginSDK");

	if(pLoad(dcpp_func) != DCPP_FALSE) {
		throw Exception("Unknown exception while calling pluginLoad function");
	}
}

void PluginsManager::init(void (*f)(void*, const tstring&), void* pv) {
	Lock l(cs);
	StringList libs = File::findFiles(Util::getPath(Util::PATH_GLOBAL_CONFIG) + "Plugins" PATH_SEPARATOR_STR, "*.dll");
	for(StringIter i = libs.begin(); i != libs.end(); ++i) {
		const tstring& fname = Text::toT(*i);
		HMODULE dll = LoadLibrary(fname.c_str());
		if(dll) {
			Plugin* p = 0;
			try {
				loadPlugin(p, dll);
				plugins.push_back(p);
				if(f && pv)
					(*f)(pv, fname);
			} catch(const Exception& e) {
				// do a cleanup
				delete p;
				p = 0;
				FreeLibrary(dll);
				LogManager::getInstance()->message(Util::getFileName(*i) + ": " + e.getError());
			}
		}
	}
}

void PluginsManager::getPluginsInfo(std::list<dcppPluginInformation*>& p) {
	Lock l(cs);
	for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
		p.push_back((*i)->info);
}

void PluginsManager::load() {
	getSpeaker().speak(DCPP_EVENT_CORE, DCPP_EVENT_TYPE_CORE_LOAD, 0, 0);
}

void PluginsManager::close() {
	getSpeaker().speak(DCPP_EVENT_CORE, DCPP_EVENT_TYPE_CORE_UNLOAD, 0, 0);

	Lock l(cs);
	for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		FreeLibrary((*i)->handle);
		delete *i;
	}
	plugins.clear();
}

dcpp_ptr_t PluginsManager::coreCallFunc(const char* type, dcpp_ptr_t p1, dcpp_ptr_t p2, dcpp_ptr_t p3, int* handled) {
	*handled = DCPP_TRUE;
	if(strncmp(type, "Core/", 5) == 0) {

	} else if(strncmp(type, "Utils/", 6) == 0) {
		if(strncmp(type+6, "LogMessage", 10) == 0) {
			LogManager::getInstance()->message(string(reinterpret_cast<const char*>(p1)));
			return DCPP_TRUE;
		} else if(strncmp(type+6, "FormatParams", 12) == 0) {
			StringMap params;
			dcppLinkedMap* ptr = reinterpret_cast<dcppLinkedMap*>(p1);
			dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p3);

			while(ptr) {
				params[static_cast<char*>(ptr->first)] = static_cast<char*>(ptr->second);
				ptr = ptr->next;
			}

			string format = Util::formatParams(reinterpret_cast<char*>(p2), params, false);
			if(format.size() < buf->size)
				buf->size = format.size();
			memcpy(buf->buf, &format[0], buf->size);
			return buf->size;
		} else if(strncmp(type+6, "WideToUtf8", 10) == 0) {
			const uint16_t* str = reinterpret_cast<const uint16_t*>(p1); // unsigned short - 2 bytes
			dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p2);
			if(!str || !buf || buf->size == 0) return DCPP_FALSE;
			string s = Text::wideToUtf8(wstring((wchar_t*)str));
			if(s.size() < buf->size)
				buf->size = s.size();
			memcpy(buf->buf, &s[0], buf->size);
			return buf->size;
		}
	}
	*handled = DCPP_FALSE;
	return DCPP_FALSE;
}

void PluginsManager::on(TimerManagerListener::Second, uint64_t tick) throw() {
	getSpeaker().speak(DCPP_EVENT_TIMER, DCPP_EVENT_TYPE_TIMER_TICK_SECOND, (dcpp_ptr_t)&tick, 0);
}

void PluginsManager::on(TimerManagerListener::Minute, uint64_t tick) throw() {
	getSpeaker().speak(DCPP_EVENT_TIMER, DCPP_EVENT_TYPE_TIMER_TICK_MINUTE, (dcpp_ptr_t)&tick, 0);
}

/*
dcpp_ptr_t PluginsManager::callFunc(int type, dcpp_ptr_t p1, dcpp_ptr_t p2, dcpp_ptr_t p3) {
	if(type > DCPP_UTILS) {
		switch(type) {
			case DCPP_UTILS_CONV_UTF8_TO_WIDE: {
				dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p2);
				std::wstring str(Text::utf8ToWide(reinterpret_cast<char*>(p1)));

				if(str.size() < buf->size)
					buf->size = str.size();
				memcpy(buf->buf, &str[0], buf->size);
				return buf->size;
			}
 			case DCPP_UTILS_CONV_WIDE_TO_UTF8: {
				dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p2);
				std::string str(Text::wideToUtf8(reinterpret_cast<wchar_t*>(p1)));

				if(str.size() < buf->size)
					buf->size = str.size();
				memcpy(buf->buf, &str[0], buf->size);
				return buf->size;
			}
			case DCPP_UTILS_FREE_LINKED_MAP: {
				dcppLinkedMap* ptr = reinterpret_cast<dcppLinkedMap*>(p1);
				dcppLinkedMap* next;
				while(ptr) {
					next = ptr->next;
					free(ptr->first);
					free(ptr->second);
					free(ptr);
					ptr = next;
				}
				ptr = 0;
			}
			default:
				return 0;
	} else if(type > DCPP_CONNECTION) {
		UserConnection* uc = reinterpret_cast<UserConnection*>(p1);
		if(!uc) return 0;

		switch(type) {
			case DCPP_CONNECTION_WRITE: {
				uc->sendRaw(reinterpret_cast<char*>(p2));
				break;
			}
			case DCPP_CONNECTION_DISCONNECT: {
				uc->disconnect(p2 ? true : false);
				break;
			}
			case DCPP_CONNECTION_FLAG_GET: {
				return uc->getFlags();
			}
			case DCPP_CONNECTION_FLAG_SET: {
				uc->setFlag(static_cast<Flags::MaskType>(p2));
				break;
			}
			case DCPP_CONNECTION_FLAG_UNSET: {
				uc->unsetFlag(static_cast<Flags::MaskType>(p2));
				break;
			}
			case DCPP_CONNECTION_FLAG_ISSET: {
				if(p3) {
					return uc->isAnySet(static_cast<Flags::MaskType>(p2)) ? 1 : 0;
				} else {
					return uc->isSet(static_cast<Flags::MaskType>(p2)) ? 1 : 0;
				}
				break;
			}
			default:
				return 0;
		}
	} else if(type > DCPP_USER) {
		OnlineUser* ou = reinterpret_cast<OnlineUser*>(p1);
		if(!ou) return 0;
		switch(type) {
			case DCPP_USER_FIELD_GET: {
				if(p3) {
					return ou->getIdentity().isSet(reinterpret_cast<char*>(p2)) ? 1 : 0;
				} else {
					return (dcpp_ptr_t)ou->getIdentity().get(reinterpret_cast<char*>(p2)).c_str();
				}
				break;
			}
			case DCPP_USER_FIELD_SET: {
				ou->getIdentity().set(reinterpret_cast<char*>(p2), string(reinterpret_cast<char*>(p3)));
				break;
			}
			default:
				return 0;
		}
	} else if(type > DCPP_HUB) {
		if(type == DCPP_HUB_OPEN) {
			Client* c = 0;
			if(!p2)
				ClientManager::getInstance()->openHub(reinterpret_cast<char*>(p1));
			else
				c = ClientManager::getInstance()->getClient(reinterpret_cast<char*>(p1));
			return (dcpp_ptr_t)c;
		}
		Client* c = reinterpret_cast<Client*>(p1);
		if(!c)
			return 0;

		switch(type) {
			case DCPP_HUB_FIELD_GET: {
				if(p3) {
					return c->getHubIdentity().isSet(reinterpret_cast<char*>(p2)) ? 1 : 0;
				} else {
					return (dcpp_ptr_t)c->getHubIdentity().get(reinterpret_cast<char*>(p2)).c_str();
				}
				break;
			}
			case DCPP_HUB_FIELD_SET: {
				c->getHubIdentity().set(reinterpret_cast<char*>(p2), string(reinterpret_cast<char*>(p3)));
				break;
			}
			case DCPP_HUB_FIELD_MY_GET: {
				if(p3) {
					return c->getMyIdentity().isSet(reinterpret_cast<char*>(p2)) ? 1 : 0;
				} else {
					return (dcpp_ptr_t)c->getMyIdentity().get(reinterpret_cast<char*>(p2)).c_str();
				}
				break;
			}
			case DCPP_HUB_FIELD_MY_SET: {
				c->getMyIdentity().set(reinterpret_cast<char*>(p2), string(reinterpret_cast<char*>(p3)));
				break;
			}
			case DCPP_HUB_SOCKET_WRITE: {
				c->send(reinterpret_cast<char*>(p2));
				break;
			}
			case DCPP_HUB_SEND_MESSAGE: {
				char* msg = reinterpret_cast<char*>(p2);
				if(strncmp(msg, "/me ", 4) == 0)
					c->hubMessage(string(msg+4), true);
				else
					c->hubMessage(string(msg), false);
				break;
			}
			case DCPP_HUB_SEND_USER_COMMAND: {
				c->sendUserCmd(reinterpret_cast<char*>(p2));
				break;
			}
			case DCPP_HUB_WRITE_LINE: {
				c->addHubLine(reinterpret_cast<char*>(p2), (int)p3);
				break;
			}
			case DCPP_HUB_CLOSE: {
				if(!p2)
					ClientManager::getInstance()->closeHub(c->getHubUrl());
				else
					ClientManager::getInstance()->putClient(c);
				break;
			}
 			case DCPP_HUB_REDIRECT: {
				c->redirect(reinterpret_cast<char*>(p2));
				break;
			}
			default:
				return 0;
		}
	} else if(type > DCPP_CORE) {
		switch(type) {
			case DCPP_CORE_GET_SETTING: {
				if(p3) {
					if(p2 == 0)
						return (dcpp_ptr_t)rsxppSettingsManager::getInstance()->getString(reinterpret_cast<char*>(p1)).c_str();
					else
						return (dcpp_ptr_t)rsxppSettingsManager::getInstance()->getInt(reinterpret_cast<char*>(p1));
				} else {
					if(p2 == 0)
						return (dcpp_ptr_t)SettingsManager::getInstance()->getString(reinterpret_cast<char*>(p1)).c_str();
					else
						return (dcpp_ptr_t)SettingsManager::getInstance()->getInt(reinterpret_cast<char*>(p1));
				}
				break;
			}
			case DCPP_CORE_SET_SETTING: {
				//@todo
				break;
			}
			default:
				return 0;
		}
	}
	return 0;
}
*/
} // namespace dcpp

/**
 * @file
 * $Id$
 */