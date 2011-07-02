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

#ifndef _COUNTRYFLAGS_H
#define _COUNTRYFLAGS_H

#include "WinUtil.h"

#include <wx/dataview.h>
#include <wx/imaglist.h>

class CountryFlags
{
public:

	static void init();

	static uint8_t getFlagIndexByCode(const char* countryCode);
	static uint8_t getFlagIndexByName(const char* countryName);

	// class for rendering flags into DataViewCtrl
	class FlagRenderer : public wxDataViewCustomRenderer
	{
	public:
		FlagRenderer() : wxDataViewCustomRenderer(wxT("wxDataViewIconText")) { }

		wxSize GetSize() const { return wxSize(30, 25); } // <-- what is this for?
		bool GetValue(wxVariant&) const { return true; }

		bool SetValue(const wxVariant& _value);
		bool Render(wxRect rect, wxDC *dc, int state);
	
	private:
		wxString value;
	};

	static wxImageList& getFlagsImages() { return flags; }

private:

	static wxImageList	flags;

	static const char* countryNames[];
	static const char* countryCodes[];
};

#endif	// _COUNTRYFLAGS_H