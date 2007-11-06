#include "../client/RawManager.h"

#ifndef CHANGE_RAW_CHEAT_DLG_H
#define CHANGE_RAW_CHEAT_DLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ChangeRawCheatDlg : public CDialogImpl<ChangeRawCheatDlg>
{
	CEdit ctrlCheat;
	CComboBox ctrlRaw;
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
	
	ChangeRawCheatDlg() { };
	
	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlCheat.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ctrlCheat.Attach(GetDlgItem(IDC_CHEAT));
		ctrlCheat.SetFocus();
		ctrlCheat.SetWindowText(Text::toT(cheatingDescription).c_str());

		//Zion++ //Raw Manager //DEBUT
		Action::List lst = RawManager::getInstance()->getActionList();

		int j = 0;
		idAction.insert(make_pair(j, j));
		for(Action::List::iterator i = lst.begin(); i != lst.end(); ++i) {
			idAction.insert(make_pair(++j, i->second->getActionId()));
		}

		ctrlRaw.Attach(GetDlgItem(IDC_CHANGE_RAW));
		for(Iter i = idAction.begin(); i != idAction.end(); ++i) {
			ctrlRaw.AddString(RawManager::getInstance()->getNameActionId(i->second).c_str());
		}
		ctrlRaw.SetCurSel(getId(raw));
		SetWindowText(Text::toT(name).c_str());
		
		CenterWindow(GetParent());
		return FALSE;
	}
	
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {
			int len = ctrlCheat.GetWindowTextLength() + 1;
			TCHAR* buf = new TCHAR[len];
			GetDlgItemText(IDC_CHEAT, buf, len);
			cheatingDescription = Text::fromT(buf);
			delete[] buf;
			raw = getIdAction(ctrlRaw.GetCurSel());
		}
		EndDialog(wID);
		return 0;
	}

	//Zion++ //Raw Manager //DEBUT
protected:
	typedef map<int, int> ActionList;
	typedef ActionList::iterator Iter;

	int getId(int actionId) {
		for(Iter i = idAction.begin(); i != idAction.end(); ++i) {
			if(i->second == actionId)
				return i->first;
		}
		return 0;
	}

	int getIdAction(int id) {
		for(Iter i = idAction.begin(); i != idAction.end(); ++i) {
			if(i->first == id)
				return i->second;
		}
		return 0;
	}

	ActionList idAction;
	//Zion++ //Raw Manager //FIN
};

#endif // CHANGE_RAW_CHEAT_DLG_H