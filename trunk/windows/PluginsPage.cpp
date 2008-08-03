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
#include "resource.h"

#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "../client/PluginsManager.h"

#include "../rsx/rsx-settings/rsx-SettingsManager.h"

#include "PluginsPage.h"

PropPage::Item PluginsPage::items[] = {
//	{ IDC_USE_REGEXP_OR_WILD,	RSXSettingsManager::IGNORE_USE_REGEXP_OR_WC,	PropPage::T_BOOL_RSX },
	{ 0, 0, PropPage::T_END }
};

LRESULT PluginsPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	ctrlTabs.SubclassWindow(GetDlgItem(IDC_PLUGINS_PAGES));
	ctrlTabs.SetParent(m_hWnd);
	const PluginsManager::Plugins p = PluginsManager::getInstance()->getPlugins();
	for(PluginsManager::Plugins::const_iterator i = p.begin(); i != p.end(); ++i) {
		if((*i)->getSettingsWindow() > 0) {
			HWND h = addPage((*i)->getId(), (*i)->getSettingsWindow(), (*i)->getName().c_str(), (*i)->getHandle());
			(*i)->getInterface()->onMainWndEvent(1, h);
		}
	}
	ctrlTabs.SetCurSel(0);
	return TRUE;
}

void PluginsPage::write() {
	for(std::map<int, HWND>::const_iterator j = plugins.begin(); j != plugins.end(); ++j) {
		PluginsManager::getInstance()->onSettingsDlgClose(j->first, j->second);
	}
	plugins.clear();

	for(int i = 0; i < ctrlTabs.m_ctrlViews.GetItemCount(); ++i)
		::DestroyWindow(ctrlTabs.m_ctrlViews.GetItem(i));

}

HWND PluginsPage::addPage(int pId, int resId, const tstring& tabName, HMODULE hInst) {
	HWND dlg = ::CreateDialog(hInst, MAKEINTRESOURCE(resId), m_hWnd, NULL);
	ctrlTabs.AddTab((LPWSTR)tabName.c_str(), dlg, true);
	plugins.insert(make_pair(pId, dlg));
	return dlg;
}