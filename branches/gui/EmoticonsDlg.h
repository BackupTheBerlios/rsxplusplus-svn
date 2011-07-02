/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
 * Copyright (C) 2010 Oyashiro-sama, oyashirosama dot hnnkni at gmail dot com
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

#ifndef _EMOTICONSDLG_H
#define _EMOTICONSDLG_H

#include <wx/wx.h>

#include "EmoticonsManager.h"

class EmoticonsDlg : public wxDialog
{
private:
	DECLARE_EVENT_TABLE()

	wxString *_result;
	int _width, _height, _iconWidth, _iconHeight;
	unsigned int _nX, _nY, _count, _oldPos;
	bool isCaptured;
	wxImageList _list;
	wxArrayString _emotText;
	Emoticon::List Emoticons;

protected:

	void onMouseUp(wxMouseEvent &event);
	void onMouseMove(wxMouseEvent &event);
	void onKeyUp(wxKeyEvent &event);
	void onPaint(wxPaintEvent &WXUNUSED(event));
	void onCaptureLost(wxMouseCaptureLostEvent &WXUNUSED(event));

public:

	EmoticonsDlg(wxWindow* parent, wxString *result, const wxRect pos);
	~EmoticonsDlg(void);
};

#endif	// _EMOTICONSDLG_H