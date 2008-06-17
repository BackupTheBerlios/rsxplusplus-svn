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

#ifndef RAWPAGE_H
#define RAWPAGE_H

#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "../client/RawManager.h"

class RawPage : public CPropertyPage<IDD_RAW_PAGE>, public PropPage {
public:
	RawPage(SettingsManager *s) : PropPage(s), nosave(true), gotFocusOnAction(false), gotFocusOnRaw(false) { 
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT) + _T('\\') + TSTRING(SETTINGS_ACTION_RAW)).c_str());
		SetTitle(title);
	};

	~RawPage() {
		ctrlAction.Detach();
		ctrlRaw.Detach();
		free(title);
	};

	BEGIN_MSG_MAP(RawPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD_ACTION, onAddAction)
		COMMAND_ID_HANDLER(IDC_RENAME_ACTION, onRenameAction)
		COMMAND_ID_HANDLER(IDC_REMOVE_ACTION, onRemoveAction)
		COMMAND_ID_HANDLER(IDC_ADD_RAW, onAddRaw)
		COMMAND_ID_HANDLER(IDC_CHANGE_RAW, onChangeRaw)
		COMMAND_ID_HANDLER(IDC_REMOVE_RAW, onRemoveRaw)
		COMMAND_ID_HANDLER(IDC_MOVE_RAW_UP, onMoveRawUp)
		COMMAND_ID_HANDLER(IDC_MOVE_RAW_DOWN, onMoveRawDown)
		NOTIFY_HANDLER(IDC_RAW_PAGE_ACTION, LVN_ITEMCHANGED, onItemChanged)
		NOTIFY_HANDLER(IDC_RAW_PAGE_RAW, LVN_ITEMCHANGED, onItemChangedRaw)
		NOTIFY_HANDLER(IDC_RAW_PAGE_RAW, NM_DBLCLK, onDblClick)
		NOTIFY_HANDLER(IDC_RAW_PAGE_ACTION, NM_DBLCLK, onDblClick)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onAddAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRenameAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddRaw(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChangeRaw(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveRaw(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMoveRawUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMoveRawDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onItemChangedRaw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onDblClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();

protected:
	static TextItem texts[];

	ExListViewCtrl ctrlAction;
	ExListViewCtrl ctrlRaw;

	bool nosave;
	bool gotFocusOnAction;
	bool gotFocusOnRaw;

	void addEntryAction(int id, const string name, bool actif, int pos);
	void addEntryRaw(const Action::Raw& ra, int pos);

	TCHAR* title;
};
#endif //RAWPAGE_H