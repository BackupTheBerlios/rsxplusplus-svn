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

#include "stdafx.h"

#include "../client/DCPlusPlus.h"
#include "../client/Util.h"
#include "../client/SettingsManager.h"
#include "../client/FavoriteManager.h"

#include "Resource.h"
#include "FavoriteDirsPage.h"
#include "WinUtil.h"
#include "LineDlg.h"
#include "DirExDlg.h" //RSX++

PropPage::TextItem FavoriteDirsPage::texts[] = {
	{ IDC_SETTINGS_FAVORITE_DIRECTORIES, ResourceManager::SETTINGS_FAVORITE_DIRS },
	{ IDC_REMOVE, ResourceManager::REMOVE },
	{ IDC_ADD, ResourceManager::SETTINGS_ADD_FOLDER },
	{ IDC_RENAME, ResourceManager::SETTINGS_RENAME_FOLDER },
	//RSX++
	{ IDC_REMOVE_EX, ResourceManager::REMOVE },
	{ IDC_ADD_EX, ResourceManager::SETTINGS_ADD_FOLDER },
	{ IDC_RENAME_EX, ResourceManager::SETTINGS_EDIT_FOLDER },
	//END
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT FavoriteDirsPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	ctrlDirectories.Attach(GetDlgItem(IDC_FAVORITE_DIRECTORIES));
	ctrlDirectories.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
		
	// Prepare shared dir list
	CRect rc; 
	ctrlDirectories.GetClientRect(rc); 
	ctrlDirectories.InsertColumn(0, CTSTRING(FAVORITE_DIR_NAME), LVCFMT_LEFT, rc.Width()/4, 0); 
	ctrlDirectories.InsertColumn(1, CTSTRING(DIRECTORY), LVCFMT_LEFT, rc.Width()*3 /4, 1);
	StringPairList directories = FavoriteManager::getInstance()->getFavoriteDirs();
	for(StringPairIter j = directories.begin(); j != directories.end(); j++)
	{
		int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), Text::toT(j->second));
		ctrlDirectories.SetItemText(i, 1, Text::toT(j->first).c_str());
	}
	//RSX++
	ctrlDirectoriesEx.Attach(GetDlgItem(IDC_DIRS_EX));
	ctrlDirectoriesEx.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);
		
	ctrlDirectoriesEx.GetClientRect(rc); 
	ctrlDirectoriesEx.InsertColumn(0, CTSTRING(SETTINGS_NAME),			LVCFMT_LEFT, (rc.Width() /3)-30, 0); 
	ctrlDirectoriesEx.InsertColumn(1, CTSTRING(DIRECTORY),				LVCFMT_LEFT, (rc.Width() /3)+30, 1);
	ctrlDirectoriesEx.InsertColumn(2, CTSTRING(SETTINGS_EXTENSIONS),	LVCFMT_LEFT, rc.Width() /3, 2);

	DirectoriesEx::List lst = FavoriteManager::getInstance()->getDirectoriesEx();
	for(DirectoriesEx::Iter j = lst.begin(); j != lst.end(); ++j) {
		DirectoriesEx::Ptr pa = *j;	
		addDirEx(pa, ctrlDirectoriesEx.GetItemCount());
	}
	//END
	return TRUE;
}


void FavoriteDirsPage::write()
{
//	PropPage::write((HWND)*this, items);
}

LRESULT FavoriteDirsPage::onDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/){
	HDROP drop = (HDROP)wParam;
	tstring buf;
	buf.resize(MAX_PATH);

	UINT nrFiles;
	
	nrFiles = DragQueryFile(drop, (UINT)-1, NULL, 0);
	
	for(UINT i = 0; i < nrFiles; ++i){
		if(DragQueryFile(drop, i, &buf[0], MAX_PATH)){
			if(PathIsDirectory(&buf[0]))
				addDirectory(buf);
		}
	}

	DragFinish(drop);

	return 0;
}

LRESULT FavoriteDirsPage::onItemchangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	if(GetFocus() == ctrlDirectories.m_hWnd) {
		NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
		::EnableWindow(GetDlgItem(IDC_REMOVE), (lv->uNewState & LVIS_FOCUSED));
		::EnableWindow(GetDlgItem(IDC_RENAME), (lv->uNewState & LVIS_FOCUSED));
	} else if(GetFocus() == ctrlDirectoriesEx.m_hWnd) {
		NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
		::EnableWindow(GetDlgItem(IDC_REMOVE_EX), (lv->uNewState & LVIS_FOCUSED));
		::EnableWindow(GetDlgItem(IDC_RENAME_EX), (lv->uNewState & LVIS_FOCUSED));
	}
	return 0;		
}

LRESULT FavoriteDirsPage::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	switch(kd->wVKey) {
	case VK_INSERT:
		PostMessage(WM_COMMAND, IDC_ADD, 0);
		break;
	case VK_DELETE:
		PostMessage(WM_COMMAND, IDC_REMOVE, 0);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT FavoriteDirsPage::onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;

	if(GetFocus() == ctrlDirectories.m_hWnd) {
		if(item->iItem >= 0) {
			PostMessage(WM_COMMAND, IDC_RENAME, 0);
		} else if(item->iItem == -1) {
			PostMessage(WM_COMMAND, IDC_ADD, 0);
		}
	} else if(GetFocus() == ctrlDirectoriesEx.m_hWnd) {
		if(item->iItem >= 0) {
			PostMessage(WM_COMMAND, IDC_RENAME_EX, 0);
		} else if(item->iItem == -1) {
			PostMessage(WM_COMMAND, IDC_ADD_EX, 0);
		}
	}

	return 0;
}

LRESULT FavoriteDirsPage::onClickedAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	tstring target;
	if(WinUtil::browseDirectory(target, m_hWnd)) {
		addDirectory(target);
	}
	
	return 0;
}

LRESULT FavoriteDirsPage::onClickedRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR buf[MAX_PATH];
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT;
	item.cchTextMax = sizeof(buf);
	item.pszText = buf;

	int i = -1;
	while((i = ctrlDirectories.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		item.iItem = i;
		item.iSubItem = 1;
		ctrlDirectories.GetItem(&item);
		if(FavoriteManager::getInstance()->removeFavoriteDir(Text::fromT(buf)))
			ctrlDirectories.DeleteItem(i);
	}
	
	return 0;
}

LRESULT FavoriteDirsPage::onClickedRename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR buf[MAX_PATH];
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT;
	item.cchTextMax = sizeof(buf);
	item.pszText = buf;

	int i = -1;
	while((i = ctrlDirectories.GetNextItem(i, LVNI_SELECTED)) != -1) {
		item.iItem = i;
		item.iSubItem = 0;
		ctrlDirectories.GetItem(&item);

		LineDlg virt;
		virt.title = TSTRING(FAVORITE_DIR_NAME);
		virt.description = TSTRING(FAVORITE_DIR_NAME_LONG);
		virt.line = tstring(buf);
		if(virt.DoModal(m_hWnd) == IDOK) {
			if (FavoriteManager::getInstance()->renameFavoriteDir(Text::fromT(buf), Text::fromT(virt.line))) {
				ctrlDirectories.SetItemText(i, 0, virt.line.c_str());
			} else {
				MessageBox(CTSTRING(DIRECTORY_ADD_ERROR));
			}
		}
	}
	return 0;
}


void FavoriteDirsPage::addDirectory(const tstring& aPath){
	tstring path = aPath;
	if( path[ path.length() -1 ] != PATH_SEPARATOR )
		path += PATH_SEPARATOR;

	LineDlg virt;
	virt.title = TSTRING(FAVORITE_DIR_NAME);
	virt.description = TSTRING(FAVORITE_DIR_NAME_LONG);
	virt.line = Util::getLastDir(path);
	if(virt.DoModal(m_hWnd) == IDOK) {
		if (FavoriteManager::getInstance()->addFavoriteDir(Text::fromT(path), Text::fromT(virt.line))) {
			int j = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), virt.line );
			ctrlDirectories.SetItemText(j, 1, path.c_str());
		} else {
			MessageBox(CTSTRING(DIRECTORY_ADD_ERROR));
		}
	}
}
//RSX++
LRESULT FavoriteDirsPage::onItemchangedDirectoriesEx(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	::EnableWindow(GetDlgItem(IDC_REMOVE_EX), (lv->uNewState & LVIS_FOCUSED));
	::EnableWindow(GetDlgItem(IDC_RENAME_EX), (lv->uNewState & LVIS_FOCUSED));
	return 0;		
}

LRESULT FavoriteDirsPage::onDoubleClickDirEx(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	if(item->iItem >= 0) {
		PostMessage(WM_COMMAND, IDC_RENAME_EX, 0);
	} else if(item->iItem == -1) {
		PostMessage(WM_COMMAND, IDC_ADD_EX, 0);
	}
	return 0;
}

LRESULT FavoriteDirsPage::onClickedAddDirEx(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	DirExDlg dlg;
	if(dlg.DoModal() == IDOK){
		addDirEx(FavoriteManager::getInstance()->addDirEx(Text::fromT(dlg.name), Text::fromT(dlg.path), Text::fromT(dlg.extensions)), ctrlDirectoriesEx.GetItemCount());
	}
	return 0;
}

LRESULT FavoriteDirsPage::onClickedRemoveDirEx(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlDirectoriesEx.GetSelectedCount() == 1) {
		int sel = ctrlDirectoriesEx.GetSelectedIndex();
		FavoriteManager::getInstance()->removeDirEx(sel);
		ctrlDirectoriesEx.DeleteItem(sel);
	}
	return 0;
}

LRESULT FavoriteDirsPage::onClickedRenameDirEx(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlDirectoriesEx.GetSelectedCount() == 1) {
		int sel = ctrlDirectoriesEx.GetSelectedIndex();
		DirectoriesEx pa;
		FavoriteManager::getInstance()->getDirEx(sel, pa);

		DirExDlg dlg;
		dlg.name = Text::toT(pa.getName());
		dlg.path = Text::toT(pa.getPath());
		dlg.extensions = Text::toT(pa.getExtension());

		if(dlg.DoModal() == IDOK) {
			pa.setName(Text::fromT(dlg.name));
			pa.setPath(Text::fromT(dlg.path));
			pa.setExtension(Text::fromT(dlg.extensions));

			FavoriteManager::getInstance()->updateDirEx(sel, pa);

			ctrlDirectoriesEx.SetItemText(sel, 0, dlg.name.c_str());
			ctrlDirectoriesEx.SetItemText(sel, 1, dlg.path.c_str());
			ctrlDirectoriesEx.SetItemText(sel, 2, dlg.extensions.c_str());
		}
	}

	return 0;
}

void FavoriteDirsPage::addDirEx(DirectoriesEx::Ptr pa, int pos) {
	TStringList lst;
	lst.push_back(Text::toT(pa->getName()));
	lst.push_back(Text::toT(pa->getPath()));
	lst.push_back(Text::toT(pa->getExtension()));
	ctrlDirectoriesEx.insert(pos, lst, 0, 0);
}
//END
/**
 * @file
 * $Id: FavoriteDirsPage.cpp 477 2010-01-29 08:59:43Z bigmuscle $
 */
