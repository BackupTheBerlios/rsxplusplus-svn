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

#ifndef FAVHUBPROPERTIES_H
#define FAVHUBPROPERTIES_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/arrstr.h>

#include "../client/HubEntry.h"

class FavHubProperties : public wxDialog 
{
	DECLARE_EVENT_TABLE()

	protected:
		wxStaticText* hubNameLabel;
		wxTextCtrl* hubNameEdit;
		wxStaticText* hubAddrLabel;
		wxTextCtrl* hubAddrEdit;
		wxStaticText* hubDescLabel;
		wxTextCtrl* hubDescEdit;
		wxStaticText* idNickLabel;
		wxTextCtrl* idNickEdit;
		wxStaticText* idPassLabel;
		wxTextCtrl* idPassEdit;
		wxStaticText* idDescLabel;
		wxTextCtrl* idDescEdit;
		wxCheckBox* idEmulDcCheck;
		wxTextCtrl* raw1Edit;
		wxStaticText* raw1Label;
		wxTextCtrl* raw2Edit;
		wxStaticText* raw2Label;
		wxTextCtrl* raw3Edit;
		wxStaticText* raw3Label;
		wxTextCtrl* raw4Edit;
		wxStaticText* raw4Label;
		wxTextCtrl* raw5Edit;
		wxStaticText* raw5Label;
		wxRadioButton* connDefRadio;
		wxRadioButton* connDirRadio;
		
		wxStaticText* connIpLabel;
		wxTextCtrl* connIpEdit;
		
		wxRadioButton* connFwRadio;
		wxStaticText* minSearchLabel;
		wxSpinCtrl* minSearchSpin;
		wxStaticText* groupLabel;
		wxComboBox* groupCombo;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;

		wxComboBox* encodingCombo;

		FavoriteHubEntry *_entry;
		
		//----- Events -----
		void onOk(wxCommandEvent& event);
		void onTextUpdated(wxCommandEvent& event);
	
	public:
		FavHubProperties(wxWindow* parent, FavoriteHubEntry *entry,
			const wxString& title = CTSTRING(FAVORITE_HUB_PROPERTIES),
			const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1),
			long style = wxDEFAULT_DIALOG_STYLE);
		~FavHubProperties();
};

#endif //FAVHUBPROPERTIES_H
