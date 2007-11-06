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

#include "ISPPage.h"
#include "ISPDlg.h"

#include "../rsx/RsxUtil.h"
#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "IPWatchDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::TextItem ISPPage::texts[] = {
	{ IDC_ADD_ISP, ResourceManager::ADD },
	{ IDC_CHANGE_ISP, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_ISP, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};
PropPage::Item ISPPage::items[] = {
	{ IDC_ENABLE_ISP, RSXSettingsManager::ISP_CHECKING, PropPage::T_BOOL_RSX },
	{ IDC_ENABLE_IPWATCH, RSXSettingsManager::USE_IPWATCH, PropPage::T_BOOL_RSX },
	{ 0, 0, PropPage::T_END }
};

LRESULT ISPPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items);

	CRect rc;

	ctrlISP.Attach(GetDlgItem(IDC_ISP));
	ctrlISP.GetClientRect(rc);

	ctrlISP.InsertColumn(0, CTSTRING(IP_RANGE), LVCFMT_LEFT, (rc.Width() / 2) - 10, 0);
	ctrlISP.InsertColumn(1, CTSTRING(ISP), LVCFMT_LEFT, (rc.Width() / 2) - 10, 1);

	ctrlISP.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	// Do specialized reading here
	Isp::List& il = IpManager::getInstance()->getIspList(); 
	TStringList cols;
	for(Isp::Iter j = il.begin(); j != il.end(); ++j) {
		string ran = RsxUtil::toIP(j->second->getLower()) + '-' + RsxUtil::toIP(j->first);
		cols.push_back(Text::toT(ran));
		cols.push_back(Text::toT(j->second->getIsp()));
		ctrlISP.insert(cols);
		cols.clear();
	}

	//IP Watch
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

void ISPPage::addEntryWatch(IPWatch::Ptr ipw, int pos) {
	TStringList lst;
	lst.push_back(Text::toT(ipw->getIp()));
	lst.push_back(Text::toT(getAction(ipw->getAction())));
	lst.push_back(RawManager::getInstance()->getNameActionId(RawManager::getInstance()->getValidAction(ipw->getActionCommand())));
	cWatch.insert(pos, lst, 0, 0);
}

LRESULT ISPPage::onAddISP(WORD , WORD , HWND , BOOL& ) {
	ISPDlg dlg;

	if(dlg.DoModal() == IDOK) {
		IpManager::getInstance()->insertISP(RsxUtil::getUpperRange(dlg.range), RsxUtil::getLowerRange(dlg.range), dlg.ISP, dlg.bad);
		TStringList lst;
		lst.push_back(Text::toT(dlg.range));
		lst.push_back(Text::toT(dlg.ISP));
		ctrlISP.insert(lst);
	}
	return 0;
}

LRESULT ISPPage::onChangeISP(WORD , WORD , HWND , BOOL& ) {
	
#define BUFLEN 256
	if(ctrlISP.GetSelectedCount() == 1) {
		int sel = ctrlISP.GetSelectedIndex();
		TCHAR buf[BUFLEN];
		ISPDlg dlg;
		ctrlISP.GetItemText(sel, 1, buf, BUFLEN);
		dlg.ISP = Text::fromT(buf);
		ctrlISP.GetItemText(sel, 0, buf, BUFLEN);
		dlg.range = Text::fromT(buf);
		uint32_t high = RsxUtil::getUpperRange(Text::fromT(buf));
		dlg.bad = IpManager::getInstance()->isBadRange(high);

		if(dlg.DoModal() == IDOK) {
			IpManager::getInstance()->removeISP(high);
			IpManager::getInstance()->insertISP(RsxUtil::getUpperRange(dlg.range), RsxUtil::getLowerRange(dlg.range), dlg.ISP, dlg.bad);
			ctrlISP.SetItemText(sel, 0, Text::toT(dlg.range).c_str());
			ctrlISP.SetItemText(sel, 1, Text::toT(dlg.ISP).c_str());
		}
	}
	return 0;
}

LRESULT ISPPage::onRemoveISP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlISP.GetSelectedCount() == 1) {
		TCHAR buf[256];
		ctrlISP.GetItemText(ctrlISP.GetSelectedIndex(), 0, buf, 256);
		IpManager::getInstance()->removeISP(RsxUtil::getUpperRange(Text::fromT(buf)));
		ctrlISP.DeleteItem(ctrlISP.GetNextItem(-1, LVNI_SELECTED));
	}
	IpManager::getInstance()->saveISPs();
	return 0;
}

LRESULT ISPPage::onReload(WORD , WORD , HWND , BOOL& ) {
	reload();
	return 0;
}

void ISPPage::reload() {
	ctrlISP.SetRedraw(FALSE);
	
	IpManager::getInstance()->reloadISPs();
	ctrlISP.DeleteAllItems();
	
	Isp::List& il = IpManager::getInstance()->getIspList(); 
	TStringList cols;

	for(Isp::Iter j = il.begin(); j != il.end(); ++j) {
		string ran = RsxUtil::toIP(j->second->getLower()) + '-' + RsxUtil::toIP(j->first);
		cols.push_back(Text::toT(ran));
		cols.push_back(Text::toT(j->second->getIsp()));
		ctrlISP.insert(cols);
		cols.clear();
	}
	ctrlISP.SetRedraw(TRUE);
}

LRESULT ISPPage::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;

	switch(cd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		{
			try {
				TCHAR buf[256];
				ctrlISP.GetItemText(cd->nmcd.dwItemSpec, 0, buf, 256);

				if(IpManager::getInstance()->isBadRange(RsxUtil::getUpperRange(Text::fromT(buf)))) {
					cd->clrText =  SETTING(BAD_CLIENT_COLOUR);
				}
				if(cd->nmcd.dwItemSpec % 2 == 0) {
					cd->clrTextBk = RGB(245, 245, 245);
				}
				return CDRF_NEWFONT | CDRF_NOTIFYSUBITEMDRAW;
			} catch(...) {
			}
		}
		return CDRF_NOTIFYSUBITEMDRAW;

	default:
		return CDRF_DODEFAULT;
	}
}

void ISPPage::write() {
	IpManager::getInstance()->saveISPs();
	IpManager::getInstance()->WatchSave();
	PropPage::write((HWND)*this, items);
}

void ISPPage::fixControls() {
	BOOL use = IsDlgButtonChecked(IDC_ENABLE_ISP) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_ISP), use);
	::EnableWindow(GetDlgItem(IDC_ADD_ISP), use);
	::EnableWindow(GetDlgItem(IDC_CHANGE_ISP), use);
	::EnableWindow(GetDlgItem(IDC_REMOVE_ISP), use);
	::EnableWindow(GetDlgItem(IDC_RELOAD_ISPS), use);
	use = IsDlgButtonChecked(IDC_ENABLE_IPWATCH) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_IPWATCH_LIST), use);
	::EnableWindow(GetDlgItem(IDC_ADD_WATCH), use);
	::EnableWindow(GetDlgItem(IDC_CHANGE_WATCH), use);
	::EnableWindow(GetDlgItem(IDC_REMOVE_WATCH), use);
}

LRESULT ISPPage::onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}

// IP Watch //////////////////////////////////////////////////////////////////////////////////////////
LRESULT ISPPage::onAddWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	IPWatchDlg dlg;
	//@todo move it to constructor
	dlg.ip = _T("0.0.0.0");
	dlg.action = 0;
	dlg.actionCmd = 0;
	dlg.display = true;
	dlg.regexp = false;
	dlg.cheat = _T("Forbidden IP (%[userI4])");
	if(dlg.DoModal() == IDOK) {
		addEntryWatch(IpManager::getInstance()->addWatch(Text::fromT(dlg.ip), dlg.action, dlg.actionCmd, dlg.display, Text::fromT(dlg.cheat), dlg.regexp),
			cWatch.GetItemCount());
	}
	return 0;
}

LRESULT ISPPage::onChangeWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(cWatch.GetSelectedCount() == 1) {
		int sel = cWatch.GetSelectedIndex();
		IPWatch ipw;
		IpManager::getInstance()->getWatch(sel, ipw);
		IPWatchDlg dlg;

		dlg.ip = Text::toT(ipw.getIp());
		dlg.action = ipw.getAction();
		dlg.actionCmd = ipw.getActionCommand();
		dlg.display = ipw.getDisplayCheat();
		dlg.cheat = Text::toT(ipw.getCheat());
		dlg.regexp = ipw.getUseRegExp();

		if(dlg.DoModal() == IDOK) {
			ipw.setIp(Text::fromT(dlg.ip));
			ipw.setAction(dlg.action);
			ipw.setActionCommand(dlg.actionCmd);
			ipw.setDisplayCheat(dlg.display);
			ipw.setCheat(Text::fromT(dlg.cheat));
			ipw.setUseRegExp(dlg.regexp);

			IpManager::getInstance()->updateWatch(sel, ipw);

			cWatch.SetItemText(sel, 0, dlg.ip.c_str());
			cWatch.SetItemText(sel, 1, Text::toT(getAction(dlg.action)).c_str());
			cWatch.SetItemText(sel, 2, RawManager::getInstance()->getNameActionId(dlg.actionCmd).c_str());
		}
	}
	return 0;
}

LRESULT ISPPage::onRemoveWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(cWatch.GetSelectedCount() == 1) {
		int sel = cWatch.GetSelectedIndex();
		IpManager::getInstance()->removeWatch(sel);
		cWatch.DeleteItem(sel);
	}
	return 0;
}
/**
 * @file
 * $Id: ISPPage.cpp,v 1.0 2004/11/16 18:04 Virus27 Exp $
 */