/* 
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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


#ifndef DCPLUSPLUS_WIN32_STDAFX_H
#define DCPLUSPLUS_WIN32_STDAFX_H

#include "../client/stdinc.h"
#include "../client/ResourceManager.h"
#include "../dht/stdafx.h"

using namespace dcpp;
using namespace dht;

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>

// Fix nt4 startup
#include <multimon.h>
#include <Shellapi.h>

#include <wx/wx.h>
#define wxUSE_GUI 1

#endif // _WIN32

#include "resource.h"

#endif

/**
 * @file
 * $Id: stdafx.h 562 2011-01-09 12:54:12Z bigmuscle $
 */
