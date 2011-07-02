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

#include "PreviewPage.h"

BEGIN_EVENT_TABLE(PreviewPage, SettingsPage)
	EVT_LIST_ITEM_ACTIVATED(IDC_MENU_ITEMS, PreviewPage::onItemActivated)
	EVT_LIST_ITEM_DESELECTED(IDC_MENU_ITEMS, PreviewPage::onItemDeselected)
	EVT_LIST_ITEM_SELECTED(IDC_MENU_ITEMS, PreviewPage::onItemSelected)
	EVT_LIST_KEY_DOWN(IDC_MENU_ITEMS, PreviewPage::onListKeyDown)
	EVT_BUTTON(IDC_ADD_MENU, PreviewPage::onAdd)
	EVT_BUTTON(IDC_CHANGE_MENU, PreviewPage::onChange)
	EVT_BUTTON(IDC_REMOVE_MENU, PreviewPage::onRemove)
END_EVENT_TABLE()

SettingsPage::TextItem PreviewPage::texts[] = {
	{ IDC_ADD_MENU, ResourceManager::ADD },
	{ IDC_CHANGE_MENU, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_MENU, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PreviewPage::PreviewPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	previewList = new wxListCtrl(this, IDC_MENU_ITEMS, wxDefaultPosition, wxSize(-1,335), wxLC_REPORT | wxLC_SINGLE_SEL);
	previewList->InsertColumn(0, CTSTRING(SETTINGS_NAME), wxLIST_FORMAT_LEFT, 90);
	previewList->InsertColumn(1, CTSTRING(SETTINGS_COMMAND), wxLIST_FORMAT_LEFT, 180);
	previewList->InsertColumn(2, CTSTRING(SETTINGS_ARGUMENT), wxLIST_FORMAT_LEFT, 90);
	previewList->InsertColumn(3, CTSTRING(SETTINGS_EXTENSIONS), wxLIST_FORMAT_LEFT, 90);
	bSizer1->Add(previewList, 0, wxEXPAND | wxALL, 5);
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	previewAddButton = new wxButton(this, IDC_ADD_MENU, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(previewAddButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	previewChangeButton = new wxButton(this, IDC_CHANGE_MENU, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(previewChangeButton, 0, wxBOTTOM | wxRIGHT, 5);
	
	previewRemButton = new wxButton(this, IDC_REMOVE_MENU, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(previewRemButton, 0, wxBOTTOM | wxRIGHT, 5);
	
	bSizer1->Add(bSizer2, 0, 0, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();

	SettingsPage::translate(texts);

	PreviewApplication::List lst = FavoriteManager::getInstance()->getPreviewApps();
	for(PreviewApplication::Iter i = lst.begin(); i != lst.end(); ++i)
	{
		PreviewApplication::Ptr pa = *i;	
		addEntry(pa, previewList->GetItemCount());
	}

	if (previewList->GetSelectedItemCount() == 0)
	{
		previewChangeButton->Enable(false);
		previewRemButton->Enable(false);
	}
}

PreviewPage::~PreviewPage()
{
}

void PreviewPage::addEntry(PreviewApplication* pa, long pos)
{
	long item = previewList->InsertItem(pos, Text::toT(pa->getName()));
	previewList->SetItem(item, 1, pa->getApplication());
	previewList->SetItem(item, 2, pa->getArguments());
	previewList->SetItem(item, 3, pa->getExtension());
}

void PreviewPage::onAdd(wxCommandEvent &WXUNUSED(event))
{
	doAdd();
}

void PreviewPage::onChange(wxCommandEvent &WXUNUSED(event))
{
	doChange();
}

void PreviewPage::onRemove(wxCommandEvent &WXUNUSED(event))
{
	doRemove();
}

void PreviewPage::onItemActivated(wxListEvent &WXUNUSED(event))
{
	doChange();
}

void PreviewPage::onItemSelected(wxListEvent &WXUNUSED(event))
{
	previewChangeButton->Enable(true);
	previewRemButton->Enable(true);
}

void PreviewPage::onItemDeselected(wxListEvent &WXUNUSED(event))
{
	if (previewList->GetSelectedItemCount() == 0)
	{
		previewChangeButton->Enable(false);
		previewRemButton->Enable(false);
	}
}

void PreviewPage::onListKeyDown(wxListEvent& event)
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

void PreviewPage::doAdd()
{
	PreviewDlg dlg(this);
	dlg.setValues(wxEmptyString, wxEmptyString, _T("%[file]"), wxEmptyString);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxString *name = new wxString(), *app = new wxString(), *arg = new wxString(), *ext = new wxString();
		dlg.getValues(name, app, arg, ext);
		addEntry(FavoriteManager::getInstance()->addPreviewApp(Text::fromT(*name), Text::fromT(*app),
			Text::fromT(*arg), Text::fromT(*ext)), previewList->GetItemCount());
		wxDELETE(name);
		wxDELETE(app);
		wxDELETE(arg);
		wxDELETE(ext);
	}
}

void PreviewPage::doChange()
{
	if(previewList->GetSelectedItemCount() == 1)
	{
		long item = previewList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		PreviewApplication pa;
		FavoriteManager::getInstance()->getPreviewApp(item, pa);

		PreviewDlg dlg(this);
		dlg.setValues(pa.getName(), pa.getApplication(), pa.getArguments(), pa.getExtension());

		if(dlg.ShowModal() == wxID_OK)
		{
			wxString *name = new wxString(), *app = new wxString(), *arg = new wxString(), *ext = new wxString();
			dlg.getValues(name, app, arg, ext);
			pa.setName(*name);
			pa.setApplication(*app);
			pa.setArguments(*arg);
			pa.setExtension(*ext);

			FavoriteManager::getInstance()->updatePreviewApp(item, pa);

			previewList->SetItem(item, 0, *name);
			previewList->SetItem(item, 1, *app);
			previewList->SetItem(item, 2, *arg);
			previewList->SetItem(item, 3, *ext);

			wxDELETE(name);
			wxDELETE(app);
			wxDELETE(arg);
			wxDELETE(ext);
		}
	}
}

void PreviewPage::doRemove()
{
	if(previewList->GetSelectedItemCount() == 1)
	{
		long item = previewList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		FavoriteManager::getInstance()->removePreviewApp(item);
		previewList->DeleteItem(item);
	}
}

void PreviewPage::write()
{
}