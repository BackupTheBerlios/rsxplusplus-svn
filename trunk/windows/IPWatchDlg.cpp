#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "IPWatchDlg.h"

#define ATTACH(id, var) var.Attach(GetDlgItem(id))
LRESULT IPWatchDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	ATTACH(IDC_WATCH_IP, cIP);
	ATTACH(IDC_WATCH_ACTION, cAction);
	ATTACH(IDC_WATCH_DISPLAY_CHEAT, cDisplay);
	ATTACH(IDC_WATCH_CHEAT, cCheat);
	ATTACH(IDC_WATCH_USE_REGEXP, cRegExp);

	cIP.SetWindowText(ip.c_str());
	cAction.AddString(_T("Protect"));
	cAction.AddString(_T("Ban"));
	cAction.AddString(_T("Notify"));
	cAction.SetCurSel(action);

	cDisplay.SetCheck(display ?	BST_CHECKED : BST_UNCHECKED);
	cRegExp.SetCheck(regexp ? BST_CHECKED : BST_UNCHECKED);

	cCheat.SetWindowText(cheat.c_str());
	createList();

	ATTACH(IDC_WATCH_ACTION_CMD,	cActionCommand);
	for(ActionList::const_iterator i = idAction.begin(); i != idAction.end(); ++i) {
		cActionCommand.AddString(RawManager::getInstance()->getNameActionId(i->second).c_str());
	}
	cActionCommand.SetCurSel(getId(actionCmd));

	fixControls();
	CenterWindow(GetParent());
	return FALSE;
}

LRESULT IPWatchDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		TCHAR buf[512];
		if (cIP.GetWindowTextLength() == 0) {
			MessageBox(CTSTRING(LINE_EMPTY));
			return 0;
		}

		GetDlgItemText(IDC_WATCH_IP, buf, 512);
		ip = buf;
		action = cAction.GetCurSel();
		actionCmd = getIdAction(cActionCommand.GetCurSel());
		display = cDisplay.GetCheck() == BST_CHECKED;
		regexp = cRegExp.GetCheck() == BST_CHECKED;
		GetDlgItemText(IDC_WATCH_CHEAT, buf, 512);
		cheat = buf;
	}
	EndDialog(wID);
	return 0;
}

LRESULT IPWatchDlg::onAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}

void IPWatchDlg::fixControls() {
	switch(cAction.GetCurSel()) {
		case 0: { //protect
			::EnableWindow(GetDlgItem(IDC_WATCH_ACTION_CMD), false);
			::EnableWindow(GetDlgItem(IDC_WATCH_DISPLAY_CHEAT), false);
			::EnableWindow(GetDlgItem(IDC_WATCH_CHEAT), false);
			break; 
				}
		case 1: { //ban
			::EnableWindow(GetDlgItem(IDC_WATCH_ACTION_CMD), true);
			::EnableWindow(GetDlgItem(IDC_WATCH_DISPLAY_CHEAT), true);
			::EnableWindow(GetDlgItem(IDC_WATCH_CHEAT), true);
			break; 
				}
		case 2: { //notify
			::EnableWindow(GetDlgItem(IDC_WATCH_ACTION_CMD), false);
			::EnableWindow(GetDlgItem(IDC_WATCH_DISPLAY_CHEAT), false);
			::EnableWindow(GetDlgItem(IDC_WATCH_CHEAT), true);
			break; 
				}
	}
}