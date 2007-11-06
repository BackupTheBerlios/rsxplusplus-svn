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

#ifndef FAKE_DETECT_PAGE_H
#define FAKE_DETECT_PAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "../client/RawManager.h"

class FakeDetect : public CPropertyPage<IDD_FAKEDETECT>, public PropPage, protected RawSelector
{
public:
	FakeDetect(SettingsManager *s) : PropPage(s) {
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT)).c_str());
		SetTitle(title);
		m_psp.dwFlags |= PSP_RTLREADING;
	};

	~FakeDetect() { 
		cRaw.Detach();
		cShowCheat.Detach();
		ctrlList.Detach();
		free(title); 
	};

	BEGIN_MSG_MAP(FakeDetect)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_RAW_DETECTOR, onRawChanged)
		COMMAND_ID_HANDLER(IDC_SHOW_CHEAT, onShowChanged)
		NOTIFY_HANDLER(IDC_DETECTOR_ITEMS, LVN_ITEMCHANGED, onItemChanged)
		NOTIFY_HANDLER(IDC_DETECTOR_ITEMS, NM_CUSTOMDRAW, onCustomDraw)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onRawChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onShowChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();

protected:
	static TextItem texts[];

	ExListViewCtrl ctrlList;
	CComboBox cRaw;
	CComboBox cShowCheat;

	void insertAllItem();
	void insertItem(const tstring& a, int b, int showCheat);
	int settingRaw[20];
	int showCheat[20];

	static Item items[];
	TCHAR* title;
};

#endif //FAKE_DETECT_PAGE_H