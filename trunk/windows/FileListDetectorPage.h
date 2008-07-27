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

#ifndef FILELIST_DETECTOR_PAGE
#define FILELIST_DETECTOR_PAGE

#include <atlcrack.h>
#include "PropPage.h"
#include "ExListViewCtrl.h"

#include "../client/RawManager.h"

class FileListDetectorPage : public CPropertyPage<IDD_FILELIST_DETECTOR_PAGE>, public PropPage {
public:
	FileListDetectorPage(SettingsManager *s) : PropPage(s) {
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT) + _T('\\') + TSTRING(SETTINGS_FILELIST_DETECTOR)).c_str());
		SetTitle(title);
	};

	~FileListDetectorPage() {
		free(title);
		ctrlProfiles.Detach();
	};

	BEGIN_MSG_MAP_EX(FileListDetectorPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		COMMAND_ID_HANDLER(IDC_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_CHANGE, onChange)
		COMMAND_ID_HANDLER(IDC_USE_SDL_KICK, onEnable)
		NOTIFY_HANDLER(IDC_FILELIST_DETECTOR_LIST, NM_CUSTOMDRAW, onCustomDraw)
		NOTIFY_HANDLER(IDC_FILELIST_DETECTOR_LIST, NM_DBLCLK, onDblClick)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	LRESULT onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
		return onChange(0, 0, 0, bHandled);
	}
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
protected:
	ExListViewCtrl ctrlProfiles;
	
	static Item items[];
	static TextItem texts[];
	TCHAR* title;

	//void addEntry(const FileListDetectorProfile& fd, int pos);
	void fixControls();
};

#endif //FILELIST_DETECTOR_PAGE_H