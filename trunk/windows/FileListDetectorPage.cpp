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

#include "FileListDetectorPage.h"
#include "FileListDetectorDlg.h"
#include "WinUtil.h"
#include "ChangeRawCheatDlg.h"
#include "ADLSearchPoints.h"

#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "../client/ClientProfileManager.h"

#define BUFLEN 256

PropPage::TextItem FileListDetectorPage::texts[] = {
	{ IDC_ADD, ResourceManager::ADD },
	{ IDC_CHANGE, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE, ResourceManager::REMOVE },
	{ IDC_FAKE_SHARE_ACCEPTED, ResourceManager::TEXT_FAKEPERCENT },
	{ IDC_ACTION, ResourceManager::ACTION },
	{ IDC_FAKE_SHARE_SET, ResourceManager::FAKE_SHARE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item FileListDetectorPage::items[] = {
	{ IDC_PERCENT_FAKE_SHARE_TOLERATED, RSXSettingsManager::PERCENT_FAKE_SHARE_TOLERATED,	PropPage::T_INT_RSX },
	{ IDC_MINIMUM_FILELIST_SIZE,		RSXSettingsManager::MINIMUM_FILELIST_SIZE,			PropPage::T_INT_RSX },
	{ IDC_MAXIMUM_FILELIST_SIZE,		RSXSettingsManager::MAXIMUM_FILELIST_SIZE,			PropPage::T_INT_RSX },
	{ IDC_SDL_SPEED,					RSXSettingsManager::SDL_SPEED,						PropPage::T_INT_RSX },
	{ IDC_SDL_TIME,						RSXSettingsManager::SDL_TIME,						PropPage::T_INT_RSX },
	{ IDC_USE_SDL_KICK,					RSXSettingsManager::USE_SDL_KICK,					PropPage::T_BOOL_RSX },
	{ 0, 0, PropPage::T_END }
};

LRESULT FileListDetectorPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items);

	CUpDownCtrl spin;
	spin.Attach(GetDlgItem(IDC_SDL_SPEED_SPIN));
	spin.SetRange32(1, 10*1024); //max 10 kB/s
	spin.Detach(); 
	spin.Attach(GetDlgItem(IDC_SDL_TIME_SPIN));
	spin.SetRange32(15, 3600); //an hour would be fine ;]
	spin.Detach();
	spin.Attach(GetDlgItem(IDC_MINIMUM_FILELIST_SIZE_SPIN));
	spin.SetRange32(0, 1024*1024*1024);
	spin.Detach();
	spin.Attach(GetDlgItem(IDC_MAXIMUM_FILELIST_SIZE_SPIN));
	spin.SetRange32(0, 1024*1024*1024);
	spin.Detach();
	spin.Attach(GetDlgItem(IDC_PERCENT_FAKE_SHARE_TOLERATED_SPIN));
	spin.SetRange32(0, 100);
	spin.Detach();
	CRect rc;

	createList();
	cRaw.Attach(GetDlgItem(IDC_FAKE_RAW));
	for(ActionList::const_iterator i = idAction.begin(); i != idAction.end(); ++i) {
		cRaw.AddString(RawManager::getInstance()->getNameActionId(i->second).c_str());
	}
	cRaw.SetCurSel(getId(RSXSETTING(FAKESHARE_RAW)));
	cRaw.Detach();

	cListType.Attach(GetDlgItem(IDC_FILELIST_PAGE_LIST_TYPE));
	cListType.AddString(_T("File List Generator"));
	cListType.AddString(_T("ADL Search Custom Points System"));
	cListType.SetCurSel(0);

	ctrlProfiles.Attach(GetDlgItem(IDC_FILELIST_DETECTOR_LIST));
	ctrlProfiles.GetClientRect(rc);

	ctrlProfiles.InsertColumn(0, CTSTRING(CLIENTID),				LVCFMT_LEFT, (rc.Width() / 7)*2, 0);
	ctrlProfiles.InsertColumn(1, CTSTRING(CHEATING_DESCRIPTION),	LVCFMT_LEFT, (rc.Width() / 7)*3, 1);
	ctrlProfiles.InsertColumn(2, CTSTRING(ACTION),					LVCFMT_LEFT, (rc.Width() / 7)*2, 3);
	ctrlProfiles.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	FileListDetectorProfile::List& lst = ClientProfileManager::getInstance()->getFileListDetectors();
	for(FileListDetectorProfile::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const FileListDetectorProfile& fd = *i;	
		addEntry(fd, ctrlProfiles.GetItemCount());
	}
	fixControls();
	return TRUE;
}

LRESULT FileListDetectorPage::onSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlProfiles.DeleteAllItems();
	ctrlProfiles.DeleteColumn(2);
	ctrlProfiles.DeleteColumn(1);
	ctrlProfiles.DeleteColumn(0);
	CRect rc;
	ctrlProfiles.GetClientRect(rc);

	if(cListType.GetCurSel() == 0) {
		ctrlProfiles.InsertColumn(0, CTSTRING(CLIENTID),				LVCFMT_LEFT, (rc.Width() / 7)*2, 0);
		ctrlProfiles.InsertColumn(1, CTSTRING(CHEATING_DESCRIPTION),	LVCFMT_LEFT, (rc.Width() / 7)*3, 1);
		ctrlProfiles.InsertColumn(2, CTSTRING(ACTION),					LVCFMT_LEFT, (rc.Width() / 7)*2, 3);

		FileListDetectorProfile::List& lst = ClientProfileManager::getInstance()->getFileListDetectors();
		for(FileListDetectorProfile::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
			const FileListDetectorProfile& fd = *i;	
			addEntry(fd, ctrlProfiles.GetItemCount());
		}
	} else if(cListType.GetCurSel() == 1) {
		ctrlProfiles.InsertColumn(0, _T("Points"), LVCFMT_LEFT, rc.Width() / 3, 0);
		ctrlProfiles.InsertColumn(1, _T("Cheat"), LVCFMT_LEFT, (rc.Width() / 3), 0);
		ctrlProfiles.InsertColumn(2, CTSTRING(ACTION), LVCFMT_LEFT, (rc.Width() / 3)-17, 0);

		RawManager::ADLPoints& p = RawManager::getInstance()->getADLPoints();
		for(RawManager::ADLPoints::const_iterator i = p.begin(); i != p.end(); ++i) {
			addEntryP(i->first, i->second.first, i->second.second, ctrlProfiles.GetItemCount());
		}
		ctrlProfiles.setSort(0, ExListViewCtrl::SORT_INT, true);
	}
	return 0;
}

LRESULT FileListDetectorPage::onAdd(WORD , WORD , HWND , BOOL& ) {
	if(cListType.GetCurSel() == 0) {
		FileListDetectorDlg dlg;
		dlg.currentProfileId = -1;
		if(dlg.DoModal() == IDOK) {
			addEntry(ClientProfileManager::getInstance()->addFileListDetector(		
				dlg.name, 
				dlg.detect, 
				dlg.cheatingDescription,
				dlg.rawToSend, 
				dlg.badClient 
				), ctrlProfiles.GetItemCount());
		}
	} else if(cListType.GetCurSel() == 1) {
		PointValue dlg;
		if(dlg.DoModal() == IDOK) {
			addEntryP(dlg.value, dlg.actionId, dlg.display, ctrlProfiles.GetItemCount());
		}
		ctrlProfiles.resort();
	}
	return 0;
}

LRESULT FileListDetectorPage::onChange(WORD , WORD , HWND , BOOL& ) {
	if(ctrlProfiles.GetSelectedCount() >= 1) {
		int sel = ctrlProfiles.GetSelectedIndex();

		if(cListType.GetCurSel() == 0) {
			FileListDetectorDlg dlg;
			dlg.currentProfileId = ctrlProfiles.GetItemData(sel);
			if(dlg.DoModal() == IDOK) {
				ctrlProfiles.SetItemText(dlg.currentProfileId, 0, Text::toT(dlg.name).c_str());
				ctrlProfiles.SetItemText(dlg.currentProfileId, 1, Text::toT(dlg.cheatingDescription).c_str());
					
				dlg.currentProfile.setName(dlg.name);
				dlg.currentProfile.setDetect(dlg.detect);
				dlg.currentProfile.setCheatingDescription(dlg.cheatingDescription);
				dlg.currentProfile.setRawToSend(dlg.rawToSend);
				dlg.currentProfile.setBadClient(dlg.badClient);
				ClientProfileManager::getInstance()->updateFileListDetector(dlg.currentProfile);
			}
			ctrlProfiles.SetRedraw(FALSE);
			ctrlProfiles.DeleteAllItems();
			FileListDetectorProfile::List lst = ClientProfileManager::getInstance()->getFileListDetectors();
			for(FileListDetectorProfile::List::const_iterator j = lst.begin(); j != lst.end(); ++j) {
				const FileListDetectorProfile& fd = *j;	
				addEntry(fd, ctrlProfiles.GetItemCount());
			}
		} else if(cListType.GetCurSel() == 1) {
			PointValue dlg;
			dlg.value = (int)ctrlProfiles.GetItemData(sel);
			int act;
			bool dsp;
			RawManager::getInstance()->calcADLAction(dlg.value, act, dsp);
			dlg.actionId = act;
			dlg.display = dsp;

			if(dlg.DoModal() == IDOK) {
				ctrlProfiles.DeleteItem(sel);
				addEntryP(dlg.value, dlg.actionId, dlg.display, sel);
			}
			ctrlProfiles.resort();
		}
		ctrlProfiles.SelectItem(sel);
		ctrlProfiles.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT FileListDetectorPage::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	while(ctrlProfiles.GetSelectedCount() >= 1) {
		int i = -1;
		while((i = ctrlProfiles.GetNextItem(i, LVNI_SELECTED)) != -1) {
			if(cListType.GetCurSel() == 0) {
				ClientProfileManager::getInstance()->removeFileListDetector(ctrlProfiles.GetItemData(i));
			} else if(cListType.GetCurSel() == 1) {
				RawManager::getInstance()->remADLPoints((int)ctrlProfiles.GetItemData(i));
			}
			ctrlProfiles.DeleteItem(i);
		}
	}		
	return 0;
}

LRESULT FileListDetectorPage::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	if(ctrlProfiles.GetSelectedCount() >= 1 && cListType.GetCurSel() == 0) {
		ctrlProfiles.GetClientRect(&rc);
		ctrlProfiles.ScreenToClient(&pt); 

		if (PtInRect(&rc, pt)) { 
			ctrlProfiles.ClientToScreen(&pt);
			if(ctrlProfiles.GetSelectedCount() >= 1) {
				ChangeRawCheatDlg dlg;
				FileListDetectorProfile fd;
				
				int x = ctrlProfiles.GetSelectedIndex();
				ClientProfileManager::getInstance()->getFileListDetector(ctrlProfiles.GetItemData(x), fd);
				dlg.name = fd.getName();
				dlg.cheatingDescription = fd.getCheatingDescription();
				dlg.raw = fd.getRawToSend();
				if(dlg.DoModal() == IDOK) {
					int i = -1;
					while((i = ctrlProfiles.GetNextItem(i, LVNI_SELECTED)) != -1) {
						ClientProfileManager::getInstance()->getFileListDetector(ctrlProfiles.GetItemData(i), fd);
						if (i == x) {
							fd.setCheatingDescription(dlg.cheatingDescription);
						}
						fd.setRawToSend(dlg.raw);
						ClientProfileManager::getInstance()->updateFileListDetector(fd);
						ctrlProfiles.SetItemText(i, 1, Text::toT(fd.getCheatingDescription()).c_str());
						ctrlProfiles.SetItemText(i, 2, RawManager::getInstance()->getNameActionId(fd.getRawToSend()).c_str());
					}
				}
			}
			return TRUE; 
		}
	}
	return FALSE; 
}

void FileListDetectorPage::write() {
	cRaw.Attach(GetDlgItem(IDC_HASH_ERROR_RAW));
	RSXSettingsManager::getInstance()->set(RSXSettingsManager::HASH_ERROR_RAW, getIdAction(cRaw.GetCurSel()));
	cRaw.Detach();
	cRaw.Attach(GetDlgItem(IDC_FAKE_RAW));
	SettingsManager::getInstance()->set(SettingsManager::FAKESHARE_RAW, getIdAction(cRaw.GetCurSel()));
	cRaw.Detach();

	ClientProfileManager::getInstance()->saveClientProfiles();
	PropPage::write((HWND)*this, items);
}

void FileListDetectorPage::addEntry(const FileListDetectorProfile& fd, int pos) {
	TStringList lst;
	lst.push_back(Text::toT(fd.getName()));
	lst.push_back(Text::toT(fd.getCheatingDescription()));
	lst.push_back(RawManager::getInstance()->getNameActionId(RawManager::getInstance()->getValidAction(fd.getRawToSend())));
	ctrlProfiles.insert(pos, lst, 0, (LPARAM)fd.getId());
}

void FileListDetectorPage::addEntryP(int p, int a, bool d, int pos) {
	TStringList l;
	l.push_back(Util::toStringW(p));
	l.push_back(d ? CTSTRING(YES) : CTSTRING(NO));
	l.push_back(RawManager::getInstance()->getNameActionId(RawManager::getInstance()->getValidAction(a)));
	ctrlProfiles.insert(pos, l, 0, (LPARAM)p);
}

LRESULT FileListDetectorPage::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;
	switch(cd->nmcd.dwDrawStage) {
		case CDDS_PREPAINT: return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT: {
			if(cListType.GetCurSel() == 0) {
				try	{
					FileListDetectorProfile fd;
					ClientProfileManager::getInstance()->getFileListDetector(ctrlProfiles.GetItemData(cd->nmcd.dwItemSpec), fd);
					if(fd.getRawToSend()) {
						cd->clrText = SETTING(BAD_CLIENT_COLOUR);
					} else if(fd.getBadClient()) {
						cd->clrText = SETTING(BAD_FILELIST_COLOUR);
					}
					if(cd->nmcd.dwItemSpec % 2 == 0) {
						cd->clrTextBk = RGB(245, 245, 245);
					}
					return CDRF_NEWFONT | CDRF_NOTIFYSUBITEMDRAW;
				} catch(...) {
					//...
				}
			}
			return CDRF_NOTIFYSUBITEMDRAW;
		}
		default: return CDRF_DODEFAULT;
	}
}

void FileListDetectorPage::fixControls() {
	BOOL use = IsDlgButtonChecked(IDC_USE_SDL_KICK) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_SDL_SPEED), use);
	::EnableWindow(GetDlgItem(IDC_SDL_SPEED_SPIN), use);
	::EnableWindow(GetDlgItem(IDC_SDL_TIME), use);
	::EnableWindow(GetDlgItem(IDC_SDL_TIME_SPIN), use);
	::EnableWindow(GetDlgItem(IDC_SDL_SPEED_BELOW_TEXT), use);
	::EnableWindow(GetDlgItem(IDC_SDL_FOR_MORE_THAN_TEXT), use);
	::EnableWindow(GetDlgItem(IDC_SECONDS), use);
	::EnableWindow(GetDlgItem(IDC_BYTE_PER_SEC), use);
}

LRESULT FileListDetectorPage::onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}
