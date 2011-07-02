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

#ifndef _MDICHILDFRAME_H
#define _MDICHILDFRAME_H

#include "AsyncHandler.h"
#include "WinUtil.h"

#include <wx/aui/aui.h>
#include <wx/wx.h>

class MDIChildFrame :
	public wxAuiMDIChildFrame,
	public AsyncHandler<MDIChildFrame>
{

public:

	struct MDIFreezer
	{
		MDIFreezer();
		~MDIFreezer();
	};

	bool isDirty() const { return dirty; }
		
protected:
	
	MDIChildFrame(wxWindowID id, const wxString& title, const wxColour& colour = wxColour(0, 0, 0), 
		const wxString& iconResource = wxT(""), const wxString& stateIconResource = wxT(""));
	~MDIChildFrame(void);

	void setDirty();
	void setTabColor(const wxColour& color);
	void setIconState();
	void unsetIconState();

	void Show();

	virtual void UpdateLayout() = 0;

private:

	DECLARE_EVENT_TABLE();

	// events
	void OnActivate(wxActivateEvent& event);
	void OnSize(wxSizeEvent& event);

	// attributes
	bool created;
	bool dirty;
	wxBitmap normalBitmap, stateBitmap;

	// fix for no status bars in wxAuiMDIChildFrames
    wxStatusBar* CreateStatusBar(int number = 1, long style = 1, wxWindowID winid = 1, const wxString& name = wxEmptyString)
	{
		statusBar = new wxStatusBar(this, winid, style, name);
		statusBar->SetFieldsCount(number);
		return statusBar;
	}

    wxStatusBar *GetStatusBar() const { return statusBar; }
    void SetStatusText( const wxString &text, int number=0 ) { statusBar->SetStatusText(text, number); }
    void SetStatusWidths( int n, const int widths_field[] ) { statusBar->SetStatusWidths(n, widths_field); }
		
	wxStatusBar* statusBar;
};

#endif	// _MDICHILDFRAME_H