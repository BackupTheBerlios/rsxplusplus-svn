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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/Util.h"

#include "../client/rsxppSettingsManager.h"

#include "SpellChecker.hpp"

void SpellChecker::load() {
	string fname_t = Util::getPath(Util::PATH_USER_CONFIG) + RSXPP_SETTING(DICTIONARY);
	if(Util::fileExists(string(fname_t + ".dic")) && Util::fileExists(string(fname_t + ".aff"))) {
		lib.open(fname_t + ".dic", fname_t + ".aff");
	}
	if(!lib.isRunning()) {
		RSXPP_SET(DICTIONARY, Util::emptyString);
	}
}

/**
 * @file
 * $Id: SpellChecker.cpp 230 2011-01-30 18:15:33Z adrian_007 $
 */
