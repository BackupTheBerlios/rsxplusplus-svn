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

#include "stdafx.h"
#include "TaskBarIcon.h"

#include "MainWindow.h"

#include "../client/version.h"

#include <wx/wx.h>

BEGIN_EVENT_TABLE(TaskBarIcon, wxTaskBarIcon)
	EVT_MENU(ID_SHOW, TaskBarIcon::OnShow)
	EVT_MENU(ID_OPEN_DOWNLOADS, TaskBarIcon::OnOpenDownloads)
	EVT_MENU(ID_REFRESH_FILE_LIST, TaskBarIcon::OnRefreshFileList)
	EVT_MENU(ID_ABOUT, TaskBarIcon::OnAbout)
	EVT_MENU(ID_EXIT, TaskBarIcon::OnExit)

	EVT_TASKBAR_LEFT_UP(TaskBarIcon::OnLeftUp)
END_EVENT_TABLE()

TaskBarIcon::TaskBarIcon(MainWindow* _mainWindow) : mainWindow(_mainWindow)
{
	normalIcon = wxIcon(wxT("IDI_APPICON"));
	pmIcon = wxIcon(wxT("IDI_TRAY_PM"));
	
	SetIcon(normalIcon, wxT(APPNAME));
}

TaskBarIcon::~TaskBarIcon(void)
{
}

void TaskBarIcon::setToolTip(const wxString& tip)
{
	SetIcon(wxIcon(), tip);
}

void TaskBarIcon::OnShow(wxCommandEvent& /*event*/)
{
	if(mainWindow->IsIconized())
	{
		mainWindow->Iconize(false);
	}
	else if(!mainWindow->IsShown())
	{
		mainWindow->Restore();
		mainWindow->Show(true);
		mainWindow->Raise();
	}
	else
	{
		mainWindow->Raise();
	}
}

void TaskBarIcon::OnOpenDownloads(wxCommandEvent& /*event*/)
{
}

void TaskBarIcon::OnRefreshFileList(wxCommandEvent& /*event*/)
{
}

void TaskBarIcon::OnAbout(wxCommandEvent& /*event*/)
{
}

void TaskBarIcon::OnExit(wxCommandEvent& /*event*/)
{
	mainWindow->Close();
}

void TaskBarIcon::OnLeftUp(wxTaskBarIconEvent& /*event*/)
{
	// process as OnShow event
	wxCommandEvent evt;
	OnShow(evt);
}

wxMenu* TaskBarIcon::CreatePopupMenu() 
{
	wxMenu* menu = new wxMenu;

	menu->Append(ID_SHOW, CTSTRING(MENU_SHOW));
	menu->Append(ID_OPEN_DOWNLOADS, CTSTRING(MENU_OPEN_DOWNLOADS_DIR));
	menu->AppendSeparator();
	menu->Append(ID_REFRESH_FILE_LIST, CTSTRING(MENU_REFRESH_FILE_LIST));
	menu->AppendSeparator();
	menu->Append(ID_ABOUT, CTSTRING(MENU_ABOUT));
	menu->Append(ID_EXIT, CTSTRING(MENU_EXIT));

	return menu;
}
