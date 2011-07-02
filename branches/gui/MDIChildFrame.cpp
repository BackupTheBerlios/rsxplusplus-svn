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
#include "MDIChildFrame.h"

#include "MainWindow.h"
#include "WinUtil.h"

#include "../client/SettingsManager.h"

BEGIN_EVENT_TABLE(MDIChildFrame, wxAuiMDIChildFrame)
	EVT_ACTIVATE(MDIChildFrame::OnActivate)
	EVT_SIZE(MDIChildFrame::OnSize)
END_EVENT_TABLE()

MDIChildFrame::MDIFreezer::MDIFreezer() 
{ 
	WinUtil::mainWindow->GetClientWindow()->Freeze();
}

MDIChildFrame::MDIFreezer::~MDIFreezer() 
{ 
	WinUtil::mainWindow->GetClientWindow()->Thaw();
	WinUtil::mainWindow->getAuiManager().Update();
}

MDIChildFrame::MDIChildFrame(wxWindowID id, const wxString& title, const wxColour& colour, 
	const wxString& iconResource, const wxString& stateIconResource) : 
	wxAuiMDIChildFrame((wxAuiMDIParentFrame*)WinUtil::mainWindow, id, title, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxDEFAULT_FRAME_STYLE),	
	created(false), statusBar(NULL), dirty(false)
{
	created = true;

	// set window icon
	normalBitmap = wxBitmap(wxIcon(iconResource, wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));
	stateBitmap = wxBitmap(wxIcon(stateIconResource, wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));
	WinUtil::premultiplyAlpha(normalBitmap);
	WinUtil::premultiplyAlpha(stateBitmap);

	// don't use SetIcon, because it doesn't premultiply alpha channel for icons
	wxAuiMDIClientWindow* client = this->GetMDIParentFrame()->GetClientWindow();
	client->SetPageBitmap(client->GetPageIndex(this), normalBitmap);
	//SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
}

MDIChildFrame::~MDIChildFrame(void)
{
}

void MDIChildFrame::Show()
{	
	// require frame to use at least one main sizer!
	dcassert(GetSizer());

	if(statusBar)
	{
		wxBoxSizer* statusBarSizer = new wxBoxSizer(wxVERTICAL);
		statusBarSizer->Add(GetSizer(), 1, wxEXPAND);
		statusBarSizer->Add(statusBar, 0, wxEXPAND);

		SetSizerAndFit(statusBarSizer, false);
	}

	wxAuiMDIClientWindow* client = this->GetMDIParentFrame()->GetClientWindow();
	wxRect r = client->GetRect();
	GetSizer()->SetMinSize(wxSize(r.width, r.height - client->GetTabCtrlHeight()));
	GetSizer()->Fit(this);

	wxAuiMDIChildFrame::Show(true);
}

void MDIChildFrame::OnActivate(wxActivateEvent& event)
{
	dirty = false;

	Refresh();
	event.Skip();
}

void MDIChildFrame::OnSize(wxSizeEvent& event)
{
	UpdateLayout();
	event.Skip();
}

void MDIChildFrame::setDirty() 
{
	dirty = true;
	
	wxAuiMDIClientWindow* client = this->GetMDIParentFrame()->GetClientWindow();
	wxWindow* wnd = client->GetPage(client->GetPageIndex(this));

	// this is wrong, we should update page header only
	client->Refresh();
}

void MDIChildFrame::setTabColor(const wxColour& color) 
{
	//getTab()->setColor(this, color);
}

void MDIChildFrame::setIconState() 
{
	wxAuiMDIClientWindow* client = this->GetMDIParentFrame()->GetClientWindow();
	client->SetPageBitmap(client->GetPageIndex(this), stateBitmap);
}

void MDIChildFrame::unsetIconState() 
{
	wxAuiMDIClientWindow* client = this->GetMDIParentFrame()->GetClientWindow();
	client->SetPageBitmap(client->GetPageIndex(this), normalBitmap);
}
