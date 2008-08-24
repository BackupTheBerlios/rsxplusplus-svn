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
	// HubLine style
	enum {
		STYLE_GENERAL = 0,
		STYLE_MYOWN,
		SYYLE_SERVER,
		STYLE_SYSTEM,
		STYLE_LOG,
		STYLE_CHEAT,
		STYLE_STATUS
	};
	// ToolTip icon style
	enum {
		TT_NONE = 0x00000000,
		TT_INFO = 0x00000001,
		TT_WARNING = 0x00000002,
		TT_ERROR = 0x00000003,
		TT_NOSOUND = 0x00000010
	};
	// Versions
	enum {
		CLIENT_PROFILE = 0,
		USER_INFO_PROFILE,
		IP_WATCH_PROFILE,
		AUTOSEARCH_PROFILE,
		RSX_VERSION,
		RSX_REVISION
	};
	// Main Window Events
	enum {
		TOOLBAR_CLICK = 0,
		SETTINGS_DLG_OPEN,
		SETTINGS_DLG_CLOSE
	};

	// rString conversions
	EXIMP static rString	__cdecl AcpToUtf8(const rString& str);
	EXIMP static rString	__cdecl Utf8ToAcp(const rString& str);
	EXIMP static rString	__cdecl fromWideToUtf8(const rStringW& str);
	EXIMP static rString	__cdecl fromWideToAcp(const rStringW& str);
	EXIMP static rStringW	__cdecl fromUtf8ToWide(const rString& str);
	EXIMP static rStringW	__cdecl fromAcpToWide(const rString& str);

	EXIMP static void		__cdecl logMessage(const rString& aMsg);
	EXIMP static rString	__cdecl getDataPath();
	EXIMP static rString	__cdecl getVersion(int type);
	EXIMP static void		__cdecl getMainWnd(HWND& h);

	EXIMP static rString	__cdecl getSetting(int id, const rString& aName);
	EXIMP static void		__cdecl setSetting(int id, const rString& aName, const rString& aVal);
	EXIMP static rString	__cdecl getClientSetting(const rString& aName, bool rsxmng = false);
	EXIMP static int		__cdecl getClientSettingInt(const rString& aName, bool rsxmng = false);
	EXIMP static void		__cdecl registerHubSetting(const char* name, const rString& aValue, const rString& description);

	EXIMP static void		__cdecl OpenHub(const rString& aHubUrl);
	EXIMP static void		__cdecl CloseHub(const rString& aUrl);

	EXIMP static void		__cdecl showToolTip(const rString& pTitle, const rString& pMsg, int pIcon);

	EXIMP static bool		__cdecl RegexMatch(const rString& strToMatch, const rString& regEx);
	EXIMP static bool		__cdecl WildcardMatch(const rString& strToMatch, const rString& pattern, char delim, bool useSet);

	EXIMP static void		__cdecl AddConnectionFeature(const rString& feat, bool isAdc);
	EXIMP static void		__cdecl RemoveConnectionFeature(const rString& feat, bool isAdc);
	EXIMP static void		__cdecl AddHubFeature(const rString& feat, bool isAdc);

};
} // namespace dcpp

#endif // RSXPLUSPLUS_PLUGIN_API

/**
 * @file
 * $Id$
 */
