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

#define APPNAME "RSX++"
#define VERSIONSTRING "1.00"
#define VERSIONFLOAT 1.003

#define DCVERSIONSTRING "0.704"
#define DCVERSIONFLOAT 0.7042

#define SVNVERSION "svn360"

// RSX++
#define SVN_REVISION $revision

#define SVN_REVISION_STR "$revision"

#define __HOMESITE "http://rsxplusplus.sf.net/"
#define __WIKI "http://rsxplusplus.wiki.sourceforge.net/"
#define __FORUM "http://rsxplusplus.sf.net/forum/"
#define VERSION_URL "http://rsxplusplus.sf.net/version.xml"

#ifdef SVN_REVISION_STR
#define EXTRA_VER _T(" SVN:") _T(SVN_REVISION_STR) _T(" [") _T(DCVERSIONSTRING) _T("/") _T(SVNVERSION) _T("]")
#endif

/* Update the .rc file as well... */
