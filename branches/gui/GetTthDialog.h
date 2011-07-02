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

#ifndef _GETTHDIALOG_H
#define _GETTHDIALOG_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/hyperlink.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/clipbrd.h>

class GetTTHDialog : public wxDialog 
{
private:

	DECLARE_EVENT_TABLE()

	enum
	{
		IDD_GETTTH,
		IDC_GETTTH_FILE_EDIT,
		IDC_GETTTH_TTH_LINK,
		IDC_GETTTH_TTH_EDIT,
		IDC_GETTTH_MAGNET_LINK,
		IDC_GETTTH_MAGNET_EDIT,
	};

protected:
		wxStaticText* fileNameLabel;
		wxTextCtrl* fileNameEdit;
		wxStaticText* tthLabel;
		wxHyperlinkCtrl* tthLink;
		wxTextCtrl* tthEdit;
		wxStaticText* magnetLabel;
		wxHyperlinkCtrl* magnetLink;
		wxTextCtrl* magnetEdit;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;

		//----- Events -----
		virtual void onTTHLink(wxHyperlinkEvent &WXUNUSED(event));
		virtual void onMagnetLink(wxHyperlinkEvent &WXUNUSED(event));
	
public:

		GetTTHDialog(wxWindow* parent, wxWindowID id = IDD_GETTTH, const wxString& title = _("Tiger Tree Hash"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
		~GetTTHDialog();
		void fillValues(const tstring& filename, const tstring& tth, const tstring& magnet);
	
};

#endif // _GETTHDIALOG_H
