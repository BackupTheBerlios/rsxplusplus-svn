/* 
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
#define EXIMP __declspec(dllimport)
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

	EXIMP static void setPluginInfo(int id, const string n, const string v, int i);

	EXIMP static void logMessage(const string& aMsg);
	EXIMP static void logMessage(const wstring& aMsg);

	EXIMP static string fromW(const wstring& aStr);
	EXIMP static wstring toW(const string& aStr);

	EXIMP static void addHubLine(Client* client, const string& aMsg, int mType = 0);
	EXIMP static void addHubLine(Client* client, const wstring& aMsg, int mType = 0);

	EXIMP static void sendHubMessage(Client* client, const string& aMsg);
	EXIMP static void sendHubMessage(Client* client, const wstring& aMsg);

	EXIMP static const string& getSetting(int id, const string& aName);
	EXIMP static void setSetting(int id, const string& aName, const string& aVal);

	EXIMP static string formatParams(const string& frm, std::tr1::unordered_map<string, string>& params);

	EXIMP static void getMainWnd(HWND& h);

	EXIMP static void showToolTip(const string& pTitle, const string& pMsg, int pIcon);
	EXIMP static void showToolTip(const wstring& pTitle, const wstring& pMsg, int pIcon);

};
#endif

/**
 * @file
 * $Id: MainFrm.cpp,v 1.20 2004/07/21 13:15:15 bigmuscle Exp
 */