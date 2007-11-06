
#if !defined(AFX_FileListDetectorDlg_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)
#define AFX_FileListDetectorDlg_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/Util.h"
#include "../client/ClientProfileManager.h"

#include "../client/RawManager.h"

class FileListDetectorDlg : public CDialogImpl<FileListDetectorDlg>, protected RawSelector
{
	CEdit ctrlName, ctrlDetect, ctrlCheatingDescription;

	CComboBox cRaw;

	CButton ctrlBadClient;

public:
	
	string name;
	string detect;
	string cheatingDescription;
	int priority;
	int rawToSend;
	bool badClient;
	int currentProfileId;
	bool adding;

	FileListDetectorProfile currentProfile;

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
			if(adding) {
				FileListDetectorProfile::List lst = ClientProfileManager::getInstance()->getFileListDetectors();
				for(FileListDetectorProfile::List::const_iterator j = lst.begin(); j != lst.end(); ++j) {
					if((*j).getName().compare(name) == 0) {
						MessageBox(_T("A file list generator profile with this name already exists"), _T("Error!"), MB_ICONSTOP);
						return 0;
					}
				}
			}
		}
		EndDialog(wID);
		return 0;
	}
private:
	enum { BUF_LEN = 1024 };

	void updateControls();
	void updateVars();
	void getProfile();
};

#endif
