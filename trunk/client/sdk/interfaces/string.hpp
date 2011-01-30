/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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

#ifndef DCPP_INTERFACE_STRING_HPP
#define DCPP_INTERFACE_STRING_HPP

#include "RefInterface.hpp"

namespace dcpp {
	namespace interfaces {
		class string {
		public:
			//virtual ~string() = 0;

			virtual const char* get() = 0;
			virtual void set(const char* str) = 0;
			virtual void append(const char* str) = 0;
			virtual size_t length() = 0;
			virtual char at(size_t index) = 0;
			virtual size_t compare(size_t start, size_t end, const char* str) = 0;
			virtual void erase(size_t pos, size_t n) = 0;
			virtual size_t find(const char* str, size_t pos = 0) = 0;
			virtual size_t rfind(const char* str, size_t pos = 0) = 0;
			virtual void replace(size_t pos, size_t n, const char* str) = 0;

			virtual dcpp::interfaces::string* copy() = 0;

			// memory managment
			virtual void refIncrement() = 0;
			virtual void refDecrement() = 0;
			virtual bool isUnique() = 0;
		};

		typedef RefInterface<dcpp::interfaces::string> RefString;

		class stringList {
		public:
			//virtual ~stringList() = 0;

			virtual const char* at(size_t index) = 0;
			virtual void set(size_t index, const char* str) = 0;
			virtual void append(const char* str) = 0;
			virtual size_t size() = 0;
			virtual bool exist(const char* str) = 0;

			virtual dcpp::interfaces::stringList* copy() = 0;

			// memory managment
			virtual void refIncrement() = 0;
			virtual void refDecrement() = 0;
			virtual bool isUnique() = 0;
		};

		typedef RefInterface<dcpp::interfaces::stringList> RefStringList;

		class stringMap {
		public:
			//virtual ~stringMap() = 0;

			virtual const char* get(const char* key) = 0;
			virtual void set(const char* key, const char* str) = 0;
			virtual size_t size() = 0;
			virtual bool exist(const char* key) = 0;

			virtual dcpp::interfaces::stringMap* copy() = 0;

			// memory managment
			virtual void refIncrement() = 0;
			virtual void refDecrement() = 0;
			virtual bool isUnique() = 0;
		};

		typedef RefInterface<dcpp::interfaces::stringMap> RefStringMap;

	} // namespace interfaces
} // namespace dcpp

#endif
