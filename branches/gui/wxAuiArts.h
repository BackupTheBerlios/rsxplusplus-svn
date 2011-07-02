/*
 * Copyright (C) 2006 Benjamin I. Williams - Kirix Corporation, 
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

#ifndef _WXAUIARTS_H
#define _WXAUIARTS_H

#include "MDIChildFrame.h"

#include <wx/aui/aui.h>

/**
 * Provider for drawing customized tabs.
 * wxAuiDefaultTabArt doesn't have virtual functions so it's not possible to derive from it.
 */
class wxAuiCustomTabArt : public wxAuiTabArt
{

public:
	wxAuiCustomTabArt() { }
    virtual ~wxAuiCustomTabArt() { }

    wxAuiTabArt* Clone() { return new wxAuiCustomTabArt(*this); }
    
	void SetFlags(unsigned int flags) { defaultArt.SetFlags(flags); }
    
	void SetSizingInfo(const wxSize& tab_ctrl_size,
                       size_t tab_count)
	{ 
		defaultArt.SetSizingInfo(tab_ctrl_size, tab_count); 
	}

    void SetNormalFont(const wxFont& font) { defaultArt.SetNormalFont(font); }
    void SetSelectedFont(const wxFont& font) { defaultArt.SetSelectedFont(font); }
    void SetMeasuringFont(const wxFont& font) { defaultArt.SetMeasuringFont(font); }

    void DrawBackground(
                 wxDC& dc,
                 wxWindow* wnd,
                 const wxRect& rect)
	{ 
		defaultArt.DrawBackground(dc, wnd, rect); 
	}

    void DrawTab(wxDC& dc,
                 wxWindow* wnd,
                 const wxAuiNotebookPage& pane,
                 const wxRect& in_rect,
                 int close_button_state,
                 wxRect* out_tab_rect,
                 wxRect* out_button_rect,
                 int* x_extent)
	{ 
		if(((MDIChildFrame*)pane.window)->isDirty())
			defaultArt.SetNormalFont((*wxNORMAL_FONT).Bold());

		defaultArt.DrawTab(dc, wnd, pane, in_rect, close_button_state, out_tab_rect, out_button_rect, x_extent); 
		defaultArt.SetNormalFont(*wxNORMAL_FONT);
	}

    void DrawButton(
                 wxDC& dc,
                 wxWindow* wnd,
                 const wxRect& in_rect,
                 int bitmap_id,
                 int button_state,
                 int orientation,
                 wxRect* out_rect)
	{ 
		defaultArt.DrawButton(dc, wnd, in_rect, bitmap_id, button_state, orientation, out_rect); 
	}

    int GetIndentSize() { return defaultArt.GetIndentSize(); }

    wxSize GetTabSize(
                 wxDC& dc,
                 wxWindow* wnd,
                 const wxString& caption,
                 const wxBitmap& bitmap,
                 bool active,
                 int close_button_state,
                 int* x_extent)
	{ 
		return defaultArt.GetTabSize(dc, wnd, caption, bitmap, active, close_button_state, x_extent); 
	}

    int ShowDropDown(
                 wxWindow* wnd,
                 const wxAuiNotebookPageArray& items,
                 int active_idx)
	{ 
		return defaultArt.ShowDropDown(wnd, items, active_idx); 
	}

    int GetBestTabCtrlSize(wxWindow* wnd,
                 const wxAuiNotebookPageArray& pages,
                 const wxSize& required_bmp_size)
	{ 
		return defaultArt.GetBestTabCtrlSize(wnd, pages, required_bmp_size); 
	}

private:

	wxAuiDefaultTabArt	defaultArt;
};

#endif