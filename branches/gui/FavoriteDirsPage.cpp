/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
 * Copyright (C) 2010 Oyashiro-sama, oyashirosama dot hnnkni at gmail dot com
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
#include "../client/FavoriteManager.h"
#include "../client/SettingsManager.h"

#include "FavoriteDirsPage.h"

BEGIN_EVENT_TABLE(FavoriteDirsPage, wxPanel)
	EVT_LIST_ITEM_ACTIVATED(IDC_FAVORITE_DIRECTORIES, FavoriteDirsPage::onItemActivated)
	EVT_LIST_ITEM_DESELECTED(IDC_FAVORITE_DIRECTORIES, FavoriteDirsPage::onItemDeselected)
	EVT_LIST_ITEM_SELECTED(IDC_FAVORITE_DIRECTORIES, FavoriteDirsPage::onItemSelected)
	EVT_LIST_KEY_DOWN(IDC_FAVORITE_DIRECTORIES, FavoriteDirsPage::onListKeyDown)
	EVT_BUTTON(IDC_ADD, FavoriteDirsPage::onAdd)
	EVT_BUTTON(IDC_RENAME, FavoriteDirsPage::onRename)
	EVT_BUTTON(IDC_REMOVE, FavoriteDirsPage::onRemove)
END_EVENT_TABLE()

SettingsPage::TextItem FavoriteDirsPage::texts[] = {
	{ IDC_SETTINGS_FAVORITE_DIRECTORIES, ResourceManager::SETTINGS_FAVORITE_DIRS },
	{ IDC_REMOVE, ResourceManager::REMOVE },
	{ IDC_ADD, ResourceManager::SETTINGS_ADD_FOLDER },
	{ IDC_RENAME, ResourceManager::SETTINGS_RENAME_FOLDER },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

FavoriteDirsPage::FavoriteDirsPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_FAVORITE_DIRECTORIES, wxEmptyString), wxVERTICAL);

	favDirsList = new wxListCtrl(this, IDC_FAVORITE_DIRECTORIES, wxDefaultPosition, wxSize(-1,200), wxLC_REPORT);
	sbSizer1->Add(favDirsList, 0, wxALL | wxEXPAND, 5);
	favDirsList->InsertColumn(0, CTSTRING(FAVORITE_DIR_NAME), wxLIST_FORMAT_LEFT, 100);
	favDirsList->InsertColumn(1, CTSTRING(DIRECTORY), wxLIST_FORMAT_LEFT, 300);

	sbSizer1->Add(0, 10, 1, wxEXPAND, 5);

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	favDirRenButton = new wxButton(this, IDC_RENAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(favDirRenButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	favDirRemButton = new wxButton(this, IDC_REMOVE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(favDirRemButton, 0, wxBOTTOM | wxRIGHT, 5);

	favDirAddButton = new wxButton(this, IDC_ADD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(favDirAddButton, 0, wxBOTTOM | wxRIGHT, 5);

	sbSizer1->Add(bSizer2, 0, wxALIGN_RIGHT, 5);

	bSizer1->Add(sbSizer1, 0, wxALL | wxEXPAND, 5);

	this->SetSizer(bSizer1);
	this->Layout();

	StringPairList directories = FavoriteManager::getInstance()->getFavoriteDirs();
	long item;
	for(StringPairIter j = directories.begin(); j != directories.end(); j++)
	{
		item = favDirsList->InsertItem(favDirsList->GetItemCount(), Text::toT(j->second));
		favDirsList->SetItem(item, 1, Text::toT(j->first));
	}

	favDirRemButton->Enable(false);
	favDirRenButton->Enable(false);
	
	SettingsPage::translate(texts);
}

FavoriteDirsPage::~FavoriteDirsPage()
{
}

void FavoriteDirsPage::onItemActivated(wxListEvent& event)
{
	doRename(event.GetItem().GetId());
}

void FavoriteDirsPage::onItemSelected(wxListEvent &WXUNUSED(event))
{
	favDirRemButton->Enable(true);
	favDirRenButton->Enable(true);
}

void FavoriteDirsPage::onItemDeselected(wxListEvent &WXUNUSED(event))
{
	if (favDirsList->GetSelectedItemCount() == 0)
	{
		favDirRemButton->Enable(false);
		favDirRenButton->Enable(false);
	}
}

void FavoriteDirsPage::onListKeyDown(wxListEvent& event)
{
	switch (event.GetKeyCode())
	{
		case WXK_INSERT:
			doAdd();
			break;
		case WXK_DELETE:
			doRemove();
		default:
			event.Skip();
			break;
	}
}

void FavoriteDirsPage::onAdd(wxCommandEvent &WXUNUSED(event))
{
	doAdd();
}

void FavoriteDirsPage::onRename(wxCommandEvent &WXUNUSED(event))
{
	long item = favDirsList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		doRename(item);
	}
}

void FavoriteDirsPage::onRemove(wxCommandEvent &WXUNUSED(event))
{
	doRemove();
}

void FavoriteDirsPage::doAdd()
{
	wxDirDialog dlg(this);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxString path = dlg.GetPath();
		if (path.Last() != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		wxString name = wxGetTextFromUser(TSTRING(FAVORITE_DIR_NAME_LONG), TSTRING(FAVORITE_DIR_NAME),
			Util::getLastDir(string(path.mb_str())));
		if (!name.IsEmpty())
		{
			if (FavoriteManager::getInstance()->addFavoriteDir(string(path.mb_str(wxConvUTF8)), string(name.mb_str(wxConvUTF8))))
			{
				long item = favDirsList->InsertItem(favDirsList->GetItemCount(), name);
				favDirsList->SetItem(item, 1, path);
			}
			else
			{
				wxMessageBox(CTSTRING(DIRECTORY_ADD_ERROR));
			}
		}
	}
}

void FavoriteDirsPage::doRename(long item)
{
	wxString oldName = favDirsList->GetItemText(item, 0);
	wxString newName = wxGetTextFromUser(TSTRING(FAVORITE_DIR_NAME_LONG), TSTRING(FAVORITE_DIR_NAME), oldName);
	if ((!newName.IsEmpty()) && (!newName.IsSameAs(oldName)))
	{
		favDirsList->SetItem(item, 0, newName);
	}
}

void FavoriteDirsPage::doRemove()
{
	long item = favDirsList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	while (item != -1)
	{
		if(FavoriteManager::getInstance()->removeFavoriteDir(string(favDirsList->GetItemText(item, 1).mb_str(wxConvUTF8))))
			favDirsList->DeleteItem(item);
		item = favDirsList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}
}

void FavoriteDirsPage::write()
{
}
