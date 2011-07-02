/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
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
 
#include <wx/stackwalk.h>

#ifndef _EXTENDEDTRACE_H
#define _EXTENDEDTRACE_H

class TextStackWalker : public wxStackWalker
{
public:

	TextStackWalker(string* _stackString) : stackString(_stackString) { }

private:
	
	string*	stackString;

	void OnStackFrame(const wxStackFrame& frame);
};

#endif	// _EXTENDEDTRACE_H

/**
 * @file
 * $Id: ExtendedTrace.h 540 2010-07-29 21:31:57Z bigmuscle $
 */
