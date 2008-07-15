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
#include "Resource.h"

#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "../client/DetectionManager.h"
#include "../client/version.h"

#include "../rsx/rsx-settings/rsx-SettingsManager.h"

#include "ParamsPage.h"
#include "ParamDlg.h"

#define BUFLEN 1024

PropPage::TextItem ParamsPage::texts[] = {
	{ IDC_ADD, ResourceManager::ADD },
	{ IDC_CHANGE, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT ParamsPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);

	CRect rc;
	ctrlParams.Attach(GetDlgItem(IDC_PARAM_ITEMS));
	ctrlParams.GetClientRect(rc);

	ctrlParams.InsertColumn(0, CTSTRING(SETTINGS_NAME), LVCFMT_LEFT, rc.Width() / 3, 0);
	ctrlParams.InsertColumn(1, CTSTRING(REGEXP), LVCFMT_LEFT, (rc.Width() / 3) * 2, 1);
	ctrlParams.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	const StringMap& pm = DetectionManager::getInstance()->getParams();
	TStringList cols;
	for(StringMap::const_iterator j = pm.begin(); j != pm.end(); ++j) {
		cols.push_back(Text::toT(j->first));
		cols.push_back(Text::toT(j->second));
		ctrlParams.insert(cols);
		cols.clear();
	}
	return TRUE;
}

LRESULT ParamsPage::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ParamDlg dlg;
	if(dlg.DoModal() == IDOK) {
		if(ctrlParams.find(Text::toT(dlg.name)) == -1) {
			TStringList lst;
			lst.push_back(Text::toT(dlg.name));
			lst.push_back(Text::toT(dlg.regexp));
			ctrlParams.insert(lst);
		} else {
			MessageBox(CTSTRING(PARAM_EXISTS), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK);
		}
	}
	return 0;
}

LRESULT ParamsPage::onChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlParams.GetSelectedCount() == 1) {
		int sel = ctrlParams.GetSelectedIndex();
		TCHAR buf[BUFLEN];
		ParamDlg dlg;
		ctrlParams.GetItemText(sel, 0, buf, BUFLEN);
		dlg.name = Text::fromT(buf);
		ctrlParams.GetItemText(sel, 1, buf, BUFLEN);
		dlg.regexp = Text::fromT(buf);

		if(dlg.DoModal() == IDOK) {
			int idx = ctrlParams.find(Text::toT(dlg.name));
			if(idx == -1 || idx == sel) {
				ctrlParams.SetItemText(sel, 0, Text::toT(dlg.name).c_str());
				ctrlParams.SetItemText(sel, 1, Text::toT(dlg.regexp).c_str());
			} else {
				MessageBox(CTSTRING(PARAM_EXISTS), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK);
			}
		}
	}
	return 0;
}

LRESULT ParamsPage::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlParams.GetSelectedCount() == 1) {
		ctrlParams.DeleteItem(ctrlParams.GetNextItem(-1, LVNI_SELECTED));
	}
	return 0;
}

void ParamsPage::write() {
	int it = ctrlParams.GetItemCount();
	TCHAR buf[BUFLEN];
	string name, regexp;
	StringMap& pm = DetectionManager::getInstance()->getParams();
	pm.clear();
	for(int i = 0; i < it; ++i) {
		ctrlParams.GetItemText(i, 0, buf, BUFLEN);
		name = Text::fromT(buf);
		ctrlParams.GetItemText(i, 1, buf, BUFLEN);
		regexp = Text::fromT(buf);
		pm[name] = regexp;
	}
	DetectionManager::getInstance()->save();
}

LRESULT ParamsPage::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;

	switch(cd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		{
			try	{
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