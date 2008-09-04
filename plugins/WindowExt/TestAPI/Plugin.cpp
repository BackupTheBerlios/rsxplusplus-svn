/*
 * Copyright (C) 2007 adrian_007, adrian-007 on o2 point pl
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
#include <dwmapi.h>

#include <ClientInterface.h>
#include <UserInterface.h>

#include "Plugin.h"
#include "version.h"

/*
  #define ATL_IDW_TOOLBAR               0xE800  // main Toolbar for window
  #define ATL_IDW_STATUS_BAR            0xE801  // Status bar window
  #define ATL_IDW_COMMAND_BAR           0xE802  // Command bar window
*/

CSnapWindow snapHandler;
WNDPROC lpfnOldWndProc;

HRESULT EnableBlurBehind(HWND hwnd) {
   HRESULT hr = S_OK;

   // Create and populate the BlurBehind structure.
   DWM_BLURBEHIND bb = { 0 };
   // Disable Blur Behind and Blur Region.
   bb.dwFlags = DWM_BB_ENABLE;
   bb.fEnable = true;
   bb.hRgnBlur = NULL;

   // Disable Blur Behind.
   hr = DwmEnableBlurBehindWindow(hwnd, &bb);
   if (SUCCEEDED(hr))
   {
      //do more things
   }
   return hr;
}

bool tbbState = false;

LRESULT CALLBACK SubClassFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_MOVING:
			return snapHandler.OnSnapMoving(hwnd, message, wParam, lParam);
		case WM_ENTERSIZEMOVE:
			return snapHandler.OnSnapEnterSizeMove(hwnd, message, wParam, lParam);
		/*case WM_COMMAND:
			{
				if(LOWORD(wParam) == 1462) {
					CallWindowProc(lpfnOldWndProc, hwnd, message, wParam, lParam);
					PluginAPI::logMessage("UpdateDialog");
					HWND wnd;
					PluginAPI::getMainWnd(wnd);
					HWND wndDlg = ::GetDlgItem(wnd, 1462);

					if(wndDlg == 0)
						PluginAPI::logMessage("dlg = 0");
					if(EnableBlurBehind(wndDlg) == S_OK) {
						PluginAPI::logMessage("S_OK");
					}
					return 0;
				}
			}*/
	}
	return CallWindowProc(lpfnOldWndProc, hwnd, message, wParam, lParam);
}

Plugin::Plugin() {
	PluginAPI::getMainWnd(r_hwnd);

	//SetWindowLongPtr(r_hwnd, GWL_EXSTYLE, GetWindowLongPtr(r_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	//::SetLayeredWindowAttributes(r_hwnd, RGB(200, 201, 202), 0, LWA_COLORKEY);

	lpfnOldWndProc = (WNDPROC)SetWindowLongPtr(r_hwnd, GWLP_WNDPROC, (LONG_PTR)SubClassFunc);
	PluginAPI::setSetting(PLUGIN_ID, "topmost", "");
}

Plugin::~Plugin() {
	SetWindowLongPtr(r_hwnd, GWLP_WNDPROC, (LONG_PTR)lpfnOldWndProc);
}

void Plugin::toggleTopmost() {
	HWND hWnd;
	PluginAPI::getMainWnd(hWnd);

	RECT rc;
	::GetWindowRect(hWnd, &rc);
	HWND order = (::GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) ? HWND_NOTOPMOST : HWND_TOPMOST;
	::SetWindowPos(hWnd, order, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
}

void Plugin::onMainWndEvent(int code, HWND hWnd) {
	switch(code) {
		case 0: // toolbar click
			{
				rString s(PluginAPI::getSetting(PLUGIN_ID, "topmost"));
				::SendMessage(hWnd, TB_CHECKBUTTON, (WPARAM)PLUGIN_ID, (BOOL)s.empty());
				PluginAPI::setSetting(PLUGIN_ID, "topmost", s.empty() ? "1" : "");
				toggleTopmost();
			}
	}
}

/**
 * @file
 * $Id: Plugin.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */