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

#ifndef DCPP_INTERFACE_IDENTITY_HPP
#define DCPP_INTERFACE_IDENTITY_HPP

namespace dcpp {
	namespace interfaces {
		class string;

		class Identity {
		public:
			// identity fields managment
			virtual const char* getField(const char* name) const = 0;
			virtual void setField(const char* name, const char* value) = 0;
			virtual bool isField(const char* name) = 0;

			virtual int64_t getBytesShared() const = 0;
			virtual void setOp(bool op) = 0;
			virtual void setHub(bool hub) = 0;
			virtual void setBot(bool bot) = 0;
			virtual void setHidden(bool hidden) = 0;
			virtual bool isHub() const = 0;
			virtual bool isOp() const = 0;
			virtual bool isRegistered() const = 0;
			virtual bool isHidden() const = 0;
			virtual bool isBot() const = 0;
			virtual bool isAway() const = 0;
			virtual bool isTcpActive() const = 0;
			virtual bool isUdpActive() const = 0;
		};
	} // namespace interfaces
} // namespace dcpp

#endif

