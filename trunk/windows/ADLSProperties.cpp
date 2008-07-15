/* 
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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
#include "Resource.h"

#include "ADLSProperties.h"
#include "../client/ADLSearch.h"
#include "../client/FavoriteManager.h"
#include "WinUtil.h"

#include "../rsx/RegexUtil.h"

// Initialize dialog
LRESULT ADLSProperties::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	// Translate the texts
	SetWindowText(CTSTRING(ADLS_PROPERTIES));
	SetDlgItemText(IDC_ADLSP_SEARCH,		CTSTRING(ADLS_SEARCH_STRING));
	SetDlgItemText(IDC_ADLSP_TYPE,			CTSTRING(ADLS_TYPE));
	SetDlgItemText(IDC_ADLSP_SIZE_MIN,		CTSTRING(ADLS_SIZE_MIN));
	SetDlgItemText(IDC_ADLSP_SIZE_MAX,		CTSTRING(ADLS_SIZE_MAX));
	SetDlgItemText(IDC_ADLSP_UNITS,			CTSTRING(ADLS_UNITS));
	SetDlgItemText(IDC_ADLSP_DESTINATION,	CTSTRING(ADLS_DESTINATION));
	SetDlgItemText(IDC_IS_ACTIVE,			CTSTRING(ADLS_ENABLED));
	SetDlgItemText(IDC_AUTOQUEUE,			CTSTRING(ADLS_DOWNLOAD));
	SetDlgItemText(IDC_IS_FORBIDDEN,		CTSTRING(FORBIDDEN));
	SetDlgItemText(IDC_ADLSEARCH_COMMENT,	CTSTRING(COMMENT));

	CUpDownCtrl spin;
	spin.Attach(GetDlgItem(IDC_ADLS_POINTS_SPIN));
	spin.SetRange32(0, 1024*1024);
	spin.Detach();

	cRaw.attach(GetDlgItem(IDC_ADLSEARCH_RAW_ACTION), search->adlsRaw);

	// Initialize combo boxes
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SourceTypeToDisplayString(ADLSearch::OnlyFile).c_str());
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SourceTypeToDisplayString(ADLSearch::OnlyDirectory).c_str());
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SourceTypeToDisplayString(ADLSearch::FullPath).c_str());
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SourceTypeToDisplayString(ADLSearch::TTHFile).c_str());

	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToDisplayString(ADLSearch::SizeBytes).c_str());
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToDisplayString(ADLSearch::SizeKiloBytes).c_str());
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToDisplayString(ADLSearch::SizeMegaBytes).c_str());
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToDisplayString(ADLSearch::SizeGigaBytes).c_str());

	// Load search data
	char* buf = new char[32];
	SetDlgItemText(IDC_SEARCH_STRING,		Text::toT(search->searchString).c_str());
	SetDlgItemText(IDC_DEST_DIR,			Text::toT(search->destDir).c_str());
	SetDlgItemText(IDC_MIN_FILE_SIZE,		Text::toT(search->minFileSize > 0 ? _i64toa(search->minFileSize, buf, 10) : "").c_str());
	SetDlgItemText(IDC_MAX_FILE_SIZE,		Text::toT(search->maxFileSize > 0 ? _i64toa(search->maxFileSize, buf, 10) : "").c_str());
	SetDlgItemText(IDC_ADLS_KICK_STRING,	Text::toT(search->kickString).c_str());
	SetDlgItemInt(IDC_ADLS_POINTS,			(search->adlsPoints > 0 ? search->adlsPoints : 0));
	SetDlgItemText(IDC_ADLS_COMMENT,		Text::toT(search->adlsComment).c_str());

	::SendMessage(GetDlgItem(IDC_IS_ACTIVE),			BM_SETCHECK,	search->isActive ? 1 : 0, 0L);
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE),			CB_SETCURSEL,	search->sourceType, 0L);
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE),			CB_SETCURSEL,	search->typeFileSize, 0L);
	::SendMessage(GetDlgItem(IDC_AUTOQUEUE),			BM_SETCHECK,	search->isAutoQueue ? 1 : 0, 0L);
	::SendMessage(GetDlgItem(IDC_IS_FORBIDDEN),			BM_SETCHECK,	search->isForbidden ? 1 : 0, 0L);
//	::SendMessage(GetDlgItem(IDC_IS_REGEXP),			BM_SETCHECK,	search->isRegExp ? 1 : 0, 0L);
	::SendMessage(GetDlgItem(IDC_OVER_RIDE_POINTS),		BM_SETCHECK,	search->overRidePoints ? 1 : 0, 0L);
	::SendMessage(GetDlgItem(IDC_IS_CASE_SENSITIVE),	BM_SETCHECK,	search->isCaseSensitive ? 1 : 0, 0L);

	if (search->fromFavs)
		::SendMessage(GetDlgItem(IDC_OVERRIDE_FAV),		BM_SETCHECK, 1, 0L);
	else
		::SendMessage(GetDlgItem(IDC_OVERRIDE_CUSTOM),	BM_SETCHECK, 1, 0L);


	delete[] buf;
	setControlState();
	setControlOverride();

	// Center dialog
	CenterWindow(GetParent());

	return FALSE;
}

// Exit dialog
LRESULT ADLSProperties::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(wID == IDOK)
	{
		// Update search
		TCHAR buf[512];

		GetDlgItemText(IDC_SEARCH_STRING, buf, 256);
		search->searchString = Text::fromT(buf);
		GetDlgItemText(IDC_DEST_DIR, buf, 256);
		search->destDir = Text::fromT(buf);

		GetDlgItemText(IDC_MIN_FILE_SIZE, buf, 256);
		search->minFileSize = (_tcslen(buf) == 0 ? -1 : Util::toInt64(Text::fromT(buf)));
		GetDlgItemText(IDC_MAX_FILE_SIZE, buf, 256);
		search->maxFileSize = (_tcslen(buf) == 0 ? -1 : Util::toInt64(Text::fromT(buf)));

		search->sourceType =		(ADLSearch::SourceType)::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_GETCURSEL, 0, 0L);
		search->typeFileSize =		(ADLSearch::SizeType)::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_GETCURSEL, 0, 0L);
		search->isActive =			(::SendMessage(GetDlgItem(IDC_IS_ACTIVE), BM_GETCHECK, 0, 0L) != 0);
		search->isAutoQueue =		(::SendMessage(GetDlgItem(IDC_AUTOQUEUE), BM_GETCHECK, 0, 0L) != 0);
		search->isForbidden =		(::SendMessage(GetDlgItem(IDC_IS_FORBIDDEN), BM_GETCHECK, 0, 0L) != 0);
//		search->isRegExp =			(::SendMessage(GetDlgItem(IDC_IS_REGEXP), BM_GETCHECK, 0, 0L) != 0);
		search->isCaseSensitive =	(::SendMessage(GetDlgItem(IDC_IS_CASE_SENSITIVE), BM_GETCHECK, 0, 0L) != 0);
		search->overRidePoints =	(::SendMessage(GetDlgItem(IDC_OVER_RIDE_POINTS), BM_GETCHECK, 0, 0L) != 0);
		if(search->isForbidden) {
			switch(search->sourceType) {
				case 1 :	search->destDir = STRING(FORBIDDEN_DIRS); break;
				case 2 :	search->destDir = STRING(FORBIDDEN_FULLPATHS); break;
				case 3 :	search->destDir = STRING(FORBIDDEN_TTHS); break;
				default :	search->destDir = STRING(FORBIDDEN_FILES);
			}
		}
		search->adlsRaw =		cRaw.getActionId();

		GetDlgItemText(IDC_ADLS_POINTS, buf, 32);
		search->adlsPoints =	(_tcslen(buf) == 0 ? -1 : Util::toInt(Text::fromT(buf)));

		GetDlgItemText(IDC_ADLS_COMMENT, buf, 512);
		search->adlsComment =	Text::fromT(buf);

		search->fromFavs =		(::SendMessage(GetDlgItem(IDC_OVERRIDE_FAV), BM_GETCHECK, 0, 0L) != 0);

		GetDlgItemText(IDC_ADLS_KICK_STRING, buf, 512);
		search->kickString =	Text::fromT(buf);
	}

	EndDialog(wID);
	return 0;
}
LRESULT ADLSProperties::onControlChecked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch (wID) {
		case IDC_IS_FORBIDDEN:		setControlState(); break;
		case IDC_IS_REGEXP:			setControlState(); break;
		case IDC_OVERRIDE_FAV:		setControlOverride(); break;
		case IDC_OVERRIDE_CUSTOM:	setControlOverride(); break;
		case IDC_OVER_RIDE_POINTS:	setControlState(); break;
		default:
			break;
	}
	return 0;
}
void ADLSProperties::setControlState() {
	bool b = (IsDlgButtonChecked(IDC_IS_FORBIDDEN) != 0);
	bool c = (IsDlgButtonChecked(IDC_OVER_RIDE_POINTS) != 0);

	::EnableWindow(GetDlgItem(IDC_OVER_RIDE_POINTS), b);
	if(c) {
		::EnableWindow(GetDlgItem(IDC_ADLSEARCH_RAW_ACTION), b);
		::EnableWindow(GetDlgItem(IDC_ADLS_KICK_STRING), b);
		::EnableWindow(GetDlgItem(IDC_OVERRIDE_FAV), b);
		::EnableWindow(GetDlgItem(IDC_OVERRIDE_CUSTOM), b);
		::EnableWindow(GetDlgItem(IDC_ADLSEARCH_ACTION), b);
	} else {
		::EnableWindow(GetDlgItem(IDC_ADLSEARCH_RAW_ACTION),	false);
		::EnableWindow(GetDlgItem(IDC_ADLS_KICK_STRING),		false);
		::EnableWindow(GetDlgItem(IDC_OVERRIDE_FAV),			false);
		::EnableWindow(GetDlgItem(IDC_OVERRIDE_CUSTOM),			false);
		::EnableWindow(GetDlgItem(IDC_ADLSEARCH_ACTION),		false);
	}
	::EnableWindow(GetDlgItem(IDC_ADLS_POINTS), b && !c);
	::EnableWindow(GetDlgItem(IDC_ADLS_POINTS_SPIN), b && !c);
	::EnableWindow(GetDlgItem(IDC_ADL_POINTS), b && !c);

	b = (IsDlgButtonChecked(IDC_IS_REGEXP) != 0);
	::EnableWindow(GetDlgItem(IDC_REGEXP_TESTER_TEXT), b);
	::EnableWindow(GetDlgItem(IDC_REGEXP_TESTER_BUTTON), b);
	::EnableWindow(GetDlgItem(IDC_IS_CASE_SENSITIVE), b);
}

void ADLSProperties::setControlOverride() {
	bool b = (::SendMessage(GetDlgItem(IDC_OVERRIDE_FAV), BM_GETCHECK, 0, 0L) != 0);
	::ShowWindow(GetDlgItem(IDC_ADLS_KICK_STRING), !b);
	::ShowWindow(GetDlgItem(IDC_ADLSEARCH_RAW_ACTION), b);
	if (b)
		SetDlgItemText(IDC_ADLSEARCH_ACTION, CTSTRING(ACTION));
	else
		SetDlgItemText(IDC_ADLSEARCH_ACTION, CTSTRING(RAW));
}

LRESULT ADLSProperties::onMatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	TCHAR buf[512];
	tstring exp, text;
	GetDlgItemText(IDC_SEARCH_STRING, buf, 256);
	exp = buf;
	GetDlgItemText(IDC_REGEXP_TESTER_TEXT, buf, 256);
	text = buf;
	MessageBox(Text::toT(RegexUtil::matchExp(Text::fromT(exp), Text::fromT(text), (::SendMessage(GetDlgItem(IDC_IS_CASE_SENSITIVE), BM_GETCHECK, 0, 0L) != 0))).c_str(), CTSTRING(REGEX_TESTER), MB_OK);

	return 0;
}