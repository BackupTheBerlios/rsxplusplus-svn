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

#include "stdafx.h"
#include "SearchSpyFrame.h"

BEGIN_EVENT_TABLE(SearchSpyFrame, BaseType)
	EVT_CLOSE(SearchSpyFrame::OnClose)
END_EVENT_TABLE()

SearchSpyFrame::SearchSpyFrame(void) : BaseType(ID_CMD_SEARCH_SPY, CTSTRING(SEARCH_SPY), wxColour(0, 0, 0), wxT("IDI_SPY"))
{
}

SearchSpyFrame::~SearchSpyFrame(void)
{
}

void SearchSpyFrame::OnClose(wxCloseEvent& event)
{
	WinUtil::setButtonPressed(GetId(), false);
	event.Skip();
}

void SearchSpyFrame::UpdateLayout()
{
}
