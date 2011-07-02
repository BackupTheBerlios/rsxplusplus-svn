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
#include "UserInfoBaseHandler.h"

#include "HubFrame.h"
#include "PrivateFrame.h"

#include "../client/ClientManager.h"
#include "../client/LogManager.h"
#include "../client/QueueManager.h"
#include "../client/UploadManager.h"

void UserInfoBase::matchQueue(const string& hubHint) 
{
	if(getUser()) 
	{
		try 
		{
			QueueManager::getInstance()->addList(HintedUser(getUser(), hubHint), QueueItem::FLAG_MATCH_QUEUE);
		} 
		catch(const Exception& e) 
		{
			LogManager::getInstance()->message(e.getError(), LogManager::LOG_ERROR);
		}
	}
}

void UserInfoBase::doReport(const string& hubHint) 
{
	if(getUser()) 
	{
		ClientManager::getInstance()->reportUser(HintedUser(getUser(), hubHint));
	}
}

void UserInfoBase::getList(const string& hubHint) 
{
	if(getUser()) 
	{
		try 
		{
			QueueManager::getInstance()->addList(HintedUser(getUser(), hubHint), QueueItem::FLAG_CLIENT_VIEW);
		}
		catch(const Exception& e) 
		{
			LogManager::getInstance()->message(e.getError(), LogManager::LOG_ERROR);		
		}
	}
}

void UserInfoBase::browseList(const string& hubHint) 
{
	if(!getUser() || getUser()->getCID().isZero())
		return;
	try 
	{
		QueueManager::getInstance()->addList(HintedUser(getUser(), hubHint), QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_PARTIAL_LIST);
	} 
	catch(const Exception& e) 
	{
		LogManager::getInstance()->message(e.getError(), LogManager::LOG_ERROR);		
	}
}

void UserInfoBase::checkList(const string& hubHint) 
{
	if(getUser()) 
	{
		try 
		{
			QueueManager::getInstance()->addList(HintedUser(getUser(), hubHint), QueueItem::FLAG_USER_CHECK);
		} 
		catch(const Exception& e) 
		{
			LogManager::getInstance()->message(e.getError(), LogManager::LOG_ERROR);		
		}
	}
}

void UserInfoBase::addFav()
 {
	if(getUser()) 
	{
		FavoriteManager::getInstance()->addFavoriteUser(getUser());
	}
}

void UserInfoBase::pm(const string& hubHint)
{
	if(getUser()) 
	{
		PrivateFrame::openWindow(HintedUser(getUser(), hubHint), Util::emptyStringT, NULL);
	}
}

void UserInfoBase::connectFav() 
{
	if(getUser()) 
	{
		string url = FavoriteManager::getInstance()->getUserURL(getUser());
		if(!url.empty()) 
		{
			HubFrame::openWindow(Text::toT(url));
		}
	}
}

void UserInfoBase::grant(const string& hubHint) 
{
	if(getUser()) 
	{
		UploadManager::getInstance()->reserveSlot(HintedUser(getUser(), hubHint), 600);
	}
}

void UserInfoBase::removeAll() 
{
	if(getUser())
	{
		QueueManager::getInstance()->removeSource(getUser(), QueueItem::Source::FLAG_REMOVED);
	}
}

void UserInfoBase::grantHour(const string& hubHint) 
{
	if(getUser()) 
	{
		UploadManager::getInstance()->reserveSlot(HintedUser(getUser(), hubHint), 3600);
	}
}

void UserInfoBase::grantDay(const string& hubHint) 
{
	if(getUser()) 
	{
		UploadManager::getInstance()->reserveSlot(HintedUser(getUser(), hubHint), 24*3600);
	}
}

void UserInfoBase::grantWeek(const string& hubHint) 
{
	if(getUser()) 
	{
		UploadManager::getInstance()->reserveSlot(HintedUser(getUser(), hubHint), 7*24*3600);
	}
}

void UserInfoBase::ungrant() 
{
	if(getUser()) 
	{
		UploadManager::getInstance()->unreserveSlot(getUser());
	}
}
