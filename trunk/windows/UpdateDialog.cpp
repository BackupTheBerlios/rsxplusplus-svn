/*
 * Copyright (C) 2007 adrian_007, adrian-007 on o2 point pl
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

#include "UpdateDialog.h"

#include "../client/ClientProfileManager.h"
#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "../client/version.h"
#include "../windows/WinUtil.h"
#include "../rsx/IpManager.h"

//some shortcuts
#define set_text(idc, val) ::SetWindowText(GetDlgItem(idc), Text::toT(val).c_str())
#define is_checked(idc) IsDlgButtonChecked(idc) == BST_CHECKED
#define set_visible(idc, val) ::ShowWindow(GetDlgItem(idc), val)

UpdateDialog::~UpdateDialog() {
	if(c != NULL) {
		c->removeListener(this);
		delete c;
		c = NULL;
	}
	if(m_hIcon) {
		DeleteObject((HGDIOBJ)m_hIcon);
	}
}

LRESULT UpdateDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	cStatus.Attach(GetDlgItem(IDC_STATUS));
	cChangeLog.Attach(GetDlgItem(IDC_UPDATE_HISTORY_TEXT));
	cProgress.Attach(GetDlgItem(IDC_PROGRESS));
	cClientCheck.Attach(GetDlgItem(IDC_CLIENT_ACTIVE));
	cMyINFOCheck.Attach(GetDlgItem(IDC_MYINFO_ACTIVE));
	cISPCheck.Attach(GetDlgItem(IDC_ISP_ACTIVE));

	set_text(IDC_UPDATE_VERSION_CURRENT_PROFIL, ClientProfileManager::getInstance()->getProfileVersion());
	set_text(IDC_UPDATE_VERSION_CURRENT_PROFIL_MYINFO, ClientProfileManager::getInstance()->getMyinfoProfileVersion());
	set_text(IDC_UPDATE_VERSION_CURRENT_IPW, IpManager::getInstance()->getIpWatchVersion());
	set_text(IDC_CLIENT_ADDRESS, RSXSETTING(UPDATE_URL));
	set_text(IDC_MYINFO_ADDRESS, RSXSETTING(UPDATE_MYINFOS));
	set_text(IDC_ISP_ADDRESS, RSXSETTING(UPDATE_IPWATCH_URL));
	set_text(IDCLOSE, STRING(CLOSE));
	set_text(IDC_UPDATE_VERSION_CURRENT, VERSIONSTRING);
	
	UpdateManager::getInstance()->addListener(this);
	if(xmlData.empty()) {
		c = new HttpConnection;
		c->addListener(this);
		c->downloadFile(VERSION_URL);
		updateStatus(TSTRING(CONNECTING_TO_SERVER) + _T("..."));
	} else {
		updateStatus(_T("Reading XML file..."));
		versionXML();
	}

	SetWindowText(CTSTRING(UPDATE_CHECK));
	m_hIcon = ::LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDR_UPDATE));
	SetIcon(m_hIcon, FALSE);
	SetIcon(m_hIcon, TRUE);
	CenterWindow(GetParent());
	return 0;
}

LRESULT UpdateDialog::OnButton(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch(wID) {
		case IDC_UPDATE_DOWNLOAD: {
			if(!downloadUrl.empty())
				WinUtil::openLink(Text::toT(downloadUrl).c_str()); 
			break;
		}
		case IDC_LOAD_BACKUP:  {
			reload = true;
			prepareFiles();
			if(!updateItems.size()){
				updateStatus(_T("Failed, You didn't choose any files."));
				return 0;
			} else {
				if(MessageBox(_T("This will replace your current profile settings, do you wish to proceed?."), _T("Warning!"), MB_YESNO | MB_ICONWARNING) != IDYES) {
					return 0;
				}
			}
			for(UpdateMap::const_iterator j = updateItems.begin(); j != updateItems.end(); ++j) {
				try {
					string fname = Util::getConfigPath() + RsxUtil::getUpdateFileNames((int)j->first);
					File::renameFile(fname, fname + ".tmp");
					try {
						File::renameFile(fname + ".old", fname);
					} catch (...) {
						File::renameFile(fname + ".tmp", fname);
						continue;
					}
					File::renameFile(fname + ".tmp", fname + ".old");
					cProgress.SetPos((int)j->first);
				} catch(...) {
					continue;
				}
				updateStatus(_T("Reloading ") + Text::toT(RsxUtil::getUpdateFileNames((int)j->first)) + _T("..."));
				UpdateManager::getInstance()->restoreOld((int)j->first);
			}
			fixControls();
			break;
		}
		case IDC_UPDATE:  {
			reload = false;
			prepareFiles();
			if(!updateItems.size()){
				updateStatus(_T("Failed, You didn't choose any files."));
				return 0;
			} else {
				if(MessageBox(_T("This will replace your current profile settings, do you wish to proceed?."), _T("Warning!"), MB_YESNO | MB_ICONWARNING) != IDYES) {
					return 0;
				}
			}
			updateStatus(_T("Downloading ") + Util::toStringW(updateItems.size()) + _T(" files..."));
			UpdateManager::getInstance()->updateFiles(updateItems);
			break;
		}
		case IDC_CLIENT_ACTIVE: {
			fixControls();
			break;
		}
		case IDC_MYINFO_ACTIVE: {
			fixControls();
			break;
		}
		case IDC_ISP_ACTIVE:  {
			fixControls();
			break;
		}
		default: break;
	}
	return 0;
}
// Listeners
void UpdateDialog::on(UpdateManagerListener::Complete, int file) throw() {
	switch(file) {
		case 0: 
			updateStatus(reload ? _T("Complete reloading Client Profiles") : _T("Complete updating Client Profiles"));
			break;
		case 1:
			updateStatus(reload ? _T("Complete reloading MyINFO Profiles") : _T("Complete updating MyINFO Profiles"));
			break;
		case 2: 
			updateStatus(reload ? _T("Complete reloading IP Watch List") : _T("Complete updating IP Watch List"));
			break;
		default: break;
	}
	setProgress();
}

void UpdateDialog::on(UpdateManagerListener::Failed, int file, const string& reason) throw() {
	setProgress();
	updateStatus(_T("Failed on ") + Text::toT(RsxUtil::getUpdateFileNames(file) + "\r\n" + reason));
}

void UpdateDialog::on(HttpConnectionListener::Complete, HttpConnection* /*conn*/, const string& /*aLine*/) throw() {
	updateStatus(TSTRING(DATA_RETRIEVED) + _T("!"));
	if(!xmlData.empty()) {
		versionXML();
	}
}

void UpdateDialog::on(HttpConnectionListener::Failed, HttpConnection* /*conn*/, const string& aLine) throw() {
	updateStatus(TSTRING(CONNECTION_ERROR) + _T("!\r\n") + Text::toT(aLine));
}
// Functions
void UpdateDialog::versionXML() {
	string xHistory, xLatestVer, cpVer, mpVer;

	try {
		SimpleXML xml;
		xml.fromXML(xmlData);
		if(xml.findChild("DCUpdate")) {
			xml.stepIn();
			if(xml.findChild("Version")) {
				set_text(IDC_UPDATE_VERSION_LATEST, xml.getChildData());
			}

			xml.resetCurrentChild();
			if(xml.findChild("SVN")) {
				xSVN = Util::toInt(xml.getChildData());
			}

			xml.resetCurrentChild();
			bool resize = false;
			if(xml.findChild("URL")) {
				downloadUrl = xml.getChildData();
				if(xSVN > SVN_REVISION && !downloadUrl.empty()) {
					set_visible(IDC_UPDATE_DOWNLOAD, true);
					resize = true;
				}
			}

			if(resize) {
				CEdit tmpEdit;
				tmpEdit.Attach(GetDlgItem(IDC_UPDATE_HISTORY_TEXT));
				CRect rcEdit, rc;
				tmpEdit.GetWindowRect(rcEdit);
				tmpEdit.MoveWindow(23, 101, rcEdit.Width(), rcEdit.Height()-25, true);
				tmpEdit.Detach();
			}

			xml.resetCurrentChild();
			while(xml.findChild("Message")) {
				xHistory += xml.getChildData();					
			}
			set_text(IDC_UPDATE_HISTORY_TEXT, xHistory);

			xml.resetCurrentChild();
			if(xml.findChild("VersionProfile")) {
				set_text(IDC_UPDATE_VERSION_LATEST_PROFIL, xml.getChildData());
				if((Util::toDouble(xml.getChildData()) > Util::toDouble(ClientProfileManager::getInstance()->getProfileVersion()))) {
					updateStatus(TSTRING(NEW_VERSION_PROFILE) + _T(" (Client Profiles)"));
					setCheck(IDC_CLIENT_ACTIVE, true);
					fixControls();

				}
			}
			xml.resetCurrentChild();
			if(xml.findChild("VersionProfileMyinfo")) {
				set_text(IDC_UPDATE_VERSION_LATEST_PROFIL_MYINFO, xml.getChildData());
				if((Util::toDouble(xml.getChildData()) > Util::toDouble(ClientProfileManager::getInstance()->getMyinfoProfileVersion()))) {
					updateStatus(TSTRING(NEW_VERSION_PROFILE) + _T(" (MyINFO Profiles)"));
					setCheck(IDC_MYINFO_ACTIVE, true);
					fixControls();
				}
			}
			xml.resetCurrentChild();
			if(xml.findChild("IpWatchVersion")) {
				set_text(IDC_UPDATE_VERSION_LATEST_IPW, xml.getChildData());
				if(Util::toDouble(xml.getChildData()) > Util::toDouble(IpManager::getInstance()->getIpWatchVersion())) {
					updateStatus(_T("New ipw ver"));
					setCheck(IDC_ISP_ACTIVE, true);
					fixControls();
				}
			}
			xml.resetCurrentChild();
			xml.stepOut();
		}
	} catch(const SimpleXMLException& e) {
		updateStatus(_T("Error: Couldn't parse xml-data!\r\n") + Text::toT(e.getError()));
	}
}

void UpdateDialog::updateStatus(const tstring& text) {
	int len = GetDlgItem(IDC_STATUS).GetWindowTextLength() + 1;
	AutoArray<TCHAR> buf(len);
	GetDlgItemText(IDC_STATUS, buf, len);

	tstring statText;
	statText += buf;
	if(len > 1) //ignore first entry
		statText += _T("\r\n");
	statText += text.c_str();

	cStatus.SetWindowText(statText.c_str());
	cStatus.PostMessage(EM_SCROLL, SB_BOTTOM, 0);
}

void UpdateDialog::initClose() {
	UpdateManager::getInstance()->removeListener(this);
	updateItems.clear();
	cStatus.Detach();
	cChangeLog.Detach();
	cProgress.Detach();
	cClientCheck.Detach();
	cMyINFOCheck.Detach();
	cISPCheck.Detach();

	TCHAR buf[512];
	GetDlgItemText(IDC_CLIENT_ADDRESS, buf, 512);
	RSXSettingsManager::getInstance()->set(RSXSettingsManager::UPDATE_URL, Text::fromT(buf));
	GetDlgItemText(IDC_MYINFO_ADDRESS, buf, 512);
	RSXSettingsManager::getInstance()->set(RSXSettingsManager::UPDATE_MYINFOS, Text::fromT(buf));
	GetDlgItemText(IDC_ISP_ADDRESS, buf, 512);
	RSXSettingsManager::getInstance()->set(RSXSettingsManager::UPDATE_IPWATCH_URL, Text::fromT(buf));
}

void UpdateDialog::prepareFiles() {
	updateItems.clear();
	TCHAR buf[512];
	if(is_checked(IDC_CLIENT_ACTIVE)) {
		GetDlgItemText(IDC_CLIENT_ADDRESS, buf, 512);
		updateItems.insert(make_pair((int8_t)UpdateManager::CLIENT, Text::fromT(buf)));
	}
	if(is_checked(IDC_MYINFO_ACTIVE)) {
		GetDlgItemText(IDC_MYINFO_ADDRESS, buf, 512);
		updateItems.insert(make_pair((int8_t)UpdateManager::MYINFO, Text::fromT(buf)));
	}
	if(is_checked(IDC_ISP_ACTIVE)) {
		GetDlgItemText(IDC_ISP_ADDRESS, buf, 512);
		updateItems.insert(make_pair((int8_t)UpdateManager::IPWATCH, Text::fromT(buf)));
	}
	cProgress.SetRange(0, updateItems.size());
	cProgress.SetPos(0);
}

void UpdateDialog::fixControls() {
	BOOL clientActive = is_checked(IDC_CLIENT_ACTIVE);
	BOOL myinfoActive = is_checked(IDC_MYINFO_ACTIVE);
	BOOL ispActive = is_checked(IDC_ISP_ACTIVE);

	::EnableWindow(GetDlgItem(IDC_LOAD_BACKUP), 
		(clientActive && Util::fileExists(Util::getConfigPath() + "Profiles.xml.old")) || 
		(myinfoActive && Util::fileExists(Util::getConfigPath() + "MyinfoProfiles.xml.old")) || 
		(ispActive && Util::fileExists(Util::getConfigPath() + "IPWatch.xml.old")));

	::EnableWindow(GetDlgItem(IDC_UPDATE),		clientActive || myinfoActive || ispActive);
	::EnableWindow(GetDlgItem(IDC_PROGRESS),	clientActive || myinfoActive || ispActive);

	::EnableWindow(GetDlgItem(IDC_CLIENT_ADDRESS),	clientActive);
	::EnableWindow(GetDlgItem(IDC_MYINFO_ADDRESS),	myinfoActive);
	::EnableWindow(GetDlgItem(IDC_ISP_ADDRESS),		ispActive);
}

void UpdateDialog::setProgress() {
	prog++;
	cProgress.SetPos(prog);
	if(prog >= updateItems.size()){
		setCheck(IDC_CLIENT_ACTIVE, false);
		setCheck(IDC_MYINFO_ACTIVE, false);
		setCheck(IDC_ISP_ACTIVE, false);
		updateItems.clear();
		prog = 0;
		fixControls();
	}
}