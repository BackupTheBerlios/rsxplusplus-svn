
#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "Resource.h"

#include "ClientsPage.h"
#include "DetectionEntryDlg.h"
#include "ChangeRawCheatDlg.h"

PropPage::TextItem ClientsPage::texts[] = {
	{ IDC_MOVE_CLIENT_UP, ResourceManager::MOVE_UP },
	{ IDC_MOVE_CLIENT_DOWN, ResourceManager::MOVE_DOWN },
	{ IDC_ADD_CLIENT, ResourceManager::ADD },
	{ IDC_CHANGE_CLIENT, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_CLIENT, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT ClientsPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);

	CRect rc;

	ctrlProfiles.Attach(GetDlgItem(IDC_CLIENT_ITEMS));
	ctrlProfiles.GetClientRect(rc);

	ctrlProfiles.InsertColumn(0, CTSTRING(SETTINGS_NAME),	LVCFMT_LEFT, rc.Width() / 3, 0);
	ctrlProfiles.InsertColumn(1, CTSTRING(COMMENT),			LVCFMT_LEFT, rc.Width() / 2 - 18, 1);
	ctrlProfiles.InsertColumn(2, CTSTRING(ACTION),			LVCFMT_LEFT, rc.Width() / 6, 0);

	ctrlProfiles.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	// Do specialized reading here
	const DetectionManager::DetectionItems& lst = DetectionManager::getInstance()->getProfiles(isUserInfo);
	for(DetectionManager::DetectionItems::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const DetectionEntry& de = *i;
		addEntry(de, ctrlProfiles.GetItemCount());
	}

	SetDlgItemText(IDC_PROFILE_COUNT, Text::toT(STRING(PROFILE_COUNT) + ": " + Util::toString(ctrlProfiles.GetItemCount())).c_str());
	SetDlgItemText(IDC_PROFILE_VERSION, Text::toT(STRING(PROFILE_VERSION) + ": " + DetectionManager::getInstance()->getProfileVersion()).c_str());
	SetDlgItemText(IDC_PROFILE_MESSAGE, Text::toT(STRING(PROFILE_MESSAGE) + ": " + DetectionManager::getInstance()->getProfileMessage()).c_str());
	return TRUE;
}

LRESULT ClientsPage::onAddClient(WORD , WORD , HWND , BOOL& ) {
	DetectionEntry de;
	DetectionEntryDlg dlg(de, isUserInfo);

	if(dlg.DoModal() == IDOK) {
		addEntry(de, ctrlProfiles.GetItemCount());
		SetDlgItemText(IDC_PROFILE_COUNT, Text::toT(STRING(PROFILE_COUNT) + ": " + Util::toString(ctrlProfiles.GetItemCount())).c_str());
	}

	return 0;
}

LRESULT ClientsPage::onItemchangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	::EnableWindow(GetDlgItem(IDC_MOVE_CLIENT_UP), (lv->uNewState & LVIS_FOCUSED));
	::EnableWindow(GetDlgItem(IDC_MOVE_CLIENT_DOWN), (lv->uNewState & LVIS_FOCUSED));
	::EnableWindow(GetDlgItem(IDC_CHANGE_CLIENT), (lv->uNewState & LVIS_FOCUSED));
	::EnableWindow(GetDlgItem(IDC_REMOVE_CLIENT), (lv->uNewState & LVIS_FOCUSED));
	return 0;
}

LRESULT ClientsPage::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	switch(kd->wVKey) {
	case VK_INSERT:
		PostMessage(WM_COMMAND, IDC_ADD_CLIENT, 0);
		break;
	case VK_DELETE:
		PostMessage(WM_COMMAND, IDC_REMOVE_CLIENT, 0);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT ClientsPage::onChangeClient(WORD , WORD , HWND , BOOL& ) {
	if(ctrlProfiles.GetSelectedCount() == 1) {
		int sel = ctrlProfiles.GetSelectedIndex();
		DetectionEntry de;

		if(DetectionManager::getInstance()->getDetectionItem(ctrlProfiles.GetItemData(sel), de, isUserInfo)) {
			DetectionEntryDlg dlg(de, isUserInfo);
			dlg.DoModal();

			ctrlProfiles.SetRedraw(FALSE);
			ctrlProfiles.DeleteAllItems();
			const DetectionManager::DetectionItems& lst = DetectionManager::getInstance()->getProfiles(isUserInfo);
			for(DetectionManager::DetectionItems::const_iterator i = lst.begin(); i != lst.end(); ++i) {
				addEntry(*i, ctrlProfiles.GetItemCount());
			}
			ctrlProfiles.SelectItem(sel);
			ctrlProfiles.SetRedraw(TRUE);
		}
	}

	return 0;
}

LRESULT ClientsPage::onRemoveClient(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlProfiles.GetSelectedCount() == 1) {
		int i = ctrlProfiles.GetNextItem(-1, LVNI_SELECTED);
		DetectionManager::getInstance()->removeDetectionItem(ctrlProfiles.GetItemData(i), isUserInfo);
		ctrlProfiles.DeleteItem(i);
	}
	return 0;
}

LRESULT ClientsPage::onMoveClientUp(WORD , WORD , HWND , BOOL& ) {
	int i = ctrlProfiles.GetSelectedIndex();
	if(i != -1 && i != 0) {
		int n = ctrlProfiles.GetItemData(i);
		DetectionManager::getInstance()->moveDetectionItem(n, -1, isUserInfo);
		ctrlProfiles.SetRedraw(FALSE);
		ctrlProfiles.DeleteItem(i);
		DetectionEntry de;
		DetectionManager::getInstance()->getDetectionItem(n, de, isUserInfo);
		addEntry(de, i-1);
		ctrlProfiles.SelectItem(i-1);
		ctrlProfiles.EnsureVisible(i-1, FALSE);
		ctrlProfiles.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT ClientsPage::onMoveClientDown(WORD , WORD , HWND , BOOL& ) {
	int i = ctrlProfiles.GetSelectedIndex();
	if(i != -1 && i != (ctrlProfiles.GetItemCount()-1) ) {
		int n = ctrlProfiles.GetItemData(i);
		DetectionManager::getInstance()->moveDetectionItem(n, 1, isUserInfo);
		ctrlProfiles.SetRedraw(FALSE);
		ctrlProfiles.DeleteItem(i);
		DetectionEntry de;
		DetectionManager::getInstance()->getDetectionItem(n, de, isUserInfo);
		addEntry(de, i+1);
		ctrlProfiles.SelectItem(i+1);
		ctrlProfiles.EnsureVisible(i+1, FALSE);
		ctrlProfiles.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT ClientsPage::onReload(WORD , WORD , HWND , BOOL& ) {
	reload();
	return 0;
}

LRESULT ClientsPage::onInfoTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	int item = ctrlProfiles.GetHotItem();
	if(item != -1) {
		NMLVGETINFOTIP* lpnmtdi = (NMLVGETINFOTIP*) pnmh;

		DetectionEntry de;
		DetectionManager::getInstance()->getDetectionItem(ctrlProfiles.GetItemData(item), de, isUserInfo);
		tstring infoTip = Text::toT(STRING(NAME) + ": " + de.name +
			"\r\n" + STRING(COMMENT) + ": " + de.comment +
			"\r\n" + STRING(CHEATING_DESCRIPTION) + ": " + de.cheat +
			"\r\n" + STRING(ACTION) + ": ") + RawManager::getInstance()->getNameActionId(de.rawToSend);

		_tcscpy(lpnmtdi->pszText, infoTip.c_str());
	}
	return 0;
}
//RSX++
LRESULT ClientsPage::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	if(ctrlProfiles.GetSelectedCount() >= 1) {
		ctrlProfiles.GetClientRect(&rc);
		ctrlProfiles.ScreenToClient(&pt); 

		if (PtInRect(&rc, pt)) 
		{ 
			ctrlProfiles.ClientToScreen(&pt);
			if(ctrlProfiles.GetSelectedCount() >= 1) {
				DetectionEntry de;
				
				int x = ctrlProfiles.GetSelectedIndex();
				DetectionManager::getInstance()->getDetectionItem(ctrlProfiles.GetItemData(x), de, isUserInfo);
				ChangeRawCheatDlg dlg(de.rawToSend);
				dlg.name = de.name;
				dlg.cheatingDescription = de.cheat;
				dlg.raw = de.rawToSend;
				if(dlg.DoModal() == IDOK) {
					int i = -1;
					while((i = ctrlProfiles.GetNextItem(i, LVNI_SELECTED)) != -1) {
						DetectionManager::getInstance()->getDetectionItem(ctrlProfiles.GetItemData(i), de, isUserInfo);
						if (i == x) {
							de.cheat = dlg.cheatingDescription;
						}
						de.rawToSend = dlg.raw;
						DetectionManager::getInstance()->updateDetectionItem(ctrlProfiles.GetItemData(i), de, isUserInfo);
						ctrlProfiles.SetItemText(i, 2, RawManager::getInstance()->getNameActionId(de.rawToSend).c_str());
					}
				}
			}
			return TRUE; 
		}
	}
	
	return FALSE; 
}
//END
void ClientsPage::reload() {
	ctrlProfiles.SetRedraw(FALSE);
	ctrlProfiles.DeleteAllItems();
	DetectionManager::getInstance()->reload(isUserInfo);
	const DetectionManager::DetectionItems& lst = DetectionManager::getInstance()->getProfiles(isUserInfo);
	for(DetectionManager::DetectionItems::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const DetectionEntry& de = *i;
		addEntry(de, ctrlProfiles.GetItemCount());
	}
	SetDlgItemText(IDC_PROFILE_COUNT, Text::toT(STRING(PROFILE_COUNT) + ": " + Util::toString(ctrlProfiles.GetItemCount())).c_str());
	SetDlgItemText(IDC_PROFILE_VERSION, Text::toT(STRING(PROFILE_VERSION) + ": " + DetectionManager::getInstance()->getProfileVersion()).c_str());
	SetDlgItemText(IDC_PROFILE_MESSAGE, Text::toT(STRING(PROFILE_MESSAGE) + ": " + DetectionManager::getInstance()->getProfileMessage()).c_str());
	ctrlProfiles.SetRedraw(TRUE);
}

void ClientsPage::addEntry(const DetectionEntry& de, int pos) {
	TStringList lst;

	lst.push_back(Text::toT(de.name));
	lst.push_back(Text::toT(de.comment));
	lst.push_back(RawManager::getInstance()->getNameActionId(de.rawToSend));

	ctrlProfiles.insert(pos, lst, 0, (LPARAM)de.Id);
}

void ClientsPage::write() {
	DetectionManager::getInstance()->save();
}
// iDC++
LRESULT ClientsPage::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;

	switch(cd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		{
			try	{
				DetectionEntry de;
				DetectionManager::getInstance()->getDetectionItem(ctrlProfiles.GetItemData(cd->nmcd.dwItemSpec), de, isUserInfo);
				if (de.rawToSend) {
					cd->clrText = SETTING(BAD_CLIENT_COLOUR);
				} else if (!de.cheat.empty()) {
					cd->clrText = SETTING(BAD_FILELIST_COLOUR);
				}
				if(cd->nmcd.dwItemSpec % 2 == 0) {
					cd->clrTextBk = RGB(245, 245, 245);
				}
				return CDRF_NEWFONT | CDRF_NOTIFYSUBITEMDRAW;
			}
			catch(const Exception&)
			{
			}
			catch(...) 
			{
			}
		}
		return CDRF_NOTIFYSUBITEMDRAW;

	default:
		return CDRF_DODEFAULT;
	}
}
// iDC++
/**
 * @file
 * $Id: ClientsPage.cpp 477 2010-01-29 08:59:43Z bigmuscle $
 */
