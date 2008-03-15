/*
 * Copyright (C) 2007-2008 adrian_007, adrian-007 on o2 point pl
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

#ifndef DETECTION_ENTRY_H
#define DETECTION_ENTRY_H

#include "Flags.h"

class DetectionEntry : public Flags {
public:
	enum ClientFlag {
		GREEN		= 0x01,
		YELLOW		= 0x02,
		RED			= 0x04
	};

	DetectionEntry() : Id(-1), name(""), cheat(""), comment(""), rawToSend(0), isEnabled(true) { };
	~DetectionEntry() { };

	int Id;
	StringMap infMap;

	string name;
	string cheat;
	string comment;
	int rawToSend;
	bool isEnabled;

private:

};
#endif

/**
 * @file
 * $Id: DetectionEntry.h 61 2008-03-09 17:26:28Z adrian_007 $
 */
