#include "stdafx.h"
#include "../client/DCPlusPlus.h"

#include "../client/SettingsManager.h"
#include "../client/DetectionManager.h"

#include "DetectionListItem.h"
#include "StringPairDlg.h"

LRESULT DetectionListItem::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc;
	ctrlList.Attach(GetDlgItem(IDC_INF_LIST));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, _T("Field"), LVCFMT_LEFT, (rc.Width() / 6), 0);
	ctrlList.InsertColumn(1, _T("Pattern"), LVCFMT_LEFT, (rc.Width() / 2) + 50, 0);
	ctrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	initData();
	cRaw.attach(GetDlgItem(IDC_DETECTION_ITEM_ACTION), tmpEntry.rawToSend);
	return 0;
}

LRESULT DetectionListItem::onClose(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		if(!saveData())
			return 0;
	}
	EndDialog(wID);
	return 0;
}

LRESULT DetectionListItem::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	StringPairDlg dlg(_T("Add/Change Field"), _T("Field:"), _T("Pattern:"));
	dlg.first = Util::emptyStringT;
	dlg.second = Util::emptyStringT;
	while(true) {
		if(dlg.DoModal() == IDOK) {
			if(dlg.first.empty() || dlg.second.empty()) {
				MessageBox(_T("Field and Pattern must not be empty!"));
				continue;
			}
			tmpEntry.infMap.push_back(make_pair(Text::fromT(dlg.first), Text::fromT(dlg.second)));
			addItem(dlg.first, dlg.second, ctrlList.GetItemCount());
			break;
		} else {
			break;
		}
	}
	return 0;
}

LRESULT DetectionListItem::onChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int index = ctrlList.GetSelectedIndex();
	if(ctrlList.GetSelectedCount() == 1 && index != -1) {
		StringPairDlg dlg(_T("Add/Change Field"), _T("Field:"), _T("Pattern:"));
		DetectionEntry::StringMapV::iterator i = tmpEntry.infMap.begin() + index;
		if(i == tmpEntry.infMap.end())
			return 0;
		dlg.first = Text::toT(i->first);
		dlg.second = Text::toT(i->second);

		while(true) {
			if(dlg.DoModal() == IDOK) {
				if(dlg.first.empty() || dlg.second.empty()) {
					MessageBox(_T("Field and Pattern must not be empty!"));
					continue;
				}
				i->first = Text::fromT(dlg.first);
				i->second = Text::fromT(dlg.second);
				ctrlList.DeleteItem(index);
				addItem(dlg.first, dlg.second, index);
				break;
			} else {
				break;
			}
		}
	}
	return 0;
}

LRESULT DetectionListItem::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int index = ctrlList.GetSelectedIndex();
	if(ctrlList.GetSelectedCount() == 1 && index != -1) {
		ctrlList.DeleteItem(index);
		DetectionEntry::StringMapV::iterator i = tmpEntry.infMap.begin() + index;
		if(i != tmpEntry.infMap.end())
			tmpEntry.infMap.erase(i);
	}
	return 0;
}

LRESULT DetectionListItem::onNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	return 0;
}

LRESULT DetectionListItem::onBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	return 0;
}

LRESULT DetectionListItem::onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	//NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	//int index = ctrlList.GetSelectedIndex();
	//bool gotFocusOnList = (l->uNewState & LVIS_FOCUSED) || ((l->uNewState & LVIS_STATEIMAGEMASK) && index != -1);
	//enableFieldsEdit(gotFocusOnList);
	return 0;
}

void DetectionListItem::initData() {
	SetDlgItemText(IDC_DETECTION_ITEM_CHEAT, Text::toT(tmpEntry.cheat).c_str());
	SetDlgItemText(IDC_DETECTION_ITEM_NAME, Text::toT(tmpEntry.name).c_str());
	SetDlgItemText(IDC_DETECTION_ITEM_COMMENT, Text::toT(tmpEntry.comment).c_str());
	SetDlgItemInt(IDC_DETECTION_ITEM_ID, tmpEntry.Id);
	ctrlList.DeleteAllItems();
	for(DetectionEntry::StringMapV::const_iterator i = tmpEntry.infMap.begin(); i != tmpEntry.infMap.end(); ++i) {
		addItem(Text::toT(i->first), Text::toT(i->second), ctrlList.GetItemCount());
	}
	cRaw.setPos(tmpEntry.rawToSend);
}

bool DetectionListItem::saveData() {
	tmpEntry.name = Text::fromT(WinUtil::getWindowText(m_hWnd, IDC_DETECTION_ITEM_NAME));
	tmpEntry.cheat = Text::fromT(WinUtil::getWindowText(m_hWnd, IDC_DETECTION_ITEM_CHEAT));
	tmpEntry.comment = Text::fromT(WinUtil::getWindowText(m_hWnd, IDC_DETECTION_ITEM_COMMENT));
	int newId = Util::toInt(Text::fromT(WinUtil::getWindowText(m_hWnd, IDC_DETECTION_ITEM_ID)));
	/*if(newId < 1) {
		MessageBox(_T("ID must be greater thant 0!"));
		return true;
	}
	if(tmpEntry.Id != newId) {
		if(DetectionManager::getInstance()->isExist(newId)) {
			MessageBox(_T("Item with this ID already exist!"));
			return false;
		}
	}
	if(tmpEntry.infMap.empty()) {
		MessageBox(_T("INF Map must not be empty!"));
		return false;
	}
	if(tmpEntry.name.empty()) {
		MessageBox(_T("Name must not be empty!"));
		return false;
	}*/

	tmpEntry.Id = newId;
	tmpEntry.rawToSend = cRaw.getActionId();
	try {
		DetectionManager::getInstance()->validateItem(tmpEntry);
	} catch(const Exception& e) {
		MessageBox(Text::toT(e.getError()).c_str(), _T("Error"));
		return false;
	}
	return true;
}

void DetectionListItem::enableFieldsEdit(bool enable) {
	::EnableWindow(GetDlgItem(IDC_DETECTION_ITEM_ADD), enable);
	::EnableWindow(GetDlgItem(IDC_DETECTION_ITEM_CHANGE), enable);
	::EnableWindow(GetDlgItem(IDC_DETECTION_ITEM_REMOVE), enable);
}