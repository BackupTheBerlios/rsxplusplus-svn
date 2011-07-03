/* 
 * Copyright (C) 2010-2011 adrian_007 adrian-007 on o2 point pl
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
#include "NotepadFrame.h"

#include "../client/File.h"

BEGIN_EVENT_TABLE(NotepadFrame, BaseType)
	EVT_CLOSE(NotepadFrame::OnClose)
END_EVENT_TABLE()

NotepadFrame::NotepadFrame() : BaseType(ID_CMD_NOTEPAD, CTSTRING(NOTEPAD), wxColour(0, 0, 0), wxT("IDI_NOTEPAD")) {
	textCtrl = new wxTextCtrl(this, ID_NOTEPAD_TEXTBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxHSCROLL | /*wxTE_AUTO_SCROLL |*/ wxTE_MULTILINE | wxTE_AUTO_URL);
	textCtrl->SetFont(WinUtil::font);
	textCtrl->SetBackgroundColour(WinUtil::bgColor);
	textCtrl->SetForegroundColour(WinUtil::textColor);

	string tmp;
	try {
		tmp = File(Util::getNotepadFile(), File::READ, File::OPEN).read();
	} catch(const FileException&) {
		// ...
	}

	textCtrl->SetValue(wxString(tmp.c_str(), wxConvUTF8));

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(textCtrl, 1, wxEXPAND);
	SetSizerAndFit(sizer);
}

NotepadFrame::~NotepadFrame() {

}

void NotepadFrame::UpdateLayout() { 
}

void NotepadFrame::on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw() {
	Refresh();
}

void NotepadFrame::OnClose(wxCloseEvent& event) {
	string tmp = textCtrl->GetValue().mb_str(wxConvUTF8);

	try {
		File(Util::getNotepadFile(), File::WRITE, File::CREATE | File::TRUNCATE).write(tmp);
	} catch(const FileException&) {
		// Oops...
	}

	WinUtil::setButtonPressed(GetId(), false);
	event.Skip();
}
