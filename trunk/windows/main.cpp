/* 
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#ifdef _DEBUG
/** 
	Memory leak detector
	You can remove following 3 lines if you don't want to detect memory leaks.
	Ignore STL pseudo-leaks, we can avoid them with _STLP_LEAKS_PEDANTIC, but it only slows down everything.
 */
#define VLD_MAX_DATA_DUMP 0
#define VLD_AGGREGATE_DUPLICATES
//#include <vld.h>
#endif

#include "../client/DCPlusPlus.h"
#include "SingleInstance.h"
#include "WinUtil.h"
#include "../client/version.h"

#include "../client/MerkleTree.h"
#include "../client/Text.h"

#include "Resource.h"
#include "ExtendedTrace.h"

#include "MainFrm.h"
#include "PopupManager.h"
//RSX++
#include "ResourceLoader.h"
#include "SpellChecker.hpp"
//END

#include <delayimp.h>
CAppModule _Module;

CriticalSection cs;
enum { DEBUG_BUFSIZE = 8192 };
static char guard[DEBUG_BUFSIZE];
static int recursion = 0;
static char tth[192*8/(5*8)+2];
static bool firstException = true;

static char buf[DEBUG_BUFSIZE];

#ifndef _DEBUG

FARPROC WINAPI FailHook(unsigned /* dliNotify */, PDelayLoadInfo  pdli) {
	char buf[DEBUG_BUFSIZE];
	sprintf(buf, "RSX++ just encountered and unhandled exception and will terminate.\nPlease do not report this as a bug. The error was caused by library %s.", pdli->szDll);
	MessageBox(WinUtil::mainWnd, Text::toT(buf).c_str(), _T("RSX++ Has Crashed"), MB_OK | MB_ICONERROR);
	exit(-1);
}

#endif

#include "../client/SSLSocket.h"

string getExceptionName(DWORD code) {
	switch(code)
    { 
		case EXCEPTION_ACCESS_VIOLATION:      return "Access violation"; break; 
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:   return "Array out of range"; break; 
		case EXCEPTION_BREAKPOINT:            return "Breakpoint"; break; 
		case EXCEPTION_DATATYPE_MISALIGNMENT:   return "Read or write error"; break; 
		case EXCEPTION_FLT_DENORMAL_OPERAND:   return "Floating-point error"; break; 
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:      return "Floating-point division by zero"; break; 
		case EXCEPTION_FLT_INEXACT_RESULT:      return "Floating-point inexact result"; break; 
		case EXCEPTION_FLT_INVALID_OPERATION:   return "Unknown floating-point error"; break; 
		case EXCEPTION_FLT_OVERFLOW:         return "Floating-point overflow"; break; 
		case EXCEPTION_FLT_STACK_CHECK:         return "Floating-point operation caused stack overflow"; break; 
		case EXCEPTION_FLT_UNDERFLOW:         return "Floating-point underflow"; break; 
		case EXCEPTION_ILLEGAL_INSTRUCTION:      return "Illegal instruction"; break; 
		case EXCEPTION_IN_PAGE_ERROR:         return "Page error"; break; 
		case EXCEPTION_INT_DIVIDE_BY_ZERO:      return "Integer division by zero"; break; 
		case EXCEPTION_INT_OVERFLOW:         return "Integer overflow"; break; 
		case EXCEPTION_INVALID_DISPOSITION:      return "Invalid disposition"; break; 
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:return "Noncontinueable exception"; break; 
		case EXCEPTION_PRIV_INSTRUCTION:      return "Invalid instruction"; break; 
		case EXCEPTION_SINGLE_STEP:            return "Single step executed"; break; 
		case EXCEPTION_STACK_OVERFLOW:         return "Stack overflow"; break; 
	}
	return "";
}

LONG __stdcall DCUnhandledExceptionFilter( LPEXCEPTION_POINTERS e )
{	
	Lock l(cs);

	if(recursion++ > 30)
		exit(-1);

#ifndef _DEBUG
#if _MSC_VER == 1200
	__pfnDliFailureHook = FailHook;
#elif _MSC_VER == 1300 || _MSC_VER == 1310 || _MSC_VER == 1400|| _MSC_VER == 1500
	__pfnDliFailureHook2 = FailHook;
#else
#error Unknown Compiler version
#endif

	// The release version loads the dll and pdb:s here...
	EXTENDEDTRACEINITIALIZE( Util::getPath(Util::PATH_RESOURCES).c_str() );

#endif

	if(firstException) {
		File::deleteFile(Util::getPath(Util::PATH_RESOURCES) + "exceptioninfo.txt");
		firstException = false;
	}

	if(File::getSize(Util::getPath(Util::PATH_RESOURCES) + "RSXPlusPlus.pdb") == -1) {
		// No debug symbols, we're not interested...
		::MessageBox(WinUtil::mainWnd, _T("RSX++ has crashed and you don't have RSXPlusPlus32.pdb/RSXPlusPlus64.pdb file installed. Hence, I can't find out why it crashed, so don't report this as a bug unless you find a solution..."), _T("RSX++ has crashed"), MB_OK);
#ifndef _DEBUG
		exit(1);
#else
		return EXCEPTION_CONTINUE_SEARCH;
#endif
	}

	File f(Util::getPath(Util::PATH_RESOURCES) + "exceptioninfo.txt", File::WRITE, File::OPEN | File::CREATE);
	f.setEndPos(0);
	
	DWORD exceptionCode = e->ExceptionRecord->ExceptionCode ;

	sprintf(buf, "Code: %x\r\nVersion: %s\r\nException code: %s\r\n", 
		exceptionCode, VERSIONSTRING, getExceptionName(exceptionCode).c_str());

	f.write(buf, strlen(buf));
#ifdef SVNBUILD
	sprintf(buf, "SVN: %s\r\n", BOOST_STRINGIZE(SVN_REVISION));	
	f.write(buf, strlen(buf));
#endif	
	
	OSVERSIONINFOEX ver;
	WinUtil::getVersionInfo(ver);

	sprintf(buf, "System Version: %d %d %d SP: %d Type: %d %s\r\n",
		(DWORD)ver.dwMajorVersion, (DWORD)ver.dwMinorVersion, (DWORD)ver.dwBuildNumber,
		(DWORD)ver.wServicePackMajor, (DWORD)ver.wProductType,
#ifdef _WIN64
		"x64"
#else
		"x32"
#endif
		);

	f.write(buf, strlen(buf));
	time_t now;
	time(&now);
	strftime(buf, DEBUG_BUFSIZE, "Time: %Y-%m-%d %H:%M:%S\r\n", localtime(&now));

	f.write(buf, strlen(buf));

	f.write(LIT("TTH: "));
	f.write(tth, strlen(tth));
	f.write(LIT("\r\n"));

    f.write(LIT("\r\n"));
    
#ifndef _WIN64   
	STACKTRACE2(f, e->ContextRecord->Eip, e->ContextRecord->Esp, e->ContextRecord->Ebp);
#else
	STACKTRACE2(f, e->ContextRecord->Rip, e->ContextRecord->Rsp, e->ContextRecord->Rbp);
#endif

	f.write(LIT("\r\n"));

	f.close();

	if ((!SETTING(SOUND_EXC).empty()) && (!BOOLSETTING(SOUNDS_DISABLED)))
		PlaySound(Text::toT(SETTING(SOUND_EXC)).c_str(), NULL, SND_FILENAME | SND_ASYNC);

	NOTIFYICONDATA m_nid;
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = MainFrame::getMainFrame()->m_hWnd;
	m_nid.uID = 0;
	m_nid.uFlags = NIF_INFO;
	m_nid.uTimeout = 5000;
	m_nid.dwInfoFlags = NIIF_WARNING;
	_tcscpy(m_nid.szInfo, _T("exceptioninfo.txt was generated"));
	_tcscpy(m_nid.szInfoTitle, _T("RSX++ has crashed"));
	Shell_NotifyIcon(NIM_MODIFY, &m_nid);

	if(MessageBox(WinUtil::mainWnd, _T("RSX++ just encountered a fatal bug and should have written an exceptioninfo.txt the same directory as the executable. You can post exceptioninfo on forum. Go there now?"), _T("RSX++ Has Crashed"), MB_YESNO | MB_ICONERROR) == IDYES) {
		WinUtil::openLink(_T(RSXPP_FORUM));
	}

#ifndef _DEBUG
	EXTENDEDTRACEUNINITIALIZE();
	
	exit(-1);
#else
	return EXCEPTION_CONTINUE_SEARCH;
#endif
}

static void sendCmdLine(HWND hOther, LPTSTR lpstrCmdLine)
{
	tstring cmdLine = lpstrCmdLine;
	LRESULT result;

	COPYDATASTRUCT cpd;
	cpd.dwData = 0;
	cpd.cbData = sizeof(TCHAR)*(cmdLine.length() + 1);
	cpd.lpData = (void *)cmdLine.c_str();
	result = SendMessage(hOther, WM_COPYDATA, NULL,	(LPARAM)&cpd);
}

BOOL CALLBACK searchOtherInstance(HWND hWnd, LPARAM lParam) {
	ULONG_PTR result;
	LRESULT ok = ::SendMessageTimeout(hWnd, WMU_WHERE_ARE_YOU, 0, 0,
		SMTO_BLOCK | SMTO_ABORTIFHUNG, 5000, &result);
	if(ok == 0)
		return TRUE;
	if(result == WMU_WHERE_ARE_YOU) {
		// found it
		HWND *target = (HWND *)lParam;
		*target = hWnd;
		return FALSE;
	}
	return TRUE;
}

static void checkCommonControls() {
#define PACKVERSION(major,minor) MAKELONG(minor,major)

	HINSTANCE hinstDll;
	DWORD dwVersion = 0;
	
	hinstDll = LoadLibrary(_T("comctl32.dll"));
	
	if(hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
	
		pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");
		
		if(pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;
			
			memzero(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
			
			hr = (*pDllGetVersion)(&dvi);
			
			if(SUCCEEDED(hr))
			{
				dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			}
		}
		
		FreeLibrary(hinstDll);
	}

	if(dwVersion < PACKVERSION(5,80)) {
		MessageBox(NULL, _T("Your version of windows common controls is too old for RSX++ to run correctly, and you will most probably experience problems with the user interface. You should download version 5.80 or higher from the DC++ homepage or from Microsoft directly."), _T("User Interface Warning"), MB_OK);
	}
#undef PACKVERSION
}

static HWND hWnd;
static tstring sText;
static tstring sTitle;

LRESULT CALLBACK splashCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_PAINT) {
		// Get some information
		HDC dc = GetDC(hwnd);
		RECT rc;
		GetWindowRect(hwnd, &rc);
		OffsetRect(&rc, -rc.left, -rc.top);
		RECT rc2 = rc;
		rc2.top = rc2.bottom - 35; 
		rc2.right = rc2.right - 10;
		::SetBkMode(dc, TRANSPARENT);
		
		// Draw the icon
		ExCImage hi;
		hi.LoadFromResource(IDP_RSX_SPLASH, _T("PNG"), _Module.get_m_hInst());

		HDC comp=CreateCompatibleDC(dc);
		SelectObject(comp,hi);	

		BitBlt(dc,0, 0 , 480, 150,comp,0,0,SRCCOPY);

		DeleteObject(hi);
		DeleteObject(comp);
		LOGFONT logFont;
		HFONT hFont;
		GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(logFont), &logFont);
		lstrcpy(logFont.lfFaceName, TEXT("Tahoma"));
		logFont.lfHeight = 13;
		logFont.lfWeight = 700;
		hFont = CreateFontIndirect(&logFont);		
		SelectObject(dc, hFont);
		::SetTextColor(dc, RGB(255,255,255));
		::DrawText(dc, sTitle.c_str(), _tcslen(sTitle.c_str()), &rc2, DT_RIGHT);
		DeleteObject(hFont);

		if(!sText.empty()) {
			rc2 = rc;
			rc2.top = rc2.bottom - 15;
			rc2.right = rc2.right - 10;

			GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(logFont), &logFont);
			lstrcpy(logFont.lfFaceName, TEXT("Tahoma"));
			logFont.lfHeight = 13;
			logFont.lfWeight = 700;
			hFont = CreateFontIndirect(&logFont);		
			SelectObject(dc, hFont);
			::SetTextColor(dc, RGB(255,255,255));
			::DrawText(dc, sText.c_str(), _tcslen(sText.c_str()), &rc2, DT_RIGHT);
			DeleteObject(hFont);
		}

		ReleaseDC(hwnd, dc);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void callBack(void* x, const tstring& a) {
	sText = a;
	SendMessage((HWND)x, WM_PAINT, 0, 0);
}

static int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	checkCommonControls();

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);	

	CEdit dummy;
	CWindow splash;
	
	CRect rc;
	rc.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	rc.top = (rc.bottom / 2) - 80;

	rc.right = GetSystemMetrics(SM_CXFULLSCREEN);
	rc.left = rc.right / 2 - 85;
	
	dummy.Create(NULL, rc, _T(APPNAME) _T(" ") _T(VERSIONSTRING), WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_CENTER | ES_READONLY, WS_EX_STATICEDGE);
	splash.Create(_T("Static"), GetDesktopWindow(), splash.rcDefault, NULL, WS_POPUP | WS_VISIBLE | SS_USERITEM | WS_EX_TOOLWINDOW);
	splash.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	
	HDC dc = splash.GetDC();
	rc.right = rc.left + 480;
	rc.bottom = rc.top + 150;
	splash.ReleaseDC(dc);
	splash.HideCaret();
	splash.SetWindowPos(NULL, &rc, SWP_SHOWWINDOW);
	splash.SetWindowLongPtr(GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&splashCallback));
	splash.CenterWindow();

	sTitle = _T(VERSIONSTRING);
	
#ifndef _WIN64	
	switch (get_cpu_type())
	{
		case 2:
			sTitle += _T(" MMX");
			break;
		case 3:
			sTitle += _T(" AMD");
			break;
		case 4:
		case 5:
			sTitle += _T(" SSE");
			break;
	}
#else
	sTitle += _T(" x64");
#endif

	splash.SetFocus();
	splash.RedrawWindow();

	startup(callBack, (void*)splash.m_hWnd);
	//RSX++
	SpellChecker::newInstance();
	if(!RSXPP_SETTING(DICTIONARY).empty()) {
		callBack(splash.m_hWnd, _T("Dictionary"));
		SpellChecker::getInstance()->load();
	}
	//END
	splash.DestroyWindow();
	dummy.DestroyWindow();

	if(ResourceManager::getInstance()->isRTL()) {
		SetProcessDefaultLayout(LAYOUT_RTL);
	}

	MainFrame wndMain;

	rc = wndMain.rcDefault;

	if( (SETTING(MAIN_WINDOW_POS_X) != CW_USEDEFAULT) &&
		(SETTING(MAIN_WINDOW_POS_Y) != CW_USEDEFAULT) &&
		(SETTING(MAIN_WINDOW_SIZE_X) != CW_USEDEFAULT) &&
		(SETTING(MAIN_WINDOW_SIZE_Y) != CW_USEDEFAULT) ) {

		rc.left = SETTING(MAIN_WINDOW_POS_X);
		rc.top = SETTING(MAIN_WINDOW_POS_Y);
		rc.right = rc.left + SETTING(MAIN_WINDOW_SIZE_X);
		rc.bottom = rc.top + SETTING(MAIN_WINDOW_SIZE_Y);
		// Now, let's ensure we have sane values here...
		if( (rc.left < 0 ) || (rc.top < 0) || (rc.right - rc.left < 10) || ((rc.bottom - rc.top) < 10) ) {
			rc = wndMain.rcDefault;
		}
	}

	int rtl = ResourceManager::getInstance()->isRTL() ? WS_EX_RTLREADING : 0;
	if(wndMain.CreateEx(NULL, rc, 0, rtl | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE) == NULL) {
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}
	
	if(BOOLSETTING(MINIMIZE_ON_STARTUP)) {
		wndMain.ShowWindow(SW_SHOWMINIMIZED);
	} else {
		wndMain.ShowWindow(((nCmdShow == SW_SHOWDEFAULT) || (nCmdShow == SW_SHOWNORMAL)) ? SETTING(MAIN_WINDOW_STATE) : nCmdShow);
	}
	int nRet = theLoop.Run();
	
	_Module.RemoveMessageLoop();

	SpellChecker::deleteInstance(); //RSX++
	shutdown();

	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow) {
#ifndef _DEBUG
	SingleInstance dcapp(_T("{RSXPLUSPLUS-AEE8350A-B49A-4753-AB4B-E55479A48351}"));
#else
	SingleInstance dcapp(_T("{RSXPLUSPLUS-AEE8350A-B49A-4753-AB4B-E55479A48350}"));
#endif

	if(dcapp.IsAnotherInstanceRunning()) {
		// Allow for more than one instance...
				bool multiple = false;
		if(_tcslen(lpstrCmdLine) == 0) {
			if (::MessageBox(NULL, _T("There is already an instance of RSX++ running.\nDo you want to launch another instance anyway?"), 
				_T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_TOPMOST) == IDYES) {
					multiple = true;
			}
		}

		if(multiple == false) {
			HWND hOther = NULL;
			EnumWindows(searchOtherInstance, (LPARAM)&hOther);

			if( hOther != NULL ) {
				// pop up
				::SetForegroundWindow(hOther);

				if( IsIconic(hOther)) {
					// restore
					::ShowWindow(hOther, SW_RESTORE);
				}
				sendCmdLine(hOther, lpstrCmdLine);
			}
			return FALSE;
		}
	}
	
	
	// For SHBrowseForFolder, UPnP
	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); 
#ifdef _DEBUG
	EXTENDEDTRACEINITIALIZE(Util::getPath(Util::PATH_RESOURCES).c_str());
#endif
	LPTOP_LEVEL_EXCEPTION_FILTER pOldSEHFilter = NULL;
	pOldSEHFilter = SetUnhandledExceptionFilter(&DCUnhandledExceptionFilter);
	
	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES |
		ICC_TAB_CLASSES | ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES);	// add flags to support other controls
	
	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));
	
	try {		
		File f(WinUtil::getAppName(), File::READ, File::OPEN);
		TigerTree tth(TigerTree::calcBlockSize(f.getSize(), 1));
		size_t n = 0;
		size_t n2 = DEBUG_BUFSIZE;
		while( (n = f.read(buf, n2)) > 0) {
			tth.update(buf, n);
			n2 = DEBUG_BUFSIZE;
		}
		tth.finalize();
		strcpy(::tth, tth.getRoot().toBase32().c_str());
		WinUtil::tth = Text::toT(::tth);
	} catch(const FileException&) {
		dcdebug("Failed reading exe\n");
	}	

	HINSTANCE hInstRich = ::LoadLibrary(_T("RICHED20.DLL"));	

	int nRet = Run(lpstrCmdLine, nCmdShow);
 
	if ( hInstRich ) {
		::FreeLibrary(hInstRich);
	}
	
	// Return back old VS SEH handler
	if (pOldSEHFilter != NULL)
		SetUnhandledExceptionFilter(pOldSEHFilter);

	_Module.Term();
	::CoUninitialize();

#ifdef _DEBUG
	EXTENDEDTRACEUNINITIALIZE();
#endif

	return nRet;
}

/**
 * @file
 * $Id: main.cpp 460 2009-09-08 10:57:07Z BigMuscle $
 */
