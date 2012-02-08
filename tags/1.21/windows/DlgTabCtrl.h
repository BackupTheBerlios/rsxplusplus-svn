#ifndef DLG_TAB_CTRL_H
#define DLG_TAB_CTRL_H

/////////////////////////////////////////////////////////////////////////////
// Tab controls with embedded views
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2002 Bjarke Viksoe.
//
// Add the following macro to the parent's message map:
//   REFLECT_NOTIFICATIONS()
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

/////////////////////////////////////////////////////////////////////////////
// CDlgContainerCtrl - A container for Dialog views

#ifndef TCN_INITIALIZE
 #define TCN_INITIALIZE TCN_FIRST-10
 #define TCN_INSERTITEM TCN_FIRST-11
 #define TCN_DELETEITEM TCN_FIRST-12
#endif // TCN_INITIALIZE

#ifndef ETDT_ENABLETAB
 #define ETDT_ENABLETAB 6
#endif

template<class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CDlgContainerImpl : public CWindowImpl<T, TBase, TWinTraits> {
public:
	CSimpleValArray<HWND> m_aViews;
	HWND m_hWndClient;
	int m_iCurPos;

	CDlgContainerImpl() : m_iCurPos(-1), m_hWndClient(NULL) { }

	BOOL SubclassWindow(HWND hWnd) {
		ATLASSERT(m_hWnd == NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = CWindowImpl< T, TBase, TWinTraits >::SubclassWindow(hWnd);
		if(bRet)
			_Init();
		return bRet;
	}

	BOOL PreTranslateMessage(MSG* pMsg) {
		if(m_hWndClient) {
			TCHAR szClassName[8] = { 0 };
			::GetClassName(m_hWndClient, szClassName, 7);
			if(::lstrcmp(_T("#32770"), szClassName) == 0 ) 
				return ::IsDialogMessage(m_hWndClient, pMsg);
		}
		return FALSE;
	}

	int AddItem(HWND hWnd) {
		ATLASSERT(::IsWindow(hWnd));
		m_aViews.Add(hWnd);
		CWindow wnd = hWnd;
		wnd.ShowWindow(SW_HIDE);
		wnd.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
		NMHDR nmh = { m_hWnd, GetDlgCtrlID(), TCN_INSERTITEM };
		::SendMessage(GetParent(), WM_NOTIFY, nmh.idFrom, (LPARAM) &nmh);
		return m_aViews.GetSize() - 1;
	}

	BOOL RemoveItem(HWND hWnd) {
		ATLASSERT(::IsWindow(hWnd));
		int iPos = m_aViews.Find(hWnd);
		return RemoveItem(iPos);
	}

	BOOL RemoveItem(int iPos) {
		if(iPos < 0 || iPos >= m_aViews.GetSize())
			return NULL;

		NMHDR nmh = {m_hWnd, GetDlgCtrlID(), TCN_DELETEITEM};
		::SendMessage(GetParent(), WM_NOTIFY, nmh.idFrom, (LPARAM) &nmh);
		return m_aViews.RemoveAt(iPos);
	}

	HWND GetItem(int iPos) const {
		if(iPos < 0 || iPos >= m_aViews.GetSize())
			return NULL;
		return m_aViews[iPos];
	}

	int GetItemCount() const {
		return m_aViews.GetSize();
	}

	BOOL SetCurSel(int iPos) {
		if(iPos < 0 || iPos >= m_aViews.GetSize())
			return FALSE;
		if(iPos == m_iCurPos)
			return TRUE;

		NMHDR nmh = { m_hWnd, GetDlgCtrlID(), TCN_SELCHANGING };
		LRESULT lRes = ::SendMessage(GetParent(), WM_NOTIFY, nmh.idFrom, (LPARAM) &nmh);
		if(lRes != 0)
			return FALSE;

		HWND hWndActive = GetItem(m_iCurPos), hWndNew = GetItem(iPos);
		m_iCurPos = iPos;      
		m_hWndClient = hWndNew;

		if(hWndActive) 
			::SetWindowPos(hWndActive, NULL, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_HIDEWINDOW);

		BOOL bDummy;
		OnSize(0, 0, 0, bDummy);
		if(hWndNew) 
			::SetWindowPos(hWndNew, NULL, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		
		if(hWndActive == NULL) {
			SetFocus(); 
		} else if(IsChild(::GetFocus())) {
			::SetFocus(::GetWindow(hWndNew, GW_CHILD));
		}

		nmh.code = TCN_SELCHANGE;
		::SendMessage(GetParent(), WM_NOTIFY, nmh.idFrom, (LPARAM) &nmh);
		return TRUE;
	}

	int GetCurSel() const {
		return m_iCurPos;
	}

	BEGIN_MSG_MAP(CDlgContainerImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_CLIENT_COMMANDS()
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		_Init();
		return 0;
	}

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return TRUE; // View fills entire client area
	}

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		if(m_hWndClient == NULL)
			return 0;
		HWND hWndChild = ::GetNextDlgTabItem(m_hWndClient, NULL, FALSE);
		if(hWndChild)
			::SetFocus(hWndChild);
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		if(m_hWndClient == NULL)
			return 0;
		RECT rc;
		GetClientRect(&rc);
		::SetWindowPos(m_hWndClient, NULL, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE | SWP_NOZORDER);
		return 0;
	}

	void _Init() {
		ModifyStyleEx(0, WS_EX_CONTROLPARENT);
		NMHDR nmh = { m_hWnd, GetDlgCtrlID(), TCN_INITIALIZE };
		::SendMessage(GetParent(), WM_NOTIFY, nmh.idFrom, (LPARAM) &nmh);
	}
};

class CDlgContainerCtrl : public CDlgContainerImpl<CDlgContainerCtrl> {
public:
	DECLARE_WND_CLASS(_T("WTL_DlgContainer"))
};

/////////////////////////////////////////////////////////////////////////////
// CDialogTabCtrl - A Tab like control with active pages/dialogs

template< class T, class TBase = CTabCtrl, class TWinTraits = CControlWinTraits >
class ATL_NO_VTABLE CDialogTabImpl : public CWindowImpl<T, TBase, TWinTraits> {
public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	CDlgContainerCtrl m_ctrlViews;
	HWND m_hWndClient;

	// Operations
	BOOL SubclassWindow(HWND hWnd) {
		ATLASSERT(m_hWnd == NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = CWindowImpl< T, TBase, TWinTraits >::SubclassWindow(hWnd);
		if(bRet)
			_Init();
		return bRet;
	}

	BOOL InsertItem(int nItem, LPTCITEM pItem, HWND hWnd, bool enableXpStyle = false) {
		ATLASSERT(nItem==GetItemCount());
		NMHDR nmh = { m_hWnd, GetDlgCtrlID(), TCN_INSERTITEM };
		::SendMessage(GetParent(), WM_NOTIFY, nmh.idFrom, (LPARAM) &nmh);
		::SetParent(hWnd, m_hWndClient);
		m_ctrlViews.AddItem(hWnd);
		BOOL bRes = (int) TBase::InsertItem(nItem, pItem) != -1;
		BOOL bDummy;
		OnSize(WM_SIZE, 0, 0, bDummy);
		if(enableXpStyle)
			EnableXPWindowStyle(hWnd, ETDT_ENABLETAB);
		return bRes;
	}

	BOOL AddTab(LPWSTR tbName, HWND wnd, int imgIndex = -1, bool xpStyle = true) {
	   	TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT | TCIF_IMAGE;
		tci.pszText = tbName;
		tci.iImage = imgIndex;
		return InsertItem(m_ctrlViews.GetItemCount(), &tci, wnd, xpStyle);
	}

	BOOL DeleteItem(int nItem) {
		NMHDR nmh = { m_hWnd, GetDlgCtrlID(), TCN_DELETEITEM };
		::SendMessage(GetParent(), WM_NOTIFY, nmh.idFrom, (LPARAM) &nmh);
		m_ctrlViews.RemoveItem(nItem);
		return TBase::DeleteItem(nItem);
	}

	BOOL DeleteAllItems() {
		while(m_ctrlViews.GetItemCount() > 0)
			m_ctrlViews.RemoveItem(0);
		return TBase::DeleteAllItems();
	}

	int SetCurSel(int iTab) {
		int iLastTab = TBase::SetCurSel(iTab);
		if(iLastTab != -1) {
			NMHDR nmh = {m_hWnd, GetDlgCtrlID(), TCN_SELCHANGE};
			::SendMessage(GetParent(), WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
		}
		return iLastTab;
	}

	HWND GetContainer() const {
		return m_hWndClient;
	}

	HRESULT EnableXPWindowStyle(HWND hWnd, DWORD dwFlags) {
		HINSTANCE hDll;
		HRESULT Hr = HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED);
		if((hDll = ::LoadLibrary(_T("uxtheme.dll"))) != NULL) {
			typedef HRESULT (WINAPI * ENABLETHEMEDIALOGTEXTURE)(HWND, DWORD);
			ENABLETHEMEDIALOGTEXTURE pfnETDT;
			if((pfnETDT = (ENABLETHEMEDIALOGTEXTURE)::GetProcAddress(hDll, "EnableThemeDialogTexture")) != NULL) {
				Hr = pfnETDT(hWnd, dwFlags);
			}
			::FreeLibrary(hDll);
		}
		return Hr;
	}

	void SetImageList(HIMAGELIST hImageList) {
		ATLASSERT(::IsWindow(m_hWnd));
		TabCtrl_SetImageList(m_hWnd, hImageList);
	}

	BEGIN_MSG_MAP(CDialogTabImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		REFLECTED_NOTIFY_CODE_HANDLER(TCN_SELCHANGE, OnTabSelect)
		CHAIN_CLIENT_COMMANDS()
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		LRESULT lRes = DefWindowProc();
		_Init();
		return lRes;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		if(m_hWndClient == NULL)
			return 0;
		RECT rc;
		GetClientRect(&rc);
		AdjustRect(FALSE, &rc);
		::SetWindowPos(m_hWndClient, NULL, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE | SWP_NOZORDER);
		return 0;
	}

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CDCHandle dc((HDC)wParam);
		RECT rc;
		GetClientRect(&rc);
		CRgn rgn1, rgn2, rgn;
		rgn1.CreateRectRgnIndirect(&rc);
		AdjustRect(FALSE, &rc);
		rgn2.CreateRectRgnIndirect(&rc);
		rgn.CreateRectRgnIndirect(&rc);
		rgn.CombineRgn(rgn1, rgn2, RGN_DIFF);
		dc.FillRgn(rgn, ::GetSysColorBrush(COLOR_BTNFACE));
		return TRUE;
	}

	LRESULT OnTabSelect(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled) {
		int iIndex = GetCurSel();
		m_ctrlViews.SetCurSel(iIndex);
		bHandled = FALSE;
		return 0;
	}

	void _Init() {
		m_hWndClient = m_ctrlViews.Create(m_hWnd, rcDefault);
		ATLASSERT(::IsWindow(m_hWndClient));
		ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	}
};

class CDialogTabCtrl : public CDialogTabImpl<CDialogTabCtrl> {
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_DialogTabCtrl"), GetWndClassName())
};

#endif //DLG_TAB_CTRL_H
