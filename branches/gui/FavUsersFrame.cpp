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
#include "FavUsersFrame.h"

#include <wx/splitter.h>

BEGIN_EVENT_TABLE(FavUsersFrame, BaseType)
	EVT_CLOSE(FavUsersFrame::OnClose)
END_EVENT_TABLE()

int FavUsersFrame::columnIndexes[] = { COLUMN_AUTOGRANT, COLUMN_NICK, COLUMN_HUB, COLUMN_SEEN, COLUMN_DESCRIPTION };
int FavUsersFrame::columnSizes[] = { 24, 200, 300, 150, 200 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_GRANT, ResourceManager::NICK, ResourceManager::LAST_HUB, ResourceManager::LAST_SEEN, ResourceManager::DESCRIPTION };
 
FavUsersFrame::FavUsersFrame(void) : BaseType(ID_CMD_FAVUSERS, _("Favorite / Ignored users"), wxColour(0, 0, 0), wxT("IDI_USERS"))
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxSP_3D | wxSP_LIVE_UPDATE);

	wxPanel* favPanel = new wxPanel(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER | wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	wxStaticBoxSizer* favBox = new wxStaticBoxSizer(wxVERTICAL, favPanel, _("Favorite users"));
	favUsers = createDataView(favBox->GetStaticBox());
	favBox->Add(favUsers, 1, wxEXPAND | wxALL, 5);
	favPanel->SetSizerAndFit(favBox);

	wxPanel* ignPanel = new wxPanel(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER | wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	wxStaticBoxSizer* ignBox = new wxStaticBoxSizer(wxVERTICAL, ignPanel, _("Ignored users"));
	ignoredUsers = createDataView(ignBox->GetStaticBox());
	ignBox->Add(ignoredUsers, 1, wxEXPAND | wxALL, 5);
	ignPanel->SetSizerAndFit(ignBox);

	splitter->SplitHorizontally(favPanel, ignPanel);
	splitter->SetMinimumPaneSize(1);

	sizer->Add(splitter, 1, wxEXPAND);
	SetSizerAndFit(sizer);

	FavoriteManager::getInstance()->addListener(this);

	// fill favorite users
	FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
	for(FavoriteManager::FavoriteMap::const_iterator i = ul.begin(); i != ul.end(); ++i) 
	{
		addUser(i->second);
	}

	// fill ignored users
	ul = FavoriteManager::getInstance()->getIgnoredUsers();
	for(FavoriteManager::FavoriteMap::const_iterator i = ul.begin(); i != ul.end(); ++i) 
	{
		addUser(i->second);
	}
}

FavUsersFrame::~FavUsersFrame(void)
{
}

void FavUsersFrame::OnClose(wxCloseEvent& event)
{
	FavoriteManager::getInstance()->removeListener(this);

	for(int i = 0; i < favUsers->getItemCount(); ++i) 
	{
		delete favUsers->getItemData(i);
	}

	WinUtil::setButtonPressed(GetId(), false);
	event.Skip();
}

void FavUsersFrame::UpdateLayout()
{
}

FavUsersFrame::UserList* FavUsersFrame::createDataView(wxWindow* parent) const
{
	UserList* list = new UserList(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	list->SetFont(WinUtil::font);
	list->SetBackgroundColour(WinUtil::bgColor);
	list->SetForegroundColour(WinUtil::textColor);

	for(uint8_t i = 0; i < COLUMN_LAST; i++) 
	{
		wxDataViewRenderer* rndr = (i == 0) ? new wxDataViewToggleRenderer(wxT("bool"), wxDATAVIEW_CELL_ACTIVATABLE) : NULL;
		list->InsertColumn(i, CTSTRING_I(columnNames[i]), wxALIGN_LEFT, columnSizes[i], rndr);
	}

	list->setSortColumn(COLUMN_NICK);
	return list;
}

void FavUsersFrame::UserInfo::update(const FavoriteUser& u) 
{
	columns[COLUMN_NICK] = Text::toT(u.getNick());
	columns[COLUMN_HUB] = user->isOnline() ? WinUtil::getHubNames(u.getUser(), Util::emptyString).first : Text::toT(u.getUrl());
	columns[COLUMN_SEEN] = user->isOnline() ? TSTRING(ONLINE) : Text::toT(Util::formatTime("%Y-%m-%d %H:%M", u.getLastSeen()));
	columns[COLUMN_DESCRIPTION] = Text::toT(u.getDescription());
}


void FavUsersFrame::addUser(const FavoriteUser& aUser) 
{
	UserList* list = aUser.isSet(FavoriteUser::FLAG_IGNORED) ? ignoredUsers : favUsers;

	int i = list->insertItem(new UserInfo(aUser));
	if(!aUser.isSet(FavoriteUser::FLAG_IGNORED))
		list->check(i, aUser.isSet(FavoriteUser::FLAG_GRANTSLOT));

	updateUser(aUser.getUser(), aUser.isSet(FavoriteUser::FLAG_IGNORED));
}

void FavUsersFrame::updateUser(const UserPtr& aUser, bool ignored) 
{
	UserList* list = ignored ? ignoredUsers : favUsers;
	for(int i = 0; i < list->getItemCount(); ++i) 
	{
		UserInfo *ui = list->getItemData(i);
		if(ui->user == aUser) 
		{
			ui->columns[COLUMN_SEEN] = aUser->isOnline() ? TSTRING(ONLINE) : Text::toT(Util::formatTime("%Y-%m-%d %H:%M", FavoriteManager::getInstance()->getLastSeen(aUser)));
			list->updateItem(i);
		}
	}
}

void FavUsersFrame::removeUser(const FavoriteUser& aUser) 
{
	UserList* list = aUser.isSet(FavoriteUser::FLAG_IGNORED) ? ignoredUsers : favUsers;
	for(int i = 0; i < list->getItemCount(); ++i) 
	{
		const UserInfo *ui = list->getItemData(i);
		if(ui->user == aUser.getUser()) 
		{
			list->deleteItem(i);
			delete ui;
			return;
		}
	}
}