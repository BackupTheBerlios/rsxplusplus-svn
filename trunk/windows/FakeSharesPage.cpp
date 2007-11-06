#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "FakeSharesPage.h"
#include "FakeSharesDlg.h"

#include "../client/SettingsManager.h"
#include "../client/ClientProfileManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::TextItem FakeSharesPage::texts[] = {
	{ IDC_ADD_FAKE_SHARE, ResourceManager::ADD },
	{ IDC_CHANGE_FAKE_SHARE, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_FAKE_SHARE, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT FakeSharesPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);

	CRect rc;

	ctrlFakeShares.Attach(GetDlgItem(IDC_FAKE_SHARE_ITEMS));
	ctrlFakeShares.GetClientRect(rc);

	ctrlFakeShares.InsertColumn(0, CTSTRING(SETTINGS_VALUE), LVCFMT_LEFT, rc.Width() / 2, 0);
	ctrlFakeShares.InsertColumn(1, CTSTRING(TYPE), LVCFMT_LEFT, rc.Width() / 2, 1);

	ctrlFakeShares.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	// Do specialized reading here

	FakeShares::List& ul = ClientProfileManager::getInstance()->getFakeShares();
	TStringList cols;
	for(FakeShares::List::const_iterator j = ul.begin(); j != ul.end(); ++j) {
		cols.push_back(Text::toT(j->getValue()));
		cols.push_back(j->getExact() ? TSTRING(EXACT) : TSTRING(CONTAINS));
		ctrlFakeShares.insert(cols);
		cols.clear();
	}

	return TRUE;
}

LRESULT FakeSharesPage::onAddFakeShare(WORD , WORD , HWND , BOOL& ) {
	FakeSharesDlg dlg;

	if(dlg.DoModal() == IDOK) {
		TStringList lst;
		lst.push_back(Text::toT(dlg.value));
		lst.push_back(Text::toT(dlg.type));
		ctrlFakeShares.insert(lst);
	}
	return 0;
}

LRESULT FakeSharesPage::onChangeFakeShare(WORD , WORD , HWND , BOOL& ) {
	
#define BUFLEN 256
	if(ctrlFakeShares.GetSelectedCount() == 1) {
		int sel = ctrlFakeShares.GetSelectedIndex();
		TCHAR buf[BUFLEN];
		FakeSharesDlg dlg;
		ctrlFakeShares.GetItemText(sel, 0, buf, BUFLEN);
		dlg.value = Text::fromT(buf);
		ctrlFakeShares.GetItemText(sel, 1, buf, BUFLEN);
		dlg.type = Text::fromT(buf);

		if(dlg.DoModal() == IDOK) {
			ctrlFakeShares.SetItemText(sel, 0, Text::toT(dlg.value).c_str());
			ctrlFakeShares.SetItemText(sel, 1, Text::toT(dlg.type).c_str());
		}
	}
	return 0;
}

LRESULT FakeSharesPage::onRemoveFakeShare(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlFakeShares.GetSelectedCount() == 1) {
		ctrlFakeShares.DeleteItem(ctrlFakeShares.GetNextItem(-1, LVNI_SELECTED));
	}
	return 0;
}

void FakeSharesPage::write() {
	int it = ctrlFakeShares.GetItemCount();
#define BUFLEN 256
	TCHAR buf[BUFLEN];
	string value;
	FakeShares::List& ul = ClientProfileManager::getInstance()->getFakeShares();
	ul.clear();
	for(int i = 0; i < it; ++i) {
		ctrlFakeShares.GetItemText(i, 0, buf, BUFLEN);
		value = Text::fromT(buf);
		ctrlFakeShares.GetItemText(i, 1, buf, BUFLEN);
		bool exact = true;
		if ( (Text::fromT(buf)).compare(STRING(CONTAINS)) == 0 )
			exact = false;
		ul.push_back(FakeShares(value, exact));
	}
	ClientProfileManager::getInstance()->saveFakeShares();
}

LRESULT FakeSharesPage::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
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
/**
 * @file
 * $Id: ClientsPage.cpp,v 1.0 2004/11/26 18:16 Virus27 Exp $
 */