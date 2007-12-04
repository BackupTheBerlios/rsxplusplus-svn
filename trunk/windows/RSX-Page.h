/*
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

#ifndef RSXPAGE_H
#define RSXPAGE_H

#include "PropPage.h"
#include "ExListViewCtrl.h"

class RSXPage : public CPropertyPage<IDD_RSX>, public PropPage {
public:

	RSXPage(SettingsManager *s) : PropPage(s) {
		title = _tcsdup(TSTRING(SETTINGS_RSX).c_str());
		SetTitle(title);
		m_psp.dwFlags |= PSP_RTLREADING;
	}
	~RSXPage() {
		free(title);
		ignoreListCtrl.Detach();
		ctrlFavGroups.Detach();
	}

	BEGIN_MSG_MAP_EX(RSXPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_IGNORE_ADD, onIgnoreAdd)
		COMMAND_ID_HANDLER(IDC_IGNORE_REMOVE, onIgnoreRemove)
		COMMAND_ID_HANDLER(IDC_IGNORE_CLEAR, onIgnoreClear)
		COMMAND_ID_HANDLER(IDC_RSX_FAV_ADD, onFavGroupBtn)
		COMMAND_ID_HANDLER(IDC_RSX_FAV_REMOVE, onFavGroupBtn)
		COMMAND_ID_HANDLER(IDC_RSX_FAV_EDIT, onFavGroupBtn)
		COMMAND_CODE_HANDLER(EN_CHANGE, onEditChange)
		NOTIFY_HANDLER(IDC_IGNORELIST, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_IGNORELIST, LVN_ITEMCHANGED, onItemchanged)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onIgnoreAdd(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */);
	LRESULT onIgnoreRemove(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */);
	LRESULT onIgnoreClear(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */);
	LRESULT onEditChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onItemchanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT onFavGroupBtn(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */);

	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
		switch(kd->wVKey) {
			case VK_DELETE:
				PostMessage(WM_COMMAND, IDC_IGNORE_REMOVE, 0);
				break;
			default:
				bHandled = FALSE;
		}
		return 0;
	}

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();

//private:
protected:
	typedef unordered_set<tstring> TStringHash;
	typedef TStringHash::iterator TStringHashIter;

	TStringHash ignoreList;
	ExListViewCtrl ignoreListCtrl, ctrlFavGroups;

	TCHAR* title;

	static Item items[];
	static TextItem texts[];
	static ListItem listItems[];

	CComboBox ctrlPrio;
	int CurSel;
};

#endif //RSXPAGE_H