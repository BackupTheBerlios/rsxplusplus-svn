/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
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

#ifndef _SEARCHSPYFRAME_H
#define _SEARCHSPYFRAME_H

#include "StaticFrame.h"

class SearchSpyFrame :
	public StaticFrame<SearchSpyFrame>
{
public:

	typedef StaticFrame<SearchSpyFrame> BaseType;

	SearchSpyFrame(void);
	~SearchSpyFrame(void);

private:

	DECLARE_EVENT_TABLE();

	// events
	void OnClose(wxCloseEvent& event);

	void UpdateLayout();

};

#endif	// _SEARCHSPYFRAME_H