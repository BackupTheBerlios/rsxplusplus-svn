/* 
 * Copyright (C) 2010 adrian_007 adrian-007 on o2 point pl
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

#ifndef _UPDATE_DIALOG_H_
#define _UPDATE_DIALOG_H_

#include "AsyncHandler.h"

#include "../client/HttpConnection.h"

class UpdateDialog : 
	public wxDialog, 
	public AsyncHandler<UpdateDialog>,
	private HttpConnectionListener 
{
public:
	UpdateDialog(wxWindow* parent);

private:
	DECLARE_EVENT_TABLE()

	void onClose(wxCommandEvent& event);
	void onDownload(wxCommandEvent& event);

	void on(HttpConnectionListener::Complete, HttpConnection* conn, string const& /*aLine*/, bool /*fromCoral*/) throw();
	void on(HttpConnectionListener::Data, HttpConnection* conn, const uint8_t* buf, size_t len) throw();	
	void on(HttpConnectionListener::Failed, HttpConnection* conn, const string& aLine) throw();

	wxStaticText* ctrlCurrentVersion;
	wxStaticText* ctrlCurrentVersionLabel;

	wxStaticText* ctrlLatestVersion;
	wxStaticText* ctrlLatestVersionLabel;

	wxTextCtrl* ctrlStatus;
	wxTextCtrl* ctrlChangelog;

	wxButton* btnClose;
	wxButton* btnDownload;

	HttpConnection* hc;
	string xmldata;
	string downloadURL;
};

#endif
