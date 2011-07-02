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

# ifdef _DEBUG
#  ifndef _WIN64
#   pragma comment(lib, "wxbase29ud.lib")
#   pragma comment(lib, "wxmsw29ud_adv.lib")
#   pragma comment(lib, "wxmsw29ud_aui.lib")
#   pragma comment(lib, "wxmsw29ud_core.lib")
#   pragma comment(lib, "wxmsw29ud_html.lib")
#   pragma comment(lib, "wxmsw29ud_richtext.lib")
#   pragma comment(lib, "wxbase29ud_xml.lib")
#   pragma comment(lib, "wxexpatd.lib")
#  else

#  endif
# else
#  ifndef _WIN64
#   pragma comment(lib, "wxbase29u.lib")
#   pragma comment(lib, "wxmsw29u_adv.lib")
#   pragma comment(lib, "wxmsw29u_aui.lib")
#   pragma comment(lib, "wxmsw29u_core.lib")
#   pragma comment(lib, "wxmsw29u_html.lib")
#   pragma comment(lib, "wxmsw29u_richtext.lib")
#   pragma comment(lib, "wxbase29u_xml.lib")
#   pragma comment(lib, "wxexpat.lib")
#  else

#  endif
# endif

/**
 * @file
 * $Id: stdafx.cpp 562 2011-01-09 12:54:12Z bigmuscle $
 */
