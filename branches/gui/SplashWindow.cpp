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
#include "SplashWindow.h"
#include "ImageManager.h"

#include <wx/dcbuffer.h>
#include <wx/settings.h>

#include "../client/version.h"

BEGIN_EVENT_TABLE(SplashWindow, wxFrame)
	EVT_PAINT(SplashWindow::OnPaint)
END_EVENT_TABLE()

SplashWindow::SplashWindow() : wxFrame(NULL, 1, wxT(APPNAME) wxT(" ") wxT(VERSIONSTRING))
{
	const wxBitmap& bmp = ImageManager::getInstance()->splash;
	
	SetTransparent(220);
	SetWindowStyle(wxBORDER_NONE | wxCENTRE_ON_SCREEN | wxFRAME_NO_TASKBAR);
	SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	SetSize(bmp.GetWidth(), bmp.GetHeight());
	Centre();

	title = _T(VERSIONSTRING);
	
#ifndef _WIN64	
	switch (get_cpu_type())
	{
		case 2:
			title += _T(" MMX");
			break;
		case 3:
			title += _T(" AMD");
			break;
		case 4:
		case 5:
			title += _T(" SSE");
			break;
	}
#else
	title += _T(" x64");
#endif

	Show();
	SetFocus();
	Update();
}

void SplashWindow::OnPaint(wxPaintEvent& /*event*/)
{
	wxPaintDC dc(this);
	dc.SetBackgroundMode(wxTRANSPARENT);
	
	wxRect rc = this->GetRect();
	rc.Offset(-rc.GetLeft(), -rc.GetTop());

	wxRect rc2 = rc;
	rc2.SetTop(rc2.GetBottom() - 35);
	rc2.SetRight(rc2.GetRight() - 10);

	wxBitmap& bmp = ImageManager::getInstance()->splash;

	wxMemoryDC memDC;
	memDC.SelectObject(bmp);
	dc.Blit(0, 0, bmp.GetWidth(), bmp.GetHeight(), &memDC, 0, 0);

	wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	//font.SetFaceName(wxT("Tahoma"));
	font.SetPixelSize(wxSize(0, 11));
	font.SetWeight(wxFONTWEIGHT_BOLD);

	dc.SetFont(font);
	dc.SetTextForeground(wxColour(255, 255, 255));
	dc.DrawLabel(wxString(title.c_str()), rc2, wxALIGN_RIGHT);

	if(!text.empty()) 
	{
		rc2 = rc;
		rc2.SetTop(rc2.GetBottom() - 15);
		rc2.SetRight(rc2.GetRight() - 10);

		//font.SetPixelSize(wxSize(0, 10));
		font.SetWeight(wxFONTWEIGHT_NORMAL);

		dc.SetFont(font);
		dc.DrawLabel(wxString(text).c_str(), rc2, wxALIGN_RIGHT);
	}
}
