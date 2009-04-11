/* 
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
#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "Resource.h"

#include "FilterPage.h"
#include "FilterPageDlg.h"
#include "HLPageDlg.h"

#include "../client/rsxppSettingsManager.h"
#include "WinUtil.h"

PropPage::TextItem FilterPage::texts[] = {
	{ IDC_ADD_FILTER, ResourceManager::ADD },
	{ IDC_CHANGE_FILTER, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_FILTER, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item FilterPage::items[] = {
	{ IDC_USE_CHAT_FILTER, rsxppSettingsManager::USE_CHAT_FILTER, PropPage::T_BOOL_RSX },
	{ IDC_USE_HL, rsxppSettingsManager::USE_HIGHLIGHT, PropPage::T_BOOL_RSX },
	{ 0, 0, PropPage::T_END }
};

LRESULT FilterPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items);

	CRect rc;
	ctrlFilters.Attach(GetDlgItem(IDC_FILTER_PAGE_ITEMS));
	ctrlFilters.GetClientRect(rc);
	ctrlFilters.InsertColumn(0, CTSTRING(SETTINGS_FSTRING), LVCFMT_LEFT, rc.Width(), 0);
	ctrlFilters.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	cHL.Attach(GetDlgItem(IDC_HL_ITEMS));
	cHL.GetClientRect(rc);
	cHL.InsertColumn(0, CTSTRING(SETTINGS_FSTRING), LVCFMT_LEFT, rc.Width(), 0);
	cHL.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	// Do specialized reading here.
	Filters::List ul = FavoriteManager::getInstance()->getFilters();
	for(Filters::Iter j = ul.begin(); j != ul.end(); ++j) {
		Filters::Ptr fs = *j;	
		addEntryFilter(fs, ctrlFilters.GetItemCount());
	}
	HighLight::List hll = FavoriteManager::getInstance()->getHLs();
	for(HighLight::Iter i = hll.begin(); i != hll.end(); i++) {
		HighLight::Ptr hl = *i;
		addEntryHL(hl, cHL.GetItemCount());
	}

	fixControls();
	return TRUE;
}

void FilterPage::addEntryFilter(Filters::Ptr fs, int pos) {
	TStringList lst;
	lst.push_back(Text::toT(fs->getFstring()));
	ctrlFilters.insert(pos, lst, 0, 0);
}

void FilterPage::addEntryHL(HighLight::Ptr hl, int pos) {
	TStringList lst2;
	lst2.push_back(Text::toT(hl->getHstring()));
	cHL.insert(pos, lst2, 0, 0);
}

LRESULT FilterPage::onAddFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	FilterPageDlg dlg;
	//load defaults settings
	dlg.fontColor = SETTING(TEXT_GENERAL_FORE_COLOR);
	dlg.backColor = SETTING(TEXT_GENERAL_BACK_COLOR);
	dlg.useColor = dlg.bold = dlg.italic = dlg.underline = dlg.strikeout = false;

	if(dlg.DoModal() == IDOK) {
		addEntryFilter(FavoriteManager::getInstance()->addFilter(Text::fromT(dlg.fstring), dlg.useColor, dlg.hasFontColor, dlg.hasBgColor, dlg.fontColor, 
			dlg.backColor, dlg.bold, dlg.italic, dlg.underline, dlg.strikeout), ctrlFilters.GetItemCount());
	}
	delete dlg;
	return 0;
}

LRESULT FilterPage::onChangeFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlFilters.GetSelectedCount() == 1) {
		int sel = ctrlFilters.GetSelectedIndex();
		Filters fs;
		FavoriteManager::getInstance()->getFilter(sel, fs);

		FilterPageDlg dlg;
		dlg.fstring = Text::toT(fs.getFstring());
		dlg.useColor = fs.getUseColor();
		dlg.hasFontColor = fs.getHasFontColor();
		dlg.hasBgColor = fs.getHasBgColor();
		dlg.fontColor = fs.getFontColor();
		dlg.backColor = fs.getBackColor();
		dlg.bold = fs.getBoldFont();
		dlg.italic = fs.getItalicFont();
		dlg.underline = fs.getUnderlineFont();
		dlg.strikeout = fs.getStrikeoutFont();

		if(dlg.DoModal() == IDOK) {
			fs.setFstring(Text::fromT(dlg.fstring));
			fs.setUseColor(dlg.useColor);
			fs.setHasFontColor(dlg.hasFontColor);
			fs.setHasBgColor(dlg.hasBgColor);
			fs.setFontColor((int)dlg.fontColor);
			fs.setBackColor((int)dlg.backColor);
			fs.setBoldFont(dlg.bold);
			fs.setItalicFont(dlg.italic);
			fs.setUnderlineFont(dlg.underline);
			fs.setStrikeoutFont(dlg.strikeout);

			FavoriteManager::getInstance()->updateFilter(sel, fs);

			ctrlFilters.SetItemText(sel, 0, dlg.fstring.c_str());
		}
		delete dlg;
	}
	return 0;
}

LRESULT FilterPage::onRemoveFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlFilters.GetSelectedCount() == 1) {
		int sel = ctrlFilters.GetSelectedIndex();
		FavoriteManager::getInstance()->removeFilter(sel);
		ctrlFilters.DeleteItem(sel);
	}
	return 0;
}

LRESULT FilterPage::onAddHL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HLPageDlg dlg;
	//@todo move it to dlg constructor
	dlg.fontColor = SETTING(TEXT_GENERAL_FORE_COLOR);
	dlg.backColor = SETTING(TEXT_GENERAL_BACK_COLOR);
	dlg.bold = dlg.italic = dlg.underline = dlg.strike = dlg.hasBgColor = dlg.hasFontColor = false;
	dlg.displayPopup = dlg.flashWindow = dlg.playSound = false;

	if(dlg.DoModal() == IDOK) {
		addEntryHL(FavoriteManager::getInstance()->addHighLight(Text::fromT(dlg.fstring), dlg.hasFontColor, dlg.hasBgColor, dlg.fontColor, dlg.backColor, dlg.bold, dlg.italic, dlg.underline, dlg.strike,
			dlg.displayPopup, dlg.flashWindow, dlg.playSound, Text::fromT(dlg.soundFile)), cHL.GetItemCount());
	}
	delete dlg;
	return 0;
}

LRESULT FilterPage::onChangeHL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(cHL.GetSelectedCount() == 1) {
		int sel = cHL.GetSelectedIndex();
		HighLight fs;
		FavoriteManager::getInstance()->getHighLight(sel, fs);
		HLPageDlg dlg;
		dlg.fstring = Text::toT(fs.getHstring());
		dlg.fontColor = fs.getFontColor();
		dlg.backColor = fs.getBackColor();
		dlg.bold = fs.getBoldFont();
		dlg.italic = fs.getItalicFont();
		dlg.underline = fs.getUnderlineFont();
		dlg.strike = fs.getStrikeoutFont();
		dlg.hasFontColor = fs.getHasFontColor();
		dlg.hasBgColor = fs.getHasBgColor();
		dlg.displayPopup = fs.getDisplayPopup();
		dlg.flashWindow = fs.getFlashWindow();
		dlg.playSound = fs.getPlaySound();
		dlg.soundFile = Text::toT(fs.getSoundFilePath());

		if(dlg.DoModal() == IDOK) {
			fs.setHstring(Text::fromT(dlg.fstring));
			fs.setFontColor((int)dlg.fontColor);
			fs.setBackColor((int)dlg.backColor);
			fs.setBoldFont(dlg.bold);
			fs.setItalicFont(dlg.italic);
			fs.setUnderlineFont(dlg.underline);
			fs.setStrikeoutFont(dlg.strike);
			fs.setHasFontColor(dlg.hasFontColor);
			fs.setHasBgColor(dlg.hasBgColor);
			fs.setDisplayPopup(dlg.displayPopup);
			fs.setFlashWindow(dlg.flashWindow);
			fs.setPlaySound(dlg.playSound);
			fs.setSoundFilePath(Text::fromT(dlg.soundFile));
			FavoriteManager::getInstance()->updateHighLight(sel, fs);

			cHL.SetItemText(sel, 0, dlg.fstring.c_str());
		}
		delete dlg;
	}
	return 0;
}

LRESULT FilterPage::onRemoveHL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(cHL.GetSelectedCount() == 1) {
		int sel = cHL.GetSelectedIndex();
		FavoriteManager::getInstance()->removeHighLight(sel);
		cHL.DeleteItem(sel);
	}
	return 0;
}

LRESULT FilterPage::onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}

void FilterPage::fixControls() {
	bool use = IsDlgButtonChecked(IDC_USE_CHAT_FILTER) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_FILTER_PAGE_ITEMS), use);
	::EnableWindow(GetDlgItem(IDC_ADD_FILTER), use);
	::EnableWindow(GetDlgItem(IDC_CHANGE_FILTER), use);
	::EnableWindow(GetDlgItem(IDC_REMOVE_FILTER), use);

	use = IsDlgButtonChecked(IDC_USE_HL) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_HL_ITEMS), use);
	::EnableWindow(GetDlgItem(IDC_ADD_HL), use);
	::EnableWindow(GetDlgItem(IDC_CHANGE_HL), use);
	::EnableWindow(GetDlgItem(IDC_REMOVE_HL), use);
}

void FilterPage::write() { 
	PropPage::write((HWND)*this, items);
	FavoriteManager::getInstance()->filtersSave();
//	FavoriteManager::getInstance()->saveHL();
}

/**
 * @file
 * $Id: FilterPage.cpp 109 2006-10-31 03:21:46Z crakter $
 */
