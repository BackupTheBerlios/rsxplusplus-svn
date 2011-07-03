/*
* Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
* Copyright (C) 2010-2011 adrian_007, adrian-007 on o2 point pl

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
#include "WinUtil.h"
#include "DebugFrame.h"

BEGIN_EVENT_TABLE(DebugFrame, wxWindow)
	EVT_SIZE(DebugFrame::OnSize)
	EVT_BUTTON(ID_CDM_CLEAR_BUTTON, DebugFrame::OnClear)
END_EVENT_TABLE()

DebugFrame::DebugFrame(wxWindow* parent) : wxWindow(parent, ID_CMD_CDMDEBUG_WINDOW)
{
	textCtrl = new wxTextCtrl(this, ID_DEBUG_TEXTBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxHSCROLL | /*wxTE_AUTO_SCROLL |*/ wxTE_MULTILINE | wxTE_AUTO_URL);
	textCtrl->SetFont(WinUtil::font);
	textCtrl->SetBackgroundColour(WinUtil::bgColor);
	textCtrl->SetForegroundColour(WinUtil::textColor);
	textCtrl->SetEditable(false);

	statusBar = new wxStatusBar(this, 1, wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE, "");
	statusBar->SetFieldsCount(6);

	ctrlClear = new wxButton(statusBar, ID_CDM_CLEAR_BUTTON, _("Clear"), wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);

	ctrlShowHubCommands = new wxCheckBox(statusBar, ID_CDM_HUB_COMMANDS, _("Hub commands"), wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlShowHubCommands->SetValue(false);

	ctrlShowClientCommands = new wxCheckBox(statusBar, ID_CDM_CLIENT_COMMANDS, _("Client commands"), wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlShowClientCommands->SetValue(false);

	ctrlShowDetectionCommands = new wxCheckBox(statusBar, ID_CDM_DETECTION, _("Detection"), wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlShowDetectionCommands->SetValue(false);

	ctrlEnableFilter = new wxCheckBox(statusBar, wxID_ANY, _("Filter"), wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlEnableFilter->SetValue(false);

	ctrlFilter = new wxTextCtrl(statusBar, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlFilter->SetFont(WinUtil::font);

	int width;
	int i = 0;

	int statusSizes[6];
	memset(statusSizes, 0, sizeof(statusSizes));

	ctrlClear->GetSize(&width, 0);
	statusSizes[i++] = width + 10;

	ctrlShowHubCommands->GetSize(&width, 0);
	statusSizes[i++] = width + 10;

	ctrlShowClientCommands->GetSize(&width, 0);
	statusSizes[i++] = width + 10;

	ctrlShowDetectionCommands->GetSize(&width, 0);
	statusSizes[i++] = width + 10;

	ctrlEnableFilter->GetSize(&width, 0);
	statusSizes[i++] = width + 10;

	ctrlFilter->GetSize(&width, 0);
	statusSizes[i++] = width + 10;

	statusBar->SetStatusWidths(i, statusSizes);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(textCtrl, 1, wxEXPAND);
	sizer->Add(statusBar, 0, wxEXPAND);

	SetSizerAndFit(sizer);

	DebugManager::getInstance()->addListener(this);
}

DebugFrame::~DebugFrame(void)
{
	DebugManager::getInstance()->removeListener(this);
}

void DebugFrame::OnClear(wxCommandEvent& event) 
{
	callAsync(std::bind(&wxTextCtrl::Clear, textCtrl));
	event.Skip();
}

void DebugFrame::OnSize(wxSizeEvent& event)
{
	setControlInStatusBar(0, ctrlClear);
	setControlInStatusBar(1, ctrlShowHubCommands, 5);
	setControlInStatusBar(2, ctrlShowClientCommands, 5);
	setControlInStatusBar(3, ctrlShowDetectionCommands, 5);
	setControlInStatusBar(4, ctrlEnableFilter, 5);
	setControlInStatusBar(5, ctrlFilter);

	event.Skip();
}

void DebugFrame::setControlInStatusBar(int pos, wxWindow* wnd, int offset /*= 0*/) {
	wxRect rc;
	statusBar->GetFieldRect(pos, rc);
	wnd->SetPosition(wxPoint(rc.x + 2 + offset, rc.y + 2));
	wnd->SetSize(rc.width - 2*(2 + offset), rc.height - 4);
}

void DebugFrame::on(DebugManagerListener::DebugDetection, const string& aLine) throw() {
	if(ctrlShowDetectionCommands->IsChecked()) 
	{
		callAsync(std::bind(&DebugFrame::addCommand, this, aLine));
	}
}

void DebugFrame::on(DebugManagerListener::DebugCommand, const string& aLine, int typeDir, const string& ip) throw() {
	if(checkFilter(ip) == false)
	{
		return;
	}

	switch(typeDir) 
	{
	case DebugManager::HUB_IN:
		{
			if(ctrlShowHubCommands->IsChecked()) 
			{
				callAsync(std::bind(&DebugFrame::addCommand, this, std::string("Hub:\t[Incoming][" + ip + "]\t \t" + aLine + "\r\n")));
			}
			break;
		}
	case DebugManager::HUB_OUT:
		{
			if(ctrlShowHubCommands->IsChecked())
			{
				callAsync(std::bind(&DebugFrame::addCommand, this, std::string("Hub:\t[Outgoing][" + ip + "]\t \t" + aLine + "\r\n")));
			}
			break;
		}
	case DebugManager::CLIENT_IN:
		{
			if(ctrlShowClientCommands->IsChecked()) 
			{
				callAsync(std::bind(&DebugFrame::addCommand, this, std::string("Client:\t[Incoming][" + ip + "]\t \t" + aLine + "\r\n")));
			}
			break;
		}
	case DebugManager::CLIENT_OUT:
		{
			if(ctrlShowClientCommands->IsChecked()) 
			{
				callAsync(std::bind(&DebugFrame::addCommand, this, std::string("Client:\t[Outgoing][" + ip + "]\t \t" + aLine + "\r\n")));
			}
			break;
		}
	}
}

void DebugFrame::addCommand(const std::string& line) 
{
	textCtrl->Freeze();
	//int scrollPos = textCtrl->GetScrollPos(wxVERTICAL);
	//int scrollMax = textCtrl->GetScrollRange(wxVERTICAL);

	long selStart, selEnd;
	textCtrl->GetSelection(&selStart, &selEnd);
	textCtrl->AppendText(wxString(line.c_str(), wxConvUTF8));
	textCtrl->SetSelection(selStart, selEnd);

	//FIXME ...
	/*if((scrollPos + 10) < scrollMax)
	{
		textCtrl->SetScrollPos(wxVERTICAL, scrollPos);
	} 
	else
	{
		textCtrl->SetScrollPos(wxVERTICAL, -1);	
	}*/
	textCtrl->Thaw();
}

bool DebugFrame::checkFilter(const std::string& ip) 
{
	if(ctrlEnableFilter->IsChecked() && ctrlFilter->IsEmpty() == false && ip.find(ctrlFilter->GetValue()) == std::string::npos) 
	{
		const wxString& f = ctrlFilter->GetValue();
		return ip.find(f) != std::string::npos;
	}
	return true;
}
