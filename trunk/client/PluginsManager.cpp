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
	udp.create(Socket::TYPE_UDP);

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
	getSpeaker().addCaller(&UserConnection::ucCallFunc);
}

PluginsManager::~PluginsManager() {
	TimerManager::getInstance()->removeListener(this);
	close();
	delete dcpp_func;
}

void PluginsManager::loadPlugin(Plugin*& p, HINSTANCE dll) throw(Exception) {
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
				throw Exception("Only one copy of this plugin is allowed");
		}
	}

	if(VER_MAJOR(nfo->sdkVersion) != VER_MAJOR(SDK_VERSION) || 
		VER_MINOR(nfo->sdkVersion) != VER_MINOR(SDK_VERSION) ||
		VER_REVISION(nfo->sdkVersion) != VER_REVISION(SDK_VERSION))
		throw Exception("Plugin is compiled with old version of PluginSDK");

	if(pLoad(dcpp_func) != DCPP_FALSE) {
		pUnload();
		throw Exception("Unknown exception while calling pluginLoad function");
	}
}

void PluginsManager::init(void (*f)(void*, const tstring&), void* pv) {
	Lock l(cs);
	StringList libs = File::findFiles(Util::getPath(Util::PATH_GLOBAL_CONFIG) + "Plugins" PATH_SEPARATOR_STR, "*.dll");
	for(StringIter i = libs.begin(); i != libs.end(); ++i) {
		const tstring& fname = Text::toT(*i);
		HINSTANCE dll = LoadLibrary(fname.c_str());
		if(dll) {
			Plugin* p = 0;
			try {
				loadPlugin(p, dll);
				plugins.push_back(p);
				if(f && pv)
					(*f)(pv, Text::toT(Util::getFileName(*i)));
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
	getSpeaker().speak(DCPP_EVENT_CORE, DCPP_EVENT_CORE_LOAD, 0, 0);
}

void PluginsManager::close() {
	getSpeaker().speak(DCPP_EVENT_CORE, DCPP_EVENT_CORE_UNLOAD, 0, 0);
	getSpeaker().cleanup();

	Lock l(cs);
	for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		FreeLibrary((*i)->handle);
		delete *i;
	}
	plugins.clear();
}

dcpp_param PluginsManager::dcppBuffer_strcpy(const string& str, dcppBuffer* buf) {
	if(!buf || buf->buf == 0 || buf->size == 0)
		return 0;
	dcpp_param len = buf->size;
	if(str.size() < len)
		len = str.size();
	if(len > 0)
		memcpy(buf->buf, &str[0], len);
	return len;
}

dcpp_param PluginsManager::coreCallFunc(const char* type, dcpp_param p1, dcpp_param p2, dcpp_param p3, int* handled) {
	*handled = DCPP_TRUE;
	if(strncmp(type, "Core/", 5) == 0) {
		if(strncmp(type+5, "Setting/", 8) == 0) {
			const char* name = reinterpret_cast<const char*>(p1);
			if(strncmp(type+13, "Plug/", 5) == 0) {
				if(strncmp(type+18, "Get", 3) == 0) {
					dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p2);
					return dcppBuffer_strcpy(rsxppSettingsManager::getInstance()->getExtSetting(name), buf);
				} else if(strncmp(type+18, "Set", 3) == 0) {
					rsxppSettingsManager::getInstance()->setExtSetting(name, reinterpret_cast<const char*>(p2));
					return DCPP_TRUE;
				}
			} else if(strncmp(type+13, "dcpp/", 5) == 0) {
				int settingType, key;
				key = SettingsManager::getInstance()->findKey(reinterpret_cast<const char*>(p1), settingType);
				if(strncmp(type+18, "Get", 3) == 0) {
					switch(settingType) {
						case 1: {
							*reinterpret_cast<dcpp_param*>(p2) = static_cast<dcpp_param>(SettingsManager::getInstance()->get((SettingsManager::IntSetting)key));
							break;
						}
						case 2: {
							*reinterpret_cast<dcpp_param*>(p2) = static_cast<dcpp_param>(SettingsManager::getInstance()->get((SettingsManager::Int64Setting)key));
							break;
						}
						case 3: {
							*reinterpret_cast<dcpp_param*>(p2) = reinterpret_cast<dcpp_param>(SettingsManager::getInstance()->get((SettingsManager::StrSetting)key).c_str());
							break;
						}
						default: {
							return DCPP_SETTINGS_TYPE_NOT_FOUND;
						}
					}
					return (dcpp_param)settingType;
				} else if(strncmp(type+18, "Set", 3) == 0) {
					switch(settingType) {
						case 1: {
							SettingsManager::getInstance()->set((SettingsManager::IntSetting)key, static_cast<int>(p2));
							break;
						}
						case 2: {
							SettingsManager::getInstance()->set((SettingsManager::Int64Setting)key, static_cast<int64_t>(p2));
							break;
						}
						case 3: {
							SettingsManager::getInstance()->set((SettingsManager::StrSetting)key, reinterpret_cast<const char*>(p2));
							break;
						}
						default: {
							return DCPP_SETTINGS_TYPE_NOT_FOUND;
						}
					}
					return DCPP_TRUE;
				} else if(strncmp(type+18, "Type", 4) == 0) {
					return (dcpp_param)settingType;
				}
				return DCPP_FALSE;
			}
		}
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

			return dcppBuffer_strcpy(Util::formatParams(reinterpret_cast<char*>(p2), params, false), buf);
		} else if(strncmp(type+6, "WideToUtf8", 10) == 0) {
			const uint16_t* str = reinterpret_cast<const uint16_t*>(p1); // unsigned short - 2 bytes
			dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p2);
			if(!str || !buf || buf->size == 0) return DCPP_FALSE;
			string s = Text::wideToUtf8(wstring((wchar_t*)str));
			size_t len = buf->size;
			if(s.size()*sizeof(wchar_t) < len)
				len = s.size()*sizeof(wchar_t);
			if(len > 0)
				memcpy(buf->buf, &s[0], len);
			return len;
		} else if(strncmp(type+6, "Utf8ToAcp", 9) == 0) {
			const char* str = reinterpret_cast<const char*>(p1);
			dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p2);
			if(!str || !buf || buf->size == 0) return DCPP_FALSE;
			string s = Text::utf8ToAcp(str);
			return dcppBuffer_strcpy(s, buf);
		} else if(strncmp(type+6, "AcpToUtf8", 9) == 0) {
			const char* str = reinterpret_cast<const char*>(p1);
			dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p2);
			if(!str || !buf || buf->size == 0) return DCPP_FALSE;
			string s = Text::acpToUtf8(str);
			return dcppBuffer_strcpy(s, buf);
		} else if(strncmp(type+6, "GetPath", 7) == 0) {
			dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p2);
			int16_t pathType;
			switch(p1) {
				case DCPP_UTILS_PATH_GLOBAL_CONFIG: {
					pathType = Util::PATH_GLOBAL_CONFIG;
					break;
				}
				case DCPP_UTILS_PATH_USER_CONFIG: {
					pathType = Util::PATH_USER_CONFIG;
					break;
				}
				case DCPP_UTILS_PATH_USER_LOCAL: {
					pathType = Util::PATH_USER_LOCAL;
					break;
				}
				case DCPP_UTILS_PATH_RESOURCES: {
					pathType = Util::PATH_RESOURCES;
					break;
				}
				case DCPP_UTILS_PATH_LOCALE: {
					pathType = Util::PATH_LOCALE;
					break;
				}
				case DCPP_UTILS_PATH_DOWNLOADS: {
					pathType = Util::PATH_DOWNLOADS;
					break;
				}
				case DCPP_UTILS_PATH_FILE_LISTS: {
					pathType = Util::PATH_FILE_LISTS;
					break;
				}
				case DCPP_UTILS_PATH_HUB_LISTS: {
					pathType = Util::PATH_HUB_LISTS;
					break;
				}
				case DCPP_UTILS_PATH_NOTEPAD: {
					pathType = Util::PATH_NOTEPAD;
					break;
				}
				case DCPP_UTILS_PATH_EMOPACKS: {
					pathType = Util::PATH_EMOPACKS;
					break;
				}
				default: { 
					pathType = -1;
				}
			}
			if(pathType != -1) {
				string path = Util::getPath((Util::Paths)pathType);
				// append - trick to make own path easily
				if(p3) {
					path += reinterpret_cast<const char*>(p3);
				}
				return dcppBuffer_strcpy(path, buf);
			}
			return DCPP_FALSE;
		} else if(strncmp(type+6, "SendUDP", 7) == 0) {
			string ipPort(reinterpret_cast<const char*>(p1));
			string::size_type i = ipPort.find(":");
			if(i != string::npos) {
				string ip = ipPort.substr(0, i);
				uint16_t port = static_cast<uint16_t>(Util::toInt(ipPort.substr(i+1)));
				try {
					PluginsManager::getInstance()->udp.writeTo(ip, port, reinterpret_cast<const void*>(p2), static_cast<int>(p3));
					return DCPP_TRUE;
				} catch(const SocketException&) {
					// damnit!
				}
			}
			return DCPP_FALSE;
		}
	}
	*handled = DCPP_FALSE;
	return DCPP_FALSE;
}

void PluginsManager::on(TimerManagerListener::Second, uint64_t tick) throw() {
	getSpeaker().speak(DCPP_EVENT_TIMER, DCPP_EVENT_TIMER_TICK_SECOND, (dcpp_param)&tick, 0);
}

void PluginsManager::on(TimerManagerListener::Minute, uint64_t tick) throw() {
	getSpeaker().speak(DCPP_EVENT_TIMER, DCPP_EVENT_TIMER_TICK_MINUTE, (dcpp_param)&tick, 0);
}

} // namespace dcpp

/**
 * @file
 * $Id$
 */