/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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

#include "../client/DetectionManager.h"
#include "../client/rsxppSettingsManager.h"
#include "../client/version.h"
#include "../windows/WinUtil.h"
#include "../rsx/IpManager.h"
#include "../rsx/UpdateManager.h"
#include "../rsx/HTTPDownloadManager.h"
#include "../rsx/RsxUtil.h"

//some shortcuts
#define set_text(idc, val) ::SetWindowText(GetDlgItem(idc), Text::toT(val).c_str())
#define is_checked(idc) IsDlgButtonChecked(idc) == BST_CHECKED
#define set_visible(idc, val) ::ShowWindow(GetDlgItem(idc), val)

UpdateDialog::UpdateDialog() : m_hIcon(NULL), reload(false), prog(0) {
	UpdateManager::getInstance()->getCacheInfo(clientInfo, profilesInfo);
}

UpdateDialog::~UpdateDialog() {
	if(m_hIcon) {
		DeleteObject((HGDIOBJ)m_hIcon);
		m_hIcon = NULL;
	}
}

LRESULT UpdateDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	cStatus.Attach(GetDlgItem(IDC_STATUS));
	cChangeLog.Attach(GetDlgItem(IDC_UPDATE_HISTORY_TEXT));
	cProgress.Attach(GetDlgItem(IDC_PROGRESS));
	cClientCheck.Attach(GetDlgItem(IDC_CLIENT_ACTIVE));
	cMyINFOCheck.Attach(GetDlgItem(IDC_MYINFO_ACTIVE));
	cISPCheck.Attach(GetDlgItem(IDC_ISP_ACTIVE));

	set_text(IDC_UPDATE_VERSION_CURRENT_PROFIL, DetectionManager::getInstance()->getProfileVersion());
	set_text(IDC_UPDATE_VERSION_CURRENT_PROFIL_MYINFO, DetectionManager::getInstance()->getUserInfoVersion());
	set_text(IDC_UPDATE_VERSION_CURRENT_IPW, IpManager::getInstance()->getIpWatchVersion());
	set_text(IDC_CLIENT_ADDRESS, RSXPP_SETTING(UPDATE_URL));
	set_text(IDC_MYINFO_ADDRESS, RSXPP_SETTING(UPDATE_MYINFOS));
	set_text(IDC_ISP_ADDRESS, RSXPP_SETTING(UPDATE_IPWATCH_URL));
	set_text(IDCLOSE, STRING(CLOSE));
	set_text(IDC_UPDATE_VERSION_CURRENT, VERSIONSTRING);

	updateStatus(_T("Using cache data"));
	UpdateManager::getInstance()->addListener(this);

	SetWindowText(CTSTRING(UPDATE_CHECK));
	m_hIcon = ::LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDR_UPDATE));
	SetIcon(m_hIcon, FALSE);
	SetIcon(m_hIcon, TRUE);
	CenterWindow(GetParent());
	updateInfo();
	return 0;
}

void UpdateDialog::updateInfo() {
	set_text(IDC_UPDATE_VERSION_LATEST, clientInfo.version);
	set_text(IDC_UPDATE_VERSION_LATEST_PROFIL, profilesInfo.clientProfile.first);
	set_text(IDC_UPDATE_VERSION_LATEST_PROFIL_MYINFO, profilesInfo.myInfoProfile.first);
	set_text(IDC_UPDATE_VERSION_LATEST_IPW, profilesInfo.ipWatchProfile.first);

	if(clientInfo.svnbuild > SVN_REVISION && !clientInfo.url.empty()) {
		::EnableWindow(GetDlgItem(IDC_UPDATE_DOWNLOAD), true);
	}

	set_text(IDC_UPDATE_HISTORY_TEXT, clientInfo.message);

	string cpWndText = "\r\n\r\n** Profiles Info **";
	cpWndText += "\r\n Profile Name: " + profilesInfo.profileName;
	cpWndText += "\r\n Profile URL: " + profilesInfo.profileUrl;
	cpWndText += "\r\n\r\n** Client Profile Info";
	cpWndText += "\r\n Version: " + profilesInfo.clientProfile.first;
	cpWndText += "\r\n Comment: " + profilesInfo.clientProfile.second;
	cpWndText += "\r\n\r\n** User Info Profile Info";
	cpWndText += "\r\n Version: " + profilesInfo.myInfoProfile.first;
	cpWndText += "\r\n Comment: " + profilesInfo.myInfoProfile.second;
	cpWndText += "\r\n\r\n** IP Watch List Info";
	cpWndText += "\r\n Version: " + profilesInfo.ipWatchProfile.first;
	cpWndText += "\r\n Comment: " + profilesInfo.ipWatchProfile.second;
	updateStatus(Text::toT(cpWndText), true);

	if(isNewClientProfiles()) {
		updateStatus(TSTRING(NEW_VERSION_PROFILE) + _T(" (Client Profiles)"));
		setCheck(IDC_CLIENT_ACTIVE, true);
		fixControls();
	}
	if(isNewMyInfoProfiles()) {
		updateStatus(TSTRING(NEW_VERSION_PROFILE) + _T(" (User Info Profiles)"));
		setCheck(IDC_MYINFO_ACTIVE, true);
		fixControls();
	}
	if(isNewIpWatchProfiles()) {
		updateStatus(TSTRING(NEW_VERSION_PROFILE) + _T(" (IP Watch List)"));
		setCheck(IDC_ISP_ACTIVE, true);
		fixControls();
	}
}

bool UpdateDialog::isNewClientVersion(bool checkSetting) {
	if(clientInfo.svnbuild > SVN_REVISION) {
		if(checkSetting)
			return !BOOLSETTING(DONT_ANNOUNCE_NEW_VERSIONS);
		return true;
	}
	return false;
}

bool UpdateDialog::isNewClientProfiles(bool checkSetting) {
	if((Util::toDouble(profilesInfo.clientProfile.first) > Util::toDouble(DetectionManager::getInstance()->getProfileVersion()))) {
		if(checkSetting)
			return RSXPP_BOOLSETTING(SHOW_CLIENT_NEW_VER);
		return true;
	}
	return false;
}

bool UpdateDialog::isNewMyInfoProfiles(bool checkSetting) {
	if((Util::toDouble(profilesInfo.myInfoProfile.first) > Util::toDouble(DetectionManager::getInstance()->getUserInfoVersion()))) {
		if(checkSetting)
			return RSXPP_BOOLSETTING(SHOW_MYINFO_NEW_VER);
		return true;
	}
	return false;
}

bool UpdateDialog::isNewIpWatchProfiles(bool checkSetting) {
	if((Util::toDouble(profilesInfo.ipWatchProfile.first) > Util::toDouble(IpManager::getInstance()->getIpWatchVersion()))) {
		if(checkSetting)
			return RSXPP_BOOLSETTING(SHOW_IPWATCH_NEW_VER);
		return true;
	}
	return false;
}


LRESULT UpdateDialog::OnButton(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch(wID) {
		case IDC_UPDATE_DOWNLOAD: 
			if(!clientInfo.url.empty())
				WinUtil::openLink(Text::toT(clientInfo.url).c_str());
			break;
		case IDC_LOAD_BACKUP:  {
			reload = true;
			prepareFiles();
			if(!updateItems.size()){
				updateStatus(_T("Failed, You didn't choose any files."));
				return 0;
			} else {
				if(MessageBox(_T("This will replace your current profile settings, do you wish to proceed?"), _T("Warning!"), MB_YESNO | MB_ICONWARNING) != IDYES) {
					return 0;
				}
			}
			cProgress.SetRange(0, updateItems.size());
			cProgress.SetPos(0);
			int curPos = 0;
			for(UpdateMap::const_iterator j = updateItems.begin(); j != updateItems.end(); ++j) {
				string fname = Util::getPath(Util::PATH_USER_CONFIG) + RsxUtil::getUpdateFileNames((int)j->first);
				try {
					File::renameFile(fname + ".old", fname + ".tmp");
					if(Util::fileExists(fname))
						File::renameFile(fname, fname + ".old");
					File::renameFile(fname + ".tmp", fname);
					updateStatus(_T("Loaded old file - ") + Text::toT(RsxUtil::getUpdateFileNames((int)j->first)));
				} catch(...) {
					cProgress.SetPos(++curPos);
					updateStatus(_T("Failed on load old file - ") + Text::toT(RsxUtil::getUpdateFileNames((int)j->first)));
					continue;
				}

				if(j->first == 0) {
					DetectionManager::getInstance()->reload(false);
				} else if(j->first == 1) {
					DetectionManager::getInstance()->reload(true);
				} else if(j->first == 2) {
					IpManager::getInstance()->reloadIpWatch();
				}
				cProgress.SetPos(++curPos);
			}
			updateItems.clear();
			setCheck(IDC_CLIENT_ACTIVE, false);
			setCheck(IDC_MYINFO_ACTIVE, false);
			setCheck(IDC_ISP_ACTIVE, false);
			fixControls();
			break;
		}
		case IDC_UPDATE: {
			reload = false;
			prepareFiles();
			if(!updateItems.size()){
				updateStatus(_T("Failed, You didn't choose any files."));
				return 0;
			} else {
				if(MessageBox(_T("This will replace your current profile settings, do you wish to proceed?"), _T("Warning!"), MB_YESNO | MB_ICONWARNING) != IDYES) {
					return 0;
				}
			}
			updateStatus(_T("Downloading ") + Util::toStringW(updateItems.size()) + _T(" files..."));
			for(UpdateMap::const_iterator j = updateItems.begin(); j != updateItems.end(); ++j) {
				if(j->first == 0) {
					HTTPDownloadManager::getInstance()->addRequest(boost::bind(&UpdateDialog::onProfileDownload, this, _1, _2), j->second);
				} else if(j->first == 1) {
					HTTPDownloadManager::getInstance()->addRequest(boost::bind(&UpdateDialog::onMyInfoDownload, this, _1, _2), j->second);
				} else if(j->first == 2) {
					HTTPDownloadManager::getInstance()->addRequest(boost::bind(&UpdateDialog::onIpWatchDownload, this, _1, _2), j->second);
				}
			}
			break;
		}
		default: fixControls();
	}
	return 0;
}

void UpdateDialog::saveFile(const string& data, const string& fileName) {
	string fname = Util::getPath(Util::PATH_USER_CONFIG) + fileName;
	File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
	f.write(data);
	f.close();
	File::copyFile(fname + ".tmp", fname + ".new");

	try {
		File::deleteFile(fname + ".old");
		File::renameFile(fname, fname + ".old");
		File::renameFile(fname + ".tmp", fname);
	} catch(...) {
		File::renameFile(fname + ".tmp", fname);
	}
	File::deleteFile(fname + ".new");
}

void UpdateDialog::on(UpdateManagerListener::VersionUpdated, const VersionInfo::Client& client, const VersionInfo::Profiles& profiles) throw() {
	updateStatus(_T("Cache Data updated!"));

	::SetWindowText(GetDlgItem(IDC_UPDATE_HISTORY_TEXT), _T(""));
	clientInfo = client;
	profilesInfo = profiles;
	updateInfo();
}

void UpdateDialog::onProfileDownload(string content, bool isFailed) {
	if(!isFailed) {
		saveFile(content, "Profiles.xml");
		DetectionManager::getInstance()->reloadFromHttp(false, false);
		updateStatus(_T("Complete updating Client Profiles"));
	} else {
		updateStatus(_T("Failed on Client Profiles update!"));
	}
	setProgress();
}

void UpdateDialog::onMyInfoDownload(string content, bool isFailed) {
	if(!isFailed) {
		saveFile(content, "UserInfoProfiles.xml");
		DetectionManager::getInstance()->reloadFromHttp(false, true);
		updateStatus(_T("Complete updating User Info Profiles"));
	} else {
		updateStatus(_T("Failed on User Info Profiles update!"));
	}
	setProgress();
}

void UpdateDialog::onIpWatchDownload(string content, bool isFailed) {
	if(!isFailed) {
		saveFile(content, "IPWatch.xml");
		updateStatus(_T("Complete updating IP Watch List"));
	} else {
		updateStatus(_T("Failed on IP Watch update!"));
	}
	setProgress();
}

void UpdateDialog::updateStatus(const tstring& text, bool history/* = false*/) {
	int wnd = history ? IDC_UPDATE_HISTORY_TEXT:IDC_STATUS;

	int len = GetDlgItem(wnd).GetWindowTextLength() + 1;
	tstring buf;
	buf.resize(len);

	GetDlgItemText(wnd, &buf[0], len);

	tstring statText;
	statText += &buf[0];
	if(len > 1) //ignore first entry
		statText += _T("\r\n");
	statText += text.c_str();

	GetDlgItem(wnd).SetWindowText(statText.c_str());
	if(!history)
		GetDlgItem(wnd).PostMessage(EM_SCROLL, SB_BOTTOM, 0);
}

LRESULT UpdateDialog::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	UpdateManager::getInstance()->removeListener(this);
	updateItems.clear();
	cStatus.Detach();
	cChangeLog.Detach();
	cProgress.Detach();
	cClientCheck.Detach();
	cMyINFOCheck.Detach();
	cISPCheck.Detach();

	tstring buf;
	int len = ::GetWindowTextLength(GetDlgItem(IDC_CLIENT_ADDRESS)) + 1;

	buf.resize(len);
	GetDlgItemText(IDC_CLIENT_ADDRESS, &buf[0], len);
	rsxppSettingsManager::getInstance()->set(rsxppSettingsManager::UPDATE_URL, Text::fromT(buf));

	len = ::GetWindowTextLength(GetDlgItem(IDC_MYINFO_ADDRESS)) + 1;
	buf.resize(len);
	GetDlgItemText(IDC_MYINFO_ADDRESS, &buf[0], len);
	rsxppSettingsManager::getInstance()->set(rsxppSettingsManager::UPDATE_MYINFOS, Text::fromT(buf));

	len = ::GetWindowTextLength(GetDlgItem(IDC_ISP_ADDRESS)) + 1;
	buf.resize(len);
	GetDlgItemText(IDC_ISP_ADDRESS, &buf[0], len);
	rsxppSettingsManager::getInstance()->set(rsxppSettingsManager::UPDATE_IPWATCH_URL, Text::fromT(buf));

	EndDialog(NULL);
	return 0;
}

void UpdateDialog::prepareFiles() {
	updateItems.clear();
	tstring buf;
	int len = 0;

	if(is_checked(IDC_CLIENT_ACTIVE)) {
		len = ::GetWindowTextLength(GetDlgItem(IDC_CLIENT_ADDRESS)) + 1;
		buf.resize(len);
		GetDlgItemText(IDC_CLIENT_ADDRESS, &buf[0], len);
		if(buf.rfind(_T("/")) == tstring::npos && buf.rfind(_T("\\")) == tstring::npos)
			buf += _T("/");
		updateItems.insert(make_pair((int8_t)0, (Text::fromT(&buf[0]) + "Profiles.xml")));
	}
	if(is_checked(IDC_MYINFO_ACTIVE)) {
		len = ::GetWindowTextLength(GetDlgItem(IDC_CLIENT_ADDRESS)) + 1;
		buf.resize(len);
		GetDlgItemText(IDC_MYINFO_ADDRESS, &buf[0], len);
		if(buf.rfind(_T("/")) == tstring::npos && buf.rfind(_T("\\")) == tstring::npos)
			buf += _T("/");	
		updateItems.insert(make_pair((int8_t)1, (Text::fromT(&buf[0]) + "UserInfoProfiles.xml")));
	}
	if(is_checked(IDC_ISP_ACTIVE)) {
		len = ::GetWindowTextLength(GetDlgItem(IDC_CLIENT_ADDRESS)) + 1;
		buf.resize(len);
		GetDlgItemText(IDC_ISP_ADDRESS, &buf[0], len);
		if(buf.rfind(_T("/")) == tstring::npos && buf.rfind(_T("\\")) == tstring::npos)
			buf += _T("/");
		updateItems.insert(make_pair((int8_t)2, (Text::fromT(&buf[0]) + "IPWatch.xml")));
	}
	prog = 0;
	cProgress.SetRange(0, updateItems.size());
	cProgress.SetPos(0);
}

void UpdateDialog::fixControls() {
	bool isRunning = updateItems.size() > 0;
	BOOL clientActive = is_checked(IDC_CLIENT_ACTIVE);
	BOOL myinfoActive = is_checked(IDC_MYINFO_ACTIVE);
	BOOL ispActive = is_checked(IDC_ISP_ACTIVE);

	::EnableWindow(GetDlgItem(IDC_LOAD_BACKUP), 
		(clientActive && Util::fileExists(Util::getPath(Util::PATH_USER_CONFIG) + "Profiles.xml.old")) || 
		(myinfoActive && Util::fileExists(Util::getPath(Util::PATH_USER_CONFIG) + "UserInfoProfiles.xml.old")) || 
		(ispActive && Util::fileExists(Util::getPath(Util::PATH_USER_CONFIG) + "IPWatch.xml.old")));

	::EnableWindow(GetDlgItem(IDC_UPDATE),		(clientActive || myinfoActive || ispActive) && !isRunning);
	::EnableWindow(GetDlgItem(IDC_PROGRESS),	(clientActive || myinfoActive || ispActive) && !isRunning);

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
	}
	fixControls();
}