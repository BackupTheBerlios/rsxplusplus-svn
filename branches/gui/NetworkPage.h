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

#ifndef _NETWORKPAGE_H
#define _NETWORKPAGE_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/panel.h>

#include "SettingsPage.h"

class NetworkPage : public SettingsPage
{
private:
	DECLARE_EVENT_TABLE()

	static Item items[];
	static TextItem texts[];

	void doInRadio();
	void getAddresses();
	
	wxStaticText* bindAddrLabel;
	wxComboBox* bindAddrCombo;
	wxCheckBox* autoDetect;
	wxRadioButton* inDirectRadio;
	wxRadioButton* inFwUpnpRadio;
	wxRadioButton* inFwManRadio;
	wxRadioButton* inFwPassRadio;
	wxStaticText* tcpPortLabel;
	wxSpinCtrl* tcpPortSpin;
	wxStaticText* udpPortLabel;
	wxSpinCtrl* udpPortSpin;
	wxStaticText* tlsPortLabel;
	wxSpinCtrl* tlsPortSpin;
	wxStaticText* dhtPortLabel;
	wxSpinCtrl* dhtPortSpin;
	wxStaticText* wanLabel;
	wxTextCtrl* wanIpEdit;
	wxCheckBox* noOverrideCheck;
	wxCheckBox* updIpCheck;
	wxCheckBox* allowNatCheck;
	
	void onInRadio(wxCommandEvent &WXUNUSED(event));

protected:

public:
	NetworkPage(wxWindow* parent, SettingsManager *s);
	~NetworkPage();

	void write();
};

#endif // _NETWORKPAGE_H
