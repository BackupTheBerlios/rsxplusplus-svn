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

#ifndef FILELIST_DETECTOR_DLG_H
#define FILELIST_DETECTOR_DLG_H

#include "../client/Util.h"

#include "CRawCombo.h"

class FileListDetectorDlg : public CDialogImpl<FileListDetectorDlg> {
public:
	string name;
	string detect;
	string cheatingDescription;
	int priority;
	int rawToSend;
	bool badClient;
	int currentProfileId;
	bool adding;

//	FileListDetectorProfile currentProfile;

	enum { IDD = IDD_ADD_FILELIST_PROFILE };

	BEGIN_MSG_MAP(FileListDetectorDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	FileListDetectorDlg() : priority(0), rawToSend(0) { };
	~FileListDetectorDlg() {
		cRaw.Detach();
	}

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlName.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(wID == IDOK) {
			updateVars();
			/*if(adding) {
				FileListDetectorProfile::List& lst = ClientProfileManager::getInstance()->getFileListDetectors();
				for(FileListDetectorProfile::List::const_iterator j = lst.begin(); j != lst.end(); ++j) {
					if((*j).getName().compare(name) == 0) {
						MessageBox(_T("A file list generator profile with this name already exists"), _T("Error!"), MB_ICONSTOP);
						return 0;
					}
				}
			}*/
		}
		EndDialog(wID);
		return 0;
	}
private:
	CEdit ctrlName, ctrlDetect, ctrlCheatingDescription;
	CRawCombo cRaw;
	CButton ctrlBadClient;

	enum { BUF_LEN = 1024 };

	void updateControls();
	void updateVars();
	void getProfile();
};
#endif //FILELIST_DETECTOR_DLG_H
