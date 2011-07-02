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

#ifndef _PREVIEWDLG_H
#define _PREVIEWDLG_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

class PreviewDlg : public wxDialog 
{
private:
	DECLARE_EVENT_TABLE()
	
protected:		
	wxStaticText* nameLabel;
	wxTextCtrl* nameEdit;
	wxStaticText* appLabel;
	wxTextCtrl* appEdit;
	wxButton* appButton;
	wxStaticText* argLabel;
	wxTextCtrl* argEdit;
	wxStaticText* extLabel;
	wxTextCtrl* extEdit;
	wxStdDialogButtonSizer* m_sdbSizer1;
	wxButton* m_sdbSizer1OK;
	wxButton* m_sdbSizer1Cancel;

	void onBrowse(wxCommandEvent &WXUNUSED(event));

public:
	PreviewDlg(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
	~PreviewDlg();

	void getValues(wxString *name, wxString *app, wxString *arg, wxString *ext);
	void setValues(const wxString name, const wxString app, const wxString arg, const wxString ext);
	
};

#endif // _PREVIEWDLG_H
