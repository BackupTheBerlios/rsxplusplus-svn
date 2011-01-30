/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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
#include "../client/RawManager.h"
#include "../client/rsxppSettingsManager.h"

#include "CRawCombo.h"

CRawCombo::~CRawCombo() {
	if(settingId > 0)
		rsxppSettingsManager::getInstance()->set((rsxppSettingsManager::IntSetting)settingId, getActionId());
	Detach();
	map.clear();
}

void CRawCombo::attach(HWND hWnd, const int actionId) {
	Attach(hWnd);
	AddString(CTSTRING(UN_ACTION));
	SetItemData(0, NULL);

	int n = 1;
	const Action::ActionList& l = RawManager::getInstance()->getActions();
	for(Action::ActionList::const_iterator i = l.begin(); i != l.end(); ++i, ++n) {
		const Action* a = *i;
		AddString(Text::toT(a->getName()).c_str());
		SetItemData(n, (DWORD_PTR)a);
	}
	SetCurSel(getActionPos(actionId));
}

int CRawCombo::getActionId() const {
	int index = GetCurSel();
	const Action* a = (const Action*)GetItemData(index);
	if(a != NULL)
		return a->getId();
	return 0;
}

tstring CRawCombo::getActionName(const int actionId) const {
	return RawManager::getInstance()->getNameActionId(actionId);
}

int CRawCombo::getActionPos(const int actionId) const {
	int c = GetCount();
	for(int i = 0; i < c; ++i) {
		const Action* a = (const Action*)GetItemData(i);
		if(a != NULL) {
			if(a->getId() == actionId) return i;
		}
	}
	return 0;
}