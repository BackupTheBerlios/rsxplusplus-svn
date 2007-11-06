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

#if !defined(ADLS_PROPERTIES_H)
#define ADLS_PROPERTIES_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/RawManager.h"

class ADLSearch;

///////////////////////////////////////////////////////////////////////////////
//
//	Dialog for new/edit ADL searches
//
///////////////////////////////////////////////////////////////////////////////
class ADLSProperties : public CDialogImpl<ADLSProperties>, protected RawSelector
{
public:

	// Constructor/destructor
	ADLSProperties::ADLSProperties(ADLSearch *_search) : search(_search) { }
	~ADLSProperties() { }

	// Dilaog unique id
	enum { IDD = IDD_ADLS_PROPERTIES };
	enum { MATCH, MISMATCH, INVALID };

	// Inline message map
	BEGIN_MSG_MAP(ADLSProperties)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_IS_FORBIDDEN, onControlChecked)
		COMMAND_ID_HANDLER(IDC_IS_REGEXP, onControlChecked)
		COMMAND_ID_HANDLER(IDC_REGEXP_TESTER_BUTTON, onMatch)
		COMMAND_ID_HANDLER(IDC_OVERRIDE_FAV, onControlChecked)
		COMMAND_ID_HANDLER(IDC_OVERRIDE_CUSTOM, onControlChecked)
		COMMAND_ID_HANDLER(IDC_OVER_RIDE_POINTS, onControlChecked)
	END_MSG_MAP()
	
	// Message handlers
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onControlChecked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:

	// Current search
	ADLSearch* search;

	void setControlState();
	void setControlOverride();
	CComboBox cRaw;
};

#endif // !defined(ADLS_PROPERTIES_H)

/**
 * @file
 * $Id: ADLSProperties.h 217 2006-07-04 11:05:30Z bigmuscle $
 */
