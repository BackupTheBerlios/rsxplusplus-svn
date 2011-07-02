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

#ifndef _PREVIEWPAGE_H
#define _PREVIEWPAGE_H

#include <wx/intl.h>

#include <wx/listctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/panel.h>

#include "../client/SettingsManager.h"
#include "../Client/FavoriteManager.h"
#include "SettingsPage.h"
#include "PreviewDlg.h"

class PreviewPage : public SettingsPage
{
private:
	DECLARE_EVENT_TABLE()
	
	static TextItem texts[];
	
	void addEntry(PreviewApplication* pa, long pos);
	void doAdd();
	void doChange();
	void doRemove();
	
	wxListCtrl* previewList;
	wxButton* previewAddButton;
	wxButton* previewChangeButton;
	wxButton* previewRemButton;
	
	void onAdd(wxCommandEvent &WXUNUSED(event));
	void onChange(wxCommandEvent &WXUNUSED(event));
	void onRemove(wxCommandEvent &WXUNUSED(event));
	void onItemActivated(wxListEvent &WXUNUSED(event));
	void onItemSelected(wxListEvent &WXUNUSED(event));
	void onItemDeselected(wxListEvent &WXUNUSED(event));
	void onListKeyDown(wxListEvent& event);

protected:
		
public:
	PreviewPage(wxWindow* parent, SettingsManager *s);
	~PreviewPage();
	
	void write();
};

#endif // _PREVIEWPAGE_H
