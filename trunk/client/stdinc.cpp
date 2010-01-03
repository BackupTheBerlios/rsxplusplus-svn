/* 
 * Copyright (C) 2001-2009 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdinc.h"

#if defined(_STLPORT_VERSION)
# if (_STLPORT_VERSION >= 0x600)
#  pragma message("Using STLport 6.x.x GIT")
# elif (_STLPORT_VERSION >= 0x520)
#  pragma message("Using STLport 5.2.x")
# else
#  error You are using too old version of STLport, please update it
# endif
#else
# pragma message("Using default STL implementation")
#endif

#ifdef _STLP_NO_IOSTREAMS
#error You must use compiled STLPort else you can't use optimized node allocator.
#endif

#if _VC80X || _VC90X
#error You can't compile RSX++ under Visual C++ Express Edition!
#endif

/**
 * @file
 * $Id: stdinc.cpp 455 2009-08-16 16:25:59Z BigMuscle $
 */
