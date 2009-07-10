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

	/*HTREEITEM item = ctrlList.GetRootItem();
	while(item != NULL) {
		ctrlList.Expand(item, TVE_EXPAND);
		item = ctrlList.GetNextItem(item, TVGN_NEXT);
	}*/

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
	::CheckDlgButton(*this, IDC_CHECK_MYINFO, hub->getCheckUserInfo()		? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemText(IDC_CHECK_PROTECTED_USER, Text::toT(hub->getUserProtected()).c_str());
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
	hub->setCheckUserInfo(RsxUtil::toBool(btn.GetCheck()));

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

	CheckDlgButton(IDC_SHOW_IP, hub->getShowIpOnChat() ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_SHOW_CC, hub->getShowCountryCodeOnChat() ? BST_CHECKED : BST_UNCHECKED);

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
	hub->setShowCountryCodeOnChat(IsDlgButtonChecked(IDC_SHOW_CC) == BST_CHECKED);
	hub->setShowIpOnChat(IsDlgButtonChecked(IDC_SHOW_IP) == BST_CHECKED);

	tstring buf;
	int len = ::GetWindowTextLength(GetDlgItem(IDC_FAV_SEARCH_INTERVAL_BOX)) + 1;
	buf.resize(len);
	GetDlgItemText(IDC_FAV_SEARCH_INTERVAL_BOX, &buf[0], len);
	hub->setSearchInterval(Util::toUInt32(Text::fromT(buf)));
}

LRESULT CFavTabSettings::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc;
	ctrlList.Attach(GetDlgItem(IDC_SETTINGS));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, _T("Code"), LVCFMT_LEFT, 40, 0);
	ctrlList.InsertColumn(1, _T("Name"), LVCFMT_LEFT, (rc.Width() / 2) - 40, 0);
	ctrlList.InsertColumn(2, _T("Value"), LVCFMT_LEFT, (rc.Width() / 2) - 20, 0);
	ctrlList.SetExtendedListViewStyle(/*LVS_EX_INFOTIP | */LVS_EX_FULLROWSELECT);

	const std::map<uint32_t, StringPair>& list = FavoriteManager::getDefHubSettings();
	const HubSettings::SettingsMap& stg = hub->getSettings();

	for(HubSettings::SettingsMap::const_iterator i = stg.begin(); i != stg.end(); ++i) {
		std::map<uint32_t, StringPair>::const_iterator j = list.find(i->first);
		if(j != list.end() && !j->second.second.empty()) {
			TStringList strings;
			strings.push_back(Text::toT(string((const char*)&i->first, 4)));
			strings.push_back(Text::toT(j->second.second));
			strings.push_back(Text::toT(i->second));
			int item = ctrlList.insert(ctrlList.GetItemCount(), strings, NULL, NULL);
			ctrlList.SetItemData(item, (int)i->second.size());
		}
	}
	return 0;
}

void CFavTabSettings::prepareClose() {
	int cnt = ctrlList.GetItemCount();
	for(int i = 0; i < cnt; ++i) {
		tstring buf;
		buf.resize(4);
		ctrlList.GetItemText(i, 0, &buf[0], buf.size()+1);
		const char* code = Text::fromT(buf).c_str();

		buf.resize((int)ctrlList.GetItemData(i));
		ctrlList.GetItemText(i, 2, &buf[0], buf.size()+1);
		hub->set(code, Text::fromT(buf));
	}
}

LRESULT CFavTabSettings::onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
	int sel = ctrlList.GetSelectedIndex();
	if(sel != -1) {
		LineDlg dlg;
		dlg.title = _T("Change Hub Setting");

		tstring buf;
		buf.resize((int)ctrlList.GetItemData(sel));
		ctrlList.GetItemText(sel, 2, &buf[0], buf.size()+1);
		dlg.line = buf;

		buf.resize(1024);
		ctrlList.GetItemText(sel, 1, &buf[0], 1024);
		dlg.description = buf;

		if(dlg.DoModal() == IDOK) {
			ctrlList.SetItemText(sel, 2, dlg.line.c_str());
			ctrlList.SetItemData(sel, (int)dlg.line.size());
		}
	}
	return 0;
}