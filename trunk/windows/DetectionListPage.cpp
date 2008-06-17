#include "stdafx.h"
#include "../client/DCPlusPlus.h"

#include "../client/SettingsManager.h"
#include "../client/DetectionManager.h"
#include "../client/Text.h"

#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "DetectionListPage.h"
#include "DetectionListItem.h"

PropPage::TextItem DetectionListPage::texts[] = {
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT DetectionListPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc;
	ctrlList.Attach(GetDlgItem(IDC_DETECTION_LIST));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, CTSTRING(NAME), LVCFMT_LEFT, (rc.Width() / 4), 0);
	ctrlList.InsertColumn(1, CTSTRING(CHEATING_DESCRIPTION), LVCFMT_LEFT, (rc.Width() / 2), 0);
	ctrlList.InsertColumn(2, CTSTRING(ACTION), LVCFMT_LEFT, (rc.Width() / 4) - 20, 0);
	ctrlList.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | /*LVS_EX_INFOTIP |*/ LVS_EX_FULLROWSELECT);

	const DetectionManager::DetectionItems& list = DetectionManager::getInstance()->getProfiles();
	for(DetectionManager::DetectionItems::const_iterator i = list.begin(); i != list.end(); ++i) {
		addEntry(*i, ctrlList.GetItemCount());
	}
	return 0;
}

LRESULT DetectionListPage::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	DetectionListItem dlg;
	if(dlg.DoModal() == IDOK) {
		const DetectionEntry& e = dlg.tmpEntry;
		try {
			DetectionManager::getInstance()->addDetectionItem(e);
			addEntry(e, ctrlList.GetItemCount());
		} catch(const Exception& e) {
			MessageBox(Text::toT(e.getError()).c_str(), _T("Error"));
		}
	}
	return 0;
}

LRESULT DetectionListPage::onEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() == 1) {
		int index = ctrlList.GetSelectedIndex();
		int aId = (int)ctrlList.GetItemData(index);
		if(aId > 0) {
			DetectionListItem dlg;
			DetectionManager::getInstance()->getDetectionItem(aId, dlg.tmpEntry);
			if(dlg.DoModal() == IDOK) {
				DetectionManager::getInstance()->updateDetectionItem(aId, dlg.tmpEntry);
				ctrlList.SetItemData(index, dlg.tmpEntry.Id);
				updateColumns(Text::toT(dlg.tmpEntry.name), Text::toT(dlg.tmpEntry.cheat), _T(""));
			}
		}
	}
	return 0;
}

LRESULT DetectionListPage::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() == 1) {
		int index = ctrlList.GetSelectedIndex();
		int aId = (int)ctrlList.GetItemData(index);
		DetectionManager::getInstance()->removeDetectionItem(aId);
		ctrlList.DeleteItem(index);
	}
	return 0;
}

LRESULT DetectionListPage::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	if(l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) != (l->uOldState & LVIS_STATEIMAGEMASK))) {
		int aId = (int)ctrlList.GetItemData(l->iItem);
		if(aId > 0) {
			const bool e = ctrlList.GetCheckState(l->iItem) != false;
			DetectionManager::getInstance()->setItemEnabled(aId, e);
			DetectionManager::getInstance()->save();
		}
	}
	return 0;
}

void DetectionListPage::addEntry(const DetectionEntry& entry, int pos) {
	TStringList strings;
	strings.push_back(Text::toT(entry.name));
	strings.push_back(Text::toT(entry.cheat));
	bool isEnabled = entry.isEnabled;
	int i = ctrlList.insert(pos, strings, 0, entry.Id);
	ctrlList.SetCheckState(i, isEnabled);
}

void DetectionListPage::write() {
	DetectionManager::getInstance()->save();
}

void DetectionListPage::updateColumns(const tstring& name, const tstring& cheat, const tstring& action) {
	int index = ctrlList.GetSelectedIndex();
	ctrlList.SetItemText(index, 0, name.c_str());
	ctrlList.SetItemText(index, 1, cheat.c_str());
	ctrlList.SetItemText(index, 2, action.c_str());
}