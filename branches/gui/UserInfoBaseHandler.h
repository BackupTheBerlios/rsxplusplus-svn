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

#ifndef _USERINFOBASEHANDLER_H
#define _USERINFOBASEHANDLER_H

#include "../client/User.h"

#include <wx/event.h>

template<class T>
class UserInfoBaseHandler
{
public:

	void OnMatchQueue(wxCommandEvent& /*event*/) 
	{
		((T*)this)->getUserList().forEachSelectedT(boost::bind(&UserInfoBase::matchQueue, _1, hubHint));
	}

	void OnGetList(wxCommandEvent& /*event*/) 
	{
		((T*)this)->getUserList().forEachSelectedT(boost::bind(&UserInfoBase::getList, _1, hubHint));
	}

	void OnBrowseList(wxCommandEvent& /*event*/) 
	{
		((T*)this)->getUserList().forEachSelectedT(boost::bind(&UserInfoBase::browseList, _1, hubHint));
	}

	void OnReport(wxCommandEvent& /*event*/)
	{
		((T*)this)->getUserList().forEachSelectedT(boost::bind(&UserInfoBase::doReport, _1, hubHint));
	}

	void OnCheckList(wxCommandEvent& /*event*/) 
	{
		((T*)this)->getUserList().forEachSelectedT(boost::bind(&UserInfoBase::checkList, _1, hubHint));
	}

	void OnAddToFavorites(wxCommandEvent& /*event*/) 
	{
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::addFav);
	}

	void OnPrivateMessage(wxCommandEvent& /*event*/) 
	{
		((T*)this)->getUserList().forEachSelectedT(boost::bind(&UserInfoBase::pm, _1, hubHint));
	}

	void OnConnectFav(wxCommandEvent& /*event*/) 
	{
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::connectFav);
	}

	void OnGrantSlot(wxCommandEvent& event)
	{
		switch(event.GetId())
		{
			case IDC_GRANTSLOT:			((T*)this)->getUserList().forEachSelectedT(boost::bind(&UserInfoBase::grant, _1, hubHint)); break;
			case IDC_GRANTSLOT_DAY:		((T*)this)->getUserList().forEachSelectedT(boost::bind(&UserInfoBase::grantDay, _1, hubHint)); break;
			case IDC_GRANTSLOT_HOUR:	((T*)this)->getUserList().forEachSelectedT(boost::bind(&UserInfoBase::grantHour, _1, hubHint)); break;
			case IDC_GRANTSLOT_WEEK:	((T*)this)->getUserList().forEachSelectedT(boost::bind(&UserInfoBase::grantWeek, _1, hubHint)); break;
			case IDC_UNGRANTSLOT:		((T*)this)->getUserList().forEachSelected(&UserInfoBase::ungrant); break;
		}
	}

	void OnRemoveAll(wxCommandEvent& /*event*/) 
	{ 
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::removeAll);
	}

	struct UserTraits 
	{
		UserTraits() : adcOnly(true), favOnly(true), nonFavOnly(true) { }
		void operator()(const UserInfoBase* ui) 
		{
			if(ui->getUser()) 
			{
				if(ui->getUser()->isSet(User::NMDC)) 
					adcOnly = false;
				bool fav = FavoriteManager::getInstance()->isFavoriteUser(ui->getUser());
				if(fav)
					nonFavOnly = false;
				if(!fav)
					favOnly = false;
			}
		}

		bool adcOnly;
		bool favOnly;
		bool nonFavOnly;
	};

	void appendUserItems(wxMenu& menu, const string& _hubHint) 
	{
		hubHint = _hubHint;
		
		UserTraits traits = ((T*)this)->getUserList().forEachSelectedT(UserTraits()); 

		menu.Append(IDC_PRIVATEMESSAGE, CTSTRING(SEND_PRIVATE_MESSAGE));
		menu.AppendSeparator();
		menu.Append(IDC_GETLIST, CTSTRING(GET_FILE_LIST));
		menu.Append(IDC_BROWSELIST, CTSTRING(BROWSE_FILE_LIST));
		menu.Append(IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
		menu.AppendSeparator();
		if(!traits.favOnly)
			menu.Append(IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
		if(!traits.nonFavOnly)
			menu.Append(IDC_CONNECT, CTSTRING(CONNECT_FAVUSER_HUB));
		menu.AppendSeparator();
		menu.Append(IDC_REMOVEALL, CTSTRING(REMOVE_FROM_ALL));
		// TODO: menu.AppendSubMenu(WinUtil::grantMenu, CTSTRING(GRANT_SLOTS_MENU));

		// set menu handlers

		((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, &UserInfoBaseHandler<T>::OnGetList, (T*)this, IDC_GETLIST);
		((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, &UserInfoBaseHandler<T>::OnBrowseList, (T*)this, IDC_BROWSELIST);
		((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, &UserInfoBaseHandler<T>::OnCheckList, (T*)this, IDC_CHECKLIST);
		((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, &UserInfoBaseHandler<T>::OnMatchQueue, (T*)this, IDC_MATCH_QUEUE);
		((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, &UserInfoBaseHandler<T>::OnPrivateMessage, (T*)this, IDC_PRIVATEMESSAGE);
		((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, &UserInfoBaseHandler<T>::OnAddToFavorites, (T*)this, IDC_ADD_TO_FAVORITES);
		((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, &UserInfoBaseHandler<T>::OnGrantSlot, (T*)this, IDC_GRANTSLOT, IDC_UNGRANTSLOT);
		((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, &UserInfoBaseHandler<T>::OnRemoveAll, (T*)this, IDC_REMOVEALL);
		((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, &UserInfoBaseHandler<T>::OnReport, (T*)this, IDC_REPORT);
		((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, &UserInfoBaseHandler<T>::OnConnectFav, (T*)this, IDC_CONNECT);
	}

	string hubHint;
};

#endif	// _USERINFOBASEHANDLER_H