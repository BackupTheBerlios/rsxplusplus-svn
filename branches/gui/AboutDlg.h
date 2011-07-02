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

#ifndef _ABOUTDLG_H
#define _ABOUTDLG_H

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
#include <wx/hyperlink.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>

static const tstring thanks = _T("I.nfraR.ed, Kulmegil, Crise, newborn & Aqualung, Morbid, Thor\r\nKeep it coming!");

class AboutDlg : public wxDialog
{
	private:

	protected:
		wxStaticBitmap* logoBitmap;
		wxStaticText* versionLabel;
		wxHyperlinkCtrl* strongLink;
		wxStaticText* tthLabel;
		wxTextCtrl* tthEdit;
		wxTextCtrl* thanksEdit;
		wxStaticText* upDownLabel;
		wxStaticText* ratioLabel;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;

	public:
		AboutDlg(wxWindow* parent, const wxString& title = _("About"),
			const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
		~AboutDlg();
	
};

#endif // _ABOUTDLG_H
