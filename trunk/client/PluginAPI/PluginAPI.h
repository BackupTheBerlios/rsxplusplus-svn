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

#ifndef RSXPLUSPLUS_PLUGIN_API
#define RSXPLUSPLUS_PLUGIN_API

#ifndef EXIMP
#define EXIMP
#endif//EXIMP

namespace dcpp {
// set of functions to interact with core
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

	EXIMP static void __fastcall logMessage(const rString& aMsg);

	EXIMP static rString __fastcall getDataPath();
	EXIMP static rString __fastcall getVersion(int type);
	EXIMP static void __fastcall getMainWnd(HWND& h);

	EXIMP static rString __fastcall getSetting(int id, const rString& aName);
	EXIMP static void __fastcall setSetting(int id, const rString& aName, const rString& aVal);

	EXIMP static rString __fastcall getClientSetting(const rString& aName, bool rsxmng = false);
	EXIMP static int __fastcall getClientSettingInt(const rString& aName, bool rsxmng = false);

	EXIMP static void __fastcall OpenHub(const rString& aHubUrl);
	EXIMP static void __fastcall CloseHub(const rString& aUrl);

	EXIMP static void __fastcall showToolTip(const rString& pTitle, const rString& pMsg, int pIcon);

	EXIMP static bool __fastcall RegExMatch(const rString& strToMatch, const rString& regEx, const char* opt = "");
	EXIMP static bool __fastcall WildcardMatch(const rString& strToMatch, const rString& pattern, char delim, bool useSet);
};
} // namespace dcpp

#endif // RSXPLUSPLUS_PLUGIN_API

/**
 * @file
 * $Id: PluginAPI.h 61 2008-03-09 17:26:28Z adrian_007 $
 */
