#ifndef FAV_TAB_PAGES_H
#define FAV_TAB_PAGES_H

#include "../client/RawManager.h"
#include "ExListViewCtrl.h"

class CFavTabRaw : public CDialogImpl<CFavTabRaw> {
public:
	enum { IDD = IDD_FAV_TAB_RAW };

	CFavTabRaw() : hub(NULL) { }
	~CFavTabRaw() {
		ctrlList.Detach();
	}

	BEGIN_MSG_MAP(CFavTabRaw)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void setHub(FavoriteHubEntry* _hub) { hub = _hub; }
	void prepareClose();
private:
	FavoriteHubEntry* hub;
	CTreeViewCtrl ctrlList;
	HTREEITEM addAction(const Action* action);
	void addRaw(HTREEITEM action, int actionId, const Raw* raw);
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

class CFavTabSettings : public CDialogImpl<CFavTabSettings> {
public:
	enum { IDD = IDD_FAV_TAB_RAWSETTINGS };

	CFavTabSettings() : hub(NULL) { };
	~CFavTabSettings() { };

	BEGIN_MSG_MAP(CFavTabSettings)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_SETTINGS, NM_DBLCLK, onDblClick)
		//REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/);

	void setHub(FavoriteHubEntry* _hub) { hub = _hub; }
	void prepareClose();
private:
	FavoriteHubEntry* hub;
	ExListViewCtrl ctrlList;
};

#endif // !defined(FAV_TAB_RAW_H)
