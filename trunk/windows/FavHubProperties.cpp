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

	ctrlTabs.SubclassWindow(GetDlgItem(IDC_FAV_TABS));

	ctrlOpTab.setHub(entry);
	ctrlRaws.setHub(entry);
	ctrlCustomTab.setHub(entry);
	ctrlOpTab.Create(m_hWnd);
	ctrlRaws.Create(m_hWnd);
	ctrlCustomTab.Create(m_hWnd);

	ctrlTabs.AddTab(_T("Custom"), ctrlCustomTab, 0, true);
	ctrlTabs.AddTab(_T("Actions and Raws"), ctrlRaws,  1, true);
	ctrlTabs.AddTab(_T("Detector"), ctrlOpTab, 2, true);

	RL_CreateImageList(icons, images, IDP_FAVTABS, 16);
	ctrlTabs.SetImageList(images);
	ctrlTabs.SetCurSel(0);

	StringList& glst = FavoriteManager::getInstance()->getFavGroups();
	CComboBox combo;
	combo.Attach(GetDlgItem(IDC_FAV_DLG_GROUP));
	combo.AddString(_T("No Group"));
	for(StringIter i = glst.begin(); i != glst.end(); ++i)
		combo.AddString(Text::toT((*i)).c_str());
	combo.SetCurSel(entry->getGroupId());
	combo.Detach();
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
	tmp.SetPasswordChar(0x25CF);
	tmp.Detach();
	//RSX++
	tmp.Attach(GetDlgItem(IDC_FAV_EMAIL));
	tmp.LimitText(50);
	tmp.Detach();
	tmp.Attach(GetDlgItem(IDC_FAV_AWAY_MSG));
	tmp.LimitText(2048);
	tmp.Detach();

	m_hIcon = ::LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDR_FAVORITES));
	SetIcon(m_hIcon, FALSE);
	SetIcon(m_hIcon, TRUE);
	//END
	CenterWindow(GetParent());
	return FALSE;
}

LRESULT FavHubProperties::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	images.Destroy();
	RL_DeleteObject(icons); //RSX++
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
		GetDlgItemText(IDC_FAV_EMAIL, buf, 256);
		entry->setFavEmail(Text::fromT(buf));

		GetDlgItemText(IDC_FAV_AWAY_MSG, buf, 256);
		entry->setAwayMsg(Text::fromT(buf));

		CComboBox combo;
		combo.Attach(GetDlgItem(IDC_FAV_DLG_GROUP));
		entry->setGroupId(combo.GetCurSel());
		combo.Detach();
		//init close and save values
		ctrlOpTab.prepareClose();
		ctrlRaws.prepareClose();
		ctrlCustomTab.prepareClose();
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

/**
 * @file
 * $Id: FavHubProperties.cpp 317 2007-08-04 14:52:24Z bigmuscle $
 */
