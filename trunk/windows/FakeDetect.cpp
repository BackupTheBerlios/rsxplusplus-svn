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
#include "../client/SettingsManager.h"
#include "../client/RawManager.h"
#include "Resource.h"

#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "../rsx/RsxUtil.h"

#include "FakeDetect.h"
#include "WinUtil.h"

PropPage::TextItem FakeDetect::texts[] = {
	{ IDC_SLEEP_TIME_S,		ResourceManager::SLEEP_TIME },
	{ IDC_CHECK_DELAY_S,	ResourceManager::CHECK_DELAY },
	{ IDC_MAX_TESTSURS_S,	ResourceManager::MAX_TESTSURS },
	{ IDC_MAX_FILELISTS_S,	ResourceManager::MAX_FILELISTS },
	{ IDC_SHOW_CHEAT_S,		ResourceManager::DISPLAY_CHEAT },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item FakeDetect::items[] = {
	{ IDC_MAX_TESTSURS,		RSXSettingsManager::MAX_TESTSURS,		PropPage::T_INT_RSX },
	{ IDC_MAX_FILELISTS,	RSXSettingsManager::MAX_FILELISTS,		PropPage::T_INT_RSX },
	{ IDC_SLEEP_TIME,		RSXSettingsManager::SLEEP_TIME,			PropPage::T_INT_RSX },
	{ IDC_CHECK_DELAY,		RSXSettingsManager::CHECK_DELAY,		PropPage::T_INT_RSX },
	{ IDC_TIMEOUTS_NO,		RSXSettingsManager::MAX_TIMEOUTS,		PropPage::T_INT_RSX },
	{ IDC_DISCONNECT_NO,	RSXSettingsManager::MAX_DISCONNECTS,	PropPage::T_INT_RSX },
	{ IDC_PROTECTED_USERS,	RSXSettingsManager::PROTECTED_USERS,	PropPage::T_STR_RSX },
	{ 0, 0, PropPage::T_END }
};

LRESULT FakeDetect::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items);

	CRect rc;
	ctrlList.Attach(GetDlgItem(IDC_DETECTOR_ITEMS));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, CTSTRING(NAME), LVCFMT_LEFT, (rc.Width() / 3) + 75, 0);
	ctrlList.InsertColumn(1, CTSTRING(ACTION), LVCFMT_LEFT, (rc.Width() / 3) - 35, 1);
	ctrlList.InsertColumn(2, CTSTRING(SETTINGS_DISPLAY_CHEATS_IN_MAIN_CHAT), LVCFMT_LEFT, (rc.Width() / 3) - 60, 2);
	ctrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	addItem(_T("ADLS Points (out of range)"),	RSXSettingsManager::ADL_OUT_OF_RANGE,			RSXSettingsManager::SHOW_ADL_OUT_OF_RANGE);
	addItem(TSTRING(SDL_RAW),					RSXSettingsManager::SDL_RAW,					RSXSettingsManager::SHOW_SDL_RAW);
	addItem(TSTRING(TIMEOUT_RAW),				RSXSettingsManager::TIMEOUT_RAW,				RSXSettingsManager::SHOW_TIMEOUT_RAW);
	addItem(TSTRING(DISCONNECT_RAW),			RSXSettingsManager::DISCONNECT_RAW,				RSXSettingsManager::SHOW_DISCONNECT_RAW);
	addItem(TSTRING(FAKE_SLOT_COUNT),			RSXSettingsManager::FAKE_SLOT_COUNT,			RSXSettingsManager::SHOW_FAKE_SLOT_COUNT);
	addItem(TSTRING(FAKESHARE_RAW),				RSXSettingsManager::FAKESHARE_RAW,				RSXSettingsManager::SHOW_FAKESHARE_RAW);
	addItem(TSTRING(LISTLEN_MISMATCH),			RSXSettingsManager::LISTLEN_MISMATCH,			RSXSettingsManager::SHOW_LISTLEN_MISMATCH);
	addItem(TSTRING(FILELIST_TOO_SMALL_BIG),	RSXSettingsManager::FILELIST_TOO_SMALL_BIG,		RSXSettingsManager::SHOW_FILELIST_TOO_SMALL_BIG);
	addItem(TSTRING(FILELIST_NA),				RSXSettingsManager::FILELIST_NA,				RSXSettingsManager::SHOW_FILELIST_NA);
	addItem(TSTRING(NO_TTHF),					RSXSettingsManager::NO_TTHF,					RSXSettingsManager::SHOW_NO_TTHF);
	addItem(TSTRING(VERSION_MISMATCH),			RSXSettingsManager::VERSION_MISMATCH,			RSXSettingsManager::SHOW_VERSION_MISMATCH);
	addItem(TSTRING(FILELIST_VERSION_MISMATCH),	RSXSettingsManager::FILELIST_VERSION_MISMATCH,	RSXSettingsManager::SHOW_FILELIST_VERSION_MISMATCH);
	addItem(TSTRING(RMDC_RAW),					RSXSettingsManager::RMDC_RAW,					RSXSettingsManager::SHOW_RMDC_RAW);
	addItem(TSTRING(DCPP_EMULATION_RAW),		RSXSettingsManager::DCPP_EMULATION_RAW,			RSXSettingsManager::SHOW_DCPP_EMULATION_RAW);
	addItem(TSTRING(MYINFO_SPAM_KICK),			RSXSettingsManager::MYINFO_SPAM_KICK,			RSXSettingsManager::SHOW_MYINFO_SPAM_KICK);
	addItem(TSTRING(CTM_SPAM_KICK),				RSXSettingsManager::CTM_SPAM_KICK,				RSXSettingsManager::SHOW_CTM_SPAM_KICK);
	addItem(TSTRING(PM_SPAM_KICK),				RSXSettingsManager::PM_SPAM_KICK,				RSXSettingsManager::SHOW_PM_SPAM_KICK);

	cRaw.attach(GetDlgItem(IDC_RAW_DETECTOR), 0);

	cShowCheat.Attach(GetDlgItem(IDC_SHOW_CHEAT));
	cShowCheat.AddString(CTSTRING(NO));
	cShowCheat.AddString(CTSTRING(YES));
	cShowCheat.SetCurSel(0);

	setSpinRange(IDC_TESTSURS_SPIN, 0, 50);
	setSpinRange(IDC_FILELISTS_SPIN, 0, 50);
	setSpinRange(IDC_TIMEOUT_SPIN, 0, 9999);
	setSpinRange(IDC_DISCONNECT_SPIN, 0, 9999);
	setSpinRange(IDC_CHECK_DELAY_SPIN, 500, 50*60*60);
	setSpinRange(IDC_SLEEP_TIME_SPIN, 200, 50*60*60);
	return TRUE;
}

void FakeDetect::write() {
	PropPage::write((HWND)*this, items, 0);

	for(int i = 0; i < ctrlList.GetItemCount(); i++) {
		delete (DetectorItem*)ctrlList.GetItemData(i);
	}
	ctrlList.DeleteAllItems();
}

void FakeDetect::addItem(const tstring& aName, int rawId, int cheatId) {
	DetectorItem* item = new DetectorItem(rawId, cheatId, aName);

	TStringList l;
	l.push_back(aName);
	l.push_back(cRaw.getActionName(item->rawId));
	l.push_back(item->displayCheat ? CTSTRING(YES) : CTSTRING(NO));

	ctrlList.insert(l, -1, (LPARAM)item);
	l.clear();
}

LRESULT FakeDetect::onRawChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = cRaw.GetCurSel();
	if(sel >= 0) {
		int item = ctrlList.GetSelectedIndex();
		if(item >= 0) {
			DetectorItem* di = (DetectorItem*)ctrlList.GetItemData(item);
			di->rawId = cRaw.getActionId();
			ctrlList.SetItemText(item, 1, cRaw.getActionName(di->rawId).c_str());
		}
	}
	return 0;
}

LRESULT FakeDetect::onShowChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int item = ctrlList.GetSelectedIndex();
	if(item >= 0) {
		DetectorItem* di = (DetectorItem*)ctrlList.GetItemData(item);
		di->displayCheat = RsxUtil::toBool(cShowCheat.GetCurSel());
		ctrlList.SetItemText(item, 2, di->displayCheat ? CTSTRING(YES) : CTSTRING(NO));
	}
	return 0;
}

LRESULT FakeDetect::onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	int sel = ctrlList.GetSelectedIndex();
	if(sel >= 0) {
		DetectorItem* di = (DetectorItem*)ctrlList.GetItemData(sel);
		cRaw.setPos(di->rawId);
		cShowCheat.SetCurSel(di->displayCheat);
	}
	return 0;
}

LRESULT FakeDetect::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;

	switch(cd->nmcd.dwDrawStage) {
		case CDDS_PREPAINT: {
			return CDRF_NOTIFYITEMDRAW;
		}
		case CDDS_ITEMPREPAINT: {
			try	{
				DetectorItem* di = (DetectorItem*)ctrlList.GetItemData(cd->nmcd.dwItemSpec);
				if(di->rawId > 0) {
					cd->clrText = SETTING(BAD_CLIENT_COLOUR);
				}
				if(cd->nmcd.dwItemSpec % 2 == 0) {
					cd->clrTextBk = RGB(245, 245, 245);
				}
				return CDRF_NEWFONT | CDRF_NOTIFYSUBITEMDRAW;
			} catch(const Exception&) {
				//...
			} catch(...) {
				//...
			}
		}
		return CDRF_NOTIFYSUBITEMDRAW;
		default: return CDRF_DODEFAULT;
	}
}

LRESULT FakeDetect::onInfoTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	int item = ctrlList.GetHotItem();
	if(item != -1) {
		NMLVGETINFOTIP* lpnmtdi = (NMLVGETINFOTIP*) pnmh;
		DetectorItem* di = (DetectorItem*)ctrlList.GetItemData(item);

		tstring infoTip = _T("Item: ") + di->itemName +
			_T("\nAction: ") + cRaw.getActionName(di->rawId) +
			_T("\nDisipay cheat: ") + (di->displayCheat ? CTSTRING(YES) : CTSTRING(NO));
		//@todo write and add cheat descriptions...
		_tcscpy(lpnmtdi->pszText, infoTip.c_str());
	}
	return 0;
}