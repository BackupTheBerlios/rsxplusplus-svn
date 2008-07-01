#include "stdafx.h"
#include "../client/DCPlusPlus.h"

#include "resource.h"
#include "../client/FavoriteManager.h"
#include "../rsx/RsxUtil.h"

#include "FavTabPages.h"

LRESULT CFavTabRaw::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc1, rc2;

	ctrlAction.Attach(GetDlgItem(IDC_FH_ACTION));
	ctrlAction.GetClientRect(rc1);
	ctrlAction.InsertColumn(0, CTSTRING(ACTION), LVCFMT_LEFT, rc1.Width() - 20, 0);
	ctrlAction.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	Action::List& lst = RawManager::getInstance()->getActionList();

	for(Action::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		addEntryAction(i->first, i->second->getName(), FavoriteManager::getInstance()->getActifAction(hub, i->second->getActionId()), ctrlAction.GetItemCount());
	}

	ctrlRaw.Attach(GetDlgItem(IDC_FH_RAW));
	ctrlRaw.GetClientRect(rc2);
	ctrlRaw.InsertColumn(0, _T("Raws or Lua Function"), LVCFMT_LEFT, rc2.Width() - 20, 0);
	ctrlRaw.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	return 0;
}

void CFavTabRaw::addEntryAction(int id, const string name, bool actif, int pos) {
	TStringList lst;

	lst.push_back(Text::toT(name));
	int i = ctrlAction.insert(pos, lst, 0, (LPARAM)id);
	ctrlAction.SetCheckState(i, actif);
}

void CFavTabRaw::addEntryRaw(const Action::Raw& ra, int pos, int actionId) {
	TStringList lst;

	lst.push_back(Text::toT(ra.getName()));
	int i = ctrlRaw.insert(pos, lst, 0, (LPARAM)ra.getRawId());
	ctrlRaw.SetCheckState(i, FavoriteManager::getInstance()->getActifRaw(hub, actionId, ra.getRawId()));
}

LRESULT CFavTabRaw::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;

	if(l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) == (l->uOldState & LVIS_STATEIMAGEMASK))) {
		int j;
		for(j = 0; j < ctrlRaw.GetItemCount(); j++) {
			FavoriteManager::getInstance()->setActifRaw(hub, RawManager::getInstance()->getActionId(ctrlAction.GetItemData(l->iItem)), ctrlRaw.GetItemData(j), RsxUtil::toBool(ctrlRaw.GetCheckState(j)));
		}
		ctrlRaw.SetRedraw(FALSE);
		ctrlRaw.DeleteAllItems();
		if(ctrlAction.GetSelectedCount() == 1) {
			Action::RawsList lst = RawManager::getInstance()->getRawList(ctrlAction.GetItemData(l->iItem));

			for(Action::RawsList::const_iterator i = lst.begin(); i != lst.end(); ++i) {
				const Action::Raw& ra = *i;
				addEntryRaw(ra, ctrlRaw.GetItemCount(), RawManager::getInstance()->getActionId(ctrlAction.GetItemData(l->iItem)));
			}
		}
		ctrlRaw.SetRedraw(TRUE);
	}
	return 0;
}

void CFavTabRaw::prepareClose() {
	int i;
	for(i = 0; i < ctrlAction.GetItemCount(); i++) {
		FavoriteManager::getInstance()->setActifAction(hub, RawManager::getInstance()->getActionId(ctrlAction.GetItemData(i)), RsxUtil::toBool(ctrlAction.GetCheckState(i)));
	}
	if(ctrlAction.GetSelectedCount() == 1) {
		int j = ctrlAction.GetNextItem(-1, LVNI_SELECTED);
		int l;
		for(l = 0; l < ctrlRaw.GetItemCount(); l++) {
			FavoriteManager::getInstance()->setActifRaw(hub, RawManager::getInstance()->getActionId(ctrlAction.GetItemData(j)), ctrlRaw.GetItemData(l), RsxUtil::toBool(ctrlRaw.GetCheckState(l)));
		}
	}
}

LRESULT CFavTabOp::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	::CheckDlgButton(*this, IDC_CHECK_ON_CONNECT, hub->getCheckOnConnect()	? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_CLIENTS, hub->getCheckClients()		? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_FILELISTS, hub->getCheckFilelists()	? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_MYINFO, hub->getCheckMyInfo()			? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemText(IDC_CHECK_PROTECTED_USER, Text::toT(hub->getUserProtected()).c_str());
	SetDlgItemText(IDC_FAV_MIN_USERS_LIMIT, Util::toStringW(hub->getUsersLimit()).c_str());

	CUpDownCtrl spin;
	spin.Attach(GetDlgItem(IDC_FAV_MIN_USERS_LIMIT_SPIN));
	spin.SetRange32(0, 1024*1024);
	spin.Detach();
	return TRUE;
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
	hub->setUserProtected(Text::fromT(buf));
}

LRESULT CCustomTab::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	SetDlgItemText(IDC_FAV_SEARCH_INTERVAL, CTSTRING(MINIMUM_SEARCH_INTERVAL));
	SetDlgItemText(IDC_FAV_SEARCH_INTERVAL_BOX, Util::toStringW(hub->getSearchInterval()).c_str());
	CUpDownCtrl updown;
	updown.Attach(GetDlgItem(IDC_FAV_SEARCH_INTERVAL_SPIN));
	updown.SetRange32(10, 9999);
	updown.Detach();

	::CheckDlgButton(*this, IDC_STEALTH, hub->getStealth()					? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_HIDE_SHARE, hub->getHideShare()				? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_USE_FILTER_FAV, hub->getUseFilter()			? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_USE_HIGHLIGHT_FAV, hub->getUseHL()			? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_AUTOSEARCH, hub->getAutosearch()		? BST_CHECKED : BST_UNCHECKED);
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
	hub->setAutosearch(RsxUtil::toBool(btn.GetCheck()));

	btn = ::GetDlgItem(m_hWnd, IDC_USE_HIGHLIGHT_FAV);
	hub->setUseHL(RsxUtil::toBool(btn.GetCheck()));

	tstring buf;
	int len = ::GetWindowTextLength(GetDlgItem(IDC_FAV_SEARCH_INTERVAL_BOX)) + 1;
	buf.resize(len);
	GetDlgItemText(IDC_FAV_SEARCH_INTERVAL_BOX, &buf[0], len);
	hub->setSearchInterval(Util::toUInt32(Text::fromT(buf)));
}