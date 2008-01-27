/*
 * Copyright (C) 2007-2008 adrian_007, adrian-007 on o2 point pl
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

#ifndef PLUGIN_API
#define PLUGIN_API

#include "PluginInterface.h"

#define HOST
#ifdef HOST
#define EXIMP __declspec(dllexport)
#else
#define EXIMP
#endif

class PluginAPI {
public:
	//hub line style
	enum {
		STYLE_GENERAL = 0,
		STYLE_MYOWN,
		SYYLE_SERVER,
		STYLE_SYSTEM,
		STYLE_LOG,
		STYLE_CHEAT
	};
	//tooltip icon style
	enum {
		TT_NONE = 0x00000000,
		TT_INFO = 0x00000001,
		TT_WARNING = 0x00000002,
		TT_ERROR = 0x00000003,
		TT_NOSOUND = 0x00000010
	};
	//versions
	enum {
		CLIENT_PROFILE = 0,
		MYINFO_PROFILE,
		IP_WATCH_PROFILE,
		AUTOSEARCH_PROFILE,
		ADLS_PROFILE,
		RSX_VERSION,
		RSX_REVISION
	};

	EXIMP static void setPluginInfo(int id, const string n, const string v, int i);

	EXIMP static void logMessage(const string& aMsg);
	EXIMP static void logMessage(const wstring& aMsg);

	EXIMP static string fromW(const wstring& aStr);
	EXIMP static wstring toW(const string& aStr);
	EXIMP static string formatParams(const string& frm, std::tr1::unordered_map<string, string>& params);

	EXIMP static string getDataPath();
	EXIMP static string getVersion(int type);
	EXIMP static void getMainWnd(HWND& h);

	EXIMP static const string& getSetting(int id, const string& aName);
	EXIMP static void setSetting(int id, const string& aName, const string& aVal);

	EXIMP static const string& getClientSetting(const string& aName, bool rsxmng = false);
	EXIMP static int getClientSettingInt(const string& aName, bool rsxmng = false);

	EXIMP static void showToolTip(const string& pTitle, const string& pMsg, int pIcon);
	EXIMP static void showToolTip(const wstring& pTitle, const wstring& pMsg, int pIcon);
};
#endif

/**
 * @file
 * $Id$
 */