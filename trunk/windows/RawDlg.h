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

#ifndef RAWDLG_H
#define RAWDLG_H

#include "../client/Util.h"
#include "../client/FavoriteManager.h"

namespace dcpp {
	struct Raw;
}

class RawDlg : public CDialogImpl<RawDlg> {
public:
	string name;
	string raw;
	int time;
	bool useLua;

	enum { IDD = IDD_RAW_DLG };

	BEGIN_MSG_MAP(RawDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_HANDLER(IDC_RAW_VAR, CBN_SELCHANGE, onSelChange)
	END_MSG_MAP()

	RawDlg() : name("Raw"), raw(""), time(0), useLua(false) { };
	RawDlg(const Raw* r) : name(r->getName()), raw(r->getRaw()), time(r->getTime()), useLua(r->getLua()) { };

	~RawDlg() { 
		ctrlName.Detach();
		ctrlRaw.Detach();
		ctrlTime.Detach();
		cVariables.Detach();
		cVar.Detach();
		ctrlLua.Detach();
	};

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSelChange(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL& /* bHandled */);

private:
	CEdit ctrlName, ctrlTime, ctrlRaw, cVar;
	CComboBox cVariables;
	CButton ctrlLua;
};

#endif

/**
 * @file
 * $Id$
 */