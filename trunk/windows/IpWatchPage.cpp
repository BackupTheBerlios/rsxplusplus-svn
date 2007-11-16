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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "IpWatchPage.h"
#include "IPWatchDlg.h"

#include "../rsx/RsxUtil.h"
#include "../rsx/rsx-settings/rsx-SettingsManager.h"

PropPage::TextItem IpWatchPage::texts[] = {
	{ IDC_ADD_WATCH, ResourceManager::ADD },
	{ IDC_CHANGE_WATCH, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_WATCH, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};
PropPage::Item IpWatchPage::items[] = {
	{ IDC_ENABLE_IPWATCH, RSXSettingsManager::USE_IPWATCH, PropPage::T_BOOL_RSX },
	{ 0, 0, PropPage::T_END }
};

LRESULT IpWatchPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items);

	CRect rc;
	cWatch.Attach(GetDlgItem(IDC_IPWATCH_LIST));
	cWatch.GetClientRect(rc);

	cWatch.InsertColumn(0, CTSTRING(SETTINGS_IPW_IP),		LVCFMT_LEFT, rc.Width() / 3, 0);
	cWatch.InsertColumn(1, CTSTRING(SETTINGS_IPW_ACTION),	LVCFMT_LEFT, rc.Width() / 3, 1);
	cWatch.InsertColumn(2, CTSTRING(SETTINGS_IPW_RAW),		LVCFMT_LEFT, rc.Width() / 3, 2);
	cWatch.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	IPWatch::List ipw = IpManager::getInstance()->getWatch();
	for(IPWatch::Iter j = ipw.begin(); j != ipw.end(); ++j) {
		IPWatch::Ptr ipwatch = *j;	
		addEntryWatch(ipwatch, cWatch.GetItemCount());
	}
	fixControls();
	return TRUE;
}

void IpWatchPage::addEntryWatch(IPWatch::Ptr ipw, int pos) {
	TStringList lst;
	lst.push_back(Text::toT(ipw->getPattern()));
	lst.push_back(Text::toT(getAction(ipw->getTask())));
	lst.push_back(RawManager::getInstance()->getNameActionId(RawManager::getInstance()->getValidAction(ipw->getAction())));
	cWatch.insert(pos, lst, 0, 0);
}

void IpWatchPage::write() {
	IpManager::getInstance()->WatchSave();
	PropPage::write((HWND)*this, items);
}

void IpWatchPage::fixControls() {
	BOOL use = IsDlgButtonChecked(IDC_ENABLE_IPWATCH) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_IPWATCH_LIST), use);
	::EnableWindow(GetDlgItem(IDC_ADD_WATCH), use);
	::EnableWindow(GetDlgItem(IDC_CHANGE_WATCH), use);
	::EnableWindow(GetDlgItem(IDC_REMOVE_WATCH), use);
}

LRESULT IpWatchPage::onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}

LRESULT IpWatchPage::onAddWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	IPWatchDlg dlg;
	if(dlg.DoModal() == IDOK) {
		addEntryWatch(IpManager::getInstance()->addWatch(dlg.mode, Text::fromT(dlg.pattern), dlg.task, dlg.action, dlg.display, Text::fromT(dlg.cheat), dlg.matchType, Text::fromT(dlg.isp)),
			cWatch.GetItemCount());
	}
	return 0;
}

LRESULT IpWatchPage::onChangeWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(cWatch.GetSelectedCount() == 1) {
		int sel = cWatch.GetSelectedIndex();
		IPWatch ipw;
		IpManager::getInstance()->getWatch(sel, ipw);
		IPWatchDlg dlg;

		dlg.mode = ipw.getMode();
		dlg.pattern = Text::toT(ipw.getPattern());
		dlg.task = ipw.getTask();
		dlg.action = ipw.getAction();
		dlg.display = ipw.getDisplayCheat();
		dlg.cheat = Text::toT(ipw.getCheat());
		dlg.matchType = ipw.getMatchType();
		dlg.isp = Text::toT(ipw.getIsp());

		if(dlg.DoModal() == IDOK) {
			ipw.setMode(dlg.mode);
			ipw.setPattern(Text::fromT(dlg.pattern));
			ipw.setTask(dlg.task);
			ipw.setAction(dlg.action);
			ipw.setDisplayCheat(dlg.display);
			ipw.setCheat(Text::fromT(dlg.cheat));
			ipw.setMatchType(dlg.matchType);
			ipw.setIsp(Text::fromT(dlg.isp));

			IpManager::getInstance()->updateWatch(sel, ipw);

			cWatch.SetItemText(sel, 0, dlg.pattern.c_str());
			cWatch.SetItemText(sel, 1, Text::toT(getAction(dlg.task)).c_str());
			cWatch.SetItemText(sel, 2, RawManager::getInstance()->getNameActionId(dlg.action).c_str());
		}
	}
	return 0;
}

LRESULT IpWatchPage::onRemoveWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(cWatch.GetSelectedCount() == 1) {
		int sel = cWatch.GetSelectedIndex();
		IpManager::getInstance()->removeWatch(sel);
		cWatch.DeleteItem(sel);
	}
	return 0;
}

LRESULT IpWatchPage::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;
	switch(cd->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;

		case CDDS_ITEMPREPAINT: {
			try {
				TCHAR* buf = new TCHAR[256];
				cWatch.GetItemText(cd->nmcd.dwItemSpec, 1, buf, 256);
				if((tstring)buf == _T("Ban")) {
					cd->clrText =  SETTING(BAD_CLIENT_COLOUR);
				}
				if(cd->nmcd.dwItemSpec % 2 == 0) {
					cd->clrTextBk = RGB(245, 245, 245);
				}
				delete buf;
				return CDRF_NEWFONT | CDRF_NOTIFYSUBITEMDRAW;
			} catch(...) {
				//...
			}
		}
		return CDRF_NOTIFYSUBITEMDRAW;
		default: return CDRF_DODEFAULT;
	}
}

/**
 * @file
 * $Id$
 */