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

#ifndef DCPP_INTERFACE_USER_HPP
#define DCPP_INTERFACE_USER_HPP

#include "RefInterface.hpp"

namespace dcpp {
	namespace interfaces {
		class string;
		class Hub;
		class Identity;

		class OnlineUser {
		public:
			virtual dcpp::interfaces::Identity* getUserIdentity() = 0;
			virtual dcpp::interfaces::Hub* getHub() = 0;

			// memory managment
			virtual void refIncrement() = 0;
			virtual void refDecrement() = 0;
			virtual bool isUnique() = 0;
		};

		typedef RefInterface<dcpp::interfaces::OnlineUser> RefOnlineUser;

	} // namespace interfaces
} // namespace dcpp

#endif

