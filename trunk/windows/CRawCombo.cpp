#include "stdafx.h"
#include "../client/DCPlusPlus.h"

#include "../client/RawManager.h"

#include "../rsx/rsx-settings/rsx-SettingsManager.h"

#include "CRawCombo.h"

CRawCombo::~CRawCombo() {
	if(settingId > 0) {
		RSXSettingsManager::getInstance()->set((RSXSettingsManager::IntSetting)settingId, getActionId());
	}

	Detach();
	map.clear();
}

void CRawCombo::attach(HWND hWnd, const int actionId) {
	Attach(hWnd);
	{
		const Action::List& lst = RawManager::getInstance()->getActionList();
		int j = 0;
		map.insert(make_pair(j, j));
		for(Action::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
			map.insert(make_pair(++j, i->second->getActionId()));
		}
	}
	for(ActionMap::const_iterator i = map.begin(); i != map.end(); ++i)
		AddString(RawManager::getInstance()->getNameActionId(i->second).c_str());
	SetCurSel(getActionPos(actionId));
}

int CRawCombo::getActionId() const {
	int index = GetCurSel();
	ActionMap::const_iterator i = map.find(index);
	if(i != map.end()) {
		return i->second;
	}
	return 0;
}

tstring CRawCombo::getActionName(const int actionId) const {
	return RawManager::getInstance()->getNameActionId(RawManager::getInstance()->getValidAction(actionId));
}
int CRawCombo::getActionPos(const int actionId) const {
	for(ActionMap::const_iterator i = map.begin(); i != map.end(); ++i) {
		if(actionId == i->second) {
			return i->first;
		}
	}
	return 0;
}