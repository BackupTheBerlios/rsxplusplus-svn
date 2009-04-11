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

#ifndef RSXPLUSPLUS_RAW_COMBO_BOX
#define RSXPLUSPLUS_RAW_COMBO_BOX

namespace dcpp {
	struct Action;
}

class CRawCombo : public CComboBox {
public:
	CRawCombo() : settingId(0) { }
	~CRawCombo();
	
	void attach(HWND hWnd, const int actionId);
	int getActionId() const;
	tstring getActionName(const int actionId) const;

	inline void setPos(const int actionId) {
		SetCurSel(getActionPos(actionId));
	}
	inline void setSettingId(const int Id) {
		settingId = Id;
	}
	inline Action* getAction() {
		return (Action*)GetItemData(GetCurSel());
	}

private:
	typedef unordered_map<int, int> ActionMap;

	int getActionPos(const int actionId) const;

	ActionMap map;
	int settingId;
};

#endif