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

#include "SharePage.h"

#include "../client/File.h"
#include "../client/ShareManager.h"
#include "../client/version.h"

#include <wx/textdlg.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/checkbox.h>
#include <wx/volume.h>
#include <wx/renderer.h>

#ifdef _WIN32
# include <uxtheme.h>
#endif

SettingsPage::TextItem SharePage::texts[] = {
	{ IDC_SETTINGS_SHARED_DIRECTORIES, ResourceManager::SETTINGS_SHARED_DIRECTORIES },
	{ IDC_SETTINGS_SHARE_SIZE, ResourceManager::SETTINGS_SHARE_SIZE }, 
	{ IDC_SHAREHIDDEN, ResourceManager::SETTINGS_SHARE_HIDDEN },
	{ IDC_SETTINGS_ONLY_HASHED, ResourceManager::SETTINGS_ONLY_HASHED },
	{ IDC_SETTINGS_AUTO_REFRESH_TIME, ResourceManager::SETTINGS_AUTO_REFRESH_TIME },
	{ IDC_SETTINGS_MAX_HASH_SPEED, ResourceManager::SETTINGS_MAX_HASH_SPEED },
	{ IDC_SETTINGS_MBS, ResourceManager::MBPS },	
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

SettingsPage::Item SharePage::items[] = {
	{ IDC_SHAREHIDDEN, SettingsManager::SHARE_HIDDEN, SettingsPage::T_BOOL },
	{ IDC_AUTO_REFRESH_TIME, SettingsManager::AUTO_REFRESH_TIME, SettingsPage::T_INT },
	{ IDC_MAX_HASH_SPEED, SettingsManager::MAX_HASH_SPEED, SettingsPage::T_INT },
	{ 0, 0, SettingsPage::T_END }
};

BEGIN_EVENT_TABLE(SharePage, SettingsPage)
	EVT_TREE_STATE_IMAGE_CLICK(IDC_DIRECTORIES, SharePage::OnToggleDirectory)
	EVT_TREE_ITEM_EXPANDING(IDC_DIRECTORIES, SharePage::OnDirExpanding)
	EVT_CHECKBOX(IDC_SHAREHIDDEN, SharePage::onClickedShareHidden)
END_EVENT_TABLE()

SharePage::SharePage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	// first box
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_SHARED_DIRECTORIES, wxEmptyString), wxVERTICAL);
	
	// create imagelist with native checkbox images
	wxImageList* states = new wxImageList(16, 16, true);
	
	wxMemoryDC dc;
	wxBitmap checkboxes(32, 16, 32);
	dc.SelectObject(checkboxes);
	dc.Clear();
		
	wxRendererNative& rndr = wxRendererNative::GetDefault();
	rndr.DrawCheckBox(this, dc, wxRect(0, 0, 16, 16));
	rndr.DrawCheckBox(this, dc, wxRect(16, 0, 16, 16), wxCONTROL_CHECKED);
	dc.SelectObject(wxNullBitmap);

	states->Add(checkboxes);

	// directory tree
	dirTree = new wxTreeCtrl(this, IDC_DIRECTORIES);
    dirTree->AssignStateImageList(states);
	sbSizer1->Add(dirTree, 1, wxALL | wxEXPAND, 5);

#ifdef _WIN32
	int major, minor;
	wxGetOsVersion(&major, &minor);
	if(BOOLSETTING(USE_EXPLORER_THEME) &&
		((major >= 5 && minor >= 1) //WinXP & WinSvr2003
		|| (major >= 6))) //Vista & Win7
	{
		SetWindowTheme((HWND)dirTree->GetHWND(), L"explorer", NULL);
	}
#endif

	// only hash stuffs string... align center
	wxBoxSizer* bSizer1_2 = new wxBoxSizer(wxHORIZONTAL);
	onlyHashLabel = new wxStaticText(this, IDC_SETTINGS_ONLY_HASHED, wxEmptyString);
	bSizer1_2->Add(onlyHashLabel, 1, wxALIGN_CENTER | wxEXPAND| wxALL, 0);
	sbSizer1->Add(bSizer1_2, 0, wxALIGN_CENTER  | wxALL, 5);

	// share informations
	shareSizeLabel = new wxStaticText(this, IDC_SETTINGS_SHARE_SIZE, wxEmptyString);
	totalSizeLabel = new wxStaticText(this, wxID_ANY, Util::formatBytes(ShareManager::getInstance()->getShareSize()));
	
	shareHiddenCheck = new wxCheckBox(this, IDC_SHAREHIDDEN, wxEmptyString);

	wxBoxSizer* bSizer1_3 = new wxBoxSizer(wxHORIZONTAL);
	bSizer1_3->Add(shareSizeLabel);
	bSizer1_3->Add(totalSizeLabel, 0, wxLEFT, 5);
	bSizer1_3->Add(shareHiddenCheck, 0, wxLEFT , 15);
	
	sbSizer1->Add(bSizer1_3, 0, wxALL, 5 );

	bSizer1->Add(sbSizer1, 1, wxEXPAND | wxALL, 5);
	
	// second box
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxEmptyString), wxVERTICAL);
	
	refreshLabel = new wxStaticText(this, IDC_SETTINGS_AUTO_REFRESH_TIME, wxEmptyString);
	maxHashLabel = new wxStaticText(this, IDC_SETTINGS_MAX_HASH_SPEED, wxEmptyString);
	maxHashSpeedLabel = new wxStaticText(this, IDC_SETTINGS_MBS, wxEmptyString);
	refreshSpin = new wxSpinCtrl(this, IDC_AUTO_REFRESH_TIME, wxEmptyString, wxDefaultPosition, wxSize(55, -1));
	refreshSpin->SetRange(0, 3000);
	
	maxHashSpin = new wxSpinCtrl(this, IDC_MAX_HASH_SPEED, wxEmptyString, wxDefaultPosition, wxSize(55, -1));
	maxHashSpin->SetRange(0,999);
	
	wxBoxSizer *bSizer2_0 = new wxBoxSizer(wxHORIZONTAL);
	
	wxBoxSizer *bSizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	bSizer2_1->Add(refreshLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL |wxRIGHT, 5 );
	bSizer2_1->Add(refreshSpin, 0, wxALIGN_CENTER, 0);
	bSizer2_1->Add(new wxStaticText(this, -1, "m"), 0, wxALIGN_CENTER | wxLEFT, 5);
	bSizer2_0->Add(bSizer2_1, 1, wxALIGN_RIGHT | wxEXPAND, 5);

	wxBoxSizer *bSizer2_2 = new wxBoxSizer(wxHORIZONTAL);
	bSizer2_2->Add(maxHashLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL |wxRIGHT, 5);
	bSizer2_2->Add(maxHashSpin, 0, wxALIGN_CENTER, 0);
	bSizer2_2->Add(maxHashSpeedLabel, 0, wxALIGN_CENTER | wxLEFT, 5);
	bSizer2_0->Add(bSizer2_2, 1, wxALIGN_RIGHT | wxEXPAND, 5);

	sbSizer2->Add(bSizer2_0, 0, wxALL | wxEXPAND, 5);
	
	bSizer1->Add(sbSizer2, 0, wxALL | wxEXPAND, 5);

	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	SettingsPage::translate(texts);
	SettingsPage::read(items);

	fillDirTree();
}

void SharePage::fillDirTree()
{
	dirTree->Freeze();

	dirTree->DeleteAllItems();

	// fill directory tree - now Windows only is 100% correct
	// wxwidgets\src\generic\dirctrlg.cpp - wxGetAvailableDrives() - could it be used here?
#ifdef __WXMSW__
	wxArrayString volumes = wxFSVolume::GetVolumes();
#else
	wxArrayString volumes;
	volumes.Add(wxT(PATH_SEPARATOR));
#endif

	wxImageList* icons = new wxImageList(16, 16);
	icons->Add(wxIcon(wxT("IDI_HUB")));	// use hub icon for Computer item
	icons->Add(wxBitmap(wxT("IDB_FOLDERS")));	// TODO: use native folder icon (opened/closed)

	dirTree->AssignImageList(icons);

	wxTreeItemId root = dirTree->AddRoot(_("Computer"), 0, 0);
	for(size_t i = 0; i < volumes.Count(); ++i)
	{
#ifdef __WXMSW__
		wxFSVolume vol(volumes[i]);
		wxString volumeName = vol.GetDisplayName();
		
		// FIXME: take care of duplicite icons?
		int iconIndex = icons->Add(vol.GetIcon(wxFS_VOL_ICO_SMALL));
		int iconSelIndex = icons->Add(vol.GetIcon(wxFS_VOL_ICO_SEL_SMALL));
#else
		wxString volumeName = volumes[i];
		int iconIndex = 0;
		int iconSelIndex = 1;
#endif
		wxTreeItemId current = dirTree->AppendItem(root, volumeName, iconIndex, iconSelIndex, new TreeData(volumes[i]));

		bool shared = ShareManager::getInstance()->shareFolder(Text::fromT(volumes[i]), true);
		dirTree->SetItemState(current, shared);
		if(!shared)
			dirTree->SetItemBold(current, hasSharedChildren(Text::fromT(volumes[i])));
	}

	dirTree->Expand(root);

	dirTree->Thaw();
}

void SharePage::fillDirectory(const wxTreeItemId& parent, const wxString& name)
{
	 wxLogNull logNo;	// don't log errors for a while

	// don't recurse, because it could be slow as hell! add subfolders on demand
	wxDir dir(name);
	if(!dir.IsOpened())
		return;
		
	wxString subDirName;
	bool cont = dir.GetFirst(&subDirName, wxEmptyString, wxDIR_DIRS | (shareHiddenCheck->IsChecked() ? wxDIR_HIDDEN : 0));
	while (cont)
	{
		wxString path = name + subDirName + wxT(PATH_SEPARATOR);
		wxTreeItemId dirItem = dirTree->AppendItem(parent, subDirName, 1, 2, new TreeData(path));

		// grey hidden directories
		FileFindIter ff = FileFindIter(Text::fromT(name + subDirName));
		if (ff != FileFindIter() && ff->isHidden()) 
		{
			dirTree->SetItemTextColour(dirItem, wxColour(128, 128, 128));
		}

		bool shared = ShareManager::getInstance()->shareFolder(Text::fromT(path), true);
		dirTree->SetItemState(dirItem, shared);
		if(!shared)
			dirTree->SetItemBold(dirItem, hasSharedChildren(Text::fromT(path)));

		cont = dir.GetNext(&subDirName);
	}
}

SharePage::~SharePage()
{
}

void SharePage::write()
{
	SettingsPage::write(items);

	// TODO: only when something changed
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true);
}

void SharePage::OnDirExpanding(wxTreeEvent& event)
{
	wxTreeItemIdValue cookie;
	for(wxTreeItemId child = dirTree->GetFirstChild(event.GetItem(), cookie); child.IsOk(); child = dirTree->GetNextSibling(child)) 
	{
		dirTree->DeleteChildren(child);

		wxString path = ((TreeData*)dirTree->GetItemData(child))->dir;
		fillDirectory(child, path);

		if(dirTree->IsExpanded(child))
		{
			wxTreeEvent evt;
			evt.SetItem(child);
			OnDirExpanding(evt);
		}
	}
}

void SharePage::OnToggleDirectory(wxTreeEvent& event)
{
	// we have only 2 checkbox states, so next state will be reversed
	dirTree->SetItemState(event.GetItem(), wxTREE_ITEMSTATE_NEXT);

	string path = Text::fromT(((TreeData*)dirTree->GetItemData(event.GetItem()))->dir);

	if(dirTree->GetItemState(event.GetItem()) == 1)	// share
	{
		// if parent is shared just remove exclusion
		// if parent is not shared but grandparent is then share parent but add all its subfolders to exclusions
		// if no parent (and no grandparent...) is shared just add new directory to share
		if(hasSharedParent(event.GetItem()))
		{
			shareParent(event.GetItem());
		}
		else
		{
			// add new directory
			try 
			{
				wxTextEntryDialog virt(this, TSTRING(VIRTUAL_NAME_LONG), TSTRING(VIRTUAL_NAME), 
					Text::toT(ShareManager::getInstance()->validateVirtual(Util::getLastDir(path))));

				if(virt.ShowModal() == wxID_OK) 
				{
					ShareManager::getInstance()->addDirectory(path, Text::fromT(virt.GetValue()));
					
					// when this is parent of already shared item, it can't be bold
					dirTree->SetItemBold(event.GetItem(), false);
				}
				else // sharing cancelled so uncheck
					dirTree->SetItemState(event.GetItem(), wxTREE_ITEMSTATE_NEXT);
			} 
			catch(const ShareException& e) 
			{
				wxMessageBox(Text::toT(e.getError()), wxT(APPNAME) wxT(" ") wxT(VERSIONSTRING), wxICON_STOP | wxOK);
			}
		}

		checkChildren(event.GetItem(), true);
	}
	else	// unshare
	{
		// remove directory
		if(!hasSharedParent(event.GetItem()))
		{
			// parent is not shared, remove folder completely
			ShareManager::getInstance()->removeExcludeFolder(path);
			ShareManager::getInstance()->removeDirectory(path);
		}
		else
		{
			ShareManager::getInstance()->addExcludeFolder(path);
		}

		checkChildren(event.GetItem(), false);
	}

	boldParents(event.GetItem());
	totalSizeLabel->SetLabel(Util::formatBytesW(ShareManager::getInstance()->getShareSize()));
}

void SharePage::checkChildren(const wxTreeItemId& item, bool check)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child = dirTree->GetFirstChild(item, cookie);

	while(child.IsOk())
	{
		dirTree->SetItemState(child, check ? 1 : 0);

		checkChildren(child, check);
		child = dirTree->GetNextChild(child, cookie);
	}
}

void SharePage::boldParents(const wxTreeItemId& item)
{
	wxTreeItemId parent = dirTree->GetItemParent(item);
	if(!parent.IsOk() || parent == dirTree->GetRootItem())
		return;

	wxString path = ((TreeData*)dirTree->GetItemData(parent))->dir;
	bool bold = hasSharedChildren(Text::fromT(path));
	
	if(dirTree->IsBold(parent) == bold)
		return;	// parent already in requested state, so grandparent should be too

	dirTree->SetItemBold(parent, bold);

	boldParents(parent);
}

bool SharePage::hasSharedChildren(const string& path)
{
	// find whether this item has shared children
	StringPairList dirs = ShareManager::getInstance()->getDirectories();
	for(StringPairIter i = dirs.begin(); i != dirs.end(); ++i)
	{
		if(i->second.size() > path.size() && strnicmp(path, i->second, path.size()) == 0)
		{
			return true;
		}
	}

	return false;
}

bool SharePage::hasSharedParent(const wxTreeItemId& item)
{
	wxTreeItemId parent = dirTree->GetItemParent(item);
	while(parent.IsOk())
	{
		if(dirTree->GetItemState(parent) == 1)
		{
			return true;
		}

		parent = dirTree->GetItemParent(parent);
	}

	return false;
}

void SharePage::shareParent(const wxTreeItemId& item)
{
	wxTreeItemId parent = dirTree->GetItemParent(item);
	if(dirTree->GetItemState(parent) == 0)
	{
		// FIXME: there's problem that parent's files are shared too
		dirTree->SetItemState(parent, wxTREE_ITEMSTATE_NEXT);

		const wxString& path = ((TreeData*)dirTree->GetItemData(parent))->dir;
		ShareManager::getInstance()->removeExcludeFolder(Text::fromT(path));

		shareParent(parent);

		// add all subfolders into exclude list
		wxTreeItemIdValue cookie;
		wxTreeItemId child = dirTree->GetFirstChild(item, cookie);

		while(child.IsOk())
		{
			if(dirTree->GetItemState(child) == 0 && child != item)
			{
				const wxString& path = ((TreeData*)dirTree->GetItemData(item))->dir;
				ShareManager::getInstance()->removeExcludeFolder(Text::fromT(path));
			}
			child = dirTree->GetNextChild(child, cookie);
		}
	}
	else
	{
		const wxString& path = ((TreeData*)dirTree->GetItemData(item))->dir;
		ShareManager::getInstance()->removeExcludeFolder(Text::fromT(path));
	}
}

void SharePage::onClickedShareHidden(wxCommandEvent& WXUNUSED(event))
{
	// Save the checkbox state so that ShareManager knows to include/exclude hidden files
	SettingsManager::getInstance()->set(SettingsManager::SHARE_HIDDEN, shareHiddenCheck->IsChecked());
	
	// Refresh the share. This is a blocking refresh. Might cause problems?
	// Hopefully people won't click the checkbox enough for it to be an issue. :-)
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true, false, true);	// FIXME: to block or not to block?

	// refresh tree to display hidden directories
	fillDirTree();

	totalSizeLabel->SetLabel(Util::formatBytesW(ShareManager::getInstance()->getShareSize()));
}

