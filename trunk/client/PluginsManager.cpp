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
#include "UserConnection.h"

#include "PluginsManager.h"
#include "Plugin.h"

#include "LogManager.h"
#include "ClientManager.h"
#include "rsxppSettingsManager.h"
#include "version.h"

namespace dcpp {

PluginsManager::PluginsManager() : dcpp_func(0) {
	dcpp_func = new dcppFunctions;
	memzero(dcpp_func, sizeof(dcppFunctions));

	// memory managment functions
	dcpp_func->malloc = &malloc;
	dcpp_func->calloc = &calloc;
	dcpp_func->realloc = &realloc;
	dcpp_func->free = &free;

	dcpp_func->call = &PluginsManager::callFunc;

	dcpp_func->addListener = &PluginsManager::addListener;
	dcpp_func->removeListener = &PluginsManager::removeListener;

	TimerManager::getInstance()->addListener(this);
}

PluginsManager::~PluginsManager() {
	TimerManager::getInstance()->removeListener(this);

	Lock l(cs);
	for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		FreeLibrary((*i)->handle);
		delete *i;
	}
	plugins.clear();
	delete dcpp_func;
}

void PluginsManager::init(void (*f)(void*, const tstring&), void* pv) {
	typedef dcppPluginInformation* (__stdcall *plugInfo)(unsigned long long, int);

	StringList libs = File::findFiles(Util::getPath(Util::PATH_GLOBAL_CONFIG) + "Plugins" PATH_SEPARATOR_STR, "*.dll");
	{
		Lock l(cs);
		for(StringIter i = libs.begin(); i != libs.end(); ++i) {
			tstring fname = Text::toT(*i);
			HMODULE dll = LoadLibrary(fname.c_str());
			if(dll) {
				Plugin::PluginLoad pLoad = reinterpret_cast<Plugin::PluginLoad>(GetProcAddress(dll, "pluginLoad"));
				Plugin::PluginUnload pUnload = reinterpret_cast<Plugin::PluginUnload>(GetProcAddress(dll, "pluginUnload"));
				plugInfo pInfo = reinterpret_cast<plugInfo>(GetProcAddress(dll, "pluginInfo"));

				if(pLoad && pUnload && pInfo) {
					dcppPluginInformation* info = pInfo(SDK_VERSION, SVN_REVISION);
					bool allow = 
						VER_MAJOR(info->sdkVersion) == VER_MAJOR(SDK_VERSION) && 
						VER_MINOR(info->sdkVersion) == VER_MINOR(SDK_VERSION) &&
						VER_REVISION(info->sdkVersion) == VER_REVISION(SDK_VERSION);
					// allow fallback, when no breaking changes were made in sdk
					if(allow) {
						if(f)
							(*f)(pv, Util::getFileName(fname));
						Plugin* plugin = new Plugin(dll);
						plugin->pluginLoad = pLoad;
						plugin->pluginUnload = pUnload;
						plugin->info = info;
						if(!pLoad(dcpp_func)) {
							plugins.push_back(plugin);
							continue;
						} else {
							delete plugin;
						}
					} else {
						LogManager::getInstance()->message(Util::getFileName(*i) + " is using old version of SDK!");
					}
				} else {
					LogManager::getInstance()->message(Util::getFileName(*i) + " is not a valid RSX++ plugin!");
				}
				FreeLibrary(dll);
			}
		}
	}
}

void PluginsManager::load() {
	call<false>(coreEvents, DCPP_CORE_INIT_FINISHED, 0);
}

void PluginsManager::initClose() { 
	call<false>(coreEvents, DCPP_CORE_INIT_CLOSE, 0);
}

void* PluginsManager::addPlugListener(int type, dcppListenerFunc f, dcpp_ptr_t pd) {
	PluginsManager::Listener* ll = 0;
	switch(type) {
		case DCPP_CORE: {
			ll = &coreEvents;
			break;
		}
		case DCPP_HUB: {
			ll = &hubEvents;
			break;
		}
		case DCPP_USER: {
			ll = &userEvents;
			break;
		}
		case DCPP_CONNECTION: {
			ll = &connEvents;
			break;
		}
		default:
			return (void*)0;
	}

	if(ll) {
		Lock l(cs);
		for(PluginsManager::Listener::iterator i = ll->begin(); i != ll->end(); ++i) {
			if((*i)->f == f)
				return (void*)0;
		}

		PlugListener* ls = new PlugListener(f, pd);
		ll->push_back(ls);
		return (void*)ls;
	}
	return (void*)0;
}

void PluginsManager::remPlugListener(PlugListener* ls) {
	if(!ls)
		return;

	Lock l(cs);
	deleteListener(hubEvents, ls);
	deleteListener(userEvents, ls);
	deleteListener(connEvents, ls);
	deleteListener(coreEvents, ls);
}

void PluginsManager::deleteListener(Listener& l, PlugListener* ls) {
	PluginsManager::Listener::iterator i = std::find(l.begin(), l.end(), ls);
	if(i != l.end()) {
		l.erase(i);
		delete ls;
		ls = 0;
	}
}

void PluginsManager::removeListener(void* ptr) {
	PluginsManager::getInstance()->remPlugListener(reinterpret_cast<PluginsManager::PlugListener*>(ptr));
}

void* PluginsManager::addListener(int type, dcppListenerFunc f, dcpp_ptr_t pd) {
	return PluginsManager::getInstance()->addPlugListener(type, f, pd);
}

void PluginsManager::getPluginsInfo(std::list<dcppPluginInformation*>& p) {
	Lock l(cs);
	for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
		p.push_back((*i)->info);
}

template<bool breakAtFirst, typename T1, typename T2>
int PluginsManager::call(Listener& l, T1 p1, T2 p2) {
	int ret = DCPP_PROCESS_EVENT;
	if(!l.size()) return ret;

	Lock lock(cs);
	for(PluginsManager::Listener::iterator i = l.begin(); i != l.end(); ++i) {
		int r = (*i)->call<T1, T2>(p1, p2);
		if(r != DCPP_PROCESS_EVENT) {
			if(breakAtFirst) {
				return r;
			} else {
				ret = r;
			}
		}
	}
	return ret;
}

bool PluginsManager::onHubMsgIn(Client* hub, const char* msg) {
	dcppHubMessage m;
	memzero(&m, sizeof(m));
	m.message = msg;
	m.hub = (dcpp_ptr_t)hub;

	int ret = call<false>(hubEvents, DCPP_HUB_MESSAGE_IN, &m);
	return ret != DCPP_PROCESS_EVENT;
}

bool PluginsManager::onHubMsgOut(Client* hub, const char* msg) {
	dcppHubMessage m;
	memzero(&m, sizeof(m));
	m.message = msg;
	m.hub = (dcpp_ptr_t)hub;

	int ret = call<false>(hubEvents, DCPP_HUB_MESSAGE_OUT, &m);
	return ret != DCPP_PROCESS_EVENT;
}

bool PluginsManager::onPMIn(Client* hub, OnlineUser* from, OnlineUser* to, OnlineUser* replyTo, const char* msg, bool thirdPerson) {
	dcppPrivateMessageIn m;
	memzero(&m, sizeof(m));
	m.message = msg;
	m.hub = (dcpp_ptr_t)hub;
	m.from = (dcpp_ptr_t)from;
	m.to = (dcpp_ptr_t)to;
	m.replyTo = (dcpp_ptr_t)replyTo;
	m.thirdPerson = (char)thirdPerson;

	int ret = call<false>(hubEvents, DCPP_HUB_PRIV_MESSAGE_IN, &m);
	return ret != DCPP_PROCESS_EVENT;
}

bool PluginsManager::onPMOut(Client* hub, OnlineUser* to, const char* msg) {
	dcppPrivateMessageOut m;
	memzero(&m, sizeof(m));
	m.message = msg;
	m.hub = (dcpp_ptr_t)hub;
	m.to = (dcpp_ptr_t)to;

	int ret = call<false>(hubEvents, DCPP_HUB_PRIV_MESSAGE_OUT, &m);
	return ret != DCPP_PROCESS_EVENT;
}

bool PluginsManager::onUserConnectionLineIn(UserConnection* uc, const char* line) {
	dcppConnectionMessage m;
	memzero(&m, sizeof(m));
	m.line = line;
	m.connection = (dcpp_ptr_t)uc;
	m.flags = uc->getFlags();

	int ret = call<false>(connEvents, DCPP_CONNECTION_MESSAGE_IN, &m);
	return ret != DCPP_PROCESS_EVENT;
}

bool PluginsManager::onUserConnectionLineOut(UserConnection* uc, const char* line) {
	dcppConnectionMessage m;
	memzero(&m, sizeof(m));
	m.line = line;
	m.connection = (dcpp_ptr_t)uc;
	m.flags = uc->getFlags();

	int ret = call<false>(connEvents, DCPP_CONNECTION_MESSAGE_OUT, &m);
	return ret != DCPP_PROCESS_EVENT;
}

void PluginsManager::onHubConnecting(Client* hub) {
	call<false>(hubEvents, DCPP_HUB_CREATED, hub);
}

void PluginsManager::onHubConnected(Client* hub) {
	call<false>(hubEvents, DCPP_HUB_CONNECTED, hub);
}

void PluginsManager::onHubDisconnected(Client* hub) {
	call<false>(hubEvents, DCPP_HUB_DISCONNECTED, hub);
}

void PluginsManager::onHubRedirect(Client* hub, const char* newUrl) {
	dcppHubRedirect m;
	memzero(&m, sizeof(m));
	m.hub = (dcpp_ptr_t)hub;
	m.url = newUrl;
	call<false>(hubEvents, DCPP_HUB_REDIRECTING, &m);
}

void PluginsManager::onUserConnected(OnlineUser* ou) {
	call<false>(userEvents, DCPP_USER_CONNECTED, ou);
}

void PluginsManager::onUserDisconnected(OnlineUser* ou) {
	call<false>(userEvents, DCPP_USER_DISCONNECTED, ou);
}

void PluginsManager::onLuaInit(lua_State* parser) {
	call<false>(coreEvents, DCPP_CORE_LUA_INIT, parser);
}

void PluginsManager::onConfigChange() {
	call<false>(coreEvents, DCPP_CONFIG_REFRESHED, 0);
}
// in timers use pointer, on 32bit system, you'll get only a half of actuall value
void PluginsManager::on(TimerManagerListener::Second, uint64_t tick) throw() {
	call<false>(coreEvents, DCPP_CORE_TIMER_SECOND, &tick);
}

void PluginsManager::on(TimerManagerListener::Minute, uint64_t tick) throw() {
	call<false>(coreEvents, DCPP_CORE_TIMER_MINUTE, &tick);
}

dcpp_ptr_t PluginsManager::callFunc(int type, dcpp_ptr_t p1, dcpp_ptr_t p2, dcpp_ptr_t p3) {
	if(type > DCPP_UTILS) {
		switch(type) {
			case DCPP_UTILS_LOG_MESSAGE: {
				LogManager::getInstance()->message(string(reinterpret_cast<char*>(p1)));
				return 0;
			}
			case DCPP_UTILS_FORMAT_PARAMS: {
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
			}
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
		}
	} else if(type > DCPP_CONFIG) {
		switch(type) {
			case DCPP_CONFIG_SET: {
				rsxppSettingsManager::getInstance()->setExtSetting(reinterpret_cast<char*>(p1), reinterpret_cast<char*>(p2));
				break;
			}
			case DCPP_CONFIG_GET: {
				return (dcpp_ptr_t)rsxppSettingsManager::getInstance()->getExtSetting(reinterpret_cast<char*>(p1)).c_str();
				break;
			}
			default:
				return 0;
		}
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

} // namespace dcpp

/**
 * @file
 * $Id$
 */