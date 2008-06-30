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

#if !defined(NETWORK_PAGE_H)
#define NETWORK_PAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"
//RSX++
#include <atlctrlx.h>
#include "../client/SimpleXML.h"
#include "../client/version.h"
#include "../rsx/HTTPDownloadManager.h"
//END

class NetworkPage : public CPropertyPage<IDD_NETWORKPAGE>, public PropPage
{
public:
	NetworkPage(SettingsManager *s) : PropPage(s) {
		SetTitle(CTSTRING(SETTINGS_NETWORK));
		m_psp.dwFlags |= PSP_RTLREADING;
	}

	BEGIN_MSG_MAP(NetworkPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_DIRECT, onClickedActive)
		COMMAND_ID_HANDLER(IDC_FIREWALL_PASSIVE, onClickedActive)
		COMMAND_ID_HANDLER(IDC_FIREWALL_UPNP, onClickedActive)
		COMMAND_ID_HANDLER(IDC_FIREWALL_NAT, onClickedActive)
		COMMAND_ID_HANDLER(IDC_DIRECT_OUT, onClickedActive)
		COMMAND_ID_HANDLER(IDC_SOCKS5, onClickedActive)
		COMMAND_ID_HANDLER(IDC_CON_CHECK, onCheckConn) //RSX++
		COMMAND_ID_HANDLER(IDC_GETIP, onGetIP) //RSX++
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClickedActive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//RSX++
	LRESULT onCheckConn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGetIP(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */);
	//END
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
private:
	static Item items[];
	static TextItem texts[];
	CEdit desc;
	CComboBox BindCombo;

	//RSX++
	CHyperLink ConnCheckUrl;
	string downBuf;
	//END
	void fixControls();
	void getAddresses();

	void onHttpRequest(string buf, bool isFailed); //RSX++
};

#endif // !defined(NETWORK_PAGE_H)

/**
 * @file
 * $Id: NetworkPage.h 272 2006-12-28 15:30:12Z bigmuscle $
 */
