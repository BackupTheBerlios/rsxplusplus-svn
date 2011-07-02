/*
 * Copyright (C) 2010-2011 Big Muscle, http://strongdc.sf.net
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

#ifndef _ASYNCHANDLER_H
#define _ASYNCHANDLER_H

#include "../client/Thread.h"

#include <wx/event.h>

template<class T>
class AsyncHandler
{

private:

	// attributes
	typedef std::function<void()> Callback;
	std::list<Callback> tasks; 

	CriticalSection cs;

public:

	// public methods
	void callAsync(const Callback& func)
	{
		Lock l(cs);
		tasks.push_back(func);
		
		wxThreadEvent event;
		event.SetEventObject((T*)this);
		wxPostEvent((T*)this, event);
	}

protected:

	// constructor - accessible from derived classes only
	AsyncHandler()
	{
		((T*)this)->Bind(wxEVT_COMMAND_THREAD, &AsyncHandler<T>::OnAsync, (T*)this);
	}

	// destructor
	virtual ~AsyncHandler() { }

	// event handling
	void OnAsync(wxThreadEvent& event)
	{
		if(event.GetEventObject() != (T*)this)
		{
			event.Skip();
			return;
		}

		Callback func;
		{
			Lock l(cs);
			if(tasks.empty())
				return;

			func = tasks.front();
			tasks.pop_front();
		}

		func();
	}
};

#endif	// _ASYNCHANDLER_H