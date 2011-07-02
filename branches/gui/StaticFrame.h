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

#ifndef _STATICFRAME_H
#define _STATICFRAME_H

#include "MDIChildFrame.h"
#include "WinUtil.h"

template<class T>
class StaticFrame :
	public MDIChildFrame
{
public:

	static void openWindow()
	{
		if(frame == NULL)
		{
			MDIFreezer freezer;

			frame = new T();
			frame->Show();
			WinUtil::setButtonPressed(frame->GetId(), true);
		}
		else
		{
			if(WinUtil::mainWindow->GetActiveChild() == frame)
			{
				frame->Close();
			}
			else
			{
				frame->Activate();
				WinUtil::setButtonPressed(frame->GetId(), true);
			}
		}
	}

private:

	static T* frame;

protected:

	StaticFrame(wxWindowID id, const wxString& title, const wxColour& colour = wxColour(0, 0, 0), 
		const wxString& iconResource = wxT(""), const wxString& stateIconResource = wxT("")) : 
		MDIChildFrame(id, title, colour, iconResource, stateIconResource)
	{
	}

	~StaticFrame()
	{
		frame = NULL;
	}
};

template<class T>
T* StaticFrame<T>::frame = NULL;

#endif	// _STATICFRAME_H