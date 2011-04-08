/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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

#define _CRT_SECURE_NO_WARNINGS 1

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <map>

typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#include "sdk/Core.hpp"
#include "sdk/PluginInfo.hpp"
#include "sdk/Hub.hpp"
#include "sdk/User.hpp"
#include "sdk/Identity.hpp"
#include "sdk/ChatMessage.hpp"
#include "sdk/String.hpp"

#include "winamp.h"

#ifdef __cplusplus
extern "C" {
#endif

dcpp::interfaces::PluginInfo info = {
	"winamp_spam",
	"{815FF396-586D-4c18-BAC1-D7AD9AB992A7}",
	"adrian_007",
	"Support for /w command on main chat to send info about song from Winamp",
	"http://rsxplusplus.sf.net",
	0,
	MAKE_VER(1, 2, 1, 0),
	SDK_VERSION
};

void formatSeconds(int aSec, char* buf) {
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%01lu:%02d:%02d", (unsigned long)(aSec / (60*60)), (int)((aSec / 60) % 60), (int)(aSec % 60));
}
// params
//%[version] 
//%[percent] 
//%[bitrate] 
//%[sample] 
//%[channels] 
//%[rawtitle] 
//%[title] 
//%[elapsed] 
//%[length]
//%[userParam]

using dcpp::interfaces::RefString;
using dcpp::interfaces::RefStringMap;

class Plugin : public dcpp::interfaces::HubManagerListener, dcpp::interfaces::HubListener {
public:
	Plugin(dcpp::interfaces::Core* c, void* h) : core(c), handle(h) { 
		core->addEventListener(this);
		core->setPluginSetting("winamp.fmt", "/me %[title] (%[elapsed]/%[length])");

	}
	~Plugin() {
		core->remEventListener(this);
	}

private:
	typedef std::map<std::string, std::string> StringMap;

	void onHubManager_HubConnected(dcpp::interfaces::Hub* h) throw() { 
		h->addEventListener(this);
	}

	void onHubManager_HubDisconnected(dcpp::interfaces::Hub* h) throw() { 
		h->remEventListener(this);
	}

	void onHub_OutgoingMessage(dcpp::interfaces::Hub* h, dcpp::interfaces::ChatMessage* cm, bool& handled) throw() {
		if(!handled) {
			std::string line = cm->getText();
			if(line.compare("/w") == 0 || line.compare("/winamp") == 0 || line.compare(0, 3, "/w ") == 0 || line.compare(0, 8, "/winamp ") == 0) {
				std::string param;
				std::string::size_type i = line.find(" ");
				if(i != std::string::npos) {
					param = line.substr(i);
				}

				sendSpam(h, core->getPluginSetting("winamp.fmt"), param, cm->getReplyTo());
				handled = true;
			}
		}
	}

	void sendSpam(dcpp::interfaces::Hub* h, const std::string& format, const std::string& param, dcpp::interfaces::OnlineUser* replyTo) {
		HWND wnd = FindWindowW(L"Winamp v1.x", NULL);
		if(wnd) {
			RefStringMap params(core->getMemoryManager()->getStringMap());

			int waVersion;
			int state;
			int curPos;
			int length;
			int waSampleRate;
			int waBitRate;
			int waChannels;
			int majorVersion;
			int minorVersion;
			int intPercent;
			char tmp[2048];
			wchar_t wtmp[2048];

			waVersion = (int)SendMessage(wnd, WM_USER, 0, IPC_GETVERSION);
			state = (int)SendMessage(wnd ,WM_USER, 0, IPC_ISPLAYING);
			curPos = (int)SendMessage(wnd, WM_USER, 0, IPC_GETOUTPUTTIME);
			length = (int)SendMessage(wnd, WM_USER, 1, IPC_GETOUTPUTTIME);
			waSampleRate = (int)SendMessage(wnd, WM_USER, 0, IPC_GETINFO);
			waBitRate = (int)SendMessage(wnd, WM_USER, 1, IPC_GETINFO);
			waChannels = (int)SendMessage(wnd, WM_USER, 2, IPC_GETINFO);

			majorVersion = waVersion >> 12;
			if (((waVersion & 0x00F0) >> 4) == 0) {
				minorVersion = ((waVersion & 0x0f00) >> 8) * 10 + (waVersion & 0x000f);
			} else {
				minorVersion = ((waVersion & 0x00f0) >> 4) * 10 + (waVersion & 0x000f);
			}
			if (curPos == -1) {
				curPos = 0;
			} else {
				curPos /= 1000;
			}
			if (length > 0 ) {
				intPercent = curPos * 100 / length;
			} else {
				length = 0;
				intPercent = 0;
			}
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "%.3f", majorVersion + minorVersion / 100.0);
			params->set("version", tmp);

			switch (state) {
				case 0: params->set("state", "stopped"); break;
				case 1: params->set("state", "playing"); break;
				case 3: params->set("state", "paused"); break;
			}

			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "%d %%", intPercent);
			params->set("percent", tmp);

			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "%d kbps", waBitRate);
			params->set("bitrate", tmp);

			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "%d kHz", waSampleRate);
			params->set("sample", tmp);

			formatSeconds(curPos, tmp);
			params->set("elapsed", tmp);

			formatSeconds(length, tmp);
			params->set("length", tmp);

			switch(waChannels) {
				case 2: params->set("channels", "stereo"); break;
				case 6: params->set("channels", "5.1 surround"); break;
				default: params->set("channels", "mono");
			}

			memset(wtmp, 0, sizeof(wtmp));
			GetWindowTextW(wnd, wtmp, sizeof(wtmp));
			RefString rawTitle(core->getUtils()->convertFromWideToUtf8(wtmp));
			params->set("rawtitle", rawTitle->get());

			strncpy(tmp, rawTitle->get(), rawTitle->length());

			{
				char* pch;
				char title[2048];
				memset(title, 0, sizeof(title));
				pch = strstr(tmp, ". ");
				if(pch) {
					pch += 2;
					strncat(title, pch, strlen(pch) - 9);
				}
				params->set("title", title);
			}

			params->set("userParam", param.c_str());

			RefString msg(core->getUtils()->formatParams(format.c_str(), params.get()));
			bool tp = msg->compare(0, 4, "/me ") == 0;
			if(tp) {
				msg->erase(0, 4);
			}
			if(replyTo != 0) {
				h->sendPrivateMessage(replyTo, msg->get(), tp);
			} else {
				h->sendMessage(msg->get(), tp);
			}
		} else {
			// notify
		}
	}

	dcpp::interfaces::Core* core;
	void* handle;
};

Plugin* p = 0;

dcpp::interfaces::PluginInfo* __stdcall pluginInfo(unsigned long long coreSdkVersion, int svnRevision) {
    return &info;
}

int __stdcall pluginLoad(dcpp::interfaces::Core* core, void* pluginHandle) {
	p = new Plugin(core, pluginHandle);
	return 0;
}

int __stdcall pluginUnload() {
	delete p;
	return 0;
}

#ifdef __cplusplus
}
#endif
