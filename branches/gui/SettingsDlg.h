/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
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

#ifndef _SETTINGSDLG_H
#define _SETTINGSDLG_H

#include "../client/SettingsManager.h"

#include <wx/propdlg.h>

enum SettingsPages
{
	SP_GENERAL,
	SP_NETWORK,
	SP_PROXY,
	SP_LIMITS,
	SP_DOWNLOAD,
	SP_FAVDIR,
	SP_PREVIEW,
	SP_QUEUE,
	SP_SHARE,
	SP_UPLOAD,
	SP_APPEARANCE,
	SP_TEXT_STYLES,
	//SP_PROGRESS_COLORS,
	SP_USERLIST_COLORS,
	SP_POPUPS,
	SP_SOUNDS,
	//SP_TOOLBAR,
	SP_WINDOWS,
	SP_ADVANCED,
	SP_EXPERTS,
	SP_LOGS,
	//SP_USERCOMMANDS,
	SP_FAKEDETECT,
	//SP_CLIENTS,
	SP_SECURITY
};

class SettingsDlg :
	public wxPropertySheetDialog
{
public:
	SettingsDlg(wxWindow* parent, SettingsManager* s);
	~SettingsDlg(void);

private:
	
	DECLARE_EVENT_TABLE();

	void OnOK(wxCommandEvent& event);

};

#endif	// _SETTINGSDLG_H