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

#include "FakeDetect.h"
#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "../rsx/RsxUtil.h"
#include "../client/RawManager.h"
#include "WinUtil.h"
//@todo rewrite
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

	int n = 0;
	settingRaw[n++] = RSXSETTING(ADL_RAW_LOW_POINTS);
	settingRaw[n++] = RSXSETTING(ADL_RAW_MEDIUM_POINTS);
	settingRaw[n++] = RSXSETTING(ADL_RAW_HIGH_POINTS);
	settingRaw[n++] = RSXSETTING(SDL_RAW);
	settingRaw[n++] = RSXSETTING(TIMEOUT_RAW);
	settingRaw[n++] = RSXSETTING(DISCONNECT_RAW);
	settingRaw[n++] = RSXSETTING(FAKE_SLOT_COUNT);
	settingRaw[n++] = RSXSETTING(FAKESHARE_RAW);
	settingRaw[n++] = RSXSETTING(LISTLEN_MISMATCH);
	settingRaw[n++] = RSXSETTING(FILELIST_TOO_SMALL_BIG);
	settingRaw[n++] = RSXSETTING(FILELIST_NA);
	settingRaw[n++] = RSXSETTING(NO_TTHF);
	settingRaw[n++] = RSXSETTING(VERSION_MISMATCH);
	settingRaw[n++] = RSXSETTING(FILELIST_VERSION_MISMATCH);
	settingRaw[n++] = RSXSETTING(RMDC_RAW);
	settingRaw[n++] = RSXSETTING(DCPP_EMULATION_RAW);
	settingRaw[n++] = RSXSETTING(MYINFO_SPAM_KICK);
	settingRaw[n++] = RSXSETTING(CTM_SPAM_KICK);
	settingRaw[n++] = RSXSETTING(PM_SPAM_KICK);
	settingRaw[n++] = RSXSETTING(BAD_ISP_RAW);

	n = 0;
	showCheat[n++] = RSXBOOLSETTING(SHOW_ADL_RAW_LOW_POINTS);
	showCheat[n++] = RSXBOOLSETTING(SHOW_ADL_RAW_MEDIUM_POINTS);
	showCheat[n++] = RSXBOOLSETTING(SHOW_ADL_RAW_HIGH_POINTS);
	showCheat[n++] = RSXBOOLSETTING(SHOW_SDL_RAW);
	showCheat[n++] = RSXBOOLSETTING(SHOW_TIMEOUT_RAW);
	showCheat[n++] = RSXBOOLSETTING(SHOW_DISCONNECT_RAW);
	showCheat[n++] = RSXBOOLSETTING(SHOW_FAKE_SLOT_COUNT);
	showCheat[n++] = RSXBOOLSETTING(SHOW_FAKESHARE_RAW);
	showCheat[n++] = RSXBOOLSETTING(SHOW_LISTLEN_MISMATCH);
	showCheat[n++] = RSXBOOLSETTING(SHOW_FILELIST_TOO_SMALL_BIG);
	showCheat[n++] = RSXBOOLSETTING(SHOW_FILELIST_NA);
	showCheat[n++] = RSXBOOLSETTING(SHOW_NO_TTHF);
	showCheat[n++] = RSXBOOLSETTING(SHOW_VERSION_MISMATCH);
	showCheat[n++] = RSXBOOLSETTING(SHOW_FILELIST_VERSION_MISMATCH);
	showCheat[n++] = RSXBOOLSETTING(SHOW_RMDC_RAW);
	showCheat[n++] = RSXBOOLSETTING(SHOW_DCPP_EMULATION_RAW);
	showCheat[n++] = RSXBOOLSETTING(SHOW_MYINFO_SPAM_KICK);
	showCheat[n++] = RSXBOOLSETTING(SHOW_CTM_SPAM_KICK);
	showCheat[n++] = RSXBOOLSETTING(SHOW_PM_SPAM_KICK);
	showCheat[n++] = RSXBOOLSETTING(SHOW_BAD_ISP_RAW);

	CRect rc;
	ctrlList.Attach(GetDlgItem(IDC_DETECTOR_ITEMS));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, CTSTRING(NAME), LVCFMT_LEFT, (rc.Width() / 3) + 75, 0);
	ctrlList.InsertColumn(1, CTSTRING(ACTION), LVCFMT_LEFT, (rc.Width() / 3) - 35, 1);
	ctrlList.InsertColumn(2, CTSTRING(SETTINGS_DISPLAY_CHEATS_IN_MAIN_CHAT), LVCFMT_LEFT, (rc.Width() / 3) - 60, 2);

	ctrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	insertAllItem();
	Action::List lst = RawManager::getInstance()->getActionList();

	int j = 0;
	idAction.insert(make_pair(j, j));
	for(Action::List::iterator i = lst.begin(); i != lst.end(); ++i) {
		idAction.insert(make_pair(++j, i->second->getActionId()));
	}

	cRaw.Attach(GetDlgItem(IDC_RAW_DETECTOR));
	for(ActionList::const_iterator i = idAction.begin(); i != idAction.end(); ++i) {
		cRaw.AddString(RawManager::getInstance()->getNameActionId(i->second).c_str());
	}
	cRaw.SetCurSel(0);

	cShowCheat.Attach(GetDlgItem(IDC_SHOW_CHEAT));
	cShowCheat.AddString(CTSTRING(NO));
	cShowCheat.AddString(CTSTRING(YES));
	cShowCheat.SetCurSel(0);

	CUpDownCtrl updown;
	updown.Attach(GetDlgItem(IDC_TESTSURS_SPIN));
	updown.SetRange32(0, 50);
	updown.Detach();
	updown.Attach(GetDlgItem(IDC_FILELISTS_SPIN));
	updown.SetRange32(0, 50);
	updown.Detach();
	updown.Attach(GetDlgItem(IDC_TIMEOUT_SPIN));
	updown.SetRange32(0, 9999);
	updown.Detach();
	updown.Attach(GetDlgItem(IDC_DISCONNECT_SPIN));
	updown.SetRange32(0, 9999);
	updown.Detach();
	updown.Attach(GetDlgItem(IDC_CHECK_DELAY_SPIN));
	updown.SetRange32(500, 50*60*60);
	updown.Detach();
	updown.Attach(GetDlgItem(IDC_SLEEP_TIME_SPIN));
	updown.SetRange32(100, 50*60*60);
	updown.Detach();

	return TRUE;
}

void FakeDetect::write() {
	PropPage::write((HWND)*this, items, 0);
	RSXSettingsManager* rsxs = RSXSettingsManager::getInstance();

	int n = 0;

	rsxs->set(RSXSettingsManager::ADL_RAW_LOW_POINTS,			RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::ADL_RAW_MEDIUM_POINTS,		RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::ADL_RAW_HIGH_POINTS,			RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::SDL_RAW,						RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::TIMEOUT_RAW,					RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::DISCONNECT_RAW,				RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::FAKE_SLOT_COUNT,				RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::FAKESHARE_RAW,				RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::LISTLEN_MISMATCH,				RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::FILELIST_TOO_SMALL_BIG,		RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::FILELIST_NA,					RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::NO_TTHF,						RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::VERSION_MISMATCH,				RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::FILELIST_VERSION_MISMATCH,	RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::RMDC_RAW,						RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::DCPP_EMULATION_RAW,			RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::MYINFO_SPAM_KICK,				RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::CTM_SPAM_KICK,				RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::PM_SPAM_KICK,					RawManager::getInstance()->getValidAction(settingRaw[n++]));
	rsxs->set(RSXSettingsManager::BAD_ISP_RAW,					RawManager::getInstance()->getValidAction(settingRaw[n++]));

	n = 0;

	rsxs->set(RSXSettingsManager::SHOW_ADL_RAW_LOW_POINTS,			showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_ADL_RAW_MEDIUM_POINTS,		showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_ADL_RAW_HIGH_POINTS,			showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_SDL_RAW,						showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_TIMEOUT_RAW,					showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_DISCONNECT_RAW,				showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_FAKE_SLOT_COUNT,				showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_FAKESHARE_RAW,				showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_LISTLEN_MISMATCH,			showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_FILELIST_TOO_SMALL_BIG,		showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_FILELIST_NA,					showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_NO_TTHF,						showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_VERSION_MISMATCH,			showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_FILELIST_VERSION_MISMATCH,	showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_RMDC_RAW,					showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_DCPP_EMULATION_RAW,			showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_MYINFO_SPAM_KICK,			showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_CTM_SPAM_KICK,				showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_PM_SPAM_KICK,				showCheat[n++]);
	rsxs->set(RSXSettingsManager::SHOW_BAD_ISP_RAW,					showCheat[n++]);
}

void FakeDetect::insertAllItem() {
	int n = 0;
	int c = 0;

	insertItem(_T("Forbidden Files (1-499 points)"),	settingRaw[n++],	showCheat[c++]);
	insertItem(_T("Forbidden Files (500-4999 points)"),	settingRaw[n++],	showCheat[c++]);
	insertItem(_T("Forbidden Files (5000+ points)"),	settingRaw[n++],	showCheat[c++]);
	insertItem(_T("Slow download"),						settingRaw[n++],	showCheat[c++]);
	insertItem(_T("Connection timeout"),				settingRaw[n++],	showCheat[c++]);
	insertItem(_T("Disconnect raw"),					settingRaw[n++],	showCheat[c++]);
	insertItem(_T("Fake Slot count"),					settingRaw[n++],	showCheat[c++]);
	insertItem(_T("Fakeshare raw"),						settingRaw[n++],	showCheat[c++]);
	insertItem(_T("ListLen mis-match"),					settingRaw[n++],	showCheat[c++]);
	insertItem(_T("FileList too small or big"),			settingRaw[n++],	showCheat[c++]);
	insertItem(_T("FileList Unavailable"),				settingRaw[n++],	showCheat[c++]);
	insertItem(_T("No TTHF/ADCGET support"),			settingRaw[n++],	showCheat[c++]);
	insertItem(_T("Version mis-match"),					settingRaw[n++],	showCheat[c++]);
	insertItem(_T("FileList Version mis-match"),		settingRaw[n++],	showCheat[c++]);
	insertItem(_T("rmDC++"),							settingRaw[n++],	showCheat[c++]);
	insertItem(_T("DC++ Emulation"),					settingRaw[n++],	showCheat[c++]);
	insertItem(_T("$MyINFO Flood"),						settingRaw[n++],	showCheat[c++]);
	insertItem(_T("$ConnectToMe Flood"),				settingRaw[n++],	showCheat[c++]);
	insertItem(_T("PM Flood"),							settingRaw[n++],	showCheat[c++]);
	insertItem(_T("Bad ISP"),							settingRaw[n++],	showCheat[c++]);
}

void FakeDetect::insertItem(const tstring& a, int b, int showCheat) {
	TStringList cols;
	cols.push_back(a);
	cols.push_back(RawManager::getInstance()->getNameActionId(RawManager::getInstance()->getValidAction(b)));
	cols.push_back(RsxUtil::toBool(showCheat) ? CTSTRING(YES) : CTSTRING(NO));

	ctrlList.insert(cols);
	cols.clear();
}

LRESULT FakeDetect::onRawChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = cRaw.GetCurSel();

	if(sel >= 0) {
		int item = ctrlList.GetSelectedIndex();
		if(item >= 0) {
			settingRaw[item] = RawManager::getInstance()->getValidAction(getIdAction(sel));
			ctrlList.SetItemText(item, 1, RawManager::getInstance()->getNameActionId(settingRaw[item]).c_str());
		}
	}
	return 0;
}

LRESULT FakeDetect::onShowChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = cShowCheat.GetCurSel();

	if(sel >= 0) {
		int item = ctrlList.GetSelectedIndex();
		if(item >=0) {
			showCheat[item] = sel;
			ctrlList.SetItemText(item, 2, showCheat[item] ? CTSTRING(YES) : CTSTRING(NO));
		}
	}
	return 0;
}
LRESULT FakeDetect::onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	int sel = ctrlList.GetSelectedIndex();
	if(sel >= 0) {
		cRaw.SetCurSel(getId(settingRaw[sel]));
		cShowCheat.SetCurSel(showCheat[sel]);
	}

	return 0;
}

LRESULT FakeDetect::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;

	switch(cd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		{
			try	{
				if (settingRaw[cd->nmcd.dwItemSpec]) {
					cd->clrText = SETTING(BAD_CLIENT_COLOUR);
				}
				if(cd->nmcd.dwItemSpec % 2 == 0) {
					cd->clrTextBk = RGB(245, 245, 245);
				}
				return CDRF_NEWFONT | CDRF_NOTIFYSUBITEMDRAW;
			} catch(const Exception&) {
			} catch(...) {
			}
		}
		return CDRF_NOTIFYSUBITEMDRAW;

	default:
		return CDRF_DODEFAULT;
	}
}