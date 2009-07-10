/*
 * Copyright (C) 2007-2009 adrian_007, adrian-007 on o2 point pl
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
#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "Resource.h"

#include "RawPage.h"
#include "LineDlg.h"
#include "RawDlg.h"

#include "../client/Util.h"
#include "../client/version.h"
#include "../rsx/RsxUtil.h"

PropPage::TextItem RawPage::texts[] = {
	{ IDC_RAW_PAGE_ACTION_CTN, ResourceManager::ACTION },
	{ IDC_MOVE_RAW_UP, ResourceManager::MOVE_UP },
	{ IDC_MOVE_RAW_DOWN, ResourceManager::MOVE_DOWN },
	{ IDC_ADD_ACTION, ResourceManager::ADD },
	{ IDC_RENAME_ACTION, ResourceManager::RENAME },
	{ IDC_REMOVE_ACTION, ResourceManager::REMOVE },
	{ IDC_ADD_RAW, ResourceManager::ADD },
	{ IDC_CHANGE_RAW, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_RAW, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT RawPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	ctrlList.Attach(GetDlgItem(IDC_ACTIONS));
	// nasty workaround to get checkboxes created in the right way
	ctrlList.ModifyStyle(TVS_CHECKBOXES, 0); 
	ctrlList.ModifyStyle(0, TVS_CHECKBOXES);

	ctrlList.SetRedraw(FALSE);
	const Action::ActionList& a = RawManager::getInstance()->getActions();
	for(Action::ActionList::const_iterator i = a.begin(); i != a.end(); ++i) {
		HTREEITEM actionItem = addAction(*i);
		for(Action::RawsList::const_iterator j = (*i)->raw.begin(); j != (*i)->raw.end(); ++j)
			addRaw(actionItem, &(*j));
	}

	ctrlList.SetRedraw(TRUE);
	ctrlList.Invalidate();
	return 0;
}

HTREEITEM RawPage::addAction(const Action* action) {
	HTREEITEM item = ctrlList.InsertItem(Text::toT(action->getName()).c_str(), 0, 0);
	ctrlList.SetItemData(item, (DWORD_PTR)action);
	ctrlList.SetCheckState(item, action->getEnabled());
	return item;
}

void RawPage::addRaw(HTREEITEM action, const Raw* raw) {
	HTREEITEM item = ctrlList.InsertItem(Text::toT(raw->getName()).c_str(), action, 0);
	ctrlList.SetItemData(item, (DWORD_PTR)raw);
	ctrlList.SetCheckState(item, raw->getEnabled());
}

void RawPage::updateRaw(HTREEITEM item, Raw* raw) {
	ctrlList.SetItemText(item, Text::toT(raw->getName()).c_str());
	ctrlList.SetItemData(item, (DWORD_PTR)raw);
	ctrlList.SetCheckState(item, raw->getEnabled());
}

void RawPage::write() {
	RawManager::getInstance()->lock();
	HTREEITEM item = ctrlList.GetRootItem();
	while(item != NULL) {
		HTREEITEM child = ctrlList.GetChildItem(item);
		while(child != NULL) {
			Raw* raw = (Raw*)ctrlList.GetItemData(child);
			if(raw != NULL)
				raw->setEnabled(ctrlList.GetCheckState(child) ? true : false);
			child = ctrlList.GetNextSiblingItem(child);
		}
		Action* action = (Action*)ctrlList.GetItemData(item);
		if(action != NULL)
			action->setEnabled(ctrlList.GetCheckState(item) ? true : false);
		item = ctrlList.GetNextItem(item, TVGN_NEXT);
	}
	RawManager::getInstance()->unlock();
}

LRESULT RawPage::onParentAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	while(true) {
		try {
			LineDlg dlg;
			dlg.title = TSTRING(ADD_ACTION);
			dlg.description = TSTRING(NAME);
			dlg.line = TSTRING(ACTION);
			if(dlg.DoModal(m_hWnd) == IDOK) {
				Action* a = RawManager::getInstance()->addAction(0, Text::fromT(dlg.line), true);
				HTREEITEM item = addAction(a);
				ctrlList.Select(item, TVGN_CARET);
			}
			break;
		} catch(const Exception& e) {
			MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
	}
	return 0;
}

LRESULT RawPage::onParentEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HTREEITEM item = ctrlList.GetSelectedItem();
	if(item != NULL) {
		HTREEITEM parent = ctrlList.GetParentItem(item);
		if(parent != NULL)
			item = parent;
		Action* a = (Action*)ctrlList.GetItemData(item);
		if(a == NULL) return 0;
		while(true) {
			try {
				LineDlg dlg;
				dlg.title = TSTRING(ADD_ACTION);
				dlg.description = TSTRING(NAME);
				dlg.line = Text::toT(a->getName());
				if(dlg.DoModal(m_hWnd) == IDOK) {
					RawManager::getInstance()->editAction(a, Text::fromT(dlg.line));
					ctrlList.SetItemText(item, dlg.line.c_str());
				}
				break;
			} catch(const Exception& e) {
				MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
			}
		}
	}
	return 0;
}

LRESULT RawPage::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HTREEITEM item = ctrlList.GetSelectedItem();
	if(item != NULL) {
		HTREEITEM parent = ctrlList.GetParentItem(item);
		bool success = false; //@todo remove it in the future - it's useless
		if(parent != NULL) {	// raw
			Raw* r = (Raw*)ctrlList.GetItemData(item);
			Action* a = (Action*)ctrlList.GetItemData(parent);
			if(a != NULL && r != NULL)
				success = RawManager::getInstance()->remRaw(a, r);
		} else {				// action
			Action* a = (Action*)ctrlList.GetItemData(item);
			if(a != NULL) success = RawManager::getInstance()->remAction(a);
		}
		if(success)
			ctrlList.DeleteItem(item);
	}
	return 0;
}

LRESULT RawPage::onChildAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HTREEITEM item = ctrlList.GetSelectedItem();
	HTREEITEM parent = ctrlList.GetParentItem(item);
	if(parent == NULL) // if we selected raw that has parent, add new one to this parent
		parent = item;

	Action* a = (Action*)ctrlList.GetItemData(parent);
	if(a == NULL) return 0;

	while(true) {
		try {
			RawDlg dlg;
			if(dlg.DoModal(m_hWnd) == IDOK) {
				Raw r;
				r.setEnabled(true);
				r.setName(dlg.name);
				r.setRaw(dlg.raw);
				r.setTime(dlg.time);
				r.setLua(dlg.useLua);
				r.setId(0);

				Raw* ptr = RawManager::getInstance()->addRaw(a, r);
				addRaw(parent, ptr);
				ctrlList.Expand(parent);
			}
			break;
		} catch(const Exception& e) {
			MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
	}
	return 0;
}

LRESULT RawPage::onChildEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HTREEITEM item = ctrlList.GetSelectedItem();
	HTREEITEM parent = ctrlList.GetParentItem(item);
	if(item != NULL && parent != NULL) {
		const Action* a = (const Action*)ctrlList.GetItemData(parent);
		Raw* old = (Raw*)ctrlList.GetItemData(item);
		if(old == NULL || a == NULL) return 0;

		while(true) {
			try {
				RawDlg dlg(old);
				if(dlg.DoModal(m_hWnd) == IDOK) {
					Raw r;
					r.setEnabled(old->getEnabled());
					r.setName(dlg.name);
					r.setRaw(dlg.raw);
					r.setTime(dlg.time);
					r.setLua(dlg.useLua);
		
					RawManager::getInstance()->editRaw(a, old, r);
					ctrlList.SetItemText(item, Text::toT(old->getName()).c_str());
					//ctrlList.Expand(parent);
				}
				break;
			} catch(const Exception& e) {
				MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
			}
		}
	}
	return 0;
}

LRESULT RawPage::onChildUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HTREEITEM item = ctrlList.GetSelectedItem();
	HTREEITEM parent = ctrlList.GetParentItem(item);
	HTREEITEM prev = ctrlList.GetPrevSiblingItem(item);

	if(item != NULL && parent != NULL && prev != NULL) {
		Raw* x = (Raw*)ctrlList.GetItemData(item);
		Raw* y = (Raw*)ctrlList.GetItemData(prev);

		if(y == NULL || x == NULL) return 0;
		x->setEnabled(ctrlList.GetCheckState(item) ? true : false);
		y->setEnabled(ctrlList.GetCheckState(prev) ? true : false);

		swap(*x, *y);
		updateRaw(item, x);
		updateRaw(prev, y);

		ctrlList.SelectItem(prev);
	}
	return 0;
}

LRESULT RawPage::onChildDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HTREEITEM item = ctrlList.GetSelectedItem();
	HTREEITEM parent = ctrlList.GetParentItem(item);
	HTREEITEM next = ctrlList.GetNextSiblingItem(item);

	if(item != NULL && parent != NULL && next != NULL) {
		Raw* x = (Raw*)ctrlList.GetItemData(item);
		Raw* y = (Raw*)ctrlList.GetItemData(next);

		if(y == NULL || x == NULL) return 0;
		x->setEnabled(ctrlList.GetCheckState(item) ? true : false);
		y->setEnabled(ctrlList.GetCheckState(next) ? true : false);

		swap(*x, *y);
		updateRaw(item, x);
		updateRaw(next, y);
		ctrlList.SelectItem(next);
	}
	return 0;
}

LRESULT RawPage::onExpand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HTREEITEM item = ctrlList.GetRootItem();
	while(item != NULL) {
		ctrlList.Expand(item, expanded ? TVE_COLLAPSE : TVE_EXPAND);
		item = ctrlList.GetNextItem(item, TVGN_NEXT);
	}
	if(expanded)
		::SetWindowText(GetDlgItem(IDC_EXPAND), _T("Expand"));
	else
		::SetWindowText(GetDlgItem(IDC_EXPAND), _T("Collapse"));
	expanded = !expanded;
	return 0;
}
