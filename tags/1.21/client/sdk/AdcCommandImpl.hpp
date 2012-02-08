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

#ifndef DCPP_INTERFACE_ADC_COMMAND_IMPL_HPP
#define DCPP_INTERFACE_ADC_COMMAND_IMPL_HPP

#include "interfaces/AdcCommand.hpp"
#include "../Pointer.h"
#include "../AdcCommand.h"
#include "StringImpl.hpp"

namespace dcpp {
class AdcCommandImpl : public intrusive_ptr_base<AdcCommandImpl>, public interfaces::AdcCommand {
public:
	AdcCommandImpl(uint32_t aCmd) : cmd(aCmd) { }
	AdcCommandImpl(uint32_t aCmd, const uint32_t aTarget, char aType) : cmd(aCmd, aTarget, aType) { }
	AdcCommandImpl(const char* str, bool nmdc) : cmd(str, nmdc) { }
	AdcCommandImpl(const dcpp::AdcCommand& c) : cmd(c) { }
	~AdcCommandImpl() { }
	
	uint32_t getCommand() const { return cmd.getCommand(); }
	char getType() const { return cmd.getType(); }
	void setType(char t) { cmd.setType(t); }

	interfaces::string* getFourCC() { return new StringImpl(cmd.getFourCC()); }
	interfaces::AdcCommand* setFeatures(const char* feat) { cmd.setFeatures(feat); return this; }
	interfaces::AdcCommand* addParam(const char* name) { cmd.addParam(name); return this; }
	interfaces::AdcCommand* addParam(const char* name, const char* value) { cmd.addParam(name, value); return this; }
	dcpp::interfaces::string* getParam(const char* name, int n = 0) {
		std::string ret;
		if(cmd.getParam(name, n, ret))
			return new StringImpl(ret);
		return 0;
	}

	const char* getParam(int n) { return cmd.getParam(n).c_str(); }
	size_t getParamsCount() { return cmd.getParameters().size(); }
	bool hasFlag(const char* name, size_t start) const { return cmd.hasFlag(name, start); }

	uint32_t getTo() const { return cmd.getTo(); }
	interfaces::AdcCommand* setTo(const uint32_t sid) { cmd.setTo(sid); return this; }
	uint32_t getFrom() const { return cmd.getFrom(); }
	void setFrom(const uint32_t sid) { cmd.setFrom(sid); }

	bool parse(const char* command, bool nmdc) {
		try {
			cmd.parse(command, nmdc);
			return true;
		} catch(...) {
			// ...
		}
		return false;
	}

	interfaces::AdcCommand* copy() {
		return new AdcCommandImpl(this->cmd);
	}

	void refIncrement() { this->inc(); }
	void refDecrement() { this->dec(); }
	bool isUnique() { return this->unique(); }

	dcpp::AdcCommand& getAdcCommand() { return cmd; }

private:
	dcpp::AdcCommand cmd;
};

} // namespace dcpp

#endif // DCPP_INTERFACE_ADC_COMMAND_IMPL_HPP
