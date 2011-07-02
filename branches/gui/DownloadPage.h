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

#ifndef _DOWNLOADPAGE_H
#define _DOWNLOADPAGE_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>

#include "SettingsPage.h"

class DownloadPage : public SettingsPage
{
private:
	DECLARE_EVENT_TABLE()

	static Item items[];
	static TextItem texts[];
	
	wxStaticText* defDirLabel;
	wxTextCtrl* defDirEdit;
	wxButton* defDirButton;
	wxStaticText* tempDirLabel;
	wxTextCtrl* tempDirEdit;
	wxButton* tempDirButton;
	wxSpinCtrl* maxSimFilesSpin;
	wxStaticText* maxSimFilesLabel;
	wxSpinCtrl* maxSimDownSpin;
	wxStaticText* maxSimDownLabel;
	wxSpinCtrl* highExtraSlotSpin;
	wxStaticText* highExtraSlotLabel;
	wxSpinCtrl* noNewIfExceedSpin;
	wxStaticText* noNewIfExceedLabel;
	wxStaticText* limitNoteLabel;
	wxStaticText* pubHubUrlLabel;
	wxButton* confPubHubButton;
	wxStaticText* hublistProxyLabel;
	wxTextCtrl* hublistProxyEdit;
	
	void onDirBrowse(wxCommandEvent &WXUNUSED(event));
	void onTempDirBrowse(wxCommandEvent &WXUNUSED(event));
	void onConfigureLists(wxCommandEvent &WXUNUSED(event));

protected:

public:
	DownloadPage(wxWindow* parent, SettingsManager *s);
	~DownloadPage();

	void write();
};

#endif // _DOWNLOADPAGE_H
