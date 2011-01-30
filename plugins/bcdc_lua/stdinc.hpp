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
