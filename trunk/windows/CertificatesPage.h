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

#if !defined(CERTIFICATES_PAGE_H)
#define CERTIFICATES_PAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"
#include "ExListViewCtrl.h"
//RSX++
#include "../client/HttpConnection.h"
#include "../client/File.h"
//END

class CertificatesPage : public CPropertyPage<IDD_CERTIFICATESPAGE>, public PropPage, private HttpConnectionListener
{
public:
	CertificatesPage(SettingsManager *s) : PropPage(s) {
		title = _tcsdup((TSTRING(SETTINGS_ADVANCED) + _T('\\') + TSTRING(SETTINGS_CERTIFICATES)).c_str());
		SetTitle(title);
		m_psp.dwFlags |= PSP_RTLREADING;
		c = new HttpConnection;	//RSX++
	}

	~CertificatesPage() { 
		free(title);
		//RSX++
		if(c != NULL) {
			delete c;
			c = NULL;
		}
		//END
	}

	BEGIN_MSG_MAP(CertificatesPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_BROWSE_PRIVATE_KEY, onBrowsePrivateKey)
		COMMAND_ID_HANDLER(IDC_BROWSE_CERTIFICATE, onBrowseCertificate)
		COMMAND_ID_HANDLER(IDC_BROWSE_TRUSTED_PATH, onBrowseTrustedPath)
		COMMAND_ID_HANDLER(IDC_GENERATE_CERTS, onGenerateCerts)
		//RSX++
		COMMAND_ID_HANDLER(IDC_RELOAD_BL, onReloadBlockList)
		COMMAND_ID_HANDLER(IDC_UPDATE_BL, onUpdateBlockList)
		COMMAND_ID_HANDLER(IDC_VIEW_BL, onViewBlockList)
		COMMAND_ID_HANDLER(IDC_PG_ENABLE, PGControls)
		COMMAND_ID_HANDLER(IDC_PG_UP, PGStatus)
		COMMAND_ID_HANDLER(IDC_PG_DOWN, PGStatus)
		COMMAND_ID_HANDLER(IDC_PG_SEARCH, PGStatus)
		//END
	END_MSG_MAP()

	LRESULT onBrowsePrivateKey(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBrowseCertificate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBrowseTrustedPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGenerateCerts(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	//RSX++
	LRESULT onReloadBlockList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onUpdateBlockList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onViewBlockList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT PGControls(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		fixControls();
		return 0;
	}

	LRESULT PGStatus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		updateStatus();
		return 0;
	}
	//END

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
protected:

	static Item items[];
	static TextItem texts[];
	static ListItem listItems[];
	TCHAR* title;

	//RSX++
	void fixControls();
	void updateStatus();
	HttpConnection* c;
	string downBuf;

	void on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
		downBuf.append((char*)buf, len);
	}

	void on(HttpConnectionListener::Complete, HttpConnection* conn, const string&) throw() {
		conn->removeListener(this);
		if(!downBuf.empty()) {
			File f(SETTING(PG_FILE) + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
			f.write(downBuf);
			f.close();
			File::deleteFile(SETTING(PG_FILE));
			File::renameFile(SETTING(PG_FILE) + ".tmp", SETTING(PG_FILE));
			MessageBox(_T("Your blocklist is now up-to-date."), _T("Blocklist updated"), MB_OK);
			SetDlgItemText(IDC_UPDATE_BL, _T("Finished!"));
			PostMessage(WM_COMMAND, IDC_RELOAD_BL);
		}
	}

	void on(HttpConnectionListener::Failed, HttpConnection* conn, const string& aLine) throw() {
		conn->removeListener(this);
		{
			string msg = "Blocklist download failed.\r\n" + aLine;
			MessageBox(Text::toT(msg).c_str(), _T("Blocklist update failed"), MB_OK);
			SetDlgItemText(IDC_UPDATE_BL, _T("Update Blocklist"));
			::EnableWindow(GetDlgItem(IDC_UPDATE_BL), true);
		}
	}
	//END
};

#endif // !defined(CERTIFICATES_PAGE_H)

/**
 * @file
 * $Id: CertificatesPage.h 308 2007-07-13 18:57:02Z bigmuscle $
 */
