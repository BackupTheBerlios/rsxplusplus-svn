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

#ifndef _UPLOADPAGE_H
#define _UPLOADPAGE_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/panel.h>

#include "SettingsPage.h"

class UploadPage : public SettingsPage
{
private:
	static Item items[];
	static TextItem texts[];
	
	wxStaticText* upSlotLabel;
	wxSpinCtrl* upSlotSpin;
	wxStaticText* slotRatioLabel;
	wxSpinCtrl* slotRatioSpin;
	wxStaticText* slotsEmptyLabel;
	wxStaticText* smallSlotLabel;
	wxSpinCtrl* smallSlotSpin;
	wxStaticText* smallFileLabel;
	wxSpinCtrl* smallFileSpin;
	wxStaticText* smallKibLabel;
	wxStaticText* openExtraLabel;
	wxSpinCtrl* openExtraSpin;
	wxStaticText* openKibLabel;
	wxStaticText* maxAutoLabel;
	wxSpinCtrl* maxAutoSpin;
	wxStaticText* slotsEmpty2Label;
	wxStaticText* extraPartLabel;
	wxSpinCtrl* extraPartSpin;
	wxStaticText* slotsNoteLabel;
	
protected:
	
public:
	UploadPage(wxWindow* parent, SettingsManager *s);
	~UploadPage();
	
	void write();
};

#endif // _UPLOADPAGE_H
