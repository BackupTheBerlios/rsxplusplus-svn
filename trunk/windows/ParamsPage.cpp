#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "Resource.h"

#include "ParamsPage.h"
#include "ParamDlg.h"

#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "../client/ClientProfileManager.h"

#define BUFLEN 256

PropPage::TextItem ParamsPage::texts[] = {
	{ IDC_ADD, ResourceManager::ADD },
	{ IDC_CHANGE, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item ParamsPage::items[] = {
	{ IDC_NAME_ONE,		RSXSettingsManager::CLIPBOARD_NAME_ONE,		PropPage::T_STR_RSX },
	{ IDC_NAME_TWO,		RSXSettingsManager::CLIPBOARD_NAME_TWO,		PropPage::T_STR_RSX },
	{ IDC_NAME_THREE,	RSXSettingsManager::CLIPBOARD_NAME_THREE,	PropPage::T_STR_RSX },
	{ IDC_NAME_FOUR,	RSXSettingsManager::CLIPBOARD_NAME_FOUR,	PropPage::T_STR_RSX },
	{ IDC_NAME_FIVE,	RSXSettingsManager::CLIPBOARD_NAME_FIVE,	PropPage::T_STR_RSX },
	{ IDC_NAME_SIX,		RSXSettingsManager::CLIPBOARD_NAME_SIX,		PropPage::T_STR_RSX },
	{ IDC_TEXT_ONE,		RSXSettingsManager::CLIPBOARD_TEXT_ONE,		PropPage::T_STR_RSX },
	{ IDC_TEXT_TWO,		RSXSettingsManager::CLIPBOARD_TEXT_TWO,		PropPage::T_STR_RSX },
	{ IDC_TEXT_THREE,	RSXSettingsManager::CLIPBOARD_TEXT_THREE,	PropPage::T_STR_RSX },
	{ IDC_TEXT_FOUR,	RSXSettingsManager::CLIPBOARD_TEXT_FOUR,	PropPage::T_STR_RSX },
	{ IDC_TEXT_FIVE,	RSXSettingsManager::CLIPBOARD_TEXT_FIVE,	PropPage::T_STR_RSX },
	{ IDC_TEXT_SIX,		RSXSettingsManager::CLIPBOARD_TEXT_SIX,		PropPage::T_STR_RSX},
	{ 0, 0, PropPage::T_END }
};

LRESULT ParamsPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items);

	CRect rc;

	ctrlParams.Attach(GetDlgItem(IDC_PARAM_ITEMS));
	ctrlParams.GetClientRect(rc);

	ctrlParams.InsertColumn(0, CTSTRING(SETTINGS_NAME), LVCFMT_LEFT, rc.Width() / 3, 0);
	ctrlParams.InsertColumn(1, CTSTRING(REGEXP), LVCFMT_LEFT, (rc.Width() / 3) * 2, 1);

	ctrlParams.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	// Do specialized reading here

	StringMap& pm = ClientProfileManager::getInstance()->getParams();
	TStringList cols;
	for(StringMap::iterator j = pm.begin(); j != pm.end(); ++j) {
		cols.push_back(Text::toT(j->first));
		cols.push_back(Text::toT(j->second));
		ctrlParams.insert(cols);
		cols.clear();
	}

	return TRUE;
}

LRESULT ParamsPage::onAdd(WORD , WORD , HWND , BOOL& ) {
	ParamDlg dlg;

	if(dlg.DoModal() == IDOK) {
		TStringList lst;
		lst.push_back(Text::toT(dlg.name));
		lst.push_back(Text::toT(dlg.regexp));
		ctrlParams.insert(lst);
	}
	return 0;
}

LRESULT ParamsPage::onChange(WORD , WORD , HWND , BOOL& ) {
	if(ctrlParams.GetSelectedCount() == 1) {
		int sel = ctrlParams.GetSelectedIndex();
		TCHAR buf[BUFLEN];
		ParamDlg dlg;
		ctrlParams.GetItemText(sel, 0, buf, BUFLEN);
		dlg.name = Text::fromT(buf);
		ctrlParams.GetItemText(sel, 1, buf, BUFLEN);
		dlg.regexp = Text::fromT(buf);

		if(dlg.DoModal() == IDOK) {
			ctrlParams.SetItemText(sel, 0, Text::toT(dlg.name).c_str());
			ctrlParams.SetItemText(sel, 1, Text::toT(dlg.regexp).c_str());
		}
	}
	return 0;
}

LRESULT ParamsPage::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlParams.GetSelectedCount() == 1) {
		ctrlParams.DeleteItem(ctrlParams.GetNextItem(-1, LVNI_SELECTED));
	}
	return 0;
}

void ParamsPage::write() {
	int it = ctrlParams.GetItemCount();
	TCHAR buf[BUFLEN];
	string name, regexp;
	StringMap& pm = ClientProfileManager::getInstance()->getParams();
	pm.clear();
	for(int i = 0; i < it; ++i) {
		ctrlParams.GetItemText(i, 0, buf, BUFLEN);
		name = Text::fromT(buf);
		ctrlParams.GetItemText(i, 1, buf, BUFLEN);
		regexp = Text::fromT(buf);
		pm[name] = regexp;
	}
	ClientProfileManager::getInstance()->saveClientProfiles();
	PropPage::write((HWND)*this, items);
}

LRESULT ParamsPage::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
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