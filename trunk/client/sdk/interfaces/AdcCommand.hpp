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

#ifndef DCPP_INTERFACE_ADC_COMMAND_HPP
#define DCPP_INTERFACE_ADC_COMMAND_HPP

#include "RefInterface.hpp"

namespace dcpp {
	namespace interfaces {
		namespace AdcCommandTypes {
			static const char Broadcast = 'B';
			static const char Client = 'C';
			static const char Direct = 'D';
			static const char Echo = 'E';
			static const char Feature = 'F';
			static const char Info = 'I';
			static const char Hub = 'H';
			static const char UDP = 'U';

			static const uint32_t HubSID = 0xffffffff;

#if defined(_WIN32) || defined(__i386__) || defined(__x86_64__) || defined(__alpha)
#define C(n, a, b, c) static const uint32_t CMD_##n = (((uint32_t)a) | (((uint32_t)b)<<8) | (((uint32_t)c)<<16));
#else
#define C(n, a, b, c) static const uint32_t CMD_##n = ((((uint32_t)a)<<24) | (((uint32_t)b)<<16) | (((uint32_t)c)<<8));
#endif
			// Base commands
			C(SUP, 'S','U','P');
			C(STA, 'S','T','A');
			C(INF, 'I','N','F');
			C(MSG, 'M','S','G');
			C(SCH, 'S','C','H');
			C(RES, 'R','E','S');
			C(CTM, 'C','T','M');
			C(RCM, 'R','C','M');
			C(GPA, 'G','P','A');
			C(PAS, 'P','A','S');
			C(QUI, 'Q','U','I');
			C(GET, 'G','E','T');
			C(GFI, 'G','F','I');
			C(SND, 'S','N','D');
			C(SID, 'S','I','D');
			// Extensions
			C(CMD, 'C','M','D');
			C(NAT, 'N','A','T');
			C(RNT, 'R','N','T');
			C(PSR, 'P','S','R');
			C(PUB, 'P','U','B');
#undef C
		}

		class string;
		class AdcCommand {
		public:
			//virtual ~AdcCommand() = 0;

			virtual uint32_t getCommand() const = 0;
			virtual char getType() const = 0;
			virtual void setType(char t) = 0;

			virtual dcpp::interfaces::string* getFourCC() = 0;
			virtual dcpp::interfaces::AdcCommand* setFeatures(const char* feat) = 0;
			virtual dcpp::interfaces::AdcCommand* addParam(const char* name) = 0;
			virtual dcpp::interfaces::AdcCommand* addParam(const char* name, const char* value) = 0;
			virtual dcpp::interfaces::string* getParam(size_t n) = 0;
			virtual size_t getParamsCount() = 0;
			virtual bool hasFlag(const char* name, size_t start) const = 0;

			virtual uint32_t getTo() const = 0;
			virtual dcpp::interfaces::AdcCommand* setTo(const uint32_t sid) = 0;
			virtual uint32_t getFrom() const = 0;
			virtual void setFrom(const uint32_t sid) = 0;

			virtual bool parse(const char* cmd, bool nmdc) = 0;

			virtual dcpp::interfaces::AdcCommand* copy() = 0;

			virtual void refIncrement() = 0;
			virtual void refDecrement() = 0;
			virtual bool isUnique() = 0;
		};

		typedef RefInterface<dcpp::interfaces::AdcCommand> RefAdcCommand;
	} // namespace interfaces
} // namespace dcpp

#endif
