#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "CDMDebugFrame.h"
#include "WinUtil.h"
#include "../client/File.h"

#define MAX_TEXT_LEN 131072

LRESULT CDMDebugFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ctrlPad.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE);
	
	EDITSTREAM es;
	es.dwCookie = (DWORD)this;
	es.pfnCallback = reinterpret_cast<EDITSTREAMCALLBACK>(funWithWin32CallBacks);
	ctrlPad.StreamIn(SF_RTF, es);
	ctrlPad.LimitText(0);
	ctrlPad.SetFont(WinUtil::font);
	ctrlPad.SetBackgroundColor(WinUtil::bgColor);

	CHARFORMAT textFormat;
	textFormat.dwMask = CFM_COLOR;
	textFormat.dwEffects = 0;
	textFormat.crTextColor = WinUtil::textColor;
	ctrlPad.SetDefaultCharFormat(textFormat);

	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	statusContainer.SubclassWindow(ctrlStatus.m_hWnd);

	ctrlClear.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | BS_PUSHBUTTON, 0, IDC_CLEAR);
	ctrlClear.SetWindowText(_T("Clear"));
	ctrlClear.SetFont(WinUtil::systemFont);
	clearContainer.SubclassWindow(ctrlClear.m_hWnd);

	ctrlHubCommands.Create(ctrlStatus.m_hWnd, rcDefault, _T("Hub Commands"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_STATICEDGE);
	ctrlHubCommands.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlHubCommands.SetFont(WinUtil::systemFont);
	ctrlHubCommands.SetCheck(showHubCommands ? BST_CHECKED : BST_UNCHECKED);
	HubCommandContainer.SubclassWindow(ctrlHubCommands.m_hWnd);

	ctrlCommands.Create(ctrlStatus.m_hWnd, rcDefault, _T("Client Commands"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_STATICEDGE);
	ctrlCommands.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlCommands.SetFont(WinUtil::systemFont);
	ctrlCommands.SetCheck(showCommands ? BST_CHECKED : BST_UNCHECKED);
	commandContainer.SubclassWindow(ctrlCommands.m_hWnd);

	ctrlDetection.Create(ctrlStatus.m_hWnd, rcDefault, _T("Detection"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_STATICEDGE);
	ctrlDetection.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlDetection.SetFont(WinUtil::systemFont);
	ctrlDetection.SetCheck(showDetection ? BST_CHECKED : BST_UNCHECKED);
	detectionContainer.SubclassWindow(ctrlDetection.m_hWnd);

	ctrlFilterIp.Create(ctrlStatus.m_hWnd, rcDefault, _T("Filter"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_STATICEDGE);
	ctrlFilterIp.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlFilterIp.SetFont(WinUtil::systemFont);
	ctrlFilterIp.SetCheck(bFilterIp ? BST_CHECKED : BST_UNCHECKED);
	cFilterContainer.SubclassWindow(ctrlFilterIp.m_hWnd);
	
	ctrlFilterText.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_NOHIDESEL, WS_EX_STATICEDGE, IDC_DEBUG_FILTER_TEXT);
	ctrlFilterText.LimitText(0);
	ctrlFilterText.SetFont(WinUtil::font);
	eFilterContainer.SubclassWindow(ctrlStatus.m_hWnd);
	
	m_hWndClient = ctrlPad;
	m_hMenu = WinUtil::mainMenu;

	start();
	DebugManager::getInstance()->addListener(this);
		
	bHandled = FALSE;
	return 1;
}

LRESULT CDMDebugFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closed) {
		DebugManager::getInstance()->removeListener(this);
		
		closed = true;
		stop = true;
		s.signal();

		PostMessage(WM_CLOSE);
	} else {
		bHandled = FALSE;
	}
	return 0;
}

void CDMDebugFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */)
{
	RECT rect = { 0 };
	GetClientRect(&rect);

	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[7];
		ctrlStatus.GetClientRect(sr);

		//int clearButtonWidth = 50;
		int tmp = ((sr.Width() - 50) / 6) - 4;
		w[0] = 50;
		w[1] = w[0] + tmp;
		w[2] = w[1] + tmp;
		w[3] = w[2] + tmp;
		w[4] = w[3] + tmp;
		w[5] = w[4] + tmp;
		w[6] = w[5] + tmp;
		
		ctrlStatus.SetParts(7, w);

		ctrlStatus.GetRect(0, sr);
		ctrlClear.MoveWindow(sr);
		ctrlStatus.GetRect(1, sr);
		ctrlCommands.MoveWindow(sr);
		ctrlStatus.GetRect(2, sr);
		ctrlHubCommands.MoveWindow(sr);
		ctrlStatus.GetRect(3, sr);
		ctrlDetection.MoveWindow(sr);
		ctrlStatus.GetRect(4, sr);
		ctrlFilterIp.MoveWindow(sr);
		ctrlStatus.GetRect(5, sr);
		ctrlFilterText.MoveWindow(sr);
		tstring msg;
		if(bFilterIp)
			msg = Text::toT("Watching IP: ") + sFilterIp;
		else
			msg = _T("Watching all IPs");
		ctrlStatus.SetText(6, msg.c_str());
	}
	
	// resize client window
	if(m_hWndClient != NULL)
		::SetWindowPos(m_hWndClient, NULL, rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			SWP_NOZORDER | SWP_NOACTIVATE);
}

void CDMDebugFrame::addLine(const string& aLine) {
	//bcdc++ rip :]
	bool noscroll = !scrollbarAtBottom();

	if(ctrlPad.GetWindowTextLength() > 25000) {
		int trimChars = ctrlPad.LineIndex(ctrlPad.LineFromChar(2000));
		CHARRANGE oldpos;
		ctrlPad.GetSel(oldpos);
		ctrlPad.SetRedraw(FALSE);
		ctrlPad.SetSel(0, trimChars);
		ctrlPad.ReplaceSel(_T(""));
		ctrlPad.SetRedraw(TRUE);
		if(noscroll) {
			oldpos.cpMin -= trimChars;
			oldpos.cpMax -= trimChars;
			if(oldpos.cpMin < 0) {
				oldpos.cpMax = 0;
				oldpos.cpMin = 0;
			}
			ctrlPad.SetSel(oldpos);
			ctrlPad.ScrollCaret();
		}
	}
	
	string line;
	if(BOOLSETTING(TIME_STAMPS)) {
		line = ("[" + Util::getShortTimeString() + "] " + aLine);
	} else {
		line = (aLine);
	}
	if(line.rfind("\n") == string::npos)
		line += "\n";

	currentMessage = Text::toT("{\\urtf1\\ansi\\ansicpg1252\\deff0\\plain0\n" + line + "}\n");
	
	if(noscroll)
		ctrlPad.SetRedraw(FALSE);

	EDITSTREAM es;
	es.dwCookie = (DWORD)this;
	es.pfnCallback = reinterpret_cast<EDITSTREAMCALLBACK>(funWithWin32CallBacks);
	//save current state...
	CHARRANGE cr;
	int firstVisibleLine = ctrlPad.GetFirstVisibleLine();
	ctrlPad.GetSel(cr);

	ctrlPad.SetSel(-1, -1);
	ctrlPad.StreamIn(SF_RTF | SFF_SELECTION | SF_UNICODE, es);

	//restore it
	ctrlPad.SetSel(cr);

	if(noscroll) {
		ctrlPad.LineScroll(firstVisibleLine - ctrlPad.GetFirstVisibleLine());
		ctrlPad.SetRedraw(TRUE);
	} else {
		ctrlPad.SendMessage(EM_SCROLL, SB_BOTTOM, 0);
	}
	ctrlPad.InvalidateRect(NULL);
	//setDirty();
}

bool CDMDebugFrame::scrollbarAtBottom() {
	if(::IsWindow(ctrlPad.m_hWnd)) {
		SCROLLBARINFO sbi;
		sbi.cbSize = sizeof(SCROLLBARINFO);
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		ctrlPad.GetScrollInfo(SB_VERT, &si);
		GetScrollBarInfo(ctrlPad.m_hWnd, OBJID_VSCROLL, &sbi);
		if ((sbi.rgstate[0] & STATE_SYSTEM_INVISIBLE) || ((sbi.rcScrollBar.bottom - sbi.rcScrollBar.top) + si.nPos > si.nMax))
			return true;
	}
	return false;
}

LRESULT CDMDebugFrame::onClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlPad.SetWindowText(_T(""));
	ctrlPad.SetFocus();
	return 0;
}

DWORD CALLBACK CDMDebugFrame::funWithWin32CallBacks(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb) {
	CDMDebugFrame *frame = (CDMDebugFrame*)dwCookie;
	size_t msgSize = frame->currentMessage.size();

	string tmp;
	// the multibyte characters were escaped to RTF in ::addLine()
	tmp = Text::wideToAcp(frame->currentMessage);
	msgSize = tmp.size();

	if(cb <= (LONG)msgSize) {
		//unable to process the whole message.
		*pcb = cb;
		strncpy((char*)pbBuff, tmp.c_str(), *pcb);
		frame->currentMessage = frame->currentMessage.substr(cb);
	} else {
		//process whole message.
		*pcb = msgSize;
		strncpy((char*)pbBuff, tmp.c_str(), *pcb);
		frame->currentMessage = Util::emptyStringT;
	}
	return 0;
}