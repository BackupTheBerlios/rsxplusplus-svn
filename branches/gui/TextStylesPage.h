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

#ifndef _TEXTSTYLESPAGE_H
#define _TEXTSTYLESPAGE_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/listbox.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/clrpicker.h>
#include <wx/panel.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/colordlg.h>
#include <wx/fontdlg.h>

#include "SettingsPage.h"

class TextStylesPage : public SettingsPage
{
DECLARE_EVENT_TABLE()
private:
	static Item items[];
	static TextItem texts[];
	enum TextStyles
	{
		TS_GENERAL, TS_MYNICK, TS_MYMSG, TS_PRIVATE, TS_SYSTEM, TS_SERVER,
		TS_TIMESTAMP, TS_URL, TS_FAVORITE, TS_OP, TS_LAST
	};

	struct clrs
	{
		ResourceManager::Strings name;
		int setting;
		COLORREF value;
	};

	static clrs colours[];
	
	struct TextStyle
	{
		wxString name, previewText;
		wxTextAttr style;
	};

	wxStaticText* availStyleLabel;
	wxStaticText* viewStyleLabel;
	wxListBox* availStyleList;
	wxRichTextCtrl* viewStyleList;
	wxCheckBox* boldAuthorCheck;
	wxButton* backColorButton;
	wxButton* textColorButton;
	wxButton* textStyleButton1;
	wxButton* defStyleButton;
	wxButton* bwStyleButton;
	wxChoice* otherColorChoice;
	wxButton* resetColorButton;
	wxColourPickerCtrl* otherColorPicker;
	wxButton* textStyleButton;
		
	wxButton* impThemeButton;
	wxButton* expThemeButton;

	SettingsManager *settings;

	wxColor bg, fg;
	TextStyle TextStyles[TS_LAST];

	wxFont m_Font;
		
	void onBackColor(wxCommandEvent &WXUNUSED(event));
	void onTextColor(wxCommandEvent &WXUNUSED(event));
	void onTextStyle(wxCommandEvent &WXUNUSED(event));
	void onDefStyle(wxCommandEvent &WXUNUSED(event));
	void onBW(wxCommandEvent &WXUNUSED(event));
	void onItemChange(wxCommandEvent &WXUNUSED(event));
	void onReset(wxCommandEvent &WXUNUSED(event));
	void onColourChange(wxColourPickerEvent& event);
	void onTextStyle2(wxCommandEvent &WXUNUSED(event));
	void onImport(wxCommandEvent &WXUNUSED(event));
	void onExport(wxCommandEvent &WXUNUSED(event));
		
	void doReset(int i);
	void refreshPreview();

	void init();
	void initStyle(TextStyle *style, wxString name, wxString preview, int bgcolor, int fgcolor, int bold, int italic);
	void saveStyles();
			
protected:

public:
	TextStylesPage(wxWindow* parent, SettingsManager *s);
	~TextStylesPage();
	
	void write();
};

#endif // _TEXTSTYLESPAGE_H
