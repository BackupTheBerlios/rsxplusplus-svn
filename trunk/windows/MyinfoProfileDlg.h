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

#ifndef MYINFO_PROFILE_DIALOG_H
#define MYINFO_PROFILE_DIALOG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/Util.h"
#include "../client/ClientProfileManager.h"
#include "WinUtil.h"

#include "PropPage.h"
#include "../client/StringTokenizer.h"
#include "LineDlg.h"
#include "../client/RawManager.h"

class MyinfoProfileDlg : public CDialogImpl<MyinfoProfileDlg>, protected RawSelector {

	CEdit ctrlName, ctrlVersion, ctrlTag, ctrlExtendedTag, ctrlNick, ctrlShared, ctrlEmail, ctrlStatus, 
		ctrlCheatingDescription, ctrlAddLine, ctrlConnection, ctrlComment, ctrlRegExpText, ctrlID;
	CComboBox ctrlRaw, ctrlRegExpCombo;
	CButton ctrlTagVersion, ctrlUseExtraVersion, ctrlRegExpButton, ctrlIsNMDC;
public:
	
	tstring name;
	tstring version;
	tstring tag;
	tstring extendedTag;
	tstring nick;
	tstring shared;
	tstring email;
	tstring status;
	tstring cheatingDescription;
	tstring addLine;
	tstring connection;
	tstring comment;
	int priority;
	int rawToSend;
	bool tagVersion;
	bool useExtraVersion;
	bool isNMDC;
	int currentProfileId;
	bool adding;
	int id;

	MyinfoProfile currentProfile;

	enum { IDD = IDD_MYINFO_PROFILE };

	BEGIN_MSG_MAP(MyinfoProfileDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_NAME, onChange)
		COMMAND_ID_HANDLER(IDC_VERSION, onChange)
		COMMAND_ID_HANDLER(IDC_TAG, onChangeTag)
		COMMAND_ID_HANDLER(IDC_EXTENDED_TAG, onChange)
		COMMAND_ID_HANDLER(IDC_NICK, onChange)
		COMMAND_ID_HANDLER(IDC_SHARED, onChange)
		COMMAND_ID_HANDLER(IDC_EMAIL, onChange)
		COMMAND_ID_HANDLER(IDC_STATUS, onChange)
		COMMAND_ID_HANDLER(IDC_CHEATING_DESCRIPTION, onChange)
		COMMAND_ID_HANDLER(IDC_CONNECTION, onChange)
		COMMAND_ID_HANDLER(IDC_COMMENT, onChange)
		COMMAND_ID_HANDLER(IDC_NEXT, onNext)
		COMMAND_ID_HANDLER(IDC_BACK, onNext)
		COMMAND_ID_HANDLER(IDC_USE_EXTRA_VERSION, onChange)
		COMMAND_ID_HANDLER(IDC_VERSION_MISMATCH, onChange)
		COMMAND_ID_HANDLER(IDC_REGEXP_TESTER_BUTTON, onMatch)
		COMMAND_ID_HANDLER(IDC_CLIPBOARD_ONE, onClip)
		COMMAND_ID_HANDLER(IDC_CLIPBOARD_TWO, onClip)
		COMMAND_ID_HANDLER(IDC_CLIPBOARD_THREE, onClip)
		COMMAND_ID_HANDLER(IDC_CLIPBOARD_FOUR, onClip)
		COMMAND_ID_HANDLER(IDC_CLIPBOARD_FIVE, onClip)
		COMMAND_ID_HANDLER(IDC_CLIPBOARD_SIX, onClip)
	END_MSG_MAP()

	MyinfoProfileDlg() : priority(0), rawToSend(0), useExtraVersion(0), id(0) { };

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlName.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChangeTag(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onNext(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCheckButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(wID == IDOK) {
			updateVars();
//			if(adding) {
				MyinfoProfile::List lst = ClientProfileManager::getInstance()->getMyinfoProfiles();
				for(MyinfoProfile::List::const_iterator j = lst.begin(); j != lst.end(); ++j) {
					if((*j).getId() != currentProfileId) {
						if(id > 0 && ((*j).getProfileId() == id)) {
							MessageBox(_T("Profile with that ID already exist"), _T("RSX++"), MB_ICONSTOP);
							return 0;
						}
						if((*j).getName().compare(Text::fromT(name)) == 0) {
							LineDlg dlg;
							dlg.line = name += _T("(2)");
							tstring msg = name + _T(" ") + TSTRING(C_PROFILE_EXIST);
							dlg.title = msg;
							if(dlg.DoModal() == IDOK){
								name = dlg.line;
							} else {
								return 0;
							}				
						}
					}
				}
//			}
		}
		params.clear();
		EndDialog(wID);
		return 0;
	}
	LRESULT onClip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	int itemCount;
private:
	enum { BUF_LEN = 1024 };
	enum { VERSION, TAG, DESCRIPTION, NICK, SHARED, EMAIL, STATUS, CONNECTION };
	void updateAddLine();
	void updateTag();
	void updateVars();
	void updateControls();
	void getProfile();
	StringMap params;
};

#endif //MYINFO_PROFILE_DIALOG_H