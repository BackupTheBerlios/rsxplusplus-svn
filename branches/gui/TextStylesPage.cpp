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

#include "stdafx.h"
#include "../client/SimpleXML.h"
#include "../client/Text.h"
#include "WinUtil.h"

#include "TextStylesPage.h"

BEGIN_EVENT_TABLE(TextStylesPage, wxPanel)
	EVT_BUTTON(IDC_BACK_COLOR, TextStylesPage::onBackColor)
	EVT_BUTTON(IDC_TEXT_COLOR, TextStylesPage::onTextColor)
	EVT_BUTTON(IDC_TEXT_STYLE, TextStylesPage::onTextStyle)
	EVT_BUTTON(IDC_DEFAULT_STYLES, TextStylesPage::onDefStyle)
	EVT_BUTTON(IDC_BLACK_AND_WHITE, TextStylesPage::onBW)
	EVT_CHOICE(IDC_TABCOLOR_LIST, TextStylesPage::onItemChange)
	EVT_BUTTON(IDC_RESET_TAB_COLOR, TextStylesPage::onReset)
	EVT_COLOURPICKER_CHANGED(IDC_SELECT_TAB_COLOR, TextStylesPage::onColourChange)
	EVT_BUTTON(IDC_SELTEXT, TextStylesPage::onTextStyle2)
	EVT_BUTTON(IDC_IMPORT, TextStylesPage::onImport)
	EVT_BUTTON(IDC_EXPORT, TextStylesPage::onExport)
END_EVENT_TABLE()

SettingsPage::TextItem TextStylesPage::texts[] = {
	{ IDC_AVAILABLE_STYLES, ResourceManager::SETSTRONGDC_STYLES },
	{ IDC_BACK_COLOR, ResourceManager::SETSTRONGDC_BACK_COLOR },
	{ IDC_TEXT_COLOR, ResourceManager::SETSTRONGDC_TEXT_COLOR },
	{ IDC_TEXT_STYLE, ResourceManager::SETSTRONGDC_TEXT_STYLE },
	{ IDC_DEFAULT_STYLES, ResourceManager::SETSTRONGDC_DEFAULT_STYLE },
	{ IDC_BLACK_AND_WHITE, ResourceManager::SETSTRONGDC_BLACK_WHITE },
	{ IDC_BOLD_AUTHOR_MESS, ResourceManager::SETSTRONGDC_BOLD },
	{ IDC_STRONGDC_PREVIEW, ResourceManager::SETSTRONGDC_PREVIEW },
	{ IDC_SELTEXT, ResourceManager::SETTINGS_SELECT_TEXT_FACE },
	{ IDC_RESET_TAB_COLOR, ResourceManager::SETTINGS_RESET },
	{ IDC_SELECT_TAB_COLOR, ResourceManager::SETTINGS_SELECT_COLOR },
	{ IDC_STYLES, ResourceManager::SETTINGS_TEXT_STYLES },
	{ IDC_CHATCOLORS, ResourceManager::SETTINGS_COLORS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
}; 

SettingsPage::Item TextStylesPage::items[] = {
	{ IDC_BOLD_AUTHOR_MESS, SettingsManager::BOLD_AUTHOR_MESS, SettingsPage::T_BOOL },
	{ 0, 0, SettingsPage::T_END }
};

TextStylesPage::clrs TextStylesPage::colours[] = {
	{ResourceManager::SETTINGS_SELECT_WINDOW_COLOR,	SettingsManager::BACKGROUND_COLOR, 0},
	{ResourceManager::SETTINGS_COLOR_ALTERNATE,	SettingsManager::SEARCH_ALTERNATE_COLOUR, 0},
	{ResourceManager::SETSTRONGDC_ERROR_COLOR,	SettingsManager::ERROR_COLOR, 0},
	{ResourceManager::PROGRESS_BACK,	SettingsManager::PROGRESS_BACK_COLOR, 0},
	{ResourceManager::PROGRESS_COMPRESS,	SettingsManager::PROGRESS_COMPRESS_COLOR, 0},
	{ResourceManager::PROGRESS_SEGMENT,	SettingsManager::PROGRESS_SEGMENT_COLOR, 0},
	{ResourceManager::PROGRESS_RUNNING,	SettingsManager::COLOR_RUNNING, 0},
	{ResourceManager::PROGRESS_DOWNLOADED,	SettingsManager::COLOR_DOWNLOADED, 0},
	{ResourceManager::PROGRESS_DONE,	SettingsManager::COLOR_DONE, 0},
};

TextStylesPage::TextStylesPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	settings = s;

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_STYLES, wxEmptyString), wxVERTICAL);
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	availStyleLabel = new wxStaticText(this, IDC_AVAILABLE_STYLES, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	availStyleLabel->Wrap(-1);
	bSizer2->Add(availStyleLabel, 1, wxALL, 5);
	
	viewStyleLabel = new wxStaticText(this, IDC_STRONGDC_PREVIEW, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	viewStyleLabel->Wrap(-1);
	bSizer2->Add(viewStyleLabel, 1, wxALL, 5);
	
	sbSizer1->Add(bSizer2, 0, wxEXPAND, 5);
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);
	
	availStyleList = new wxListBox(this, IDC_TEXT_STYLES, wxDefaultPosition, wxSize(-1,190), 0, NULL, 0); 
	bSizer3->Add(availStyleList, 1, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	viewStyleList = new wxRichTextCtrl(this, IDC_PREVIEW, wxEmptyString, wxDefaultPosition, wxSize(-1,190), wxTE_READONLY); 
	bSizer3->Add(viewStyleList, 1, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	sbSizer1->Add(bSizer3, 1, wxEXPAND, 5);
	
	boldAuthorCheck = new wxCheckBox(this, IDC_BOLD_AUTHOR_MESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	
	sbSizer1->Add(boldAuthorCheck, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxHORIZONTAL);
	
	backColorButton = new wxButton(this, IDC_BACK_COLOR, wxEmptyString, wxDefaultPosition, wxSize(-1,-1), 0);
	bSizer4->Add(backColorButton, 1, wxBOTTOM, 5);
	
	textColorButton = new wxButton(this, IDC_TEXT_COLOR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(textColorButton, 1, wxBOTTOM, 5);
	
	textStyleButton1 = new wxButton(this, IDC_TEXT_STYLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(textStyleButton1, 1, wxBOTTOM, 5);
	
	defStyleButton = new wxButton(this, IDC_DEFAULT_STYLES, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(defStyleButton, 1, wxBOTTOM, 5);
	
	bwStyleButton = new wxButton(this, IDC_BLACK_AND_WHITE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(bwStyleButton, 1, wxBOTTOM, 5);
	
	sbSizer1->Add(bSizer4, 0, wxEXPAND, 5);
	
	bSizer1->Add(sbSizer1, 0, wxALL | wxEXPAND, 5);
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxHORIZONTAL);
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_CHATCOLORS, wxEmptyString), wxVERTICAL);
	
	wxArrayString otherColorChoiceChoices;
	otherColorChoice = new wxChoice(this, IDC_TABCOLOR_LIST, wxDefaultPosition, wxSize(200,-1), otherColorChoiceChoices, 0);
	otherColorChoice->SetSelection(0);
	sbSizer2->Add(otherColorChoice, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxHORIZONTAL);
	
	resetColorButton = new wxButton(this, IDC_RESET_TAB_COLOR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer6->Add(resetColorButton, 0, wxALIGN_RIGHT | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	otherColorPicker = new wxColourPickerCtrl(this, IDC_SELECT_TAB_COLOR, *wxBLACK, wxDefaultPosition, wxSize(-1,-1), wxCLRP_DEFAULT_STYLE);
	bSizer6->Add(otherColorPicker, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT, 5);
	
	sbSizer2->Add(bSizer6, 0, wxALIGN_CENTER | wxALIGN_RIGHT, 5);
	
	bSizer5->Add(sbSizer2, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer(wxVERTICAL);
	
	textStyleButton = new wxButton(this, IDC_SELTEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer7->Add(textStyleButton, 0, wxALIGN_RIGHT | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	
	bSizer7->Add(0, 10, 1, wxEXPAND, 5);
	
	impThemeButton = new wxButton(this, IDC_IMPORT, _("Import theme"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer7->Add(impThemeButton, 0, wxALIGN_RIGHT | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	expThemeButton = new wxButton(this, IDC_EXPORT, _("Export theme"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer7->Add(expThemeButton, 0, wxALIGN_RIGHT | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	bSizer5->Add(bSizer7, 1, wxALIGN_RIGHT | wxEXPAND, 5);
	
	bSizer1->Add(bSizer5, 0, wxEXPAND, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);
		
	SettingsPage::translate(texts);
	SettingsPage::read(items);

	WinUtil::decodeFont(SETTING(TEXT_FONT), m_Font);

	init();
	refreshPreview();
}

TextStylesPage::~TextStylesPage()
{
}

void TextStylesPage::onBackColor(wxCommandEvent &WXUNUSED(event))
{
	int index = availStyleList->GetSelection();

	if (index != -1)
	{
		wxColourData clrData;
		clrData.SetColour(TextStyles[index].style.GetBackgroundColour());

		wxColourDialog dlg(this, &clrData);
		if (dlg.ShowModal() == wxID_OK)
			TextStyles[index].style.SetBackgroundColour(dlg.GetColourData().GetColour());
		refreshPreview();
	}
}

void TextStylesPage::onTextColor(wxCommandEvent &WXUNUSED(event))
{
	int index = availStyleList->GetSelection();

	if (index != -1)
	{
		wxColourData clrData;
		clrData.SetColour(TextStyles[index].style.GetTextColour());

		wxColourDialog dlg(this, &clrData);
		if (dlg.ShowModal() == wxID_OK)
		{
			TextStyles[index].style.SetTextColour(dlg.GetColourData().GetColour());
			refreshPreview();
		}
	}
}

void TextStylesPage::onTextStyle(wxCommandEvent &WXUNUSED(event))
{
	int index = availStyleList->GetSelection();
	if (index != -1)
	{
		wxFontData fontData;
		fontData.SetInitialFont(TextStyles[index].style.GetFont());
		wxFontDialog dlg(this, fontData);
		if (dlg.ShowModal() == wxID_OK)
		{
			TextStyles[index].style.SetFont(dlg.GetFontData().GetChosenFont());

			wxString face = dlg.GetFontData().GetChosenFont().GetFaceName();

			for (int i = 0; i < TS_LAST; i++)
				TextStyles[i].style.SetFontFaceName(face);

			TextStyles[index].style.SetFontUnderlined(false);
			refreshPreview();
		}
	}
}

void TextStylesPage::onDefStyle(wxCommandEvent &WXUNUSED(event))
{
	bg.Set(242, 245, 255);
	fg.Set(0, 0, 128);

	TextStyles[TS_GENERAL].style.SetBackgroundColour(wxColor(242, 245, 255));
	TextStyles[TS_GENERAL].style.SetTextColour(wxColor(67, 98, 154));
	TextStyles[TS_GENERAL].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_GENERAL].style.SetFontStyle(wxFONTSTYLE_NORMAL);
	
	TextStyles[TS_MYNICK].style.SetBackgroundColour(wxColor(242, 245, 255));
	TextStyles[TS_MYNICK].style.SetTextColour(wxColor(128, 0, 0));
	TextStyles[TS_MYNICK].style.SetFontWeight(wxFONTWEIGHT_BOLD);
	TextStyles[TS_MYNICK].style.SetFontStyle(wxFONTSTYLE_NORMAL);
	
	TextStyles[TS_MYMSG].style.SetBackgroundColour(wxColor(242, 245, 255));
	TextStyles[TS_MYMSG].style.SetTextColour(wxColor(128, 0, 0));
	TextStyles[TS_MYMSG].style.SetFontWeight(wxFONTWEIGHT_BOLD);
	TextStyles[TS_MYMSG].style.SetFontStyle(wxFONTSTYLE_NORMAL);
	
	TextStyles[TS_PRIVATE].style.SetBackgroundColour(wxColor(242, 245, 255));
	TextStyles[TS_PRIVATE].style.SetTextColour(wxColor(67, 98, 154));
	TextStyles[TS_PRIVATE].style.SetFontWeight(wxFONTWEIGHT_BOLD);
	TextStyles[TS_PRIVATE].style.SetFontStyle(wxFONTSTYLE_NORMAL);
	
	TextStyles[TS_SYSTEM].style.SetBackgroundColour(wxColor(242, 245, 255));
	TextStyles[TS_SYSTEM].style.SetTextColour(wxColor(0, 128, 64));
	TextStyles[TS_SYSTEM].style.SetFontWeight(wxFONTWEIGHT_BOLD);
	TextStyles[TS_SYSTEM].style.SetFontStyle(wxFONTSTYLE_ITALIC);
	
	TextStyles[TS_SERVER].style.SetBackgroundColour(wxColor(242, 245, 255));
	TextStyles[TS_SERVER].style.SetTextColour(wxColor(0, 128, 64));
	TextStyles[TS_SERVER].style.SetFontWeight(wxFONTWEIGHT_BOLD);
	TextStyles[TS_SERVER].style.SetFontStyle(wxFONTSTYLE_NORMAL);
	
	TextStyles[TS_TIMESTAMP].style.SetBackgroundColour(wxColor(242, 245, 255));
	TextStyles[TS_TIMESTAMP].style.SetTextColour(wxColor(67, 98, 154));
	TextStyles[TS_TIMESTAMP].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_TIMESTAMP].style.SetFontStyle(wxFONTSTYLE_NORMAL);
	
	TextStyles[TS_URL].style.SetBackgroundColour(wxColor(242, 245, 255));
	TextStyles[TS_URL].style.SetTextColour(wxColor(0, 0, 255));
	TextStyles[TS_URL].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_URL].style.SetFontStyle(wxFONTSTYLE_NORMAL);
	
	TextStyles[TS_FAVORITE].style.SetBackgroundColour(wxColor(242, 245, 255));
	TextStyles[TS_FAVORITE].style.SetTextColour(wxColor(255, 128, 64));
	TextStyles[TS_FAVORITE].style.SetFontWeight(wxFONTWEIGHT_BOLD);
	TextStyles[TS_FAVORITE].style.SetFontStyle(wxFONTSTYLE_ITALIC);
	
	TextStyles[TS_OP].style.SetBackgroundColour(wxColor(242, 245, 255));
	TextStyles[TS_OP].style.SetTextColour(wxColor(0, 128, 0));
	TextStyles[TS_OP].style.SetFontWeight(wxFONTWEIGHT_BOLD);
	TextStyles[TS_OP].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	refreshPreview();
}

void TextStylesPage::onBW(wxCommandEvent &WXUNUSED(event))
{
	bg = RGB(255, 255, 255);
	fg = RGB(0, 0, 0);

	TextStyles[TS_GENERAL].style.SetBackgroundColour(wxColor(255, 255, 255));
	TextStyles[TS_GENERAL].style.SetTextColour(wxColor(37, 60, 121));
	TextStyles[TS_GENERAL].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_GENERAL].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	TextStyles[TS_MYNICK].style.SetBackgroundColour(wxColor(255, 255, 255));
	TextStyles[TS_MYNICK].style.SetTextColour(wxColor(37, 60, 121));
	TextStyles[TS_MYNICK].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_MYNICK].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	TextStyles[TS_MYMSG].style.SetBackgroundColour(wxColor(255, 255, 255));
	TextStyles[TS_MYMSG].style.SetTextColour(wxColor(37, 60, 121));
	TextStyles[TS_MYMSG].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_MYMSG].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	TextStyles[TS_PRIVATE].style.SetBackgroundColour(wxColor(255, 255, 255));
	TextStyles[TS_PRIVATE].style.SetTextColour(wxColor(37, 60, 121));
	TextStyles[TS_PRIVATE].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_PRIVATE].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	TextStyles[TS_SYSTEM].style.SetBackgroundColour(wxColor(255, 255, 255));
	TextStyles[TS_SYSTEM].style.SetTextColour(wxColor(37, 60, 121));
	TextStyles[TS_SYSTEM].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_SYSTEM].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	TextStyles[TS_SERVER].style.SetBackgroundColour(wxColor(255, 255, 255));
	TextStyles[TS_SERVER].style.SetTextColour(wxColor(37, 60, 121));
	TextStyles[TS_SERVER].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_SERVER].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	TextStyles[TS_TIMESTAMP].style.SetBackgroundColour(wxColor(255, 255, 255));
	TextStyles[TS_TIMESTAMP].style.SetTextColour(wxColor(37, 60, 121));
	TextStyles[TS_TIMESTAMP].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_TIMESTAMP].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	TextStyles[TS_URL].style.SetBackgroundColour(wxColor(255, 255, 255));
	TextStyles[TS_URL].style.SetTextColour(wxColor(37, 60, 121));
	TextStyles[TS_URL].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_URL].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	TextStyles[TS_FAVORITE].style.SetBackgroundColour(wxColor(255, 255, 255));
	TextStyles[TS_FAVORITE].style.SetTextColour(wxColor(37, 60, 121));
	TextStyles[TS_FAVORITE].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_FAVORITE].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	TextStyles[TS_OP].style.SetBackgroundColour(wxColor(255, 255, 255));
	TextStyles[TS_OP].style.SetTextColour(wxColor(37, 60, 121));
	TextStyles[TS_OP].style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	TextStyles[TS_OP].style.SetFontStyle(wxFONTSTYLE_NORMAL);

	refreshPreview();
}

void TextStylesPage::onItemChange(wxCommandEvent &WXUNUSED(event))
{
	otherColorPicker->SetColour(wxColor(colours[otherColorChoice->GetSelection()].value));
}

void TextStylesPage::onReset(wxCommandEvent &WXUNUSED(event))
{
	doReset(otherColorChoice->GetSelection());
}

void TextStylesPage::onColourChange(wxColourPickerEvent& event)
{
	colours[otherColorChoice->GetSelection()].value = event.GetColour().GetRGB();
	switch(otherColorChoice->GetSelection())
	{
		case 0: 
			bg.Set(event.GetColour().GetRGB());
			break;
	}
	refreshPreview();
}

void TextStylesPage::onTextStyle2(wxCommandEvent &WXUNUSED(event))
{
	wxFontData fontData;
	fontData.SetInitialFont(m_Font);
	fontData.SetColour(fg);
	wxFontDialog dlg(this, fontData);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_Font = dlg.GetFontData().GetChosenFont();
		fg = dlg.GetFontData().GetColour();
	}
}

#define importData(x, y) \
	if(xml.findChild(x)) { SettingsManager::getInstance()->set(SettingsManager::y, xml.getChildData());} \
	xml.resetCurrentChild();

void TextStylesPage::onImport(wxCommandEvent &WXUNUSED(event))
{
	wxFileDialog dlg(this);
	dlg.SetWildcard(_T("DC++ Theme Files|*.dctheme|All files|*.*"));
	if (dlg.ShowModal() == wxID_OK)
	{
		SimpleXML xml;
		xml.fromXML(File(Text::fromT(dlg.GetPath()), File::READ, File::OPEN).read());
		xml.resetCurrentChild();
		xml.stepIn();
		if(xml.findChild(("Settings"))) {
			xml.stepIn();

			importData("Font", TEXT_FONT);
			importData("BackgroundColor", BACKGROUND_COLOR);
			importData("TextColor", TEXT_COLOR);
			importData("DownloadBarColor", DOWNLOAD_BAR_COLOR);
			importData("UploadBarColor", UPLOAD_BAR_COLOR);
			importData("TextGeneralBackColor", TEXT_GENERAL_BACK_COLOR);
			importData("TextGeneralForeColor", TEXT_GENERAL_FORE_COLOR);
			importData("TextGeneralBold", TEXT_GENERAL_BOLD);
			importData("TextGeneralItalic", TEXT_GENERAL_ITALIC);
			importData("TextMyOwnBackColor", TEXT_MYOWN_BACK_COLOR);
			importData("TextMyOwnForeColor", TEXT_MYOWN_FORE_COLOR);
			importData("TextMyOwnBold", TEXT_MYOWN_BOLD);
			importData("TextMyOwnItalic", TEXT_MYOWN_ITALIC);
			importData("TextPrivateBackColor", TEXT_PRIVATE_BACK_COLOR);
			importData("TextPrivateForeColor", TEXT_PRIVATE_FORE_COLOR);
			importData("TextPrivateBold", TEXT_PRIVATE_BOLD);
			importData("TextPrivateItalic", TEXT_PRIVATE_ITALIC);
			importData("TextSystemBackColor", TEXT_SYSTEM_BACK_COLOR);
			importData("TextSystemForeColor", TEXT_SYSTEM_FORE_COLOR);
			importData("TextSystemBold", TEXT_SYSTEM_BOLD);
			importData("TextSystemItalic", TEXT_SYSTEM_ITALIC);
			importData("TextServerBackColor", TEXT_SERVER_BACK_COLOR);
			importData("TextServerForeColor", TEXT_SERVER_FORE_COLOR);
			importData("TextServerBold", TEXT_SERVER_BOLD);
			importData("TextServerItalic", TEXT_SERVER_ITALIC);
			importData("TextTimestampBackColor", TEXT_TIMESTAMP_BACK_COLOR);
			importData("TextTimestampForeColor", TEXT_TIMESTAMP_FORE_COLOR);
			importData("TextTimestampBold", TEXT_TIMESTAMP_BOLD);
			importData("TextTimestampItalic", TEXT_TIMESTAMP_ITALIC);
			importData("TextMyNickBackColor", TEXT_MYNICK_BACK_COLOR);
			importData("TextMyNickForeColor", TEXT_MYNICK_FORE_COLOR);
			importData("TextMyNickBold", TEXT_MYNICK_BOLD);
			importData("TextMyNickItalic", TEXT_MYNICK_ITALIC);
			importData("TextFavBackColor", TEXT_FAV_BACK_COLOR);
			importData("TextFavForeColor", TEXT_FAV_FORE_COLOR);
			importData("TextFavBold", TEXT_FAV_BOLD);
			importData("TextFavItalic", TEXT_FAV_ITALIC);
			importData("TextURLBackColor", TEXT_URL_BACK_COLOR);
			importData("TextURLForeColor", TEXT_URL_FORE_COLOR);
			importData("TextURLBold", TEXT_URL_BOLD);
			importData("TextURLItalic", TEXT_URL_ITALIC);
			importData("BoldAuthorsMess", BOLD_AUTHOR_MESS);
			importData("ProgressTextDown", PROGRESS_TEXT_COLOR_DOWN);
			importData("ProgressTextUp", PROGRESS_TEXT_COLOR_UP);
			importData("ErrorColor", ERROR_COLOR);
			importData("ProgressOverrideColors", PROGRESS_OVERRIDE_COLORS);
			importData("MenubarTwoColors", MENUBAR_TWO_COLORS);
			importData("MenubarLeftColor", MENUBAR_LEFT_COLOR);
			importData("MenubarRightColor", MENUBAR_RIGHT_COLOR);
			importData("MenubarBumped", MENUBAR_BUMPED);
			importData("Progress3DDepth", PROGRESS_3DDEPTH);
			importData("ProgressOverrideColors2", PROGRESS_OVERRIDE_COLORS2);
			importData("TextOPBackColor", TEXT_OP_BACK_COLOR);
			importData("TextOPForeColor", TEXT_OP_FORE_COLOR);
			importData("TextOPBold", TEXT_OP_BOLD);
			importData("TextOPItalic", TEXT_OP_ITALIC);
			importData("SearchAlternateColour", SEARCH_ALTERNATE_COLOUR);
			importData("ProgressBackColor", PROGRESS_BACK_COLOR);
			importData("ProgressCompressColor", PROGRESS_COMPRESS_COLOR);
			importData("ProgressSegmentColor", PROGRESS_SEGMENT_COLOR);
			importData("ColorDone", COLOR_DONE);
			importData("ColorDownloaded", COLOR_DOWNLOADED);
			importData("ColorRunning", COLOR_RUNNING);
			importData("ReservedSlotColor", RESERVED_SLOT_COLOR);
			importData("IgnoredColor", IGNORED_COLOR);
			importData("FavoriteColor", FAVORITE_COLOR);
			importData("NormalColour", NORMAL_COLOUR);
			importData("FireballColor", FIREBALL_COLOR);
			importData("ServerColor", SERVER_COLOR);
			importData("PasiveColor", PASIVE_COLOR);
			importData("OpColor", OP_COLOR);
			importData("FileListAndClientCheckedColour", FULL_CHECKED_COLOUR);
			importData("BadClientColour", BAD_CLIENT_COLOUR);
			importData("BadFilelistColour", BAD_FILELIST_COLOUR);
			importData("ProgressbaroDCStyle", PROGRESSBAR_ODC_STYLE);
		}
		xml.resetCurrentChild();
		xml.stepOut();
	}

	init();
	refreshPreview();
}

#define exportData(x, y) \
	xml.addTag(x, SETTING(y)); \
	xml.addChildAttrib(type, curType);

void TextStylesPage::onExport(wxCommandEvent &WXUNUSED(event))
{
	wxFileDialog dlg(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString,
		_T("DC++ Theme Files|*.dctheme|All files|*.*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK)
	{
		SimpleXML xml;
		xml.addTag("DCPlusPlus");
		xml.stepIn();
		xml.addTag("Settings");
		xml.stepIn();

		string type("type"), curType("string");
		exportData("Font", TEXT_FONT);

		curType = "int";
		exportData("BackgroundColor", BACKGROUND_COLOR);
		exportData("TextColor", TEXT_COLOR);
		exportData("DownloadBarColor", DOWNLOAD_BAR_COLOR);
		exportData("UploadBarColor", UPLOAD_BAR_COLOR);
		exportData("TextGeneralBackColor", TEXT_GENERAL_BACK_COLOR);
		exportData("TextGeneralForeColor", TEXT_GENERAL_FORE_COLOR);
		exportData("TextGeneralBold", TEXT_GENERAL_BOLD);
		exportData("TextGeneralItalic", TEXT_GENERAL_ITALIC);
		exportData("TextMyOwnBackColor", TEXT_MYOWN_BACK_COLOR);
		exportData("TextMyOwnForeColor", TEXT_MYOWN_FORE_COLOR);
		exportData("TextMyOwnBold", TEXT_MYOWN_BOLD);
		exportData("TextMyOwnItalic", TEXT_MYOWN_ITALIC);
		exportData("TextPrivateBackColor", TEXT_PRIVATE_BACK_COLOR);
		exportData("TextPrivateForeColor", TEXT_PRIVATE_FORE_COLOR);
		exportData("TextPrivateBold", TEXT_PRIVATE_BOLD);
		exportData("TextPrivateItalic", TEXT_PRIVATE_ITALIC);
		exportData("TextSystemBackColor", TEXT_SYSTEM_BACK_COLOR);
		exportData("TextSystemForeColor", TEXT_SYSTEM_FORE_COLOR);
		exportData("TextSystemBold", TEXT_SYSTEM_BOLD);
		exportData("TextSystemItalic", TEXT_SYSTEM_ITALIC);
		exportData("TextServerBackColor", TEXT_SERVER_BACK_COLOR);
		exportData("TextServerForeColor", TEXT_SERVER_FORE_COLOR);
		exportData("TextServerBold", TEXT_SERVER_BOLD);
		exportData("TextServerItalic", TEXT_SERVER_ITALIC);
		exportData("TextTimestampBackColor", TEXT_TIMESTAMP_BACK_COLOR);
		exportData("TextTimestampForeColor", TEXT_TIMESTAMP_FORE_COLOR);
		exportData("TextTimestampBold", TEXT_TIMESTAMP_BOLD);
		exportData("TextTimestampItalic", TEXT_TIMESTAMP_ITALIC);
		exportData("TextMyNickBackColor", TEXT_MYNICK_BACK_COLOR);
		exportData("TextMyNickForeColor", TEXT_MYNICK_FORE_COLOR);
		exportData("TextMyNickBold", TEXT_MYNICK_BOLD);
		exportData("TextMyNickItalic", TEXT_MYNICK_ITALIC);
		exportData("TextFavBackColor", TEXT_FAV_BACK_COLOR);
		exportData("TextFavForeColor", TEXT_FAV_FORE_COLOR);
		exportData("TextFavBold", TEXT_FAV_BOLD);
		exportData("TextFavItalic", TEXT_FAV_ITALIC);
		exportData("TextURLBackColor", TEXT_URL_BACK_COLOR);
		exportData("TextURLForeColor", TEXT_URL_FORE_COLOR);
		exportData("TextURLBold", TEXT_URL_BOLD);
		exportData("TextURLItalic", TEXT_URL_ITALIC);
		exportData("BoldAuthorsMess", BOLD_AUTHOR_MESS);
		exportData("ProgressTextDown", PROGRESS_TEXT_COLOR_DOWN);
		exportData("ProgressTextUp", PROGRESS_TEXT_COLOR_UP);
		exportData("ErrorColor", ERROR_COLOR);
		exportData("ProgressOverrideColors", PROGRESS_OVERRIDE_COLORS);
		exportData("MenubarTwoColors", MENUBAR_TWO_COLORS);
		exportData("MenubarLeftColor", MENUBAR_LEFT_COLOR);
		exportData("MenubarRightColor", MENUBAR_RIGHT_COLOR);
		exportData("MenubarBumped", MENUBAR_BUMPED);
		exportData("Progress3DDepth", PROGRESS_3DDEPTH);
		exportData("ProgressOverrideColors2", PROGRESS_OVERRIDE_COLORS2);
		exportData("TextOPBackColor", TEXT_OP_BACK_COLOR);
		exportData("TextOPForeColor", TEXT_OP_FORE_COLOR);
		exportData("TextOPBold", TEXT_OP_BOLD);
		exportData("TextOPItalic", TEXT_OP_ITALIC);
		exportData("SearchAlternateColour", SEARCH_ALTERNATE_COLOUR);
		exportData("ProgressBackColor", PROGRESS_BACK_COLOR);
		exportData("ProgressCompressColor", PROGRESS_COMPRESS_COLOR);
		exportData("ProgressSegmentColor", PROGRESS_SEGMENT_COLOR);
		exportData("ColorDone", COLOR_DONE);
		exportData("ColorDownloaded", COLOR_DOWNLOADED);
		exportData("ColorRunning", COLOR_RUNNING);
		exportData("ReservedSlotColor", RESERVED_SLOT_COLOR);
		exportData("IgnoredColor", IGNORED_COLOR);
		exportData("FavoriteColor", FAVORITE_COLOR);
		exportData("NormalColour", NORMAL_COLOUR);
		exportData("FireballColor", FIREBALL_COLOR);
		exportData("ServerColor", SERVER_COLOR);
		exportData("PasiveColor", PASIVE_COLOR);
		exportData("OpColor", OP_COLOR);
		exportData("FileListAndClientCheckedColour", FULL_CHECKED_COLOUR);
		exportData("BadClientColour", BAD_CLIENT_COLOUR);
		exportData("BadFilelistColour", BAD_FILELIST_COLOUR);
		exportData("ProgressbaroDCStyle", PROGRESSBAR_ODC_STYLE);
	
		try {
			File ff(Text::fromT(dlg.GetPath()) , File::WRITE, File::CREATE | File::TRUNCATE);
			BufferedOutputStream<false> f(&ff);
			f.write(SimpleXML::utf8Header);
			xml.toXML(&f);
			f.flush();
			ff.close();
		} catch(const FileException&) {
			// ...
		}

	}
}

void TextStylesPage::doReset(int i)
{
	colours[i].value = SettingsManager::getInstance()->get((SettingsManager::IntSetting)colours[i].setting, true);
	wxColor clr = wxColor();
	clr.Set(colours[i].value);
	otherColorPicker->SetColour(clr);
}

void TextStylesPage::refreshPreview()
{
	viewStyleList->Clear();

	viewStyleList->SetBackgroundColour(bg);

	viewStyleList->SetDefaultStyle(TextStyles[TS_TIMESTAMP].style);

	for (int i = 0; i < TS_LAST; ++i)
	{
		viewStyleList->WriteText(_T("12:34 "));
		viewStyleList->BeginStyle(TextStyles[i].style);
		viewStyleList->WriteText(TextStyles[i].previewText + _T("\n"));
		viewStyleList->EndStyle();
	}
}

void TextStylesPage::init()
{
	initStyle(&TextStyles[TS_GENERAL], _("General text"), _("General chat text"),
		SETTING(TEXT_GENERAL_BACK_COLOR), SETTING(TEXT_GENERAL_FORE_COLOR), 
		SETTING(TEXT_GENERAL_BOLD), SETTING(TEXT_GENERAL_ITALIC));

	initStyle(&TextStyles[TS_MYNICK], _("My nick"), _("My nick"),
		SETTING(TEXT_MYNICK_BACK_COLOR), SETTING(TEXT_MYNICK_FORE_COLOR), 
		SETTING(TEXT_MYNICK_BOLD), SETTING(TEXT_MYNICK_ITALIC));

	initStyle(&TextStyles[TS_MYMSG], _("My own message"), _("My own message"),
		SETTING(TEXT_MYOWN_BACK_COLOR), SETTING(TEXT_MYOWN_FORE_COLOR), 
		SETTING(TEXT_MYOWN_BOLD), SETTING(TEXT_MYOWN_ITALIC));

	initStyle(&TextStyles[TS_PRIVATE], _("Private message"), _("Private message"),
		SETTING(TEXT_PRIVATE_BACK_COLOR), SETTING(TEXT_PRIVATE_FORE_COLOR), 
		SETTING(TEXT_PRIVATE_BOLD), SETTING(TEXT_PRIVATE_ITALIC));

	initStyle(&TextStyles[TS_SYSTEM], _("System message"), _("System message"),
		SETTING(TEXT_SYSTEM_BACK_COLOR), SETTING(TEXT_SYSTEM_FORE_COLOR), 
		SETTING(TEXT_SYSTEM_BOLD), SETTING(TEXT_SYSTEM_ITALIC));

	initStyle(&TextStyles[TS_SERVER], _("Server message"), _("Server message"),
		SETTING(TEXT_SERVER_BACK_COLOR), SETTING(TEXT_SERVER_FORE_COLOR), 
		SETTING(TEXT_SERVER_BOLD), SETTING(TEXT_SERVER_ITALIC));

	initStyle(&TextStyles[TS_TIMESTAMP], _("Timestamp"), _("The style for timestamp"),
		SETTING(TEXT_TIMESTAMP_BACK_COLOR), SETTING(TEXT_TIMESTAMP_FORE_COLOR), 
		SETTING(TEXT_TIMESTAMP_BOLD), SETTING(TEXT_TIMESTAMP_ITALIC));

	initStyle(&TextStyles[TS_URL], _("URL (http, mailto, ...)"), _("URL"),
		SETTING(TEXT_URL_BACK_COLOR), SETTING(TEXT_URL_FORE_COLOR), 
		SETTING(TEXT_URL_BOLD), SETTING(TEXT_URL_ITALIC));

	initStyle(&TextStyles[TS_FAVORITE], _("Favorite user"), _("Favorite user"),
		SETTING(TEXT_FAV_BACK_COLOR), SETTING(TEXT_FAV_FORE_COLOR), 
		SETTING(TEXT_FAV_BOLD), SETTING(TEXT_FAV_ITALIC));

	initStyle(&TextStyles[TS_OP], _("Operator"), _("Operator"),
		SETTING(TEXT_OP_BACK_COLOR), SETTING(TEXT_OP_FORE_COLOR), 
		SETTING(TEXT_OP_BOLD), SETTING(TEXT_OP_ITALIC));

	//----- Load styles -----
	fg = SETTING(TEXT_COLOR);
	bg = SETTING(BACKGROUND_COLOR);

	int i;

	availStyleList->Clear();
	for (i = 0; i < TS_LAST; i++)
	{
		availStyleList->AppendString(TextStyles[i].name);
	}
	
	otherColorChoice->Clear();
	for(i = 0; i < sizeof(colours) / sizeof(clrs); i++)
	{				
		otherColorChoice->AppendString(Text::toT(ResourceManager::getInstance()->getString(colours[i].name)).c_str());
		doReset(i);
	}

	otherColorChoice->SetSelection(0);
	otherColorPicker->SetColour(wxColor(GetRValue(colours[0].value), GetGValue(colours[0].value), GetBValue(colours[0].value)));
}

void TextStylesPage::initStyle(TextStyle *style, wxString name, wxString preview, int bgcolor, int fgcolor, int bold, int italic)
{
	style->name = name;
	style->previewText = preview;

	style->style.SetBackgroundColour(wxColor(bgcolor));
	style->style.SetTextColour(wxColor(fgcolor));
	
	wxFont font;
	WinUtil::decodeFont(SETTING(TEXT_FONT), font);
	style->style.SetFont(font);

	if (bold)
	{
		style->style.SetFontWeight(wxFONTWEIGHT_BOLD);
	}
	else
	{
		style->style.SetFontWeight(wxFONTWEIGHT_NORMAL);
	}
	if (italic)
	{
		style->style.SetFontStyle(wxFONTSTYLE_ITALIC);
	}
	else
	{
		style->style.SetFontStyle(wxFONTSTYLE_NORMAL);
	}
}

void TextStylesPage::saveStyles()
{
	wxColor bg, fg;

	bg = TextStyles[TS_GENERAL].style.GetBackgroundColour();
	fg = TextStyles[TS_GENERAL].style.GetTextColour();
	settings->set(SettingsManager::TEXT_GENERAL_BACK_COLOR, (int)bg.GetRGB());
	settings->set(SettingsManager::TEXT_GENERAL_FORE_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::TEXT_GENERAL_BOLD, (int)(TextStyles[TS_GENERAL].style.GetFontWeight() == wxFONTWEIGHT_BOLD));
	settings->set(SettingsManager::TEXT_GENERAL_ITALIC, (int)(TextStyles[TS_GENERAL].style.GetFontStyle() == wxFONTSTYLE_ITALIC));

	bg = TextStyles[TS_MYNICK].style.GetBackgroundColour();
	fg = TextStyles[TS_MYNICK].style.GetTextColour();
	settings->set(SettingsManager::TEXT_MYNICK_BACK_COLOR, (int)bg.GetRGB());
	settings->set(SettingsManager::TEXT_MYNICK_FORE_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::TEXT_MYNICK_BOLD, (int)(TextStyles[TS_MYNICK].style.GetFontWeight() == wxFONTWEIGHT_BOLD));
	settings->set(SettingsManager::TEXT_MYNICK_ITALIC, (int)(TextStyles[TS_MYNICK].style.GetFontStyle() == wxFONTSTYLE_ITALIC));

	bg = TextStyles[TS_MYMSG].style.GetBackgroundColour();
	fg = TextStyles[TS_MYMSG].style.GetTextColour();
	settings->set(SettingsManager::TEXT_MYOWN_BACK_COLOR, (int)bg.GetRGB());
	settings->set(SettingsManager::TEXT_MYOWN_FORE_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::TEXT_MYOWN_BOLD, (int)(TextStyles[TS_MYMSG].style.GetFontWeight() == wxFONTWEIGHT_BOLD));
	settings->set(SettingsManager::TEXT_MYOWN_ITALIC, (int)(TextStyles[TS_MYMSG].style.GetFontStyle() == wxFONTSTYLE_ITALIC));

	bg = TextStyles[TS_PRIVATE].style.GetBackgroundColour();
	fg = TextStyles[TS_PRIVATE].style.GetTextColour();
	settings->set(SettingsManager::TEXT_PRIVATE_BACK_COLOR, (int)bg.GetRGB());
	settings->set(SettingsManager::TEXT_PRIVATE_FORE_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::TEXT_PRIVATE_BOLD, (int)(TextStyles[TS_PRIVATE].style.GetFontWeight() == wxFONTWEIGHT_BOLD));
	settings->set(SettingsManager::TEXT_PRIVATE_ITALIC, (int)(TextStyles[TS_PRIVATE].style.GetFontStyle() == wxFONTSTYLE_ITALIC));

	bg = TextStyles[TS_SYSTEM].style.GetBackgroundColour();
	fg = TextStyles[TS_SYSTEM].style.GetTextColour();
	settings->set(SettingsManager::TEXT_SYSTEM_BACK_COLOR, (int)bg.GetRGB());
	settings->set(SettingsManager::TEXT_SYSTEM_FORE_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::TEXT_SYSTEM_BOLD, (int)(TextStyles[TS_SYSTEM].style.GetFontWeight() == wxFONTWEIGHT_BOLD));
	settings->set(SettingsManager::TEXT_SYSTEM_ITALIC, (int)(TextStyles[TS_SYSTEM].style.GetFontStyle() == wxFONTSTYLE_ITALIC));

	bg = TextStyles[TS_SERVER].style.GetBackgroundColour();
	fg = TextStyles[TS_SERVER].style.GetTextColour();
	settings->set(SettingsManager::TEXT_SERVER_BACK_COLOR, (int)bg.GetRGB());
	settings->set(SettingsManager::TEXT_SERVER_FORE_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::TEXT_SERVER_BOLD, (int)(TextStyles[TS_SERVER].style.GetFontWeight() == wxFONTWEIGHT_BOLD));
	settings->set(SettingsManager::TEXT_SERVER_ITALIC, (int)(TextStyles[TS_SERVER].style.GetFontStyle() == wxFONTSTYLE_ITALIC));

	bg = TextStyles[TS_TIMESTAMP].style.GetBackgroundColour();
	fg = TextStyles[TS_TIMESTAMP].style.GetTextColour();
	settings->set(SettingsManager::TEXT_TIMESTAMP_BACK_COLOR, (int)bg.GetRGB());
	settings->set(SettingsManager::TEXT_TIMESTAMP_FORE_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::TEXT_TIMESTAMP_BOLD, (int)(TextStyles[TS_TIMESTAMP].style.GetFontWeight() == wxFONTWEIGHT_BOLD));
	settings->set(SettingsManager::TEXT_TIMESTAMP_ITALIC, (int)(TextStyles[TS_TIMESTAMP].style.GetFontStyle() == wxFONTSTYLE_ITALIC));

	bg = TextStyles[TS_URL].style.GetBackgroundColour();
	fg = TextStyles[TS_URL].style.GetTextColour();
	settings->set(SettingsManager::TEXT_URL_BACK_COLOR, (int)bg.GetRGB());
	settings->set(SettingsManager::TEXT_URL_FORE_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::TEXT_URL_BOLD, (int)(TextStyles[TS_URL].style.GetFontWeight() == wxFONTWEIGHT_BOLD));
	settings->set(SettingsManager::TEXT_URL_ITALIC, (int)(TextStyles[TS_URL].style.GetFontStyle() == wxFONTSTYLE_ITALIC));

	bg = TextStyles[TS_FAVORITE].style.GetBackgroundColour();
	fg = TextStyles[TS_FAVORITE].style.GetTextColour();
	settings->set(SettingsManager::TEXT_FAV_BACK_COLOR, (int)bg.GetRGB());
	settings->set(SettingsManager::TEXT_FAV_FORE_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::TEXT_FAV_BOLD, (int)(TextStyles[TS_FAVORITE].style.GetFontWeight() == wxFONTWEIGHT_BOLD));
	settings->set(SettingsManager::TEXT_FAV_ITALIC, (int)(TextStyles[TS_FAVORITE].style.GetFontStyle() == wxFONTSTYLE_ITALIC));

	bg = TextStyles[TS_OP].style.GetBackgroundColour();
	fg = TextStyles[TS_OP].style.GetTextColour();
	settings->set(SettingsManager::TEXT_OP_BACK_COLOR, (int)bg.GetRGB());
	settings->set(SettingsManager::TEXT_OP_FORE_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::TEXT_OP_BOLD, (int)(TextStyles[TS_OP].style.GetFontWeight() == wxFONTWEIGHT_BOLD));
	settings->set(SettingsManager::TEXT_OP_ITALIC, (int)(TextStyles[TS_OP].style.GetFontStyle() == wxFONTSTYLE_ITALIC));
}

void TextStylesPage::write()
{
	SettingsPage::write(items);

	string f = WinUtil::encodeFont(m_Font);
	settings->set(SettingsManager::TEXT_FONT, f);

	settings->set(SettingsManager::TEXT_COLOR, (int)fg.GetRGB());
	settings->set(SettingsManager::BACKGROUND_COLOR, (int)bg.GetRGB());
	
	for(int i = 1; i < sizeof(colours) / sizeof(clrs); i++){
		settings->set((SettingsManager::IntSetting)colours[i].setting, (int)colours[i].value);
	}

	saveStyles();

	WinUtil::initColors();
}