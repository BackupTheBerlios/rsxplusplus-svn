#include "stdafx.h"
#include "../client/DCPlusPlus.h"

#include "resource.h"
#include "../client/FavoriteManager.h"
#include "../client/version.h"
#include "../rsx/RsxUtil.h"
#include "../rsx/RegexUtil.h"

#include "FavTabPages.h"
#include "LineDlg.h"

LRESULT CFavTabRaw::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	ctrlList.Attach(GetDlgItem(IDC_ACTIONS));
	// nasty workaround to get checkboxes created in the right way
	ctrlList.ModifyStyle(TVS_CHECKBOXES, 0); 
	ctrlList.ModifyStyle(0, TVS_CHECKBOXES);

	ctrlList.SetRedraw(FALSE);
	const Action::ActionList& a = RawManager::getInstance()->getActions();
	for(Action::ActionList::const_iterator i = a.begin(); i != a.end(); ++i) {
		HTREEITEM actionItem = addAction(*i);
		for(Action::RawsList::const_iterator j = (*i)->raw.begin(); j != (*i)->raw.end(); ++j)
			addRaw(actionItem, (*i)->getId(), &(*j));
	}

	ctrlList.SetRedraw(TRUE);
	ctrlList.Invalidate();
	return 0;
}

HTREEITEM CFavTabRaw::addAction(const Action* action) {
	HTREEITEM item = ctrlList.InsertItem(Text::toT(action->getName()).c_str(), 0, 0);
	ctrlList.SetItemData(item, (DWORD_PTR)action);
	ctrlList.SetCheckState(item, FavoriteManager::getInstance()->getEnabledAction(hub, action->getId()));
	return item;
}

void CFavTabRaw::addRaw(HTREEITEM action, int actionId, const Raw* raw) {
	HTREEITEM item = ctrlList.InsertItem(Text::toT(raw->getName()).c_str(), action, 0);
	ctrlList.SetItemData(item, (DWORD_PTR)raw);
	ctrlList.SetCheckState(item, FavoriteManager::getInstance()->getEnabledRaw(hub, actionId, raw->getId()));
}

void CFavTabRaw::prepareClose() {
	HTREEITEM item = ctrlList.GetRootItem();
	while(item != NULL) {
		HTREEITEM child = ctrlList.GetChildItem(item);
		Action* action = (Action*)ctrlList.GetItemData(item);
		if(action != NULL) {
			FavoriteManager::getInstance()->setEnabledAction(hub, action->getId(), ctrlList.GetCheckState(item) ? true : false);
			while(child != NULL) {
				Raw* raw = (Raw*)ctrlList.GetItemData(child);
				if(raw != NULL) {
					FavoriteManager::getInstance()->setEnabledRaw(hub, action->getId(), raw->getId(), ctrlList.GetCheckState(child) ? true : false);
				}
				child = ctrlList.GetNextSiblingItem(child);
			}
		}
		item = ctrlList.GetNextItem(item, TVGN_NEXT);
	}
}

LRESULT CFavTabOp::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	::CheckDlgButton(*this, IDC_CHECK_ON_CONNECT, hub->getCheckOnConnect()	? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_CLIENTS, hub->getCheckClients()		? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_FILELISTS, hub->getCheckFilelists()	? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_MYINFO, hub->getCheckMyInfo()			? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemText(IDC_CHECK_PROTECTED_USER, Text::toT(hub->getProtectedUsers()).c_str());
	SetDlgItemText(IDC_FAV_MIN_USERS_LIMIT, Util::toStringW(hub->getUsersLimit()).c_str());

	CUpDownCtrl spin;
	spin.Attach(GetDlgItem(IDC_FAV_MIN_USERS_LIMIT_SPIN));
	spin.SetRange32(0, 1024*1024);
	spin.Detach();
	return TRUE;
}

LRESULT CFavTabOp::onMatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring pattern = Util::emptyStringT, nick = Util::emptyStringT;
	int len = ::GetWindowTextLength(GetDlgItem(IDC_CHECK_PROTECTED_USER)) + 1;
	pattern.resize(len);
	GetDlgItemText(IDC_CHECK_PROTECTED_USER, &pattern[0], len);

	len = ::GetWindowTextLength(GetDlgItem(IDC_EDIT1)) + 1;
	nick.resize(len);
	GetDlgItemText(IDC_EDIT1, &nick[0], len);

	MessageBox(Text::toT(RegexUtil::matchExp(Text::fromT(pattern), Text::fromT(nick), true)).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONINFORMATION);
	return 0;
}

void CFavTabOp::prepareClose() {
	CButton btn = ::GetDlgItem(m_hWnd, IDC_CHECK_ON_CONNECT);
	hub->setCheckOnConnect(RsxUtil::toBool(btn.GetCheck()));
		
	btn = ::GetDlgItem(m_hWnd, IDC_CHECK_CLIENTS);
	hub->setCheckClients(RsxUtil::toBool(btn.GetCheck()));

	btn = ::GetDlgItem(m_hWnd, IDC_CHECK_FILELISTS);
	hub->setCheckFilelists(RsxUtil::toBool(btn.GetCheck()));

	btn = ::GetDlgItem(m_hWnd, IDC_CHECK_MYINFO);
	hub->setCheckMyInfo(RsxUtil::toBool(btn.GetCheck()));

	TCHAR buf[512];
	GetDlgItemText(IDC_FAV_MIN_USERS_LIMIT, buf, 128);
	hub->setUsersLimit(Util::toInt(Text::fromT(buf)));

	GetDlgItemText(IDC_CHECK_PROTECTED_USER, buf, 512);
	hub->setProtectedUsers(Text::fromT(buf));
}

LRESULT CCustomTab::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	SetDlgItemText(IDC_FAV_SEARCH_INTERVAL, CTSTRING(MINIMUM_SEARCH_INTERVAL));
	SetDlgItemText(IDC_FAV_SEARCH_INTERVAL_BOX, Util::toStringW(hub->getSearchInterval()).c_str());
	CUpDownCtrl updown;
	updown.Attach(GetDlgItem(IDC_FAV_SEARCH_INTERVAL_SPIN));
	updown.SetRange32(10, 9999);
	updown.Detach();

	CheckDlgButton(IDC_STEALTH, hub->getStealth()					? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_HIDE_SHARE, hub->getHideShare()				? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_USE_FILTER_FAV, hub->getUseFilter()			? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_USE_HIGHLIGHT_FAV, hub->getUseHL()			? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CHECK_AUTOSEARCH, hub->getUseAutosearch()	? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemText(IDC_CHAT_EXTRA_INFO, Text::toT(hub->getChatExtraInfo()).c_str());
	return 0;
}

void CCustomTab::prepareClose() {
	CButton btn = ::GetDlgItem(m_hWnd, IDC_STEALTH);
	hub->setStealth(RsxUtil::toBool(btn.GetCheck()));

	btn = ::GetDlgItem(m_hWnd, IDC_HIDE_SHARE);
	hub->setHideShare(RsxUtil::toBool(btn.GetCheck()));

	btn = ::GetDlgItem(m_hWnd, IDC_USE_FILTER_FAV);
	hub->setUseFilter(RsxUtil::toBool(btn.GetCheck()));

	btn = ::GetDlgItem(m_hWnd, IDC_CHECK_AUTOSEARCH);
	hub->setUseAutosearch(RsxUtil::toBool(btn.GetCheck()));

	btn = ::GetDlgItem(m_hWnd, IDC_USE_HIGHLIGHT_FAV);
	hub->setUseHL(RsxUtil::toBool(btn.GetCheck()));

	tstring buf;
	buf.resize(::GetWindowTextLength(GetDlgItem(IDC_FAV_SEARCH_INTERVAL_BOX)) + 1);
	buf.resize(GetDlgItemText(IDC_FAV_SEARCH_INTERVAL_BOX, &buf[0], buf.size()));
	hub->setSearchInterval(Util::toUInt32(Text::fromT(buf)));

	buf.resize(::GetWindowTextLength(GetDlgItem(IDC_CHAT_EXTRA_INFO)) + 1);
	buf.resize(GetDlgItemText(IDC_CHAT_EXTRA_INFO, &buf[0], buf.size()));
	hub->setChatExtraInfo(Text::fromT(buf));
}

LRESULT CFavTabSettings::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc;
	ctrlList.Attach(GetDlgItem(IDC_SETTINGS));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, _T("Name"), LVCFMT_LEFT, (rc.Width() / 2) - 20, 0);
	ctrlList.InsertColumn(1, _T("Value"), LVCFMT_LEFT, (rc.Width() / 2), 0);
	ctrlList.SetExtendedListViewStyle(/*LVS_EX_INFOTIP | */LVS_EX_FULLROWSELECT);

#pragma message("//@todo: plug settings here")

	return 0;
}

void CFavTabSettings::prepareClose() {
	int cnt = ctrlList.GetItemCount();
	for(int i = 0; i < cnt; ++i) {

	}
}

LRESULT CFavTabSettings::onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
	int sel = ctrlList.GetSelectedIndex();
	if(sel != -1) {

	}
	return 0;
}