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

#ifndef _LIMITSPAGE_H
#define _LIMITSPAGE_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/combobox.h>
#include <wx/panel.h>

#include "SettingsPage.h"

class LimitsPage : public SettingsPage
{
private:
	DECLARE_EVENT_TABLE()
	
	static Item items[];
	static TextItem texts[];
		
	wxCheckBox* segmentedCheck;
	wxStaticText* disconnectLabel;
	wxSpinCtrl* disconnectSpin;
	wxStaticText* kbps1Label;
	wxStaticText* moreLabel;
	wxSpinCtrl* moreSpin;
	wxStaticText* secLabel;
	wxStaticText* fileSpeedLabel;
	wxSpinCtrl* fileSpeedSpin;
	wxStaticText* kbps2Label;
	wxStaticText* fileSizeLabel;
	wxSpinCtrl* fileSizeSpin;
	wxStaticText* mbpsLabel;
	wxStaticText* removeLabel;
	wxSpinCtrl* removeSpin;
	wxStaticText* kbps3Label;
	wxCheckBox* enableLimitCheck;
	wxStaticText* downLabel;
	wxSpinCtrl* downSpin;
	wxStaticText* kbps4Label;
	wxStaticText* upLabel;
	wxSpinCtrl* upSpin;
	wxStaticText* kbps5Label;
	wxCheckBox* altCheck;
	wxComboBox* beginCombo;
	wxStaticText* toLabel;
	wxComboBox* endCombo;
	wxStaticText* downAltLabel;
	wxSpinCtrl* downAltSpin;
	wxStaticText* kbps6Label;
	wxStaticText* upAltLabel;
	wxSpinCtrl* upAltSpin;
	wxStaticText* kbps7Label;
	wxStaticText* upNoteLabel;
	wxStaticText* downNoteLabel;
		
	void onEnableCheck(wxCommandEvent& event);
	void onAltCheck(wxCommandEvent& event);

	void doCheck();
	
protected:
	
public:
	LimitsPage(wxWindow* parent, SettingsManager *s);
	~LimitsPage();
	
	void write();
};

#endif // _LIMITSPAGE_H
