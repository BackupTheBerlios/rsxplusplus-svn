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
#include "Resource.h"

#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "../client/RawManager.h"

#include "../client/rsxppSettingsManager.h"
#include "../rsx/RsxUtil.h"

#include "AutosearchPage.h"
#include "AutosearchDlg.h"

#define BUFLEN 256

PropPage::TextItem AutosearchPage::texts[] = {
	{ IDC_AUTOSEARCH_ENABLE,		ResourceManager::AUTOSEARCH_ENABLE },
	{ IDC_AUTOSEARCH_ENABLE_TIME,	ResourceManager::AUTOSEARCH_ENABLE_TIME },
	{ IDC_MINUTES,					ResourceManager::MINUTES },
	{ IDC_MINUTES2,					ResourceManager::MINUTES },
	{ IDC_ADD,						ResourceManager::ADD },
	{ IDC_CHANGE,					ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE,					ResourceManager::REMOVE },
	{ 0,							ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item AutosearchPage::items[] = {
	{ IDC_AUTOSEARCH_ENABLE,		rsxppSettingsManager::AUTOSEARCH_ENABLED,		PropPage::T_BOOL_RSX },
	{ IDC_AUTOSEARCH_ENABLE_TIME,	rsxppSettingsManager::AUTOSEARCH_ENABLED_TIME,	PropPage::T_BOOL_RSX },
	{ IDC_AUTOSEARCH_EVERY,			rsxppSettingsManager::AUTOSEARCH_EVERY,			PropPage::T_INT_RSX },
	{ IDC_AUTOSEARCH_RECHECK_TIME,	rsxppSettingsManager::AUTOSEARCH_RECHECK_TIME,	PropPage::T_INT_RSX },
	{ 0, 0, PropPage::T_END }
};

LRESULT AutosearchPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items);

	CUpDownCtrl spin;
	spin.Attach(GetDlgItem(IDC_AUTOSEARCH_SPIN));
	spin.SetRange32(1, 360);
	spin.Detach();
	spin.Attach(GetDlgItem(IDC_AUTOSEARCH_RECHECK_SPIN));
	spin.SetRange32(1, 360);
	spin.Detach();

	CRect rc;
	ctrlAutosearch.Attach(GetDlgItem(IDC_AUTOSEARCH_ITEMS));
	ctrlAutosearch.GetClientRect(rc);
	ctrlAutosearch.InsertColumn(0, CTSTRING(SETTINGS_VALUE),		LVCFMT_LEFT, (rc.Width() / 2), 0);
	ctrlAutosearch.InsertColumn(1, CTSTRING(TYPE),					LVCFMT_LEFT, (rc.Width() / 4), 0);
	//ctrlAutosearch.InsertColumn(2, CTSTRING(CHEATING_DESCRIPTION),	LVCFMT_LEFT, rc.Width() / 3, 1);
	ctrlAutosearch.InsertColumn(2, CTSTRING(ACTION),				LVCFMT_LEFT, (rc.Width() / 4) - 20, 0);
	ctrlAutosearch.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);

	Autosearch::List& lst = AutoSearchManager::getInstance()->getAutosearch();
	for(Autosearch::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const Autosearch::Ptr as = *i;	
		addEntry(as, ctrlAutosearch.GetItemCount());
	}
	gotFocusOnList = false; //page created = no focus
	fixControls();
	return TRUE;
}

void AutosearchPage::write() {
	AutoSearchManager::getInstance()->AutosearchSave();
	PropPage::write((HWND)*this, items);
}

void AutosearchPage::addEntry(const Autosearch::Ptr as, int pos) {
	TStringList lst;
	lst.push_back(Text::toT(as->getSearchString()));
	lst.push_back(Text::toT(getType(as->getFileType())));
	//lst.push_back(Text::toT(as->getCheat()));
	lst.push_back(RawManager::getInstance()->getNameActionId(RawManager::getInstance()->getValidAction(as->getRaw())));
	bool b = as->getEnabled();
	int i = ctrlAutosearch.insert(pos, lst, 0, (LPARAM)as);
	ctrlAutosearch.SetCheckState(i, b);
}

LRESULT AutosearchPage::onAdd(WORD , WORD , HWND , BOOL& ) {
	AutosearchPageDlg dlg;
	if(dlg.DoModal() == IDOK) {
		addEntry(AutoSearchManager::getInstance()->addAutosearch(true, Text::fromT(dlg.search), dlg.fileType, dlg.action,
			dlg.rawToSend, dlg.display, Text::fromT(dlg.comment)), ctrlAutosearch.GetItemCount());
	}
	return 0;
}

LRESULT AutosearchPage::onChange(WORD , WORD , HWND , BOOL& ) {
	if(ctrlAutosearch.GetSelectedCount() == 1) {
		int sel = ctrlAutosearch.GetSelectedIndex();
		Autosearch* as = (Autosearch*)ctrlAutosearch.GetItemData(sel);

		AutosearchPageDlg dlg;
		dlg.search = Text::toT(as->getSearchString());
		dlg.fileType = as->getFileType();
		dlg.action = as->getAction();
		dlg.rawToSend = as->getRaw();
		dlg.display = as->getDisplayCheat();
		dlg.comment = Text::toT(as->getCheat());

		if(dlg.DoModal() == IDOK) {
			as->setSearchString(Text::fromT(dlg.search));
			as->setFileType(dlg.fileType);
			as->setAction(dlg.action);
			as->setRaw(dlg.rawToSend);
			as->setDisplayCheat(dlg.display);
			as->setCheat(Text::fromT(dlg.comment));

			ctrlAutosearch.SetCheckState(sel, as->getEnabled());
			ctrlAutosearch.SetItemText(sel, 0, dlg.search.c_str());
			ctrlAutosearch.SetItemText(sel, 1, Text::toT(getType(dlg.fileType)).c_str());
			//ctrlAutosearch.SetItemText(sel, 2, dlg.comment.c_str());
			ctrlAutosearch.SetItemText(sel, 2, RawManager::getInstance()->getNameActionId(dlg.rawToSend).c_str());
		}
	}
	return 0;
}

LRESULT AutosearchPage::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlAutosearch.GetSelectedCount() == 1) {
		int sel = ctrlAutosearch.GetSelectedIndex();
		AutoSearchManager::getInstance()->removeAutosearch(sel);
		ctrlAutosearch.DeleteItem(sel);
	}
	return 0;
}

LRESULT AutosearchPage::onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	gotFocusOnList = false;
	fixControls();
	return 0;
}

LRESULT AutosearchPage::onMoveUp(WORD , WORD , HWND , BOOL& ) {
	int j = ctrlAutosearch.GetItemCount();
	ctrlAutosearch.SetRedraw(FALSE);
	for(int i = 1; i < j; ++i) {
		if(ctrlAutosearch.GetItemState(i, LVIS_SELECTED)) {
			Autosearch* i1 = (Autosearch*)ctrlAutosearch.GetItemData(i);
			Autosearch* i2 = (Autosearch*)ctrlAutosearch.GetItemData(i-1);

			ctrlAutosearch.DeleteItem(i);
			ctrlAutosearch.DeleteItem(i-1);
			swap(*i1, *i2);

			addEntry(i2, i-1);
			addEntry(i1, i);

			ctrlAutosearch.SetItemState(i-1, LVIS_SELECTED, LVIS_SELECTED);
			ctrlAutosearch.EnsureVisible(i-1, FALSE);

			::EnableWindow(GetDlgItem(IDC_MOVE_UP), true);
			::EnableWindow(GetDlgItem(IDC_MOVE_DOWN), true);
		}
	}
	ctrlAutosearch.SetRedraw(TRUE);
	return 0;
}

LRESULT AutosearchPage::onMoveDown(WORD , WORD , HWND , BOOL& ) {
	int j = ctrlAutosearch.GetItemCount() - 2;
	ctrlAutosearch.SetRedraw(FALSE);
	for(int i = j; i >= 0; --i) {
		if(ctrlAutosearch.GetItemState(i, LVIS_SELECTED)) {
			Autosearch* i1 = (Autosearch*)ctrlAutosearch.GetItemData(i);
			Autosearch* i2 = (Autosearch*)ctrlAutosearch.GetItemData(i+1);
	
			ctrlAutosearch.DeleteItem(i+1);
			ctrlAutosearch.DeleteItem(i);
			swap(*i1, *i2);

			addEntry(i1, i);
			addEntry(i2, i+1);

			ctrlAutosearch.SetItemState(i+1, LVIS_SELECTED, LVIS_SELECTED);
			ctrlAutosearch.EnsureVisible(i+1, FALSE);

			::EnableWindow(GetDlgItem(IDC_MOVE_UP), true);
			::EnableWindow(GetDlgItem(IDC_MOVE_DOWN), true);
		}
	}
	ctrlAutosearch.SetRedraw(TRUE);
	return 0;
}

LRESULT AutosearchPage::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	gotFocusOnList = (l->uNewState & LVIS_FOCUSED) || ((l->uNewState & LVIS_STATEIMAGEMASK) && ctrlAutosearch.GetSelectedIndex() != -1);

	if(l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) != (l->uOldState & LVIS_STATEIMAGEMASK))) {
		Autosearch& a = *(Autosearch*)ctrlAutosearch.GetItemData(l->iItem);
		a.setEnabled(ctrlAutosearch.GetCheckState(l->iItem) != FALSE);
		AutoSearchManager::getInstance()->AutosearchSave();
	}

	::EnableWindow(GetDlgItem(IDC_CHANGE),		gotFocusOnList);
	::EnableWindow(GetDlgItem(IDC_REMOVE),		gotFocusOnList);
	::EnableWindow(GetDlgItem(IDC_MOVE_UP),		gotFocusOnList);
	::EnableWindow(GetDlgItem(IDC_MOVE_DOWN),	gotFocusOnList);
	return 0;		
}

void AutosearchPage::fixControls() {
	BOOL enable = IsDlgButtonChecked(IDC_AUTOSEARCH_ENABLE) == BST_CHECKED;
	BOOL time = enable && IsDlgButtonChecked(IDC_AUTOSEARCH_ENABLE_TIME) == BST_CHECKED;

	::EnableWindow(GetDlgItem(IDC_AUTOSEARCH_ITEMS),		enable);
	::EnableWindow(GetDlgItem(IDC_AUTOSEARCH_ENABLE_TIME),	enable);

	::EnableWindow(GetDlgItem(IDC_ADD),						enable);
	::EnableWindow(GetDlgItem(IDC_CHANGE),					enable && gotFocusOnList);
	::EnableWindow(GetDlgItem(IDC_REMOVE),					enable && gotFocusOnList);
	::EnableWindow(GetDlgItem(IDC_MOVE_UP),					enable && gotFocusOnList);
	::EnableWindow(GetDlgItem(IDC_MOVE_DOWN),				enable && gotFocusOnList);

	::EnableWindow(GetDlgItem(IDC_AUTOSEARCH_EVERY),		time);
	::EnableWindow(GetDlgItem(IDC_AUTOSEARCH_SPIN),			time);
	::EnableWindow(GetDlgItem(IDC_MINUTES),					time);
	::EnableWindow(GetDlgItem(IDC_MINUTES2),				time);
	::EnableWindow(GetDlgItem(IDC_AUTOSEARCH_RECHECK_TIME),	time);
	::EnableWindow(GetDlgItem(IDC_AUTOSEARCH_RECHECK_SPIN),	time);
	::EnableWindow(GetDlgItem(IDC_AUTOSEARCH_RECHECK_TEXT),	time);
}

LRESULT AutosearchPage::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;

	switch(cd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		{
			try	{
				if(cd->nmcd.dwItemSpec % 2 == 0) {
					cd->clrTextBk = RGB(245, 245, 245);
				}
				return CDRF_NEWFONT | CDRF_NOTIFYSUBITEMDRAW;
			} catch(const Exception&) {
			} catch(...) {
			}
		}
		return CDRF_NOTIFYSUBITEMDRAW;

	default:
		return CDRF_DODEFAULT;
	}
}

LRESULT AutosearchPage::onInfoTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	int item = ctrlAutosearch.GetHotItem();
	if(item != -1) {
		NMLVGETINFOTIP* lpnmtdi = (NMLVGETINFOTIP*) pnmh;
		Autosearch* as = (Autosearch*)ctrlAutosearch.GetItemData(item);

		tstring infoTip = _T("Item: ") + Text::toT(as->getSearchString()) +
			_T("\nCheat: ") + (as->getCheat().empty() ? _T("N/A") : Text::toT(as->getCheat())) +
			_T("\nFile Type: ") + Text::toT(getType(as->getFileType())) +
			_T("\nAction: ") + RawManager::getInstance()->getNameActionId(RawManager::getInstance()->getValidAction(as->getRaw())) +
			_T("\nDisipay cheat: ") + (as->getDisplayCheat() ? CTSTRING(YES) : CTSTRING(NO));
		_tcscpy(lpnmtdi->pszText, infoTip.c_str());
	}
	return 0;
}