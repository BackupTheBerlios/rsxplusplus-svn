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

#ifndef _WINDOWSPAGE_H
#define _WINDOWSPAGE_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/checklst.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>

#include "SettingsPage.h"

class WindowsPage : public SettingsPage
{
private:
	static TextItem textItem[];
	static Item items[];
	static ListItem listItems[];
	static ListItem optionItems[];

	wxCheckListBox* autoOpenList;
	wxCheckListBox* optionsList;
	wxStaticText* positionLabel;
	wxComboBox* positionCombo;
	wxStaticText* rowsLabel;
	wxSpinCtrl* rowsSpin;
	
protected:
	
public:
	WindowsPage(wxWindow* parent, SettingsManager *s);
	~WindowsPage();
	
	void write();
};

#endif // _WINDOWSPAGE_H
