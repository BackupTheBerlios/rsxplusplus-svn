
#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "MyinfoPage.h"
#include "MyinfoProfileDlg.h"
#include "ChangeRawCheatDlg.h"

PropPage::TextItem MyinfoPage::texts[] = {
	{ IDC_MOVE_UP, ResourceManager::MOVE_UP },
	{ IDC_MOVE_DOWN, ResourceManager::MOVE_DOWN },
	{ IDC_ADD, ResourceManager::ADD },
	{ IDC_CHANGE, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT MyinfoPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);

	CRect rc;

	ctrlProfiles.Attach(GetDlgItem(IDC_LIST));
	ctrlProfiles.GetClientRect(rc);

	ctrlProfiles.InsertColumn(0, CTSTRING(SETTINGS_NAME),	LVCFMT_LEFT, rc.Width() / 3, 0);
	ctrlProfiles.InsertColumn(1, CTSTRING(COMMENT),			LVCFMT_LEFT, rc.Width() / 2 - 18, 1);
	ctrlProfiles.InsertColumn(2, CTSTRING(ACTION),			LVCFMT_LEFT, rc.Width() / 6, 0);
	ctrlProfiles.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);

	// Do specialized reading here
	MyinfoProfile::List lst = ClientProfileManager::getInstance()->getMyinfoProfiles();
	for(MyinfoProfile::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const MyinfoProfile& cp = *i;	
		addEntry(cp, ctrlProfiles.GetItemCount());
	}
	SetDlgItemText(IDC_PROFILE_COUNT, Text::toT(STRING(PROFILE_COUNT) + ": " + Util::toString(ctrlProfiles.GetItemCount())).c_str());
	SetDlgItemText(IDC_PROFILE_VERSION, Text::toT(STRING(PROFILE_VERSION) + ": " + ClientProfileManager::getInstance()->getMyinfoProfileVersion()).c_str());//@Fixme
	return TRUE;
}

LRESULT MyinfoPage::onAddClient(WORD , WORD , HWND , BOOL& ) {
	MyinfoProfileDlg dlg;
	dlg.currentProfileId = -1;
	dlg.itemCount = ctrlProfiles.GetItemCount();

	if(dlg.DoModal() == IDOK) {
		addEntry(ClientProfileManager::getInstance()->addMyinfoProfile(
			Text::fromT(dlg.name),
			Text::fromT(dlg.version), 
			Text::fromT(dlg.tag), 
			Text::fromT(dlg.extendedTag), 
			Text::fromT(dlg.nick),
			Text::fromT(dlg.shared),
			Text::fromT(dlg.email),
			Text::fromT(dlg.status),
			Text::fromT(dlg.cheatingDescription),
			dlg.rawToSend,
			dlg.useExtraVersion,
			Text::fromT(dlg.connection),
			Text::fromT(dlg.comment),
			dlg.id
			), ctrlProfiles.GetItemCount());
	}
	SetDlgItemText(IDC_PROFILE_COUNT, (Text::toT(STRING(PROFILE_COUNT) + ": " + Util::toString(ctrlProfiles.GetItemCount()))).c_str());
	return 0;
}

LRESULT MyinfoPage::onChangeClient(WORD , WORD , HWND , BOOL& ) {
	if(ctrlProfiles.GetSelectedCount() >= 1) {
		int sel = ctrlProfiles.GetSelectedIndex();
		
		MyinfoProfileDlg dlg;

		dlg.currentProfileId = ctrlProfiles.GetItemData(sel);
		dlg.itemCount = ctrlProfiles.GetItemCount();

		if(dlg.DoModal() == IDOK) {
			ctrlProfiles.SetItemText(dlg.currentProfileId, 0, dlg.name.c_str());
			ctrlProfiles.SetItemText(dlg.currentProfileId, 1, dlg.version.c_str());
			dlg.currentProfile.setName(Text::fromT(dlg.name));
			dlg.currentProfile.setVersion(Text::fromT(dlg.version));
			dlg.currentProfile.setTag(Text::fromT(dlg.tag));
			dlg.currentProfile.setExtendedTag(Text::fromT(dlg.extendedTag));
			dlg.currentProfile.setStatus(Text::fromT(dlg.status));
			dlg.currentProfile.setNick(Text::fromT(dlg.nick));
			dlg.currentProfile.setShared(Text::fromT(dlg.shared));
			dlg.currentProfile.setEmail(Text::fromT(dlg.email));
			dlg.currentProfile.setCheatingDescription(Text::fromT(dlg.cheatingDescription));
			dlg.currentProfile.setUseExtraVersion(dlg.useExtraVersion);
			dlg.currentProfile.setConnection(Text::fromT(dlg.connection));
			dlg.currentProfile.setComment(Text::fromT(dlg.comment));
			dlg.currentProfile.setRawToSend(dlg.rawToSend);
			dlg.currentProfile.setProfileId(dlg.id);
			ClientProfileManager::getInstance()->updateMyinfoProfile(dlg.currentProfile);
		}
		ctrlProfiles.SetRedraw(FALSE);
		ctrlProfiles.DeleteAllItems();
		MyinfoProfile::List lst = ClientProfileManager::getInstance()->getMyinfoProfiles();
		for(MyinfoProfile::List::const_iterator j = lst.begin(); j != lst.end(); ++j) {
			const MyinfoProfile& cp = *j;	
			addEntry(cp, ctrlProfiles.GetItemCount());
		}
		ctrlProfiles.SelectItem(sel);
		ctrlProfiles.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT MyinfoPage::onRemoveClient(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	while(ctrlProfiles.GetSelectedCount() >= 1) {
		int i = -1;
		while((i = ctrlProfiles.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ClientProfileManager::getInstance()->removeMyinfoProfile(ctrlProfiles.GetItemData(i));
			ctrlProfiles.DeleteItem(i);
		}
		SetDlgItemText(IDC_PROFILE_COUNT, (Text::toT(STRING(PROFILE_COUNT) + ": " + Util::toString(ctrlProfiles.GetItemCount()))).c_str());
	}
	return 0;
}

LRESULT MyinfoPage::onMoveClientUp(WORD , WORD , HWND , BOOL& ) {
	int i = ctrlProfiles.GetSelectedIndex();
	if(i != -1 && i != 0) {
		int n = ctrlProfiles.GetItemData(i);
		ClientProfileManager::getInstance()->moveMyinfoProfile(n, -1/*, i*/);
		ctrlProfiles.SetRedraw(FALSE);
		ctrlProfiles.DeleteItem(i);
		MyinfoProfile cp;
		ClientProfileManager::getInstance()->getMyinfoProfile(n, cp);
		addEntry(cp, i-1);
		ctrlProfiles.SelectItem(i-1);
		ctrlProfiles.EnsureVisible(i-1, FALSE);
		ctrlProfiles.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT MyinfoPage::onMoveClientDown(WORD , WORD , HWND , BOOL& ) {
	int i = ctrlProfiles.GetSelectedIndex();
	if(i != -1 && i != (ctrlProfiles.GetItemCount()-1) ) {
		int n = ctrlProfiles.GetItemData(i);
		ClientProfileManager::getInstance()->moveMyinfoProfile(n, 1/*, i*/);
		ctrlProfiles.SetRedraw(FALSE);
		ctrlProfiles.DeleteItem(i);
		MyinfoProfile cp;
		ClientProfileManager::getInstance()->getMyinfoProfile(n, cp);
		addEntry(cp, i+1);
		ctrlProfiles.SelectItem(i+1);
		ctrlProfiles.EnsureVisible(i+1, FALSE);
		ctrlProfiles.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT MyinfoPage::onReload(WORD , WORD , HWND , BOOL& ) {
	reload();
	return 0;
}

LRESULT MyinfoPage::onInfoTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	int item = ctrlProfiles.GetHotItem();
	if(item != -1) {
		NMLVGETINFOTIP* lpnmtdi = (NMLVGETINFOTIP*) pnmh;
		MyinfoProfile cp;

		ClientProfileManager::getInstance()->getMyinfoProfile(ctrlProfiles.GetItemData(item), cp);

		tstring infoTip = Text::toT(STRING(NAME) + ": " + cp.getName() +
			"\r\n" + STRING(CHEATING_DESCRIPTION) + ": " + cp.getCheatingDescription() +
			"\r\n" + STRING(ACTION) + ": ") + RawManager::getInstance()->getNameActionId(cp.getRawToSend());

		_tcscpy(lpnmtdi->pszText, infoTip.c_str());
	}
	return 0;
}

LRESULT MyinfoPage::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
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
				MyinfoProfile cp;
				
				int x = ctrlProfiles.GetSelectedIndex();
				ClientProfileManager::getInstance()->getMyinfoProfile(ctrlProfiles.GetItemData(x), cp);
				dlg.name = cp.getName();
				dlg.cheatingDescription = cp.getCheatingDescription();
				dlg.raw = cp.getRawToSend();
				if(dlg.DoModal() == IDOK) {
					int i = -1;
					while((i = ctrlProfiles.GetNextItem(i, LVNI_SELECTED)) != -1) {
						ClientProfileManager::getInstance()->getMyinfoProfile(ctrlProfiles.GetItemData(i), cp);
						if (i == x) {
							cp.setCheatingDescription(dlg.cheatingDescription);
						}
						cp.setRawToSend(dlg.raw);
						ClientProfileManager::getInstance()->updateMyinfoProfile(cp);
						ctrlProfiles.SetItemText(i, 2, RawManager::getInstance()->getNameActionId(cp.getRawToSend()).c_str());
					}
				}
			}
			return TRUE; 
		}
	}
	
	return FALSE; 
}

LRESULT MyinfoPage::onItemchanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	::EnableWindow(GetDlgItem(IDC_MOVE_UP),		(lv->uNewState & LVIS_FOCUSED));
	::EnableWindow(GetDlgItem(IDC_MOVE_DOWN),	(lv->uNewState & LVIS_FOCUSED));
	::EnableWindow(GetDlgItem(IDC_CHANGE),		(lv->uNewState & LVIS_FOCUSED));
	::EnableWindow(GetDlgItem(IDC_REMOVE),		(lv->uNewState & LVIS_FOCUSED));
	return 0;
}

LRESULT MyinfoPage::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	switch(kd->wVKey) {
	case VK_INSERT:
		PostMessage(WM_COMMAND, IDC_ADD, 0);
		break;
	case VK_DELETE:
		PostMessage(WM_COMMAND, IDC_REMOVE, 0);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

void MyinfoPage::reload() {
	ctrlProfiles.SetRedraw(FALSE);
	ctrlProfiles.DeleteAllItems();
	MyinfoProfile::List lst = ClientProfileManager::getInstance()->reloadMyinfoProfiles();
	for(MyinfoProfile::List::const_iterator j = lst.begin(); j != lst.end(); ++j) {
		const MyinfoProfile& cp = *j;	
		addEntry(cp, ctrlProfiles.GetItemCount());
	}
	SetDlgItemText(IDC_PROFILE_VERSION, (Text::toT(STRING(PROFILE_VERSION) + ": " + ClientProfileManager::getInstance()->getMyinfoProfileVersion()).c_str()));
	ctrlProfiles.SetRedraw(TRUE);
}

void MyinfoPage::addEntry(const MyinfoProfile& cp, int pos) {
	TStringList lst;

	lst.push_back(Text::toT(cp.getName()));
	lst.push_back(Text::toT(cp.getComment()));
	lst.push_back(RawManager::getInstance()->getNameActionId(cp.getRawToSend()));
	ctrlProfiles.insert(pos, lst, 0, (LPARAM)cp.getId());
}

void MyinfoPage::write() { ClientProfileManager::getInstance()->saveMyinfoProfiles(); }

LRESULT MyinfoPage::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;

	switch(cd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		{
			try	{
				MyinfoProfile cp;
				ClientProfileManager::getInstance()->getMyinfoProfile(ctrlProfiles.GetItemData(cd->nmcd.dwItemSpec), cp);
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