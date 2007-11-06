/* 
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
 *
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
#include "WinUtil.h"

#include "FavHubProperties.h"

#include "../client/FavoriteManager.h"
#include "../client/ResourceManager.h"
//#include "../client/pme.h" //RSX++
#include "../client/StringTokenizer.h"
#include "../rsx/RsxUtil.h"

LRESULT FavHubProperties::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	// Translate dialog
	SetWindowText(CTSTRING(FAVORITE_HUB_PROPERTIES));
	SetDlgItemText(IDC_FH_HUB, CTSTRING(HUB));
	SetDlgItemText(IDC_FH_IDENT, CTSTRING(FAVORITE_HUB_IDENTITY));
	SetDlgItemText(IDC_FH_NAME, CTSTRING(HUB_NAME));
	SetDlgItemText(IDC_FH_ADDRESS, CTSTRING(HUB_ADDRESS));
	SetDlgItemText(IDC_FH_HUB_DESC, CTSTRING(DESCRIPTION));
	SetDlgItemText(IDC_FH_NICK, CTSTRING(NICK));
	SetDlgItemText(IDC_FH_PASSWORD, CTSTRING(PASSWORD));
	SetDlgItemText(IDC_FH_USER_DESC, CTSTRING(DESCRIPTION));
	SetDlgItemText(IDC_DEFAULT, CTSTRING(DEFAULT));
	SetDlgItemText(IDC_ACTIVE, CTSTRING(SETTINGS_DIRECT));
	SetDlgItemText(IDC_PASSIVE, CTSTRING(SETTINGS_FIREWALL_PASSIVE));
	SetDlgItemText(IDC_STEALTH, CTSTRING(STEALTH_MODE));

	// Fill in values
	SetDlgItemText(IDC_HUBNAME, Text::toT(entry->getName()).c_str());
	SetDlgItemText(IDC_HUBDESCR, Text::toT(entry->getDescription()).c_str());
	SetDlgItemText(IDC_HUBADDR, Text::toT(entry->getServer()).c_str());
	SetDlgItemText(IDC_HUBNICK, Text::toT(entry->getNick(false)).c_str());
	SetDlgItemText(IDC_HUBPASS, Text::toT(entry->getPassword()).c_str());
	SetDlgItemText(IDC_HUBUSERDESCR, Text::toT(entry->getUserDescription()).c_str());
	SetDlgItemText(IDC_SERVER, Text::toT(entry->getIP()).c_str());
//RSX++
	SetDlgItemText(IDC_FAV_EMAIL, Text::toT(entry->getFavEmail()).c_str());
	SetDlgItemText(IDC_FAV_AWAY_MSG, Text::toT(entry->getAwayMsg()).c_str());
	SetDlgItemText(IDC_CHECK_PROTECTED_USER, Text::toT(entry->getUserProtected()).c_str());
	SetDlgItemText(IDC_FAV_MIN_USERS_LIMIT, Util::toStringW(entry->getUsersLimit()).c_str());
	ProtectedUsers = entry->getUserProtected();

	::CheckDlgButton(*this, IDC_STEALTH, entry->getStealth()					? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_ON_CONNECT, entry->getCheckOnConnect()	? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_CLIENTS, entry->getCheckClients()			? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_FILELISTS, entry->getCheckFilelists()		? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_MYINFO, entry->getCheckMyInfo()			? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_HIDE_SHARE, entry->getHideShare()				? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_FAKE_SHARE, entry->getCheckFakeShare()	? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_USE_FILTER_FAV, entry->getUseFilter()			? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_USE_HIGHLIGHT_FAV, entry->getUseHL()			? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(*this, IDC_CHECK_AUTOSEARCH, entry->getAutosearch()		? BST_CHECKED : BST_UNCHECKED);
//END
	if(entry->getMode() == 0)
		CheckRadioButton(IDC_ACTIVE, IDC_DEFAULT, IDC_DEFAULT);
	else if(entry->getMode() == 1)
		CheckRadioButton(IDC_ACTIVE, IDC_DEFAULT, IDC_ACTIVE);
	else if(entry->getMode() == 2)
		CheckRadioButton(IDC_ACTIVE, IDC_DEFAULT, IDC_PASSIVE);

	CEdit tmp;
	tmp.Attach(GetDlgItem(IDC_HUBNAME));
	tmp.SetFocus();
	tmp.SetSel(0,-1);
	tmp.Detach();
	tmp.Attach(GetDlgItem(IDC_HUBNICK));
	tmp.LimitText(35);
	tmp.Detach();
	tmp.Attach(GetDlgItem(IDC_HUBUSERDESCR));
	tmp.LimitText(50);
	tmp.Detach();
	tmp.Attach(GetDlgItem(IDC_HUBPASS));
	tmp.SetPasswordChar('*');
	tmp.Detach();
	//RSX++
	tmp.Attach(GetDlgItem(IDC_FAV_EMAIL));
	tmp.LimitText(50);
	tmp.Detach();
	tmp.Attach(GetDlgItem(IDC_FAV_AWAY_MSG));
	tmp.LimitText(2048);
	tmp.Detach();

	CUpDownCtrl spin;
	spin.Attach(GetDlgItem(IDC_FAV_MIN_USERS_LIMIT_SPIN));
	spin.SetRange32(0, 1024*1024);
	spin.Detach();
	//END
	CenterWindow(GetParent());

	//RSX++ //Raw Manager
	CRect rc1, rc2;

	ctrlAction.Attach(GetDlgItem(IDC_FH_ACTION));
	ctrlAction.GetClientRect(rc1);
	ctrlAction.InsertColumn(0, CTSTRING(ACTION), LVCFMT_LEFT, rc1.Width() - 20, 0);
	ctrlAction.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	Action::List lst = RawManager::getInstance()->getActionList();

	for(Action::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		addEntryAction(i->first, i->second->getName(), FavoriteManager::getInstance()->getActifAction(entry, i->second->getActionId()), ctrlAction.GetItemCount());
	}

	ctrlRaw.Attach(GetDlgItem(IDC_FH_RAW));
	ctrlRaw.GetClientRect(rc2);
	ctrlRaw.InsertColumn(0, _T("Raw"), LVCFMT_LEFT, rc2.Width() - 20, 0);
	ctrlRaw.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	::EnableWindow(GetDlgItem(IDC_FH_RAW), false);

	nosave = false;
	gotFocusOnAction = false;
	//END
	return FALSE;
}

LRESULT FavHubProperties::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(wID == IDOK)
	{
		TCHAR buf[512];
		GetDlgItemText(IDC_HUBADDR, buf, 256);
		if(buf[0] == '\0') {
			MessageBox(CTSTRING(INCOMPLETE_FAV_HUB), _T(""), MB_ICONWARNING | MB_OK);
			return 0;
		}
		entry->setServer(Text::fromT(buf));
		GetDlgItemText(IDC_HUBNAME, buf, 256);
		entry->setName(Text::fromT(buf));
		GetDlgItemText(IDC_HUBDESCR, buf, 256);
		entry->setDescription(Text::fromT(buf));
		GetDlgItemText(IDC_HUBNICK, buf, 256);
		entry->setNick(Text::fromT(buf));
		GetDlgItemText(IDC_HUBPASS, buf, 256);
		entry->setPassword(Text::fromT(buf));
		GetDlgItemText(IDC_HUBUSERDESCR, buf, 256);
		entry->setUserDescription(Text::fromT(buf));
		
		//RSX++
		GetDlgItemText(IDC_CHECK_PROTECTED_USER, buf, 512);
		entry->setUserProtected(Text::fromT(buf));

		GetDlgItemText(IDC_FAV_EMAIL, buf, 256);
		entry->setFavEmail(Text::fromT(buf));

		GetDlgItemText(IDC_FAV_AWAY_MSG, buf, 256);
		entry->setAwayMsg(Text::fromT(buf));

		GetDlgItemText(IDC_FAV_MIN_USERS_LIMIT, buf, 128);
		entry->setUsersLimit(Util::toInt(Text::fromT(buf)));

		CButton btn = ::GetDlgItem(m_hWnd, IDC_STEALTH);
		entry->setStealth(RsxUtil::toBool(btn.GetCheck()));

		btn = ::GetDlgItem(m_hWnd, IDC_CHECK_ON_CONNECT);
		entry->setCheckOnConnect(RsxUtil::toBool(btn.GetCheck()));

		btn = ::GetDlgItem(m_hWnd, IDC_CHECK_CLIENTS);
		entry->setCheckClients(RsxUtil::toBool(btn.GetCheck()));

		btn = ::GetDlgItem(m_hWnd, IDC_CHECK_FILELISTS);
		entry->setCheckFilelists(RsxUtil::toBool(btn.GetCheck()));

		btn = ::GetDlgItem(m_hWnd, IDC_CHECK_MYINFO);
		entry->setCheckMyInfo(RsxUtil::toBool(btn.GetCheck()));

		btn = ::GetDlgItem(m_hWnd, IDC_HIDE_SHARE);
		entry->setHideShare(RsxUtil::toBool(btn.GetCheck()));

		btn = ::GetDlgItem(m_hWnd, IDC_CHECK_FAKE_SHARE);
		entry->setCheckFakeShare(RsxUtil::toBool(btn.GetCheck()));

		btn = ::GetDlgItem(m_hWnd, IDC_USE_FILTER_FAV);
		entry->setUseFilter(RsxUtil::toBool(btn.GetCheck()));

		btn = ::GetDlgItem(m_hWnd, IDC_CHECK_AUTOSEARCH);
		entry->setAutosearch(RsxUtil::toBool(btn.GetCheck()));

		btn = ::GetDlgItem(m_hWnd, IDC_USE_HIGHLIGHT_FAV);
		entry->setUseHL(RsxUtil::toBool(btn.GetCheck()));
		//END

		GetDlgItemText(IDC_SERVER, buf, 512);
		entry->setIP(Text::fromT(buf));

		int	ct = -1;
		if(IsDlgButtonChecked(IDC_DEFAULT))
			ct = 0;
		else if(IsDlgButtonChecked(IDC_ACTIVE))
			ct = 1;
		else if(IsDlgButtonChecked(IDC_PASSIVE))
			ct = 2;

		entry->setMode(ct);

		//RSX++ //Raw Manager
		int i;
		for(i = 0; i < ctrlAction.GetItemCount(); i++) {
			FavoriteManager::getInstance()->setActifAction(entry, RawManager::getInstance()->getActionId(ctrlAction.GetItemData(i)), RsxUtil::toBool(ctrlAction.GetCheckState(i)));
		}
		if(ctrlAction.GetSelectedCount() == 1) {
			int j = ctrlAction.GetNextItem(-1, LVNI_SELECTED);
			int l;
			for(l = 0; l < ctrlRaw.GetItemCount(); l++) {
				FavoriteManager::getInstance()->setActifRaw(entry, RawManager::getInstance()->getActionId(ctrlAction.GetItemData(j)), ctrlRaw.GetItemData(l), RsxUtil::toBool(ctrlRaw.GetCheckState(l)));
			}
		}
		//END	

		FavoriteManager::getInstance()->save();
	}
	EndDialog(wID);
	return 0;
}

LRESULT FavHubProperties::OnTextChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	TCHAR buf[256];

	GetDlgItemText(wID, buf, 256);
	tstring old = buf;

	// Strip '$', '|' and ' ' from text
	TCHAR *b = buf, *f = buf, c;
	while( (c = *b++) != 0 )
	{
		if(c != '$' && c != '|' && (wID == IDC_HUBUSERDESCR || c != ' ') && ( (wID != IDC_HUBNICK) || (c != '<' && c != '>')) )
			*f++ = c;
	}

	*f = '\0';

	if(old != buf)
	{
		// Something changed; update window text without changing cursor pos
		CEdit tmp;
		tmp.Attach(hWndCtl);
		int start, end;
		tmp.GetSel(start, end);
		tmp.SetWindowText(buf);
		if(start > 0) start--;
		if(end > 0) end--;
		tmp.SetSel(start, end);
		tmp.Detach();
	}

	return TRUE;
}
//RSX++ //Raw Manager
void FavHubProperties::addEntryAction(int id, const string name, bool actif, int pos) {
	TStringList lst;

	lst.push_back(Text::toT(name));
	int i = ctrlAction.insert(pos, lst, 0, (LPARAM)id);
	ctrlAction.SetCheckState(i, actif);
}

void FavHubProperties::addEntryRaw(const Action::Raw& ra, int pos, int actionId) {
	TStringList lst;

	lst.push_back(Text::toT(ra.getName()));
	int i = ctrlRaw.insert(pos, lst, 0, (LPARAM)ra.getRawId());
	ctrlRaw.SetCheckState(i, FavoriteManager::getInstance()->getActifRaw(entry, actionId, ra.getRawId()));
}

LRESULT FavHubProperties::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	gotFocusOnAction = (l->uNewState & LVIS_FOCUSED) || ((l->uNewState & LVIS_STATEIMAGEMASK) && ctrlAction.GetSelectedIndex() != -1);
	::EnableWindow(GetDlgItem(IDC_FH_RAW), gotFocusOnAction);

	if(!nosave && l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) == (l->uOldState & LVIS_STATEIMAGEMASK))) {
		int j;
		for(j = 0; j < ctrlRaw.GetItemCount(); j++) {
			FavoriteManager::getInstance()->setActifRaw(entry, RawManager::getInstance()->getActionId(ctrlAction.GetItemData(l->iItem)), ctrlRaw.GetItemData(j), RsxUtil::toBool(ctrlRaw.GetCheckState(j)));
		}
		ctrlRaw.SetRedraw(FALSE);
		ctrlRaw.DeleteAllItems();
		if(ctrlAction.GetSelectedCount() == 1) {
			Action::RawsList lst = RawManager::getInstance()->getRawList(ctrlAction.GetItemData(l->iItem));

			for(Action::RawsList::const_iterator i = lst.begin(); i != lst.end(); ++i) {
				const Action::Raw& ra = *i;
				addEntryRaw(ra, ctrlRaw.GetItemCount(), RawManager::getInstance()->getActionId(ctrlAction.GetItemData(l->iItem)));
			}
		} else {
			::EnableWindow(GetDlgItem(IDC_FH_RAW), false);
		}
		ctrlRaw.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT FavHubProperties::onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;

	if(GetFocus() == ctrlAction.m_hWnd) {
		if(ctrlAction.GetCheckState(item->iItem))
			ctrlAction.SetCheckState(item->iItem, false);
		else
			ctrlAction.SetCheckState(item->iItem, true);
	} else if(GetFocus() == ctrlRaw.m_hWnd) {
		if(ctrlRaw.GetCheckState(item->iItem))
			ctrlRaw.SetCheckState(item->iItem,  false);
		else
			ctrlRaw.SetCheckState(item->iItem,  true);
	}
	return 0;
}
//END
/**
 * @file
 * $Id: FavHubProperties.cpp 234 2006-08-06 17:51:36Z bigmuscle $
 */
