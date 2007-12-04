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

#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "../client/RawManager.h"

class FakeDetect : public CPropertyPage<IDD_FAKEDETECT>, public PropPage, protected RawSelector {
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

private:
	struct DetectorItem {
		DetectorItem(int rpos, int cpos) : rawPos(rpos), dcPos(cpos) {
			rawId = RSXSettingsManager::getInstance()->get((RSXSettingsManager::IntSetting)rpos);
			displayCheat = RSXSettingsManager::getInstance()->getBool((RSXSettingsManager::IntSetting)cpos);
		};

		~DetectorItem() {
			RSXSettingsManager::getInstance()->set((RSXSettingsManager::IntSetting)rawPos, rawId);
			RSXSettingsManager::getInstance()->set((RSXSettingsManager::IntSetting)dcPos, displayCheat);
		};

		int rawId;
		bool displayCheat;
		int rawPos;
		int dcPos;
	};

	void addItem(const tstring& aName, int rawId, int cheatId);
	void setSpinRange(int ctrl, int rMin, int rMax) {
		CUpDownCtrl updown;
		updown.Attach(GetDlgItem(ctrl));
		updown.SetRange32(rMin, rMax);
		updown.Detach();
	}

	static TextItem texts[];
	static Item items[];

	ExListViewCtrl ctrlList;
	CComboBox cRaw;
	CComboBox cShowCheat;

	TCHAR* title;
};

#endif //FAKE_DETECT_PAGE_H