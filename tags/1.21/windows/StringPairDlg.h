#ifndef RSXPLUSPLUS_STRING_PAIR_DLG
#define RSXPLUSPLUS_STRING_PAIR_DLG

class StringPairDlg : public CDialogImpl<StringPairDlg> {
public:
	enum { IDD = IDD_ADD_STRING_PAIR };

	BEGIN_MSG_MAP(StringPairDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, onClose)
		COMMAND_ID_HANDLER(IDCANCEL, onClose)
	END_MSG_MAP()

	StringPairDlg(tstring _wndTitle, tstring _fname, tstring _sname) :
	wndTitle(_wndTitle), firstName(_fname), secondName(_sname) { }

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		SetWindowText(wndTitle.c_str());
		setText(IDC_ADD_STRING_PAIR_FNAME, firstName);
		setText(IDC_ADD_STRING_PAIR_SNAME, secondName);
		setText(IDC_ADD_STRING_PAIR_FIRST, first);
		setText(IDC_ADD_STRING_PAIR_SECOND, second);

		CenterWindow(GetParent());
		return 0;
	}

	LRESULT onClose(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(wID == IDOK) {
			first = WinUtil::getWindowText(m_hWnd, IDC_ADD_STRING_PAIR_FIRST);
			second = WinUtil::getWindowText(m_hWnd, IDC_ADD_STRING_PAIR_SECOND);
		}
		EndDialog(wID);
		return 0;
	}

	tstring first;
	tstring second;
private:
	void setText(int id, const tstring& txt) {
		CEdit edit;
		edit.Attach(GetDlgItem(id));
		edit.SetWindowText(txt.c_str());
		edit.Detach();
	}
	tstring wndTitle;
	tstring firstName;
	tstring secondName;
};
#endif