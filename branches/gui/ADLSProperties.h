/* 
 * Copyright (C) 2010 adrian_007 adrian-007 on o2 point pl
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

#ifndef _ADL_SEARCH_PROPERTIES_H_
#define _ADL_SEARCH_PROPERTIES_H_

namespace dcpp {
	class ADLSearch;
};

class ADLSProperties : public wxDialog {
public:
	ADLSProperties(ADLSearch* search, wxWindow* parent,
			const wxString& title = CTSTRING(ADLS_PROPERTIES),
			const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1),
			long style = wxDEFAULT_DIALOG_STYLE);

private:
	DECLARE_EVENT_TABLE()

	void onOk(wxCommandEvent& event);

	ADLSearch* search;

	wxTextCtrl* ctrlSearchString;
	wxStaticText* ctrlSearchStringLabel;

	wxTextCtrl* ctrlMinSize;
	wxStaticText* ctrlMinSizeLabel;

	wxTextCtrl* ctrlMaxSize;
	wxStaticText* ctrlMaxSizeLabel;

	wxComboBox* ctrlSizeType;
	wxStaticText* ctrlSizeTypeLabel;

	wxComboBox* ctrlSearchType;
	wxStaticText* ctrlSearchTypeLabel;

	wxTextCtrl* ctrlDestinationDir;
	wxStaticText* ctrlDestinationDirLabel;

	wxCheckBox* ctrlEnabled;
	wxCheckBox* ctrlDownloadMatches;
	wxCheckBox* ctrlForbidden;

	wxComboBox* ctrlRaw;
	wxStaticText* ctrlRawLabel;

	wxButton* btnOK;
	wxButton* btnCancel;

};

#endif
