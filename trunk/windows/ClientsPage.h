#ifndef CLIENTSPAGE_H
#define CLIENTSPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"
#include "ExListViewCtrl.h"

#include "../client/DetectionManager.h"

class ClientsPage : public CPropertyPage<IDD_CLIENTS_PAGE>, public PropPage
{
public:
	enum { WM_PROFILE = WM_APP + 53 };

	ClientsPage(SettingsManager *s, bool isUI) : PropPage(s), isUserInfo(isUI) {
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT) + _T('\\') + 
			(isUI ? TSTRING(SETTINGS_USER_INFO_DETECTOR) : TSTRING(SETTINGS_CLIENTS))).c_str());
		SetTitle(title);
		m_psp.dwFlags |= PSP_RTLREADING;
	};

	~ClientsPage() { 
		ctrlProfiles.Detach();
		free(title);
	};

	BEGIN_MSG_MAP(ClientsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD_CLIENT, onAddClient)
		COMMAND_ID_HANDLER(IDC_REMOVE_CLIENT, onRemoveClient)
		COMMAND_ID_HANDLER(IDC_CHANGE_CLIENT, onChangeClient)
		COMMAND_ID_HANDLER(IDC_MOVE_CLIENT_UP, onMoveClientUp)
		COMMAND_ID_HANDLER(IDC_MOVE_CLIENT_DOWN, onMoveClientDown)
		COMMAND_ID_HANDLER(IDC_RELOAD_CLIENTS, onReload)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu) //RSX++
		NOTIFY_HANDLER(IDC_CLIENT_ITEMS, NM_CUSTOMDRAW, onCustomDraw)
		NOTIFY_HANDLER(IDC_CLIENT_ITEMS, NM_DBLCLK, onDblClick)
		NOTIFY_HANDLER(IDC_CLIENT_ITEMS, LVN_GETINFOTIP, onInfoTip)
		NOTIFY_HANDLER(IDC_CLIENT_ITEMS, LVN_ITEMCHANGED, onItemchangedDirectories)
		NOTIFY_HANDLER(IDC_CLIENT_ITEMS, LVN_KEYDOWN, onKeyDown)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	LRESULT onAddClient(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChangeClient(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveClient(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMoveClientUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMoveClientDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onReload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onInfoTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/); //RSX++
	LRESULT onItemchangedDirectories(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
		return onChangeClient(0, 0, 0, bHandled);
	}
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
private:
	bool isUserInfo;
	ExListViewCtrl ctrlProfiles;

	static TextItem texts[];
	TCHAR* title;
	void addEntry(const DetectionEntry& de, int pos);
	void reload();

	string downBuf;
};

#endif //CLIENTSPAGE_H

/**
 * @file
 * $Id: ClientsPage.h 399 2008-07-06 19:48:02Z BigMuscle $
 */
