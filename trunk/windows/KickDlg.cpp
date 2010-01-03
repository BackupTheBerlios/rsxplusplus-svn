/*
 * Copyright (C) 2007-2010 adrian_007, adrian-007 on o2 point pl
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
#include "../client/RawManager.h"

#include "Resource.h"
#include "KickDlg.h"

LRESULT RsxKickDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	cAction.attach(GetDlgItem(IDC_ACTION_LIST), 0);

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
		tstring buf;
		buf.resize(len);

		GetDlgItemText(IDC_KICK_DLG_CMD, &buf[0], len);
		rawCommand = &buf[0];
	}
	EndDialog(wID);
	return 0;
}

LRESULT RsxKickDlg::onSelChange(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	int sel = cAction.GetCurSel();
	if(wID == IDC_ACTION_LIST) {
		while(cRaw.GetCount() > 0)
			cRaw.DeleteString(0);

		if(sel > 0) {
			Action* a = RawManager::getInstance()->findAction(cAction.getActionId());
			if(a != NULL) {
				for(Action::RawsList::const_iterator j = a->raw.begin(); j != a->raw.end(); ++j) {
					cRaw.AddString(Text::toT(j->getName()).c_str());
				}
			} else {
				cRaw.AddString(_T("No Raw"));
			}
		} else {
			cRaw.AddString(_T("No Raw"));
		}
		cRaw.SetCurSel(0);
	}
	Action* a = cAction.getAction();
	if(a != NULL) {
		tstring buf;
		buf.resize(cRaw.GetLBTextLen(cRaw.GetCurSel()) + 1);
		cRaw.GetLBText(cRaw.GetCurSel(), &buf[0]);
		string name = Text::fromT(buf);
		for(Action::RawsList::const_iterator i = a->raw.begin(); i != a->raw.end(); ++i) {
			if(stricmp(name, i->getName()) == 0) {
				ctrlCommand.SetWindowText(Text::toT(i->getRaw()).c_str());
				return 0;
			}
		}
	}
	return 0;
}
