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
#include "KickDlg.h"

LRESULT RsxKickDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	createList();
	cActionList.Attach(GetDlgItem(IDC_ACTION_LIST)); 
	for(ActionList::const_iterator i = idAction.begin(); i != idAction.end(); ++i) {
		cActionList.AddString(RawManager::getInstance()->getNameActionId(i->second).c_str());
	}
	cActionList.SetCurSel(0);

	cRaw.Attach(GetDlgItem(IDC_LINE_RAW));
	cRaw.AddString(_T("No Raw"));
	cRaw.SetCurSel(0);

	ctrlCommand.Attach(GetDlgItem(IDC_KICK_DLG_CMD));
	CenterWindow(GetParent());
	return FALSE;
}
	
LRESULT RsxKickDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		int len = ctrlCommand.GetWindowTextLength() + 1;
		AutoArray<TCHAR> buf(len);
		GetDlgItemText(IDC_KICK_DLG_CMD, buf, len);
		rawCommand = buf;
	}
	EndDialog(wID);
	return 0;
}

LRESULT RsxKickDlg::onSelChange(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	int sel = cActionList.GetCurSel();
	if(wID == IDC_ACTION_LIST) {
		while(cRaw.GetCount() > 0)
			cRaw.DeleteString(0);

		if(sel > 0) {
			Action::RawsList rawList = RawManager::getInstance()->getRawListActionId(getIdAction(sel));
			for(Action::RawsList::const_iterator j = rawList.begin(); j != rawList.end(); ++j) {
				cRaw.AddString(Text::toT(j->getName()).c_str());
			}
		} else {
			cRaw.AddString(_T("No Raw"));
		}
		cRaw.SetCurSel(0);
	}
	const string& aCmd = RawManager::getInstance()->getRawCommand(sel, cRaw.GetCurSel());
	ctrlCommand.SetWindowText(Text::toT(aCmd).c_str());
	return 0;
}