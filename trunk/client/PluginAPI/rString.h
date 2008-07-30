/*
 * Copyright (C) 2007-2008 adrian_007, adrian-007 on o2 point pl
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

#ifndef RSXPLUSPLUS_RSTRING
#define RSXPLUSPLUS_RSTRING

#ifndef EXIMP
#define EXIMP __declspec(dllimport)
#endif//EXIMP

namespace dcpp {
// scoped stringholder
class EXIMP rString {
public:
	rString(const char* str);
	rString() : buf(0) { };
	~rString();

	__cdecl operator char* () { return buf; }
	__cdecl operator const char*() const { return buf; }
	const char * __cdecl c_str() const { return buf; }
	bool __cdecl empty() const;
private:
	char* buf;
};
} // namespace dcpp

#endif // RSXPLUSPLUS_RSTRING

/**
 * @file
 * $Id$
 */
