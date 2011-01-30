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

#ifndef STDINC_HPP
#define STDINC_HPP

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#include "sdk/Core.hpp"
#include "sdk/PluginInfo.hpp"
#include "sdk/Hub.hpp"
#include "sdk/User.hpp"
#include "sdk/Identity.hpp"
#include "sdk/ChatMessage.hpp"
#include "sdk/UserConnection.hpp"
#include "sdk/AdcCommand.hpp"
#include "sdk/string.hpp"

extern dcpp::interfaces::Core* core;

#endif
