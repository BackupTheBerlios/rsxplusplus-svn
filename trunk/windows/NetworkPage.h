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
#include "../client/HttpConnection.h"
#include "../client/SimpleXML.h"
#include "../client/version.h"
//END

class NetworkPage : public CPropertyPage<IDD_NETWORKPAGE>, public PropPage, private HttpConnectionListener
{
public:
	NetworkPage(SettingsManager *s) : PropPage(s) {
		SetTitle(CTSTRING(SETTINGS_NETWORK));
		m_psp.dwFlags |= PSP_RTLREADING;
		c = new HttpConnection; //RSX++
	}
	//RSX++
	~NetworkPage() {
		if(c != NULL) {
			delete c;
			c = NULL;
		}
	}
	//END

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
	HttpConnection* c;
	string downBuf;
	//END
	void fixControls();
	void getAddresses();

	//RSX++
	void on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
		downBuf = string((const char*)buf, len);
	}

	void on(HttpConnectionListener::Complete, HttpConnection* conn, const string&) throw() {
		conn->removeListener(this);
		if(!downBuf.empty()) {
			SimpleXML xml;
			xml.fromXML(downBuf);
			if(xml.findChild("html")) {
				xml.stepIn();
				if(xml.findChild("body")) {
					string x = xml.getChildData().substr(20);
					if(Util::isPrivateIp(x)) {
							CheckRadioButton(IDC_DIRECT, IDC_FIREWALL_PASSIVE, IDC_FIREWALL_PASSIVE);
							fixControls();
					}
					SetDlgItemText(IDC_SERVER, Text::toT(x).c_str());
					//::MessageBox(NULL, _T("IP fetched: checkip.dyndns.org"), _T("Debug"), MB_OK);
				} else {
					if(Util::isPrivateIp(Util::getLocalIp())) {
							CheckRadioButton(IDC_DIRECT, IDC_FIREWALL_PASSIVE, IDC_FIREWALL_PASSIVE);
							fixControls();
					}
					SetDlgItemText(IDC_SERVER, Text::toT(Util::getLocalIp()).c_str());
				}
			}
		}
		::EnableWindow(GetDlgItem(IDC_GETIP), true);
	}

	void on(HttpConnectionListener::Failed, HttpConnection* conn, const string& /*aLine*/) throw() {
		conn->removeListener(this);
		{
			if(Util::isPrivateIp(Util::getLocalIp())) {
					CheckRadioButton(IDC_DIRECT, IDC_FIREWALL_PASSIVE, IDC_FIREWALL_PASSIVE);
					fixControls();
			}
			SetDlgItemText(IDC_SERVER, Text::toT(Util::getLocalIp()).c_str());	
		}
		::EnableWindow(GetDlgItem(IDC_GETIP), true);
	}
	//END
};

#endif // !defined(NETWORK_PAGE_H)

/**
 * @file
 * $Id: NetworkPage.h 272 2006-12-28 15:30:12Z bigmuscle $
 */
