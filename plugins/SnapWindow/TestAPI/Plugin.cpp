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

#include <ClientInterface.h>
#include <UserInterface.h>

#include "Plugin.h"
#include "version.h"

CSnapWindow snapHandler;
WNDPROC lpfnOldWndProc;

LRESULT CALLBACK SubClassFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_MOVING:
			return snapHandler.OnSnapMoving(hwnd, message, wParam, lParam);
		case WM_ENTERSIZEMOVE:
			return snapHandler.OnSnapEnterSizeMove(hwnd, message, wParam, lParam);
	}
	return CallWindowProc(lpfnOldWndProc, hwnd, message, wParam, lParam);
}

Plugin::Plugin() {
	PluginAPI::getMainWnd(r_hwnd);
	lpfnOldWndProc = (WNDPROC)SetWindowLong(r_hwnd, GWL_WNDPROC, (LONG)SubClassFunc);
}

Plugin::~Plugin() {
	SetWindowLong(r_hwnd, GWL_WNDPROC, (DWORD)lpfnOldWndProc);
}

/**
 * @file
 * $Id: Plugin.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */