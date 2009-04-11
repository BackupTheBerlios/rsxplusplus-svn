#ifndef CHANGE_RAW_CHEAT_DLG_H
#define CHANGE_RAW_CHEAT_DLG_H

#include "../client/RawManager.h"
#include "CRawCombo.h"

class ChangeRawCheatDlg : public CDialogImpl<ChangeRawCheatDlg> {
public:
	int raw;
	string cheatingDescription;
	string name;

	enum { IDD = IDD_RAW_CHEAT };
	
	BEGIN_MSG_MAP(ChangeRawCheatDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	ChangeRawCheatDlg(int _raw) : raw(_raw) { };

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlCheat.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlCheat.Attach(GetDlgItem(IDC_CHEAT));
		ctrlCheat.SetFocus();
		ctrlCheat.SetWindowText(Text::toT(cheatingDescription).c_str());

		ctrlRaw.attach(GetDlgItem(IDC_CHANGE_RAW), raw);

		SetWindowText(Text::toT(name).c_str());
		
		CenterWindow(GetParent());
		return FALSE;
	}
	
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(wID == IDOK) {
			int len = ctrlCheat.GetWindowTextLength() + 1;
			tstring buf;
			buf.resize(len);
			GetDlgItemText(IDC_CHEAT, &buf[0], len);
			cheatingDescription = Text::fromT(buf);
			raw = ctrlRaw.getActionId();
		}
		EndDialog(wID);
		return 0;
	}
private:
	CEdit ctrlCheat;
	CRawCombo ctrlRaw;
};

#endif // CHANGE_RAW_CHEAT_DLG_H