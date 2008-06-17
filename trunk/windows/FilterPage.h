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

#ifndef FILTERPAGE_H
#define FILTERPAGE_H

#include "../client/FavoriteManager.h"

#include "PropPage.h"
#include "ExListViewCtrl.h"

class FilterPage : public CPropertyPage<IDD_FILTER_PAGE>, public PropPage {
public:
	FilterPage(SettingsManager *s) : PropPage(s) { 
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FILTER_PAGE)).c_str());
		SetTitle(title);
	};
	~FilterPage() { 
		ctrlFilters.Detach();
		cHL.Detach();
	};
	BEGIN_MSG_MAP(FilterPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD_FILTER, onAddFilter)
		COMMAND_ID_HANDLER(IDC_REMOVE_FILTER, onRemoveFilter)
		COMMAND_ID_HANDLER(IDC_CHANGE_FILTER, onChangeFilter)
		COMMAND_ID_HANDLER(IDC_ADD_HL, onAddHL)
		COMMAND_ID_HANDLER(IDC_REMOVE_HL, onRemoveHL)
		COMMAND_ID_HANDLER(IDC_CHANGE_HL, onChangeHL)
		COMMAND_ID_HANDLER(IDC_USE_CHAT_FILTER, onEnable)
		COMMAND_ID_HANDLER(IDC_USE_HL, onEnable)
		NOTIFY_HANDLER(IDC_FILTER_PAGE_ITEMS, NM_DBLCLK, onDblClick)
		NOTIFY_HANDLER(IDC_HL_ITEMS, NM_DBLCLK, onDblClickHL)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onAddFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChangeFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddHL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChangeHL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveHL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
		return onChangeFilter(0, 0, 0, bHandled);
	}
	LRESULT onDblClickHL(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
		return onChangeHL(0, 0, 0, bHandled);
	}

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
protected:
	ExListViewCtrl ctrlFilters;
	ExListViewCtrl cHL;

	TCHAR* title;
	void addEntryFilter(Filters* fs, int pos);
	void addEntryHL(HighLight* hl, int pos);

	static Item items[];
	static TextItem texts[];
	void fixControls();
};
#endif //FILTERPAGE_H

/**
 * @file
 * $Id: FilterPage.h 73 2006-01-20 17:35:16Z crakter $
 */