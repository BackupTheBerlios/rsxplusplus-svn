#ifndef FAKESHARESPAGE_H
#define FAKESHARESPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"
#include "ExListViewCtrl.h"

class FakeSharesPage : public CPropertyPage<IDD_FAKE_SHARES>, public PropPage
{
public:
	FakeSharesPage(SettingsManager *s) : PropPage(s) { 
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT) + _T('\\') + TSTRING(SETTINGS_FAKESHARE)).c_str());
		SetTitle(title);
	};

	~FakeSharesPage() {
	 	free(title);
		ctrlFakeShares.Detach();
	};

	BEGIN_MSG_MAP_EX(FakeSharesPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD_FAKE_SHARE, onAddFakeShare)
		COMMAND_ID_HANDLER(IDC_REMOVE_FAKE_SHARE, onRemoveFakeShare)
		COMMAND_ID_HANDLER(IDC_CHANGE_FAKE_SHARE, onChangeFakeShare)
		NOTIFY_HANDLER(IDC_FAKE_SHARE_ITEMS, NM_DBLCLK, onDblClick)
		NOTIFY_HANDLER(IDC_FAKE_SHARE_ITEMS, NM_CUSTOMDRAW, onCustomDraw)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	LRESULT onAddFakeShare(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChangeFakeShare(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveFakeShare(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
		return onChangeFakeShare(0, 0, 0, bHandled);
	}
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
protected:
	ExListViewCtrl ctrlFakeShares;

	static TextItem texts[];
	TCHAR* title;
};

#endif //FAKESHARESPAGE_H

/**
 * @file
 * $Id: FakeSharesPage.h,v 1.0 2004/11/26 18:16 Virus27 Exp $
 */