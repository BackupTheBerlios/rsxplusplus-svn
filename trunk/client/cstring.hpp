/*
 * Copyright (C) 2007-2009 adrian_007, adrian-007 on o2 point pl
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

#ifndef RSXPLUSPLUS_CSTRING_HPP
#define RSXPLUSPLUS_CSTRING_HPP

// simple class to copy c++ string to plain c char array and free it when it's not used

namespace dcpp {
class cstring {
public:
	cstring(const std::string& s) : buf(NULL) {
		allocBuf(s);
	};
	cstring(const std::wstring& s) : buf(NULL) {
		allocBuf(Text::fromT(s));
	};

	~cstring() {
		if(buf != NULL) {
			std::free(buf);
			buf = NULL;
		}
	};

	char* data() {
		return reinterpret_cast<char*>(buf);
	}
private:
	void allocBuf(const std::string& s) {
		buf = std::malloc(sizeof(char) * s.length() + 1);
		std::memset(buf, 0, s.length()+1);
		std::memcpy(buf, s.data(), s.length());
	}

	void* buf;
};
} // namespace dcpp
#endif // RSXPLUSPLUS_CSTRING_HPP