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

#ifndef _FAVUSERSFRAME_H
#define _FAVUSERSFRAME_H

#include "StaticFrame.h"
#include "TypedDataViewCtrl.h"

#include "../client/FavoriteManager.h"

class FavUsersFrame :
	public StaticFrame<FavUsersFrame>,
	private FavoriteManagerListener
{
public:

	typedef StaticFrame<FavUsersFrame> BaseType;

	FavUsersFrame(void);
	~FavUsersFrame(void);

private:

	enum 
	{
		COLUMN_FIRST,
		COLUMN_AUTOGRANT = COLUMN_FIRST,
		COLUMN_NICK,
		COLUMN_HUB,
		COLUMN_SEEN,
		COLUMN_DESCRIPTION,
		COLUMN_LAST
	};

	class UserInfo : public UserInfoBase 
	{
	public:
		UserInfo(const FavoriteUser& u) : user(u.getUser()) 
		{ 
			update(u);
		}

		wxString getText(int col) const { return columns[col]; }

		static int compareItems(const UserInfo* a, const UserInfo* b, int col) 
		{
			return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
		}
		
		int getImageIndex() const { return 2; }

		void remove() { FavoriteManager::getInstance()->removeFavoriteUser(getUser()); }

		void update(const FavoriteUser& u);

		tstring columns[COLUMN_LAST];

		const UserPtr& getUser() const { return user; }
		UserPtr user;
	};

	DECLARE_EVENT_TABLE();

	// events
	void OnClose(wxCloseEvent& event);

	// methods
	void UpdateLayout();
	
	typedef TypedDataViewCtrl<UserInfo> UserList;
	UserList* createDataView(wxWindow* parent) const;

	UserList* favUsers;
	UserList* ignoredUsers;

	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	// FavoriteManagerListener
	void on(UserAdded, const FavoriteUser& aUser) throw() { addUser(aUser);/*callAsync(std::bind(&FavUsersFrame::addUser, this, aUser));*/ }
	void on(UserRemoved, const FavoriteUser& aUser) throw() { callAsync(std::bind(&FavUsersFrame::removeUser, this, aUser)); }
	void on(StatusChanged, const UserPtr& aUser) throw() 
	{ 
		callAsync(std::bind(&FavUsersFrame::updateUser, this, aUser, false)); 
		callAsync(std::bind(&FavUsersFrame::updateUser, this, aUser, true)); 
	}

	void addUser(const FavoriteUser& aUser);
	void updateUser(const UserPtr& aUser, bool ignored);
	void removeUser(const FavoriteUser& aUser);
};

#endif	// _FAVUSERSFRAME_H