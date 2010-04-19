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

#define _CRT_SECURE_NO_WARNINGS 1

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "sdk/sdk.h"
#include "winamp.h"

#ifdef __cplusplus
extern "C" {
#endif

dcppFunctions* f = NULL;
dcppPluginInformation info = {
	"winamp_spam",
	"{815FF396-586D-4c18-BAC1-D7AD9AB992A7}",
	"adrian_007",
	"simple winamp spam plugin",
	"http://rsxplusplus.sf.net",
	MAKE_VER(1, 1, 0, 0),
	SDK_VERSION
};

void addParam(dcppLinkedMap** map, const char* p1, const char* p2) {
    size_t len;
    dcppLinkedMap* n = (dcppLinkedMap*)malloc(sizeof(dcppLinkedMap));
    len = strlen(p1);
    n->first = (char*)malloc(len+1);
    memset(n->first, 0, len+1);
    strncpy(n->first, p1, len);
    len = strlen(p2);
    n->second = (char*)malloc(len+1);
    memset(n->second, 0, len+1);
    strncpy(n->second, p2, len);

    if(!*map) n->next = NULL;
    else   n->next = *map;
    *map = n;
}

void freeMap(dcppLinkedMap** map) {
	dcppLinkedMap* next = NULL;
	while(*map) {
		next = (*map)->next;
		free((*map)->first);
		free((*map)->second);
		free(*map);
		*map = next;
	}
}

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

int DCPP_CALL_CONV sendSpam(int callReason, dcpp_param lParam, dcpp_param wParam, void* userData) {
    if(callReason == DCPP_EVNET_HUB_CHAT_SEND_LINE) {
        const char* msg = (const char*)wParam;
        if(strncmp(msg, "/winamp", 7) == 0 || strncmp(msg, "/w", 2) == 0) {
            HWND wnd = FindWindowW(L"Winamp v1.x", NULL);
            if(wnd) {
                dcppLinkedMap* params = NULL;
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
                addParam(&params, "version", tmp);
                switch (state) {
                    case 0: addParam(&params, "state", "stopped\0"); break;
                    case 1: addParam(&params, "state", "playing\0"); break;
                    case 3: addParam(&params, "state", "paused\0"); break;
                    default: 
						break;
                }
				memset(tmp, 0, sizeof(tmp));
                sprintf(tmp, "%d %%", intPercent);
                addParam(&params, "percent", tmp);
				
				memset(tmp, 0, sizeof(tmp));
                sprintf(tmp, "%d kbps", waBitRate);
                addParam(&params, "bitrate", tmp);
	
				memset(tmp, 0, sizeof(tmp));
                sprintf(tmp, "%d kHz", waSampleRate);
                addParam(&params, "sample", tmp);

				formatSeconds(curPos, tmp);
				addParam(&params, "elapsed", tmp);

				formatSeconds(length, tmp);
				addParam(&params, "length", tmp);

                switch(waChannels) {
                    case 2: addParam(&params, "channels", "stereo\0"); break;
                    case 6: addParam(&params, "channels", "5.1 surround\0"); break;
                    default:
						addParam(&params, "channels", "mono\0"); break;
                }

				memset(wtmp, 0, sizeof(wtmp));
                GetWindowTextW(wnd, wtmp, sizeof(wtmp));

                {
                    size_t size;
					dcppBuffer tmpBuf;

					tmpBuf.size = 2048 * sizeof(wchar_t);
                    tmpBuf.buf = malloc(tmpBuf.size + 1);
					memset(tmpBuf.buf, 0, tmpBuf.size + 1);
                    size = (size_t)f->call(DCPP_CALL_UTILS_WIDE_TO_UTF8, (dcpp_param)wtmp, (dcpp_param)&tmpBuf, 0);
					memset(tmp, 0, sizeof(tmp));
                    strncpy(tmp, tmpBuf.buf, size);
					free(tmpBuf.buf);
					addParam(&params, "rawtitle", tmp);
                }
				{
					char* pch;
					char title[2048];
					memset(title, 0, sizeof(title));
					pch = strstr(tmp, ". ");
					if(pch) {
						pch += 2;
						strncat(title, pch, strlen(pch) - 9);
					}
					addParam(&params, "title", title);
				}
				{
					char* pch;
					pch = strstr(msg, " ");
					memset(tmp, 0, sizeof(tmp));
					if(pch) {
						strcat(tmp, pch);
					}
					addParam(&params, "userParam", tmp);
				}
				{
					char format[2048];
					char thirdPerson;
					dcppBuffer buf;
					memset(format, 0, sizeof(format));
					{
						dcppBuffer tbuf;
						tbuf.buf = format;
						tbuf.size = 2047;
						f->call(DCPP_CALL_CORE_SETTING_PLUG_GET, (dcpp_param)(char*)"winamp.fmt", (dcpp_param)&tbuf, 0);
					}
					buf.size = 4096;
					buf.buf = (char*)malloc(buf.size + 1);
					memset(buf.buf, 0, buf.size + 1);

					f->call(DCPP_CALL_UTILS_FORMAT_PARAMS, (dcpp_param)params, (dcpp_param)format, (dcpp_param)&buf);
					thirdPerson = strncmp(format, "/me ", 4) == 0;
					if(thirdPerson)
						buf.buf += 4;
					f->call(DCPP_CALL_HUB_SEND_CHAT_MESSAGE, lParam, (dcpp_param)buf.buf, thirdPerson);
					if(thirdPerson)
						buf.buf -= 4;
					free(buf.buf);
					freeMap(&params);
				}
            } else {
                f->call(DCPP_CALL_HUB_CHAT_WINDOW_WRITE, lParam, (dcpp_param)(char*)"Supported version of Winamp is not running!", 6);
            }
            return DCPP_TRUE;
        }
    }
    return DCPP_FALSE;
}

dcppPluginInformation* DCPP_CALL_CONV pluginInfo(unsigned long long coreSdkVersion, int svnRevision) {
    return &info;
}

int DCPP_CALL_CONV pluginLoad(dcppFunctions* pF) {
	f = pF;
	f->addListener(DCPP_EVENT_HUB, sendSpam, NULL);
	f->call(DCPP_CALL_CORE_SETTING_PLUG_SET, (dcpp_param)(char*)"winamp.fmt", (dcpp_param)(char*)"/me is listening to  %[title] (%[elapsed]/%[length])", 0);
	return 0;
}

int DCPP_CALL_CONV pluginUnload() {
	f->removeListener(DCPP_EVENT_HUB, sendSpam);
	f = NULL;
	return 0;
}

#ifdef __cplusplus
}
#endif
