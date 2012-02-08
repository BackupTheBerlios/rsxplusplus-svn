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

#ifndef DCPP_INTERFACE_REF_INTERFACE_HPP
#define DCPP_INTERFACE_REF_INTERFACE_HPP

namespace dcpp {
	namespace interfaces {
		template<typename T>
		class RefInterface {
		public:
			RefInterface() : obj(0) { }
			RefInterface(T* o) : obj(o) {
				if(obj)
					obj->refIncrement();
			}
			RefInterface(const RefInterface& rhs) : obj(rhs.obj) {
				if(obj)
					obj->refIncrement();
			}
			~RefInterface() {
				if(obj)
					obj->refDecrement();
			}

			inline RefInterface& operator =(T* o) {
				if(obj)
					obj->refDecrement();
				obj = o;
				if(obj)
					obj->refIncrement();
				return *this;
			}
			inline T* operator ->() {
				return obj;
			}
			inline T* get() {
				return obj;
			}
			inline bool operator == (T* o) {
				return o == obj;
			}
			inline bool operator != (T* o) {
				return o != obj;
			}
			inline bool operator == (const RefInterface<T>& ref) {
				return obj == ref.get();
			}
			inline bool operator != (const RefInterface<T>& ref) {
				return obj != ref.get();
			}
			inline operator bool() {
				return obj != 0;
			}
			inline bool operator !() {
				return obj == 0;
			}

		protected:
			T* obj;
		};
	} // namespace interfaces
} // namespace dcpp

#endif
