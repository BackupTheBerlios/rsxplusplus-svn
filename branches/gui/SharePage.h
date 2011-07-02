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

#ifndef _SHAREPAGE_H
#define _SHAREPAGE_H

#include "SettingsPage.h"

#include "../client/FastAlloc.h"

#include <wx/intl.h>

#include <wx/dir.h>
#include <wx/string.h>
#include <wx/checklst.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/button.h>
#include <wx/colour.h>
#include <wx/treectrl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/panel.h>

class SharePage : public SettingsPage
{
private:
	
	struct TreeData : public wxTreeItemData, public FastAlloc<TreeData>
	{
		TreeData(const wxString& _dir) : dir(_dir) { }
		wxString dir;
	};

	static TextItem texts[];
	static Item items[];
	
	wxTreeCtrl*		dirTree;
	wxStaticText*	onlyHashLabel;
	wxStaticText*	totalSizeLabel;
	wxStaticText*	shareSizeLabel;
	wxCheckBox*		shareHiddenCheck;

	wxStaticText*	refreshLabel;
	wxSpinCtrl*		refreshSpin;
	wxStaticText*	maxHashLabel;
	wxStaticText*	maxHashSpeedLabel;
	wxSpinCtrl*		maxHashSpin;

	void fillDirTree();
	void fillDirectory(const wxTreeItemId& parent, const wxString& name);

	void checkChildren(const wxTreeItemId& item, bool check);
	void boldParents(const wxTreeItemId& item);

	bool hasSharedChildren(const string& path);
	bool hasSharedParent(const wxTreeItemId& item);
	void shareParent(const wxTreeItemId& item);

public:
	SharePage(wxWindow* parent, SettingsManager *s);
	~SharePage();
	
	void write();
	
	DECLARE_EVENT_TABLE()
		
	void OnToggleDirectory(wxTreeEvent& event);
	void OnDirExpanding(wxTreeEvent& event);
	void onClickedShareHidden(wxCommandEvent& WXUNUSED(event));

};

#endif // _SHAREPAGE_H
