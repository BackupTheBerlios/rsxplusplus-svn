/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
 * Copyright (C) 2010 Oyashiro-sama, oyashirosama dot hnnkni at gmail dot com
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

#ifndef _MAGNETDLG_H
#define _MAGNETDLG_H

#include <wx/intl.h>

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

#include "../client/QueueManager.h"

class MagnetDlg : public wxDialog 
{
private:
	DECLARE_EVENT_TABLE()

	tstring mHash, mFileName;
	int64_t mSize;

protected:
	wxStaticBitmap* magnetBitmap;
	wxStaticText* detectedLabel;
	wxStaticText* hashLabel;
	wxTextCtrl* hashEdit;
	wxStaticText* nameLabel;
	wxTextCtrl* nameEdit;
	wxStaticText* sizeLabel;
	wxTextCtrl* sizeEdit;
	wxRadioButton* queueRadio;
	wxRadioButton* searchRadio;
	wxRadioButton* nothingRadio;
	wxCheckBox* rememberCheck;
	wxButton* OK;
	wxButton* Cancel;
	wxBoxSizer* bSizer1;

	void onRadioButton(wxCommandEvent& event);
	void onOK(wxCommandEvent &WXUNUSED(event));
	
public:
	MagnetDlg(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = CTSTRING(MAGNET_DLG_TITLE),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
	~MagnetDlg();

	void setValues(const tstring& aHash, const tstring& aFileName, const int64_t aSize);
	
};

#endif // _MAGNETDLG_H
