#ifndef MYINFOPAGE_H
#define MYINOFPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"
#include "ExListViewCtrl.h"

#include "../client/ClientProfileManager.h"

class MyinfoPage : public CPropertyPage<IDD_MYINFO_PAGE>, public PropPage {
public:
	enum { WM_PROFILE = WM_APP + 53 };

	MyinfoPage(SettingsManager *s) : PropPage(s) { 
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT) + _T('\\') + TSTRING(SETTINGS_MYINFO_DETECTOR)).c_str());
		SetTitle(title);
	};

	~MyinfoPage() { 
		ctrlProfiles.Detach();
		free(title);
	};

	BEGIN_MSG_MAP(MyinfoPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD, onAddClient)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemoveClient)
		COMMAND_ID_HANDLER(IDC_CHANGE, onChangeClient)
		COMMAND_ID_HANDLER(IDC_MOVE_UP, onMoveClientUp)
		COMMAND_ID_HANDLER(IDC_MOVE_DOWN, onMoveClientDown)
		COMMAND_ID_HANDLER(IDC_RELOAD, onReload)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		NOTIFY_HANDLER(IDC_LIST, NM_CUSTOMDRAW, onCustomDraw)
		NOTIFY_HANDLER(IDC_LIST, NM_DBLCLK, onDblClick)
		NOTIFY_HANDLER(IDC_LIST, LVN_GETINFOTIP, onInfoTip)
		NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, onItemchanged)
		NOTIFY_HANDLER(IDC_LIST, LVN_KEYDOWN, onKeyDown)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	LRESULT onAddClient(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChangeClient(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveClient(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMoveClientUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMoveClientDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onReload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onInfoTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onItemchanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
		return onChangeClient(0, 0, 0, bHandled);
	}
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
protected:
	ExListViewCtrl ctrlProfiles;

	static TextItem texts[];
	TCHAR* title;
	void addEntry(const MyinfoProfile& cp, int pos);
private:

	void reload();

	string downBuf;
};
#endif //MYINFOPAGE_H
/**
 * @file
 * $Id: MyinfoPage.h 1.0 2005-08-03 20:39:12 Virus27 $
 */