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

#ifndef PARAMS_PAGE_H
#define PARAMS_PAGE_H

#include "PropPage.h"
#include "ExListViewCtrl.h"

class ParamsPage : public CPropertyPage<IDD_PARAMS>, public PropPage {
public:
	ParamsPage(SettingsManager *s) : PropPage(s) {
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT) + _T('\\') /*+ TSTRING(SETTINGS_CLIENTS) + _T('\\')*/ + TSTRING(SETTINGS_PARAMS)).c_str());
		SetTitle(title);
	};

	~ParamsPage() { 
		ctrlParams.Detach();
		free(title);
	};

	BEGIN_MSG_MAP(ParamsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_CHANGE, onChange)
		NOTIFY_HANDLER(IDC_PARAM_ITEMS, NM_DBLCLK, onDblClick)
		NOTIFY_HANDLER(IDC_PARAM_ITEMS, NM_CUSTOMDRAW, onCustomDraw)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
		return onChange(0, 0, 0, bHandled);
	}
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
protected:
	ExListViewCtrl ctrlParams;
	static TextItem texts[];
	TCHAR* title;
};

#endif //PARAMS_PAGE_H