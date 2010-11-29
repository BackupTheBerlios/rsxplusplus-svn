/*
 * Copyright (C) 2007-2010 adrian_007, adrian-007 on o2 point pl
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

#ifndef DCPP_INTERFACE_STRING_IMPL_HPP
#define DCPP_INTERFACE_STRING_IMPL_HPP

#include "interfaces/string.hpp"
#include "../Pointer.h"

namespace dcpp {
class StringImpl : public intrusive_ptr_base<StringImpl>, public interfaces::string {
public:
	StringImpl() { }
	StringImpl(const std::string& buf) {
		s = buf;
	}
	StringImpl(const char* buf) {
		s = buf;
	}
	~StringImpl() { }

	const char* get() { 
		return s.c_str(); 
	}
	void set(const char* str) { 
		s = str; 
	}
	void append(const char* str)  { 
		s += str; 
	}
	size_t length() { 
		return s.length(); 
	}
	char at(size_t index) { 
		if(index < s.size()) 
			return s[index]; 
		return -1; 
	}
	size_t compare(size_t start, size_t end, const char* str) { 
		return s.compare(start, end, str); 
	}
	void erase(size_t pos, size_t n) {
		s.erase(pos, n); 
	}
	size_t find(const char* str, size_t pos = 0) {
		return s.find(str, pos);
	}
	size_t rfind(const char* str, size_t pos = 0) {
		return s.rfind(str, pos);
	}
	void replace(size_t pos, size_t n, const char* str) {
		s.replace(pos, n, str);
	}

	interfaces::string* copy() {
		return new StringImpl(this->s);
	}

	void refIncrement() { this->inc(); }
	void refDecrement() { this->dec(); }
	bool isUnique() { return this->unique(); }

	std::string& getString() { return s; }
private:
	std::string s;
};

class StringListImpl : public intrusive_ptr_base<StringListImpl>, public interfaces::stringList {
public:
	StringListImpl(size_t size) {
		if(size > 0)
			list.resize(size);
	}
	StringListImpl(const StringList& l) {
		list = l;
	}
	~StringListImpl() { }

	const char* at(size_t index) {
		if(index < list.size()) {
			return list[index].c_str();
		}
		return 0;
	}
	void set(size_t index, const char* str) {
		if(index < list.size()) {
			list[index] = str;
		}
	}
	void append(const char* str) {
		list.push_back(str);
	}
	size_t size() { 
		return list.size();
	}
	bool exist(const char* str) {
		return std::find(list.begin(), list.end(), std::string(str)) != list.end();
	}

	interfaces::stringList* copy() {
		return new StringListImpl(this->list);
	}

	void refIncrement() { this->inc(); }
	void refDecrement() { this->dec(); }
	bool isUnique() { return this->unique(); }

	StringList& getList() { return list; }
private:
	StringList list;
};

class StringMapImpl : public intrusive_ptr_base<StringMapImpl>, public interfaces::stringMap {
public:
	StringMapImpl() { }
	StringMapImpl(const StringMap& m) {
		map = m;
	}
	~StringMapImpl() { }

	const char* get(const char* key) {
		StringMap::iterator i = map.find(key);
		if(i == map.end())
			return 0;
		return i->second.c_str();
	}
	void set(const char* key, const char* str) {
		StringMap::iterator i = map.find(key);
		if(i != map.end()) {
			i->second = str;
		} else {
			map.insert(std::make_pair(key, str));
		}
	}
	size_t size() {
		return map.size();
	}
	bool exist(const char* key) {
		return map.find(key) != map.end();
	}

	interfaces::stringMap* copy() {
		return new StringMapImpl(this->map);
	}

	void refIncrement() { this->inc(); }
	void refDecrement() { this->dec(); }
	bool isUnique() { return this->unique(); }

	StringMap& getMap() { return map; }
private:
	StringMap map;
};

}

#endif
