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

#include "stdafx.h"
#include "MainDlg.h"
#include "version.h"
#include "../MediaPlayersLib/src/Util.h"

#define _set_text(x, y) SetDlgItemText(x, Util::convertToWide(y).c_str());
#define _get_text(wID, buf) /***/\
	{ int len = ::GetWindowTextLength(GetDlgItem(wID))+1;\
	buf.resize(len);\
	GetDlgItemText(wID, &buf[0], len); }

LRESULT MainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	_set_text(IDC_WINAMP, PluginAPI::getSetting(PLUGIN_ID, "winamp").c_str());
	_set_text(IDC_WMP, PluginAPI::getSetting(PLUGIN_ID, "wmp").c_str());
	_set_text(IDC_ITUNES, PluginAPI::getSetting(PLUGIN_ID, "itunes").c_str());
	_set_text(IDC_MPC, PluginAPI::getSetting(PLUGIN_ID, "mpc").c_str());
	return 0;
}

LRESULT MainDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	string tmp = "";
	wstring buf;

	_get_text(IDC_WINAMP, buf);
	tmp = Util::convertFromWide(buf);
	PluginAPI::setSetting(PLUGIN_ID, "winamp", tmp.c_str());
	tmp.clear(); buf.clear();

	_get_text(IDC_WMP, buf);
	tmp = Util::convertFromWide(buf);
	PluginAPI::setSetting(PLUGIN_ID, "wmp", tmp.c_str());
	tmp.clear(); buf.clear();

	_get_text(IDC_ITUNES, buf);
	tmp = Util::convertFromWide(buf);
	PluginAPI::setSetting(PLUGIN_ID, "itunes", tmp.c_str());
	tmp.clear(); buf.clear();

	_get_text(IDC_MPC, buf);
	tmp = Util::convertFromWide(buf);
	PluginAPI::setSetting(PLUGIN_ID, "mpc", tmp.c_str());
	tmp.clear(); buf.clear();

	EndDialog(wID);
	return 0;
}