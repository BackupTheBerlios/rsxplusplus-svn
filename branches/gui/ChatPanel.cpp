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
#include "ChatPanel.h"

#include "EmoticonsDlg.h"
#include "WinUtil.h"
#include "ImageManager.h" //RSX++

#include "../client/File.h"
#include "../client/StringTokenizer.h"

#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/sound.h>
#include <wx/wx.h>
#include <wx/bmpbuttn.h>

BEGIN_EVENT_TABLE(ChatPanel, wxSplitterWindow)
	EVT_BUTTON(IDC_EMOT, ChatPanel::OnEmoticons)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(SEND_MESSAGE)

ChatPanel::ChatPanel(wxWindow* parent, long style) : wxSplitterWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style | wxSP_3D | wxSP_LIVE_UPDATE)
{
	ctrlChat = new RichChatCtrl(this, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxRE_MULTILINE | wxRE_READONLY);
	ctrlChat->SetBackgroundColour(WinUtil::bgColor);	
	ctrlChat->GetCaret()->Hide(); // don't show caret in readonly chat

	wxPanel* messagePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* msgSizer = new wxBoxSizer(wxHORIZONTAL);

	ctrlMessage = new wxTextCtrl(messagePanel, ID_MESSAGEBOX, wxEmptyString, wxDefaultPosition, wxSize(0, 0), wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxTE_MULTILINE | wxTE_PROCESS_TAB);
	ctrlMessage->SetFont(WinUtil::font);
	ctrlMessage->SetBackgroundColour(WinUtil::bgColor);
	ctrlMessage->SetForegroundColour(WinUtil::textColor);
	ctrlMessage->SetMaxLength(4096); // FIXME: some good limit
	
	// connect some event handlers
	ctrlMessage->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(ChatPanel::OnMessageChar), NULL, this);
	ctrlMessage->Connect(wxEVT_KEY_UP, wxKeyEventHandler(ChatPanel::OnMessageChar), NULL, this);

	msgSizer->Add(ctrlMessage, 1, wxLEFT | wxRIGHT | wxEXPAND, 2);

	wxBitmapButton* emoticon = new wxBitmapButton(messagePanel, IDC_EMOT, ImageManager::getInstance()->emoticon, wxDefaultPosition, wxSize(-1, ctrlMessage->GetSize().GetHeight()), wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxNO_BORDER);
	emoticon->SetBitmapMargins(0, 0);

	//emoticon->Enable(Util::fileExists(Util::getPath(Util::PATH_EMOPACKS) + SETTING(EMOTICONS_FILE) + ".xml"));
	emoticon->Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(ChatPanel::OnEmotContextMenu), NULL, this);
	msgSizer->Add(emoticon, 0, wxALL | wxTILE, 0);

	messagePanel->SetSizerAndFit(msgSizer);

	// split chat and message box
	SplitHorizontally(ctrlChat, messagePanel);
	SetMinimumPaneSize(WinUtil::fontHeight + 8);
	SetSashGravity(1.0);
	SetSashPosition(-(WinUtil::fontHeight + 12), false);
}

void ChatPanel::readLog(const string& logFile, unsigned int maxLines)
{
	if(maxLines == 0) 
		return;

	try 
	{
		File f(logFile, File::READ, File::OPEN);
		
		int64_t size = f.getSize();

		if(size > 32*1024) 
		{
			f.setPos(size - 32*1024);
		}
		string buf = f.read(32*1024);
		StringList lines;

		if(strnicmp(buf.c_str(), "\xef\xbb\xbf", 3) == 0)
			lines = StringTokenizer<string>(buf.substr(3), "\r\n").getTokens();
		else
			lines = StringTokenizer<string>(buf, "\r\n").getTokens();

		unsigned int linesCount = lines.size();
		unsigned int i = linesCount > (maxLines + 1) ? linesCount - maxLines : 0;

		for(; i < linesCount; ++i)
		{
			ctrlChat->AppendText(Identity(NULL, 0), _T("- "), _T(""), Text::toT(lines[i]) + _T('\n'), WinUtil::m_ChatTextLog, true);
		}

		f.close();
	} 
	catch(const FileException&)
	{
		// don't show error when logfile doesn't exist
	}

	StringList lines;

	try {
		const int MAX_SIZE = 32 * 1024;

		File f(logFile, File::READ, File::OPEN);
		if(f.getSize() > MAX_SIZE) {
			f.setEndPos(-MAX_SIZE + 1);
		}

		lines = StringTokenizer<string>(f.read(MAX_SIZE), "\r\n").getTokens();
	} catch(const FileException&) { }

	if(lines.empty())
		return;

	// the last line in the log file is an empty line; remove it
	lines.pop_back();

	const size_t linesCount = lines.size();
	for(size_t i = ((linesCount > maxLines) ? (linesCount - maxLines) : 0); i < linesCount; ++i) 
	{
		ctrlChat->AppendText(Identity(NULL, 0), _T("- "), _T(""), Text::toT(lines[i]) + _T('\n'), WinUtil::m_ChatTextLog, true);
	}
}

void ChatPanel::OnMessageChar(wxKeyEvent& event)
{
	if(event.GetEventType() != wxEVT_KEY_DOWN)
	{
		// if ((uMsg == WM_CHAR) && (GetFocus() == ctrlMessage.m_hWnd) && (wParam != VK_RETURN) && (wParam != VK_TAB) && (wParam != VK_BACK)) {
		if ((!SETTING(SOUND_TYPING_NOTIFY).empty()) && (!BOOLSETTING(SOUNDS_DISABLED)))
			wxSound::Play(Text::toT(SETTING(SOUND_TYPING_NOTIFY)), wxSOUND_ASYNC);

		// TODO: make this settable?
		int possibleHeight = min(max(1, ctrlMessage->GetNumberOfLines()), 10) * WinUtil::fontHeight + 8;
		SetMinimumPaneSize(possibleHeight);
		return;
	}

	/* TODO if(!complete.empty() && event.GetKeyCode() != WXK_TAB)
		complete.clear();*/

	switch(event.GetKeyCode())
	{
		case WXK_RETURN:
			if(!ctrlMessage->GetValue().empty())
			{
				tstring s = ctrlMessage->GetValue();

				// save command in history, reset current buffer pointer to the newest command
				curCommandPosition = prevCommands.size();		//this places it one position beyond a legal subscript
				if (!curCommandPosition || curCommandPosition > 0 && prevCommands[curCommandPosition - 1] != s) 
				{
					++curCommandPosition;
					prevCommands.push_back(s);
				}
				currentCommand = Util::emptyStringT;

				wxCommandEvent evt(SEND_MESSAGE);
				evt.SetString(s);
				
				if(GetParent()->GetEventHandler()->ProcessEvent(evt))
				{
					ctrlMessage->Clear();
				}
			} 
			else 
			{
				wxBell();
			}
			return;

		/* TODO case WXK_TAB:
			OnTab(event);
			return;*/

		case WXK_UP:
			if (event.AltDown() ||	(!event.ControlDown() ^ BOOLSETTING(USE_CTRL_FOR_LINE_HISTORY)))
			{
				//scroll up in chat command history
				//currently beyond the last command?
				if (curCommandPosition > 0) 
				{
					//check whether current command needs to be saved
					if (curCommandPosition == prevCommands.size()) 
					{
						currentCommand = ctrlMessage->GetValue();
					}

					//replace current chat buffer with current command
					ctrlMessage->SetValue(prevCommands[--curCommandPosition].c_str());
				}
				// move cursor to end of line
				ctrlMessage->SetSelection(ctrlMessage->GetValue().Length(), ctrlMessage->GetValue().Length());
				return;
			}
			break;

		case WXK_DOWN:
			if (event.AltDown() ||	(!event.ControlDown() ^ BOOLSETTING(USE_CTRL_FOR_LINE_HISTORY)))
			{
				//scroll down in chat command history

				//currently beyond the last command?
				if (curCommandPosition + 1 < prevCommands.size()) 
				{
					//replace current chat buffer with current command
					ctrlMessage->SetValue(prevCommands[++curCommandPosition].c_str());
				} 
				else if (curCommandPosition + 1 == prevCommands.size()) 
				{
					//revert to last saved, unfinished command

					ctrlMessage->SetValue(currentCommand.c_str());
					++curCommandPosition;
				}
				// move cursor to end of line
				ctrlMessage->SetSelection(ctrlMessage->GetValue().Length(), ctrlMessage->GetValue().Length());
				return;
			}
			break;
	}

	event.Skip();
}

void ChatPanel::OnEmoticons(wxCommandEvent& event)
{
	wxString result;
	EmoticonsDlg dlg(this, &result, ((wxButton*)event.GetEventObject())->GetScreenRect());
	ctrlMessage->WriteText(result);

	ctrlMessage->SetFocus();
}

void ChatPanel::OnEmotContextMenu(wxContextMenuEvent& /*event*/)
{
	// load all emoticons packs
	wxDir dir(wxPathOnly(Text::toT(Util::getPath(Util::PATH_EMOPACKS))));
	if(!dir.IsOpened())
		return;

	wxMenu emoMenu;
	emoMenu.SetTitle(_T("Emoticons Pack"));
	wxMenuItem* disabledItem = emoMenu.AppendCheckItem(IDC_EMOMENU, TSTRING(DISABLED));	

	unsigned menuItems = 0;
	wxString fileName;

	bool success = false;
	bool more = dir.GetFirst(&fileName, wxT("*.xml"));

	while(more)
	{
		menuItems++;

		tstring packName(fileName.BeforeLast('.'));

		wxMenuItem* item = emoMenu.AppendCheckItem(IDC_EMOMENU + menuItems, packName);
		if(packName == Text::toT(SETTING(EMOTICONS_FILE))) 
		{
			item->Check(true);
			success = true;
		}

		// TODO: connect

		more = dir.GetNext(&fileName);
	}

	// if emopack doesn't exist set it to disabled
	if(!success) 
		disabledItem->Check(true);

	PopupMenu(&emoMenu);
}
