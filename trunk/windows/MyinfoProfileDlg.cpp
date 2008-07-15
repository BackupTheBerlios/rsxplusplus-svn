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

#include "MyinfoProfileDlg.h"
#include "../rsx/RegexUtil.h"
#include "../client/ClientProfileManager.h"
#include "../rsx/rsx-settings/rsx-SettingsManager.h"

#define GET_TEXT(id, var) \
	GetDlgItemText(id, buf, 1024); \
	var = buf;

#define ATTACH(id, var) var.Attach(GetDlgItem(id))

LRESULT MyinfoProfileDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(currentProfileId != -1) {
		adding = false;
		getProfile();
	} else {
		adding = true;
		::EnableWindow(GetDlgItem(IDC_NEXT), false);
		::EnableWindow(GetDlgItem(IDC_BACK), false);
	}

	ATTACH(IDC_NAME, ctrlName);
	ATTACH(IDC_VERSION, ctrlVersion);
	ATTACH(IDC_TAG, ctrlTag);
	ATTACH(IDC_EXTENDED_TAG, ctrlExtendedTag);
	ATTACH(IDC_STATUS, ctrlStatus);
	ATTACH(IDC_NICK, ctrlNick);
	ATTACH(IDC_SHARED, ctrlShared);
	ATTACH(IDC_EMAIL, ctrlEmail);
	ATTACH(IDC_CHEATING_DESCRIPTION, ctrlCheatingDescription);
	ATTACH(IDC_MYINFO_CONNECTION, ctrlConnection);
	ATTACH(IDC_USE_EXTRA_VERSION, ctrlUseExtraVersion);
	ATTACH(IDC_ADD_LINE, ctrlAddLine);
	ATTACH(IDC_COMMENT, ctrlComment);
	ATTACH(IDC_MP_ID, ctrlID);

	createList();

	ATTACH(IDC_PROFILE_RAW, ctrlRaw);
	for(ActionList::const_iterator i = idAction.begin(); i != idAction.end(); ++i) {
		ctrlRaw.AddString(RawManager::getInstance()->getNameActionId(i->second).c_str());
	}

	ATTACH(IDC_REGEXP_TESTER_COMBO, ctrlRegExpCombo);
	ctrlRegExpCombo.AddString(CTSTRING(VERSION));
	ctrlRegExpCombo.AddString(CTSTRING(TAG));
	ctrlRegExpCombo.AddString(CTSTRING(DESCRIPTION));
	ctrlRegExpCombo.AddString(CTSTRING(NICK));
	ctrlRegExpCombo.AddString(CTSTRING(SHARED));
	ctrlRegExpCombo.AddString(CTSTRING(EMAIL));
	ctrlRegExpCombo.AddString(CTSTRING(STATUS));
	ctrlRegExpCombo.AddString(CTSTRING(CONNECTION));
	ctrlRegExpCombo.SetCurSel(0);

	params = ClientProfileManager::getInstance()->getParams();

	CUpDownCtrl spin;
	spin.Attach(GetDlgItem(IDC_MP_ID_SPIN));
	spin.SetRange32(0, 1024*1024);
	spin.Detach();

	updateControls();
	CenterWindow(GetParent());
	return FALSE;
}

LRESULT MyinfoProfileDlg::onChange(WORD , WORD , HWND , BOOL& ) {
	updateAddLine();
	return 0;
}

LRESULT MyinfoProfileDlg::onChangeTag(WORD , WORD , HWND , BOOL& ) {
	updateAddLine();
	updateTag();
	return 0;
}

void MyinfoProfileDlg::updateTag() {
	TCHAR buf[BUF_LEN];
	tstring exp;

	GET_TEXT(IDC_TAG, exp);
	exp = Text::toT(RegexUtil::formatRegExp(Text::fromT(exp), params));
	SetDlgItemText(IDC_FORMATTED_TAG, exp.c_str());
}

void MyinfoProfileDlg::updateAddLine() {
	addLine = Util::emptyStringT;
	TCHAR buf[BUF_LEN];

#define UPDATE \
	GetWindowText(buf, BUF_LEN-1); \
	addLine += buf; \
	addLine += _T(';');

	ctrlName.UPDATE;
	ctrlVersion.UPDATE;
	ctrlTag.UPDATE;
	ctrlExtendedTag.UPDATE;
	ctrlNick.UPDATE;
	ctrlShared.UPDATE;
	ctrlEmail.UPDATE;
	ctrlStatus.UPDATE;
	ctrlConnection.UPDATE;
	ctrlCheatingDescription.GetWindowText(buf, BUF_LEN-1);
	addLine += buf;

	ctrlAddLine.SetWindowText(addLine.c_str());
}

void MyinfoProfileDlg::getProfile() {
	ClientProfileManager::getInstance()->getMyinfoProfile(currentProfileId, currentProfile);

	name = Text::toT(currentProfile.getName());
	version = Text::toT(currentProfile.getVersion());
	tag = Text::toT(currentProfile.getTag());
	extendedTag = Text::toT(currentProfile.getExtendedTag());
	nick = Text::toT(currentProfile.getNick());
	shared = Text::toT(currentProfile.getShared());
	email = Text::toT(currentProfile.getEmail());
	status = Text::toT(currentProfile.getStatus());
	cheatingDescription = Text::toT(currentProfile.getCheatingDescription());
	rawToSend = currentProfile.getRawToSend();
	useExtraVersion = currentProfile.getUseExtraVersion();
	connection = Text::toT(currentProfile.getConnection());
	comment = Text::toT(currentProfile.getComment());
	id = currentProfile.getProfileId();
}

void MyinfoProfileDlg::updateVars() {
	TCHAR buf[1024];

	GET_TEXT(IDC_NAME, name);
	GET_TEXT(IDC_VERSION, version);
	GET_TEXT(IDC_TAG, tag);
	GET_TEXT(IDC_EXTENDED_TAG, extendedTag);
	GET_TEXT(IDC_NICK, nick);
	GET_TEXT(IDC_SHARED, shared);
	GET_TEXT(IDC_EMAIL, email);
	GET_TEXT(IDC_STATUS, status);
	GET_TEXT(IDC_CHEATING_DESCRIPTION, cheatingDescription);
	GET_TEXT(IDC_MYINFO_CONNECTION, connection);
	GET_TEXT(IDC_COMMENT, comment);

	useExtraVersion = (ctrlUseExtraVersion.GetCheck() == BST_CHECKED) ? 1 : 0;
	rawToSend = getIdAction(ctrlRaw.GetCurSel());
	id = GetDlgItemInt(IDC_MP_ID);
}

void MyinfoProfileDlg::updateControls() {
	ctrlName.SetWindowText(name.c_str());
	ctrlVersion.SetWindowText(version.c_str());
	ctrlTag.SetWindowText(tag.c_str());
	ctrlExtendedTag.SetWindowText(extendedTag.c_str());
	ctrlNick.SetWindowText(nick.c_str());
	ctrlShared.SetWindowText(shared.c_str());
	ctrlEmail.SetWindowText(email.c_str());
	ctrlStatus.SetWindowText(status.c_str());
	ctrlCheatingDescription.SetWindowText(cheatingDescription.c_str());
	ctrlAddLine.SetWindowText(addLine.c_str());
	ctrlConnection.SetWindowText(connection.c_str());
	ctrlComment.SetWindowText(comment.c_str());
	ctrlUseExtraVersion.SetCheck((useExtraVersion) ? BST_CHECKED : BST_UNCHECKED);

	ctrlRaw.SetCurSel(getId(rawToSend));
	ctrlID.SetWindowText(Util::toStringW(id).c_str());

	ctrlName.SetFocus();
}

LRESULT MyinfoProfileDlg::onNext(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	updateVars();

	currentProfile.setName(Text::fromT(name));
	currentProfile.setVersion(Text::fromT(version));
	currentProfile.setTag(Text::fromT(tag));
	currentProfile.setExtendedTag(Text::fromT(extendedTag));
	currentProfile.setNick(Text::fromT(nick));
	currentProfile.setShared(Text::fromT(shared));
	currentProfile.setEmail(Text::fromT(email));
	currentProfile.setStatus(Text::fromT(status));
	currentProfile.setCheatingDescription(Text::fromT(cheatingDescription));
	currentProfile.setRawToSend(rawToSend);
	currentProfile.setUseExtraVersion(useExtraVersion);
	currentProfile.setConnection(Text::fromT(connection));
	currentProfile.setComment(Text::fromT(comment));
	currentProfile.setProfileId(id);

	ClientProfileManager::getInstance()->updateMyinfoProfile(currentProfile);

	if(wID == IDC_NEXT && currentProfileId < itemCount-1)
		currentProfileId++;
	else if( wID == IDC_BACK && currentProfileId > 0)
		currentProfileId--;

	getProfile();
	updateControls();
	return 0;
}

LRESULT MyinfoProfileDlg::onMatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	TCHAR buf[1024];
	tstring exp, text;
	GET_TEXT(IDC_REGEXP_TESTER_TEXT, text);
	switch(ctrlRegExpCombo.GetCurSel()) {
		case VERSION:	GET_TEXT(IDC_VERSION, exp);	break;
		case TAG: 
			{
				string version, verTagExp, formattedExp;
				tstring versionExp;
				GET_TEXT(IDC_TAG, exp); 
				
				verTagExp = RegexUtil::formatRegExp(Text::fromT(exp), params);
				formattedExp = verTagExp;
				string::size_type j = formattedExp.find("%[version]");
				if(j != string::npos) {
					formattedExp.replace(j, 10, ".*");
					version = RegexUtil::getVersion(verTagExp, Text::fromT(text));
					GET_TEXT(IDC_VERSION, versionExp)
				}
				string temp = RegexUtil::matchExp(formattedExp, Text::fromT(text));
				if(temp.find(STRING(S_MISSMATCH)) != string::npos || temp.find(STRING(S_INVALID)) != string::npos){
					MessageBox(Text::toT(temp).c_str(), CTSTRING(REGEX_TESTER), MB_OK);
					return 0;
				}
				if(version.empty()) {
					MessageBox(CTSTRING(S_MATCH), CTSTRING(REGEX_TESTER), MB_OK);
					return 0;
				} else {
					MessageBox(Text::toT(RegexUtil::matchExp(Text::fromT(versionExp), version)).c_str(), CTSTRING(REGEX_TESTER), MB_OK);
					return 0;
				}
			}
		case DESCRIPTION: 
			{
				GET_TEXT(IDC_EXTENDED_TAG, exp);
				tstring::size_type j = exp.find(_T("%[version2]"));
				if(j != string::npos) {
					exp.replace(j, 11, _T(".*"));
				}
				break;
			}
		case NICK: GET_TEXT(IDC_NICK, exp); break;
		case SHARED: GET_TEXT(IDC_SHARED, exp); break;
		case EMAIL: GET_TEXT(IDC_EMAIL, exp); break;
		case STATUS: GET_TEXT(IDC_STATUS, exp); break;
		case CONNECTION: GET_TEXT(IDC_MYINFO_CONNECTION, exp); break;
		default: dcdebug("We shouldn't be here!\n");
	}
	MessageBox(Text::toT(RegexUtil::matchExp(Text::fromT(exp), Text::fromT(text))).c_str(), CTSTRING(REGEX_TESTER), MB_OK);
	return 0;
}

LRESULT MyinfoProfileDlg::onClip(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	return 0;
}