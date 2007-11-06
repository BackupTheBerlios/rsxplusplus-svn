#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "ISPDlg.h"
#include "../rsx/IpManager.h"

#include "../rsx/RegExpHandler.h"
#include "../rsx/RsxUtil.h"

#define GET_TEXT(id, var) \
	GetDlgItemText(id, buf, 256); \
	var = Text::fromT(buf);

#define ATTACH(id, var) var.Attach(GetDlgItem(id))

LRESULT ISPDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATTACH(IDC_RANGE, ctrlRange);
	ATTACH(IDC_ISP, ctrlISP);

	ctrlRange.SetWindowText(Text::toT(range).c_str());
	ctrlISP.SetWindowText(Text::toT(ISP).c_str());
	::CheckDlgButton(*this, IDC_BAD_ISP, bad ? BST_CHECKED : BST_UNCHECKED);

	ctrlRange.SetFocus();
	
	CenterWindow(GetParent());
	return FALSE;
}

LRESULT ISPDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		TCHAR buf[256];
		string r, i;			
		GET_TEXT(IDC_ISP, i);
		GetDlgItemText(IDC_RANGE, buf, 256);
		TCHAR *b = buf, *f = buf, c;
		while( (c = *b++) != 0 ) {
			if(c != ' ') {
				*f++ = c;
			}
		}
		*f = '\0';
		r = Text::fromT(buf);

		if(RegexpHandler::isIp(r)) {
			if(i.empty()) {
				MessageBox(CTSTRING(ISPDLG_ENTER_ISP), CTSTRING(ERRORS), MB_ICONSTOP);
			} else {
				uint32_t low = RsxUtil::getLowerRange(r);
				uint32_t high = RsxUtil::getUpperRange(r);
				if(low > high) {
					MessageBox(CTSTRING(ISPDLG_LOWER_THEN_FIRST), CTSTRING(ERRORS), MB_ICONSTOP);
					return 0;
				}
				Isp::List& il = IpManager::getInstance()->getIspList(); 
				for(Isp::Iter j = il.begin(); j != il.end(); ++j) {
					if( (j->second->getLower() <= low && j->first >= low) || (j->second->getLower() <= high && j->first >= high) ) {
						if(range.compare(RsxUtil::toIP(j->second->getLower()) + '-' + RsxUtil::toIP(j->first)) != 0) {
							tstring msg = Text::toT(STRING(ISPDLG_RANGE) + " " + STRING(ISPDLG_CONFLICT) + RsxUtil::toIP(j->second->getLower()) + '-' + RsxUtil::toIP(j->first) + " " + STRING(ISP) + ": " + j->second->getIsp());
							MessageBox(msg.c_str(), CTSTRING(ERRORS), MB_ICONSTOP);
							return 0;
						}
					}
				}
				range = r;
				ISP = i;
				CButton btn = ::GetDlgItem(m_hWnd, IDC_BAD_ISP);
				bad = RsxUtil::toBool(btn.GetCheck());
				EndDialog(wID);
			}
		} else {
			MessageBox(CTSTRING(ISPDLG_INVAILD), CTSTRING(ERRORS), MB_ICONSTOP);
		}
	} else {
		EndDialog(wID);
	}
	return 0;
}