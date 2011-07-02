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

#ifndef _TASKBARICON_H
#define _TASKBARICON_H

#include <wx/taskbar.h>

class MainWindow;

class TaskBarIcon :
	public wxTaskBarIcon
{
public:
	TaskBarIcon(MainWindow* _mainWindow);
	~TaskBarIcon(void);

	void setToolTip(const wxString& tip);

	void setNormalIcon()	{ SetIcon(normalIcon); }
	void setPMIcon()		{ SetIcon(pmIcon); }

private:

	enum
	{
		ID_SHOW = 1,
		ID_OPEN_DOWNLOADS,
		ID_REFRESH_FILE_LIST,
		ID_ABOUT,
		ID_EXIT,
	};

	DECLARE_EVENT_TABLE()

	void OnShow(wxCommandEvent& /*event*/);
	void OnOpenDownloads(wxCommandEvent& /*event*/);
	void OnRefreshFileList(wxCommandEvent& /*event*/);
	void OnAbout(wxCommandEvent& /*event*/);
	void OnExit(wxCommandEvent& /*event*/);

	void OnLeftUp(wxTaskBarIconEvent& event);

	virtual wxMenu *CreatePopupMenu();

	wxIcon				normalIcon;
	wxIcon				pmIcon;

	MainWindow*			mainWindow;
		
};

#endif // _TASKBARICON_H