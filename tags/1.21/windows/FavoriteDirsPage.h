/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(FAVORITE_DIRS_PAGE_H)
#define FAVORITE_DIRS_PAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "WinUtil.h"

class FavoriteDirsPage : public CPropertyPage<IDD_FAVORITE_DIRSPAGE>, public PropPage
{
public:
	FavoriteDirsPage(SettingsManager *s) : PropPage(s) {
		title = _tcsdup((TSTRING(SETTINGS_DOWNLOADS) + _T('\\') + TSTRING(SETTINGS_FAVORITE_DIRS_PAGE)).c_str());
		SetTitle(title);
		m_psp.dwFlags |= PSP_RTLREADING;
	}
	~FavoriteDirsPage() {
		ctrlDirectories.Detach();
		ctrlDirectoriesEx.Detach(); //RSX++
		free(title);
	}

	BEGIN_MSG_MAP_EX(FavoriteDirsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_DROPFILES, onDropFiles)
		NOTIFY_HANDLER(IDC_FAVORITE_DIRECTORIES, LVN_ITEMCHANGED, onItemchangedDirectories)
		NOTIFY_HANDLER(IDC_FAVORITE_DIRECTORIES, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_FAVORITE_DIRECTORIES, NM_DBLCLK, onDoubleClick)
		COMMAND_ID_HANDLER(IDC_ADD, onClickedAdd)
		COMMAND_ID_HANDLER(IDC_REMOVE, onClickedRemove)
		COMMAND_ID_HANDLER(IDC_RENAME, onClickedRename)
		//RSX++
		NOTIFY_HANDLER(IDC_DIRS_EX, LVN_ITEMCHANGED, onItemchangedDirectoriesEx)
		NOTIFY_HANDLER(IDC_DIRS_EX, NM_DBLCLK, onDoubleClickDirEx)
		COMMAND_ID_HANDLER(IDC_ADD_EX, onClickedAddDirEx)
		COMMAND_ID_HANDLER(IDC_REMOVE_EX, onClickedRemoveDirEx)
		COMMAND_ID_HANDLER(IDC_RENAME_EX, onClickedRenameDirEx)
		//END
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onItemchangedDirectories(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onClickedAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClickedRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClickedRename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//RSX++
	LRESULT onItemchangedDirectoriesEx(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickDirEx(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onClickedAddDirEx(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClickedRemoveDirEx(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClickedRenameDirEx(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//END

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
	
protected:
	static TextItem texts[];
	ExListViewCtrl ctrlDirectories;
	ExListViewCtrl ctrlDirectoriesEx; //RSX++

	TCHAR* title;

	void addDirEx(DirectoriesEx* pa, int pos); //RSX++
	void addDirectory(const tstring& aPath);
};

#endif // !defined(FAVORITE_DIR_SPAGE_H)

/**
 * @file
 * $Id: FavoriteDirsPage.h 308 2007-07-13 18:57:02Z bigmuscle $
 */
