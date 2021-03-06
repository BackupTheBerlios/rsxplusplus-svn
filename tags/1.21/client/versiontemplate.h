/* 
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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
#define VERSIONSTRING "1.21"
#define VERSIONFLOAT 1.21

#define DCVERSIONSTRING "0.762"
#define DCVERSIONFLOAT 0.762

#define SVNVERSION "svn499"

#define SVN_REVISION $WCREV$

#define RSXPP_SITE		"http://rsxplusplus.sf.net/"
#define RSXPP_TRAC		"http://sourceforge.net/apps/trac/rsxplusplus/"
#define RSXPP_FORUM		"http://sourceforge.net/apps/phpbb/rsxplusplus/"
#define RSXPP_DONATE	"http://rsxplusplus.sf.net/donate.html"
#define VERSION_URL		"http://rsxplusplus.sf.net/version.xml"

#ifdef _WIN32
# ifdef _WIN64
#  define CONFIGURATION_NAME "x86-64"
# else
#  define CONFIGURATION_NAME "x86-32"
# endif
#endif

#if defined(SVNBUILD)
# define COMPLETEVERSIONSTRING _T(APPNAME) _T(" ") _T(VERSIONSTRING) _T(" ") _T(CONFIGURATION_NAME) _T(" [SVN:") _T(BOOST_STRINGIZE(SVN_REVISION)) _T(" / ") _T(DCVERSIONSTRING) _T(" / ") _T(SVNVERSION) _T("]")
#elif defined(_DEBUG)
# define COMPLETEVERSIONSTRING _T(APPNAME) _T(" ") _T(VERSIONSTRING) _T(" ") _T(CONFIGURATION_NAME) _T(" [DEBUG SVN:") _T(BOOST_STRINGIZE(SVN_REVISION)) _T(" / ") _T(DCVERSIONSTRING) _T(" / ") _T(SVNVERSION) _T("]")
#else
# define COMPLETEVERSIONSTRING _T(APPNAME) _T(" ") _T(VERSIONSTRING) _T(" ") _T(CONFIGURATION_NAME)
#endif

/* Update the .rc file as well... */
