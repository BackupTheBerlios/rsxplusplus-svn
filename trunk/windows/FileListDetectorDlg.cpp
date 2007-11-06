
#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"
#include "WinUtil.h"

#include "FileListDetectorDlg.h"

#define GET_TEXT(id, var) \
	GetDlgItemText(id, buf, 1024); \
	var = Text::fromT(buf);

#define ATTACH(id, var) var.Attach(GetDlgItem(id))

LRESULT FileListDetectorDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(currentProfileId != -1) {
		// FIXME disable this for now to stop potential dupes (ahh. fuck it, leave it enabled :p)
		//::EnableWindow(GetDlgItem(IDC_FILELIST_PROFILE_NAME), false);
		adding = false;
		getProfile();
	} else {
		adding = true;
		//::EnableWindow(GetDlgItem(IDC_NEXT), false);
	}

	// Translate dialog
	SetDlgItemText(IDC_FILELIST_DLG_DETECT_STRING, /*CTSTRING(DETECT_STRING)*/_T("Detect String"));

	ATTACH(IDC_FILELIST_PROFILE_NAME, ctrlName);
	ATTACH(IDC_FILELIST_PROFILE_DETECT, ctrlDetect);
	ATTACH(IDC_FILELIST_PROFILE_CHEAT, ctrlCheatingDescription);

	//Zion++ //Raw Manager //DEBUT
	createList();

	ATTACH(IDC_FILELIST_PROFILE_RAW, cRaw);
	for(ActionList::const_iterator i = idAction.begin(); i != idAction.end(); ++i) {
		cRaw.AddString(RawManager::getInstance()->getNameActionId(i->second).c_str());
	}
	//Zion++ //Raw Manager //FIN

	ATTACH(IDC_BAD_CLIENT, ctrlBadClient);

	updateControls();
	
	CenterWindow(GetParent());
	return FALSE;
}

void FileListDetectorDlg::getProfile() {
	ClientProfileManager::getInstance()->getFileListDetector(currentProfileId, currentProfile);

	name = currentProfile.getName();
	detect = currentProfile.getDetect();
	cheatingDescription = currentProfile.getCheatingDescription();
	rawToSend = currentProfile.getRawToSend();
	badClient = currentProfile.getBadClient();
}

void FileListDetectorDlg::updateControls() {
	
	ctrlName.SetWindowText(Text::toT(name).c_str());
	ctrlDetect.SetWindowText(Text::toT(detect).c_str());
	ctrlCheatingDescription.SetWindowText(Text::toT(cheatingDescription).c_str());
	cRaw.SetCurSel(getId(rawToSend));
	ctrlBadClient.SetCheck(badClient ? BST_CHECKED : BST_UNCHECKED);
	
	ctrlName.SetFocus();
}
void FileListDetectorDlg::updateVars() {
	TCHAR buf[1024];

	GET_TEXT(IDC_FILELIST_PROFILE_NAME, name);
	GET_TEXT(IDC_FILELIST_PROFILE_DETECT, detect);
	GET_TEXT(IDC_FILELIST_PROFILE_CHEAT, cheatingDescription);
	rawToSend = getIdAction(cRaw.GetCurSel());
	badClient = ctrlBadClient.GetCheck() == BST_CHECKED;
}