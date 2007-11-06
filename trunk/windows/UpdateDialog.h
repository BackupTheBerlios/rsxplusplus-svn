/*
 * Copyright (C) 2007 adrian_007, adrian-007 on o2 point pl
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

#ifndef UPDATE_DIALOG_H
#define UPDATE_DIALOG_H

#include "../rsx/UpdateManager.h"
#include "../windows/Resource.h"
#include "../client/HttpConnection.h"
#include "../client/SimpleXML.h"

class UpdateDialog : public CDialogImpl<UpdateDialog>, private UpdateManagerListener, private HttpConnectionListener {
public:
	enum { IDD = IDD_UPDATE };

	BEGIN_MSG_MAP(UpdateDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDCLOSE, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_UPDATE_DOWNLOAD, OnButton)
		COMMAND_ID_HANDLER(IDC_UPDATE, OnButton)
		COMMAND_ID_HANDLER(IDC_LOAD_BACKUP, OnButton)
		COMMAND_ID_HANDLER(IDC_CLIENT_ACTIVE, OnButton)
		COMMAND_ID_HANDLER(IDC_MYINFO_ACTIVE, OnButton)
		COMMAND_ID_HANDLER(IDC_ISP_ACTIVE, OnButton)
	END_MSG_MAP()

	UpdateDialog(string xml = Util::emptyString) : xmlData(xml), c(NULL), prog(0), m_hIcon(NULL), reload(false) { };
	~UpdateDialog();

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		::SetFocus(GetDlgItem(IDCLOSE));
		return FALSE;
	}
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		initClose();
		EndDialog(NULL);
		return 0;
	}
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		initClose();
		EndDialog(wID);
		return 0;
	}
	LRESULT OnButton(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CEdit cStatus, cChangeLog;
	CButton cMyINFOCheck, cClientCheck, cISPCheck, cLoadOld, cUpdateProfiles;
	CProgressBarCtrl cProgress;
	HICON m_hIcon;

	UpdateMap updateItems;
	string xmlData, downloadUrl;
	int xSVN;
	bool reload;
	uint8_t prog;
	HttpConnection* c;

	void updateStatus(const tstring& text);
	void versionXML();
	void initClose();
	void prepareFiles();
	void fixControls();
	void setProgress();

	void setCheck(int id, bool val){ 
		CButton tmp;
		tmp.Attach(GetDlgItem(id));
		tmp.SetCheck(val ? BST_CHECKED : BST_UNCHECKED);
		tmp.Detach();
	}

	// UpdateManagerListener
	void on(UpdateManagerListener::Complete, int file) throw();
	void on(UpdateManagerListener::Failed, int file, const string& reason) throw();

	// HttpConnectionListener
	void on(HttpConnectionListener::Complete, HttpConnection* /*conn*/, const string& /*aLine*/) throw();
	void on(HttpConnectionListener::Failed, HttpConnection* /*conn*/, const string& aLine) throw();
	void on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
		if(xmlData.empty())
			updateStatus(TSTRING(RETRIEVING_DATA) + _T("..."));
		xmlData.append((char*)buf, len);
	}
};
#endif