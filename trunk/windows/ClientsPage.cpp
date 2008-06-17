
#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "Resource.h"

#include "ClientsPage.h"
#include "ClientProfileDlg.h"
#include "ChangeRawCheatDlg.h"

#include "../client/ClientProfileManager.h"
//#include "../client/RawManager.h"

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

	ctrlProfiles.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);

	// Do specialized reading here
	ClientProfile::List lst = ClientProfileManager::getInstance()->getClientProfiles();
	for(ClientProfile::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const ClientProfile& cp = *i;	
		addEntry(cp, ctrlProfiles.GetItemCount());
	}
//RSX++
	SetDlgItemText(IDC_PROFILE_COUNT, Text::toT(STRING(PROFILE_COUNT) + ": " + Util::toString(ctrlProfiles.GetItemCount())).c_str());
	SetDlgItemText(IDC_PROFILE_VERSION, Text::toT(STRING(PROFILE_VERSION) + ": " + ClientProfileManager::getInstance()->getProfileVersion()).c_str());//@Fixme
//END
	return TRUE;
}

LRESULT ClientsPage::onAddClient(WORD , WORD , HWND , BOOL& ) {
	ClientProfileDlg dlg;
	dlg.currentProfileId = -1;
	dlg.itemCount = ctrlProfiles.GetItemCount();

	if(dlg.DoModal() == IDOK) {
		addEntry(ClientProfileManager::getInstance()->addClientProfile(Text::fromT(dlg.name), 
			Text::fromT(dlg.version), 
			Text::fromT(dlg.tag), 
			Text::fromT(dlg.extendedTag), 
			Text::fromT(dlg.lock), 
			Text::fromT(dlg.pk), 
			Text::fromT(dlg.supports), 
			Text::fromT(dlg.testSUR), 
			Text::fromT(dlg.userConCom), 
			Text::fromT(dlg.status),
			Text::fromT(dlg.cheatingDescription),
			dlg.rawToSend, 
			dlg.useExtraVersion,
			dlg.checkMismatch,
			Text::fromT(dlg.connection),
			Text::fromT(dlg.comment),
			false, //recheck
			0, //key
			dlg.id
			), ctrlProfiles.GetItemCount());
		
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
		
		ClientProfileDlg dlg;

		dlg.currentProfileId = ctrlProfiles.GetItemData(sel);
		dlg.itemCount = ctrlProfiles.GetItemCount();

		if(dlg.DoModal() == IDOK) {
			ctrlProfiles.SetItemText(dlg.currentProfileId, 0, dlg.name.c_str());
			ctrlProfiles.SetItemText(dlg.currentProfileId, 1, dlg.version.c_str());
			dlg.currentProfile.setProfileId(dlg.id);
			dlg.currentProfile.setName(Text::fromT(dlg.name));
			dlg.currentProfile.setVersion(Text::fromT(dlg.version));
			dlg.currentProfile.setTag(Text::fromT(dlg.tag));
			dlg.currentProfile.setExtendedTag(Text::fromT(dlg.extendedTag));
			dlg.currentProfile.setLock(Text::fromT(dlg.lock));
			dlg.currentProfile.setPk(Text::fromT(dlg.pk));
			dlg.currentProfile.setSupports(Text::fromT(dlg.supports));
			dlg.currentProfile.setTestSUR(Text::fromT(dlg.testSUR));
			dlg.currentProfile.setUserConCom(Text::fromT(dlg.userConCom));
			dlg.currentProfile.setStatus(Text::fromT(dlg.status));
			dlg.currentProfile.setCheatingDescription(Text::fromT(dlg.cheatingDescription));
			dlg.currentProfile.setRawToSend(dlg.rawToSend);
			dlg.currentProfile.setUseExtraVersion(dlg.useExtraVersion);
			dlg.currentProfile.setCheckMismatch(dlg.checkMismatch);
			dlg.currentProfile.setConnection(Text::fromT(dlg.connection));
			dlg.currentProfile.setComment(Text::fromT(dlg.comment));
//			dlg.currentProfile.setRecheck(dlg.recheck);
//			dlg.currentProfile.setKeyType(dlg.key);
			ClientProfileManager::getInstance()->updateClientProfile(dlg.currentProfile);
		}
		ctrlProfiles.SetRedraw(FALSE);
		ctrlProfiles.DeleteAllItems();
		ClientProfile::List lst = ClientProfileManager::getInstance()->getClientProfiles();
		for(ClientProfile::List::const_iterator j = lst.begin(); j != lst.end(); ++j) {
			const ClientProfile& cp = *j;	
			addEntry(cp, ctrlProfiles.GetItemCount());
		}
		ctrlProfiles.SelectItem(sel);
		ctrlProfiles.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT ClientsPage::onRemoveClient(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlProfiles.GetSelectedCount() == 1) {
		int i = ctrlProfiles.GetNextItem(-1, LVNI_SELECTED);
		ClientProfileManager::getInstance()->removeClientProfile(ctrlProfiles.GetItemData(i));
		ctrlProfiles.DeleteItem(i);
	}
	return 0;
}

LRESULT ClientsPage::onMoveClientUp(WORD , WORD , HWND , BOOL& ) {
	int i = ctrlProfiles.GetSelectedIndex();
	if(i != -1 && i != 0) {
		int n = ctrlProfiles.GetItemData(i);
		ClientProfileManager::getInstance()->moveClientProfile(n, -1);
		ctrlProfiles.SetRedraw(FALSE);
		ctrlProfiles.DeleteItem(i);
		ClientProfile cp;
		ClientProfileManager::getInstance()->getClientProfile(n, cp);
		addEntry(cp, i-1);
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
		ClientProfileManager::getInstance()->moveClientProfile(n, 1);
		ctrlProfiles.SetRedraw(FALSE);
		ctrlProfiles.DeleteItem(i);
		ClientProfile cp;
		ClientProfileManager::getInstance()->getClientProfile(n, cp);
		addEntry(cp, i+1);
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
		ClientProfile cp;

		ClientProfileManager::getInstance()->getClientProfile(ctrlProfiles.GetItemData(item), cp);

		tstring infoTip = Text::toT(STRING(NAME) + ": " + cp.getName() +
			"\r\n" + STRING(COMMENT) + ": " + cp.getComment() +
			"\r\n" + STRING(CHEATING_DESCRIPTION) + ": " + cp.getCheatingDescription() +
			"\r\n" + STRING(ACTION) + ": ") + RawManager::getInstance()->getNameActionId(cp.getRawToSend());

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
				ChangeRawCheatDlg dlg;
				ClientProfile cp;
				
				int x = ctrlProfiles.GetSelectedIndex();
				ClientProfileManager::getInstance()->getClientProfile(ctrlProfiles.GetItemData(x), cp);
				dlg.name = cp.getName();
				dlg.cheatingDescription = cp.getCheatingDescription();
				dlg.raw = cp.getRawToSend();
				if(dlg.DoModal() == IDOK) {
					int i = -1;
					while((i = ctrlProfiles.GetNextItem(i, LVNI_SELECTED)) != -1) {
						ClientProfileManager::getInstance()->getClientProfile(ctrlProfiles.GetItemData(i), cp);
						if (i == x) {
							cp.setCheatingDescription(dlg.cheatingDescription);
						}
						cp.setRawToSend(dlg.raw);
						ClientProfileManager::getInstance()->updateClientProfile(cp);
						ctrlProfiles.SetItemText(i, 2, RawManager::getInstance()->getNameActionId(cp.getRawToSend()).c_str());
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
	ClientProfile::List lst = ClientProfileManager::getInstance()->reloadClientProfiles();
	for(ClientProfile::List::const_iterator j = lst.begin(); j != lst.end(); ++j) {
		const ClientProfile& cp = *j;	
		addEntry(cp, ctrlProfiles.GetItemCount());
	}
	SetDlgItemText(IDC_PROFILE_VERSION, (Text::toT(STRING(PROFILE_VERSION) + ": " + ClientProfileManager::getInstance()->getProfileVersion()).c_str()));
	ctrlProfiles.SetRedraw(TRUE);
}

void ClientsPage::addEntry(const ClientProfile& cp, int pos) {
	TStringList lst;

	lst.push_back(Text::toT(cp.getName()));
	lst.push_back(Text::toT(cp.getComment()));
	lst.push_back(RawManager::getInstance()->getNameActionId(cp.getRawToSend()));

	ctrlProfiles.insert(pos, lst, 0, (LPARAM)cp.getId());
}

void ClientsPage::write() {
	ClientProfileManager::getInstance()->saveClientProfiles();
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
				ClientProfile cp;
				ClientProfileManager::getInstance()->getClientProfile(ctrlProfiles.GetItemData(cd->nmcd.dwItemSpec), cp);
				if (cp.getRawToSend()) {
					cd->clrText = SETTING(BAD_CLIENT_COLOUR);
				} else if (!cp.getCheatingDescription().empty()) {
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
 * $Id: ClientsPage.cpp 298 2007-07-06 13:59:59Z bigmuscle $
 */
