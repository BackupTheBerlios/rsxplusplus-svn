#ifndef FAV_TAB_PAGES_H
#define FAV_TAB_PAGES_H

#include "../client/RawManager.h"
#include "ExListViewCtrl.h"

class CFavTabRaw : public CDialogImpl<CFavTabRaw> {
public:
	enum { IDD = IDD_FAV_TAB_RAW };

	CFavTabRaw() : hub(NULL) { }
	~CFavTabRaw() {
		ctrlAction.Detach();
		ctrlRaw.Detach();
	}

	BEGIN_MSG_MAP(CFavTabRaw)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_FH_ACTION, LVN_ITEMCHANGED, onItemChanged)
		//REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	void setHub(FavoriteHubEntry* _hub) { hub = _hub; }
	void prepareClose();
private:
	FavoriteHubEntry* hub;
	ExListViewCtrl ctrlAction;
	ExListViewCtrl ctrlRaw;
	void addEntryAction(int id, const string name, bool actif, int pos);
	void addEntryRaw(const Action::Raw& ra, int pos, int actionId);
};

class CFavTabOp : public CDialogImpl<CFavTabOp> {
public:
	enum { IDD = IDD_FAV_TAB_OP };

	CFavTabOp() : hub(NULL) { };
	~CFavTabOp() { };

	BEGIN_MSG_MAP(CFavTabOp)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_MATCH, onMatch)
		//REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onMatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void setHub(FavoriteHubEntry* _hub) { hub = _hub; }
	void prepareClose();
private:
	FavoriteHubEntry* hub;
};

class CCustomTab : public CDialogImpl<CCustomTab> {
public:
	enum { IDD = IDD_FAV_TAB_CUSTOM };

	CCustomTab() : hub(NULL) { };
	~CCustomTab() { };

	BEGIN_MSG_MAP(CCustomTab)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void setHub(FavoriteHubEntry* _hub) { hub = _hub; }
	void prepareClose();
private:
	FavoriteHubEntry* hub;
};

#endif // !defined(FAV_TAB_RAW_H)
