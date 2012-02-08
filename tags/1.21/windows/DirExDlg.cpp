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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "DirExDlg.h"
#include "WinUtil.h"

LRESULT DirExDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	#define ATTACH(id, var) var.Attach(GetDlgItem(id))
	ATTACH(IDC_DIR_EX_NAME, ctrlName);
	ATTACH(IDC_DIR_EX_PATH, ctrlPath);
	ATTACH(IDC_DIR_EX_EXTENSION, ctrlExtensions);

	ctrlName.SetWindowText(name.c_str());
	ctrlPath.SetWindowText(path.c_str());
	ctrlExtensions.SetWindowText(extensions.c_str());

	return 0;
}

LRESULT DirExDlg::OnBrowse(UINT /*uMsg*/, WPARAM /*wParam*/, HWND /*lParam*/, BOOL& /*bHandled*/) {
	TCHAR buf[MAX_PATH];

	GetDlgItemText(IDC_DIR_EX_PATH, buf, MAX_PATH);
	tstring dir = buf;
	if(WinUtil::browseDirectory(dir, m_hWnd)) {
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';
	
		SetDlgItemText(IDC_DIR_EX_PATH, dir.c_str());
	}
	return 0;
}