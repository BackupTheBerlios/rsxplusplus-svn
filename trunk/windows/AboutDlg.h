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

#ifndef ABOUT_DLG_H
#define ABOUT_DLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../rsx/UpdateManager.h"
#include <atlctrlx.h>

static const TCHAR thanks[] = _T("I.nfraR.ed, Kulmegil, Crise, newborn & Aqualung, Morbid, Thor\r\nKeep it coming!");

class AboutDlg : public CDialogImpl<AboutDlg>, private TimerManagerListener
{
public:
	enum { IDD = IDD_ABOUTBOX };
	enum { WM_VERSIONDATA = WM_APP + 53 };

	AboutDlg() { }
	~AboutDlg() { }

	BEGIN_MSG_MAP(AboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_ABOUT_LINK, onLink)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		SetDlgItemText(IDC_VERSION, _T("RSX++ v") _T(VERSIONSTRING) _T(" (c) Copyright 2007-2009 adrian_007\nBased on: StrongDC++ 2.31 (c) Copyright 2001-2009 Big Muscle\n"));
		CEdit ctrlThanks(GetDlgItem(IDC_THANKS));
		ctrlThanks.FmtLines(TRUE);
		ctrlThanks.AppendText(thanks, TRUE);
		ctrlThanks.Detach();
		SetDlgItemText(IDC_TTH, WinUtil::tth.c_str());
		SetDlgItemText(IDC_TOTALS, (_T("Upload: ") + Util::formatBytesW(SETTING(TOTAL_UPLOAD)) + _T(", Download: ") + 
			Util::formatBytesW(SETTING(TOTAL_DOWNLOAD))).c_str());

		SetDlgItemText(IDC_ABOUT_LINK, Text::toT(RSXPP_SITE).c_str());
		url.SubclassWindow(GetDlgItem(IDC_ABOUT_LINK));
		url.SetHyperLinkExtendedStyle(HLINK_COMMANDBUTTON|HLINK_UNDERLINEHOVER);
		url.m_tip.AddTool(url, Text::toT(RSXPP_SITE).c_str(), &url.m_rcLink, 1);

		TCHAR buf[128];
		if(SETTING(TOTAL_DOWNLOAD) > 0) {
			snwprintf(buf, sizeof(buf), _T("Ratio (up/down): %.2f"), ((double)SETTING(TOTAL_UPLOAD)) / ((double)SETTING(TOTAL_DOWNLOAD)));
			SetDlgItemText(IDC_RATIO, buf);
		}
		//RSX++
		SetDlgItemText(IDC_COMPILE_TIME, WinUtil::getCompileInfo().c_str());
		snwprintf(buf, sizeof(buf), _T("Uptime: %s"), Text::toT(WinUtil::formatTime(time(NULL) - Util::getStartTime())).c_str());
		SetDlgItemText(IDC_UPTIME, buf);

		TimerManager::getInstance()->addListener(this);
		SetDlgItemText(IDC_LATEST, Text::toT(UpdateManager::getInstance()->getLatestVersion()).c_str());
		//END
		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		TimerManager::getInstance()->removeListener(this);
		EndDialog(wID);
		return 0;
	}

	LRESULT onLink(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		WinUtil::openLink(Text::toT(RSXPP_SITE).c_str());
		return 0;
	}

private:
	CHyperLink url;
	AboutDlg(const AboutDlg&) { }

	void on(TimerManagerListener::Second /*type*/, uint64_t /*aTick*/) throw() {
		TCHAR buf[128];
		snwprintf(buf, sizeof(buf), _T("Uptime: %s"), Text::toT(WinUtil::formatTime(time(NULL) - Util::getStartTime())).c_str());
		SetDlgItemText(IDC_UPTIME, buf);
	}
};

#endif // !defined(ABOUT_DLG_H)

/**
 * @file
 * $Id: AboutDlg.h 385 2008-04-26 13:05:09Z BigMuscle $
 */
