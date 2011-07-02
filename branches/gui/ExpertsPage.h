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

#ifndef _EXPERTSPAGE_H
#define _EXPERTSPAGE_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/panel.h>

#include "SettingsPage.h"

class ExpertsPage : public SettingsPage
{
private:
	static Item items[];
	static TextItem texts[];
		
	wxStaticText* webserverLabel;
	wxStaticLine* line1;
	wxStaticText* portLabel;
	wxSpinCtrl* portSpin;
	wxStaticText* userLabel;
	wxTextCtrl* userEdit;
	wxStaticText* passwordLabel;
	wxTextCtrl* passwordEdit;
	wxStaticLine* line2;
	wxStaticText* shutdownLabel;
	wxComboBox* shutdownCombo;
	wxStaticText* timeoutLabel;
	wxSpinCtrl* timeoutSpin;
	wxStaticText* sLabel;
	wxStaticLine* line3;
	wxStaticText* readBufLabel;
	wxSpinCtrl* readBufSpin;
	wxStaticText* bLabel;
	wxStaticText* bufSizeLabel;
	wxSpinCtrl* bufSizeSpin;
	wxStaticText* kibLabel;
	wxStaticText* writeBufLabel;
	wxSpinCtrl* writeBufSpin;
	wxStaticText* b2Label;
	wxStaticLine* line4;
	wxStaticText* searchHistLabel;
	wxSpinCtrl* searchHistSpin;
	wxStaticText* pmHistLabel;
	wxSpinCtrl* pmHistSpin;
	wxStaticText* compressionLabel;
	wxSpinCtrl* compressionSpin;
	wxStaticText* attemptsLabel;
	wxSpinCtrl* attemptsSpin;
	wxStaticText* intervalLabel;
	wxSpinCtrl* intervalSpin;
	wxStaticText* s2Label;
	wxStaticText* matchLabel;
	wxSpinCtrl* matchSpin;
	wxStaticText* emptyLabel;
	wxStaticText* autosearchLabel;
	wxSpinCtrl* autosearchSpin;
	wxStaticLine* line5;
	wxStaticText* transActionLabel;
	wxComboBox* transActionCombo;
	wxStaticText* chatActionLabel;
	wxComboBox* chatActionCombo;
	wxStaticText* listActionLabel;
	wxComboBox* listActionCombo;
	
protected:
	
public:
	ExpertsPage(wxWindow* parent, SettingsManager *s);
	~ExpertsPage();
	
	void write();
};

#endif // _EXPERTSPAGE_H
