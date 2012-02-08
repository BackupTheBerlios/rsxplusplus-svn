/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
 *
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

#ifndef IPWATCHPAGE_H
#define IPWATCHPAGE_H

#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "../rsx/IpManager.h"

class IpWatchPage : public CPropertyPage<IDD_IP_PAGE>, public PropPage {
public:
	IpWatchPage(SettingsManager *s) : PropPage(s) { 
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT) + _T('\\') + TSTRING(SETTINGS_IP_PAGE)).c_str());
		SetTitle(title);
	};

	~IpWatchPage() { 
		cWatch.Detach();
		free(title);
	};

	BEGIN_MSG_MAP(IpWatchPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD_WATCH, onAddWatch)
		COMMAND_ID_HANDLER(IDC_REMOVE_WATCH, onRemoveWatch)
		COMMAND_ID_HANDLER(IDC_CHANGE_WATCH, onChangeWatch)
		COMMAND_ID_HANDLER(IDC_ENABLE_IPWATCH, onEnable)
		NOTIFY_HANDLER(IDC_IPWATCH_LIST, NM_CUSTOMDRAW, onCustomDraw)
		NOTIFY_HANDLER(IDC_IPWATCH_LIST, NM_DBLCLK, onIPWDblClick)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onAddWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChangeWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onIPWDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
		return onChangeWatch(0,0,0, bHandled);
	}
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
private:
	ExListViewCtrl cWatch;
	static TextItem texts[];
	static Item items[];

	TCHAR* title;

	void fixControls();

	void addEntryWatch(IPWatch* fs, int pos);
	string getAction(int i) {
		switch(i) {
			case 0: return "Protect"; break;
			case 1: return "Ban"; break;
			case 2: return "Notify"; break;
			default: return "None";
		}
	}

};

#endif //IPWATCHPAGE_H

/**
 * @file
 * $Id: IpWatchPage.h 230 2011-01-30 18:15:33Z adrian_007 $
 */