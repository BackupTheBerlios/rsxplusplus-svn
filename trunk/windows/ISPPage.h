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

#ifndef ISPPAGE_H
#define ISPPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"
#include "ExListViewCtrl.h"
#include <atlcrack.h>

#include "../rsx/IpManager.h"
#include "../client/RawManager.h"

class ISPPage : public CPropertyPage<IDD_ISP>, public PropPage, protected RawSelector {
public:
	ISPPage(SettingsManager *s) : PropPage(s) { 
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT) + _T('\\') + TSTRING(SETTINGS_IP_PAGE)).c_str());
		SetTitle(title);
	};

	~ISPPage() { 
		ctrlISP.Detach();
		cWatch.Detach();
		free(title);
	};

	BEGIN_MSG_MAP(ISPPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD_ISP, onAddISP)
		COMMAND_ID_HANDLER(IDC_ADD_WATCH, onAddWatch)
		COMMAND_ID_HANDLER(IDC_REMOVE_ISP, onRemoveISP)
		COMMAND_ID_HANDLER(IDC_REMOVE_WATCH, onRemoveWatch)
		COMMAND_ID_HANDLER(IDC_CHANGE_ISP, onChangeISP)
		COMMAND_ID_HANDLER(IDC_CHANGE_WATCH, onChangeWatch)
		COMMAND_ID_HANDLER(IDC_RELOAD_ISPS, onReload)
		NOTIFY_HANDLER(IDC_ISP, NM_DBLCLK, onDblClick)
		NOTIFY_HANDLER(IDC_ISP, NM_CUSTOMDRAW, onCustomDraw)
		NOTIFY_HANDLER(IDC_IPWATCH_LIST, NM_CUSTOMDRAW, onCustomDraw)
		NOTIFY_HANDLER(IDC_IPWATCH_LIST, NM_DBLCLK, onIPWDblClick)
		COMMAND_ID_HANDLER(IDC_ENABLE_ISP, onEnable)
		COMMAND_ID_HANDLER(IDC_ENABLE_IPWATCH, onEnable)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	LRESULT onAddISP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChangeISP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveISP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onReload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChangeWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
		return onChangeISP(0, 0, 0, bHandled);
	}
	LRESULT onIPWDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
		return onChangeWatch(0,0,0, bHandled);
	}
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
protected:
	ExListViewCtrl ctrlISP;
	ExListViewCtrl cWatch;

	static TextItem texts[];
	static Item items[];

	TCHAR* title;

private:
	void reload();
	void fixControls();

	void addEntryWatch(IPWatch* fs, int pos);
	string getAction(int i) {
		switch(i) {
			case 0: return "Protect"; break;
			case 1: return "Ban"; break;
			case 2: return "Notify"; break;
			default: return "Protect";
		}
	}

};

#endif //ISPPAGE_H

/**
 * @file
 * $Id: ISPPage.h,v 1.0 2004/16/11 18:04 Virus27 Exp $
 */