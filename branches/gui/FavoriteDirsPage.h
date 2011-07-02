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

#ifndef _FAVORITEDIRSPAGE_H
#define _FAVORITEDIRSPAGE_H

#include <wx/intl.h>

#include <wx/listctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/panel.h>

#include "SettingsPage.h"

class FavoriteDirsPage : public SettingsPage
{
private:
	DECLARE_EVENT_TABLE()
	
	static TextItem texts[];

	void doAdd();
	void doRename(long item);
	void doRemove();
	
	wxListCtrl* favDirsList;

	wxButton* favDirRenButton;
	wxButton* favDirRemButton;
	wxButton* favDirAddButton;
	
	void onItemActivated(wxListEvent& event);
	void onItemSelected(wxListEvent &WXUNUSED(event));
	void onItemDeselected(wxListEvent &WXUNUSED(event));
	void onListKeyDown(wxListEvent& event);
	void onRename(wxCommandEvent &WXUNUSED(event));
	void onRemove(wxCommandEvent &WXUNUSED(event));
	void onAdd(wxCommandEvent &WXUNUSED(event));

protected:

public:
	FavoriteDirsPage(wxWindow* parent, SettingsManager *s);
	~FavoriteDirsPage();

	void write();
};

#endif // _FAVORITEDIRSPAGE_H
