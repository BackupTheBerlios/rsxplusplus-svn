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

#include "CertificatesPage.h"
#include "CommandDlg.h"

#include "../client/SettingsManager.h"
#include "../client/FavoriteManager.h"
#include "../client/CryptoManager.h"
#include "../client/PGManager.h" //RSX++
#include "PGListDlg.h"

#include "WinUtil.h"

PropPage::TextItem CertificatesPage::texts[] = {
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item CertificatesPage::items[] = {
	{ IDC_TLS_CERTIFICATE_FILE, SettingsManager::TLS_CERTIFICATE_FILE, PropPage::T_STR },
	{ IDC_TLS_PRIVATE_KEY_FILE, SettingsManager::TLS_PRIVATE_KEY_FILE, PropPage::T_STR },
	{ IDC_TLS_TRUSTED_CERTIFICATES_PATH, SettingsManager::TLS_TRUSTED_CERTIFICATES_PATH, PropPage::T_STR },
	//RSX++
	{ IDC_PG_ENABLE, SettingsManager::PG_ENABLE, PropPage::T_BOOL },
	{ IDC_PG_UP, SettingsManager::PG_UP, PropPage::T_BOOL },
	{ IDC_PG_DOWN, SettingsManager::PG_DOWN, PropPage::T_BOOL },
	{ IDC_PG_SEARCH, SettingsManager::PG_SEARCH, PropPage::T_BOOL },
	//END
	{ 0, 0, PropPage::T_END }
};

PropPage::ListItem CertificatesPage::listItems[] = {
	{ SettingsManager::USE_TLS, ResourceManager::SETTINGS_USE_TLS },
	{ SettingsManager::ALLOW_UNTRUSTED_HUBS, ResourceManager::SETTINGS_ALLOW_UNTRUSTED_HUBS	},
	{ SettingsManager::ALLOW_UNTRUSTED_CLIENTS, ResourceManager::SETTINGS_ALLOW_UNTRUSTED_CLIENTS, },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT CertificatesPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_TLS_OPTIONS));

	//RSX++
	if(!PGManager::getInstance()->notAbused() || (FindWindow(NULL, _T("PeerGuardian")) || FindWindow(NULL, _T("PeerGuardian 2")) || FindWindow(NULL, _T("ProtoWall")))) {
		CheckDlgButton(IDC_PG_ENABLE, BST_UNCHECKED);
		::EnableWindow(GetDlgItem(IDC_PG_ENABLE), false);
		SettingsManager::getInstance()->set(SettingsManager::PG_ENABLE, false);
	}

	CertificatesPage::fixControls();
	//END

	// Do specialized reading here
	return TRUE;
}

void CertificatesPage::write() {
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_TLS_OPTIONS));
}

LRESULT CertificatesPage::onBrowsePrivateKey(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring target = Text::toT(SETTING(TLS_PRIVATE_KEY_FILE));
	CEdit edt(GetDlgItem(IDC_TLS_PRIVATE_KEY_FILE));

	if(WinUtil::browseFile(target, m_hWnd, false, target)) {
		edt.SetWindowText(&target[0]);
	}
	return 0;
}

LRESULT CertificatesPage::onBrowseCertificate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring target = Text::toT(SETTING(TLS_CERTIFICATE_FILE));
	CEdit edt(GetDlgItem(IDC_TLS_CERTIFICATE_FILE));

	if(WinUtil::browseFile(target, m_hWnd, false, target)) {
		edt.SetWindowText(&target[0]);
	}
	return 0;
}

LRESULT CertificatesPage::onBrowseTrustedPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring target = Text::toT(SETTING(TLS_TRUSTED_CERTIFICATES_PATH));
	CEdit edt(GetDlgItem(IDC_TLS_TRUSTED_CERTIFICATES_PATH));

	if(WinUtil::browseDirectory(target, m_hWnd)) {
		edt.SetWindowText(&target[0]);
	}
	return 0;
}

LRESULT CertificatesPage::onGenerateCerts(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		CryptoManager::getInstance()->generateCertificate();
	} catch(const CryptoException& e) {
		MessageBox(Text::toT(e.getError()).c_str(), L"Error generating certificate");
	}
	return 0;
}

//RSX++
LRESULT CertificatesPage::onReloadBlockList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	::EnableWindow(GetDlgItem(IDC_RELOAD_BL), false);
	PGManager::getInstance()->updateBlockList();
	if(PGManager::getInstance()->notAbused()) {
		SetDlgItemText(IDC_PG_STATUS, (_T("Status: Blocklist reloaded, with ") + PGManager::getInstance()->getTotalIPRangesTStr() + _T(" IP ranges.")).c_str());
		::EnableWindow(GetDlgItem(IDC_RELOAD_BL), true);
	} else {
		CheckDlgButton(IDC_PG_ENABLE, BST_UNCHECKED);
		::EnableWindow(GetDlgItem(IDC_PG_ENABLE), false);
		SettingsManager::getInstance()->set(SettingsManager::PG_ENABLE, false);
		SetDlgItemText(IDC_PG_STATUS, Text::toT("Status: Abusive usage detected!").c_str());
		CertificatesPage::fixControls();
	}
	return 0;
}

LRESULT CertificatesPage::onUpdateBlockList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	::EnableWindow(GetDlgItem(IDC_UPDATE_BL), false);
	SetDlgItemText(IDC_UPDATE_BL, _T("Downloading..."));
	c->addListener(this);	
	c->downloadFile(SETTING(PG_UPDATE_URL) + Util::getFileName(SETTING(PG_FILE)));	
	return 0;
}

LRESULT CertificatesPage::onViewBlockList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	::EnableWindow(GetDlgItem(IDC_VIEW_BL), false);
	PGListDlg dlg;
	dlg.DoModal(m_hWnd);
	::EnableWindow(GetDlgItem(IDC_VIEW_BL), true);
	return 0;
}

void CertificatesPage::updateStatus() {	
	if (!(FindWindow(NULL, _T("PeerGuardian")) || FindWindow(NULL, _T("PeerGuardian 2")) || FindWindow(NULL, _T("ProtoWall")))) {
		if((IsDlgButtonChecked(IDC_PG_ENABLE) == BST_CHECKED)) {
			if((PGManager::getInstance()->getTotalIPRanges() != 0) && (IsDlgButtonChecked(IDC_PG_UP) == BST_CHECKED || IsDlgButtonChecked(IDC_PG_DOWN) == BST_CHECKED || IsDlgButtonChecked(IDC_PG_SEARCH) == BST_CHECKED)) {
				SetDlgItemText(IDC_PG_STATUS, (_T("Status: Running with ") + PGManager::getInstance()->getTotalIPRangesTStr() + _T(" IP ranges.")).c_str());
			} else {
				SetDlgItemText(IDC_PG_STATUS, _T("Status: Running but not active."));
			}
		} else {
			SetDlgItemText(IDC_PG_STATUS, _T("Status: Not running"));
		}
	} else {
		SetDlgItemText(IDC_PG_STATUS, Text::toT("Status: Stand alone blocking software detected!").c_str());
	}
}

void CertificatesPage::fixControls() {
	bool pg = IsDlgButtonChecked(IDC_PG_ENABLE) == BST_CHECKED;
	CertificatesPage::updateStatus();
	::EnableWindow(GetDlgItem(IDC_PG_GP), pg);
	::EnableWindow(GetDlgItem(IDC_PG_STATUS), pg);
	::EnableWindow(GetDlgItem(IDC_PG_UP), pg);
	::EnableWindow(GetDlgItem(IDC_PG_DOWN), pg);
	::EnableWindow(GetDlgItem(IDC_PG_SEARCH), pg);
	::EnableWindow(GetDlgItem(IDC_RELOAD_BL), pg);
	::EnableWindow(GetDlgItem(IDC_SEP), pg);
	::EnableWindow(GetDlgItem(IDC_UPDATE_BL), pg);
	::EnableWindow(GetDlgItem(IDC_VIEW_BL), pg);
}
//END
/**
 * @file
 * $Id: CertificatesPage.cpp 219 2006-07-08 23:19:56Z bigmuscle $
 */
