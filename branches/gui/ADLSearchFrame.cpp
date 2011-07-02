/* 
 * Copyright (C) 2010 adrian_007 adrian-007 on o2 point pl
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
#include "ADLSearchFrame.h"
#include "ADLSProperties.h"

#include "../client/version.h"
#include "../client/ADLSearch.h"

int ADLSearchFrame::columnIndexes[] = { 
	COLUMN_ACTIVE_SEARCH_STRING,
	COLUMN_SOURCE_TYPE,
	COLUMN_DEST_DIR,
	COLUMN_MIN_FILE_SIZE,
	COLUMN_MAX_FILE_SIZE
};

int ADLSearchFrame::columnSizes[] = { 
	120, 
	90, 
	90, 
	90, 
	90 
};

static ResourceManager::Strings columnNames[] = { 
	ResourceManager::ACTIVE_SEARCH_STRING, 
	ResourceManager::SOURCE_TYPE, 
	ResourceManager::DESTINATION, 
	ResourceManager::SIZE_MIN, 
	ResourceManager::MAX_SIZE, 
};

BEGIN_EVENT_TABLE(ADLSearchFrame, BaseType)
	EVT_CLOSE(ADLSearchFrame::OnClose)
	EVT_BUTTON(IDC_ADD, ADLSearchFrame::OnAdd)
	EVT_BUTTON(IDC_EDIT, ADLSearchFrame::OnEdit)
	EVT_BUTTON(IDC_REMOVE, ADLSearchFrame::OnRemove)
	EVT_BUTTON(IDC_MOVE_UP, ADLSearchFrame::OnMoveUp)
	EVT_BUTTON(IDC_MOVE_DOWN, ADLSearchFrame::OnMoveDown)
	EVT_LIST_ITEM_SELECTED(ID_LIST_CTRL, ADLSearchFrame::OnItemSelected)
	EVT_LIST_ITEM_DESELECTED(ID_LIST_CTRL, ADLSearchFrame::OnItemDeselected)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_LIST_CTRL, ADLSearchFrame::OnContextMenu)
	EVT_LIST_ITEM_ACTIVATED(ID_LIST_CTRL, ADLSearchFrame::OnActivate)
END_EVENT_TABLE()

ADLSearchFrame::ADLSearchFrame() : BaseType(ID_CMD_ADL_SEARCH, CTSTRING(ADL_SEARCH), wxColour(0, 0, 0), wxT("IDI_ADLSEARCH")) {
	ctrlList = new ListViewCtrl(this, ID_LIST_CTRL, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxHSCROLL | wxLC_REPORT);
	ctrlList->SetBackgroundColour(WinUtil::bgColor);
	ctrlList->SetForegroundColour(WinUtil::textColor);

	for(int j = 0; j < COLUMN_LAST; ++j) {
		int fmt = wxALIGN_LEFT;
		ctrlList->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j]);
	}

	ctrlAdd = new wxButton(this, IDC_ADD, CTSTRING(NEW), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlAdd->SetFont(WinUtil::font);

	ctrlProps = new wxButton(this, IDC_EDIT, CTSTRING(PROPERTIES), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlProps->Enable(false);
	ctrlProps->SetFont(WinUtil::font);

	ctrlRemove = new wxButton(this, IDC_REMOVE, CTSTRING(REMOVE), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlRemove->Enable(false);
	ctrlRemove->SetFont(WinUtil::font);

	ctrlUp = new wxButton(this, IDC_MOVE_UP, CTSTRING(MOVE_UP), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlUp->SetFont(WinUtil::font);

	ctrlDown = new wxButton(this, IDC_MOVE_DOWN, CTSTRING(MOVE_DOWN), wxDefaultPosition, wxSize(-1, 22), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlDown->SetFont(WinUtil::font);

	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonsSizer->Add(ctrlAdd);
	buttonsSizer->Add(ctrlProps);
	buttonsSizer->Add(ctrlRemove, 0, wxRIGHT, 5);
	buttonsSizer->Add(ctrlUp);
	buttonsSizer->Add(ctrlDown, 0, wxRIGHT, 5);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(ctrlList, 1, wxEXPAND);
	sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);

	SetSizerAndFit(sizer);

	LoadAll();

	UpdateLayout();
}

void ADLSearchFrame::OnClose(wxCloseEvent& event) {
	ADLSearchManager::getInstance()->save();

	WinUtil::setButtonPressed(GetId(), false);
	event.Skip();
}

void ADLSearchFrame::UpdateLayout() {
}

void ADLSearchFrame::LoadAll() {
	Freeze();
	ctrlList->DeleteAllItems();

	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
	for(size_t i = 0; i < collection.size(); ++i) {
		UpdateSearch(i, false);
	}
	Thaw();
}

void ADLSearchFrame::UpdateSearch(int index, bool doDelete) {
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Check args
	if(index >= (int)collection.size())
		return;

	ADLSearch& search = collection[index];

	// Delete from list control
	if(doDelete)
		ctrlList->DeleteItem(index);

	// Generate values
	TStringList line;
	tstring fs;
	line.push_back(Text::toT(search.searchString));
	line.push_back(search.SourceTypeToDisplayString(search.sourceType));
	line.push_back(Text::toT(search.destDir));

	fs = _T("");
	if(search.minFileSize >= 0) {
		fs = Util::toStringW(search.minFileSize);
		fs += _T(" ");
		fs += search.SizeTypeToDisplayString(search.typeFileSize);
	}
	line.push_back(fs);

	fs = _T("");
	if(search.maxFileSize >= 0) {
		fs = Util::toStringW(search.maxFileSize);
		fs += _T(" ");
		fs += search.SizeTypeToDisplayString(search.typeFileSize);
	}
	line.push_back(fs);

	// Insert in list control
	ctrlList->insert(index, line);

	//FIXME
	// Update 'Active' check box
	//ctrlList.SetCheckState(index, search.isActive);
}

void ADLSearchFrame::OnAdd(wxCommandEvent& /*event*/) {
	ADLSearch search;
	ADLSProperties dlg(&search, this);
	if(dlg.ShowModal() == wxID_OK)
	{
		// Add new search to the end or if selected, just before
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
		

		int i = ctrlList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if(i < 0)
		{
			// Add to end
			collection.push_back(search);
			i = collection.size() - 1;
		}
		else
		{
			// Add before selection
			collection.insert(collection.begin() + i, search);
		}

		// Update list control
		int j = i;
		while(j < (int)collection.size())
		{
			UpdateSearch(j++);
		}
		ctrlList->EnsureVisible(i);
	}
}

void ADLSearchFrame::OnEdit(wxCommandEvent& /*event*/) {
	// Get selection info
	int i = ctrlList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(i >= 0) {
		// Edit existing
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
		ADLSearch search = collection[i];

		// Invoke dialog with selected search
		ADLSProperties dlg(&search, this);
		if(dlg.ShowModal() == wxID_OK) {
			collection[i] = search;

			UpdateSearch(i);	  
		}
	}
}

void ADLSearchFrame::OnRemove(wxCommandEvent& /*event*/) {
	if(!BOOLSETTING(CONFIRM_HUB_REMOVAL) || wxMessageBox(CTSTRING(REALLY_REMOVE), wxT(APPNAME) wxT(" ") wxT(VERSIONSTRING), wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT) == wxYES) {
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

		// Loop over all selected items
		int i;
		while((i = ctrlList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) >= 0) {
			collection.erase(collection.begin() + i);
			ctrlList->DeleteItem(i);
		}
	}
}

void ADLSearchFrame::OnMoveUp(wxCommandEvent& /*event*/) {
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Get selection
	vector<int> sel;
	int i = -1;
	while((i = ctrlList->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) >= 0)
		sel.push_back(i);

	if(sel.size() < 1)
		return;

	// Find out where to insert
	int i0 = sel[0];
	if(i0 > 0)
		i0 = i0 - 1;

	// Backup selected searches
	ADLSearchManager::SearchCollection backup;
	for(i = 0; i < (int)sel.size(); ++i)
		backup.push_back(collection[sel[i]]);

	// Erase selected searches
	for(i = sel.size() - 1; i >= 0; --i)
		collection.erase(collection.begin() + sel[i]);

	// Insert (grouped together)
	for(i = 0; i < (int)sel.size(); ++i)
		collection.insert(collection.begin() + i0 + i, backup[i]);

	// Update UI
	LoadAll();

	// Restore selection
	for(i = 0; i < (int)sel.size(); ++i)
		ctrlList->SetItemState(i0 + i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void ADLSearchFrame::OnMoveDown(wxCommandEvent& /*event*/) {
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Get selection
	vector<int> sel;
	int i = -1;
	while((i = ctrlList->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) >= 0)
		sel.push_back(i);

	if(sel.size() < 1)
		return;

	// Find out where to insert
	int i0 = sel[sel.size() - 1] + 2;
	if(i0 > (int)collection.size())
		i0 = collection.size();

	// Backup selected searches
	ADLSearchManager::SearchCollection backup;
	for(i = 0; i < (int)sel.size(); ++i)
		backup.push_back(collection[sel[i]]);

	// Erase selected searches
	for(i = sel.size() - 1; i >= 0; --i) {
		collection.erase(collection.begin() + sel[i]);
		if(i < i0) {
			i0--;
		}
	}

	// Insert (grouped together)
	for(i = 0; i < (int)sel.size(); ++i)
		collection.insert(collection.begin() + i0 + i, backup[i]);

	// Update UI
	LoadAll();

	// Restore selection
	for(i = 0; i < (int)sel.size(); ++i)
		ctrlList->SetItemState(i0 + i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	ctrlList->EnsureVisible(i0);
}

void ADLSearchFrame::OnItemSelected(wxListEvent& /*event*/) {
	ctrlAdd->Enable(true);
	ctrlRemove->Enable(true);
	ctrlProps->Enable(true);
}

void ADLSearchFrame::OnItemDeselected(wxListEvent& /*event*/) {
	ctrlAdd->Enable(false);
	ctrlRemove->Enable(false);
	ctrlProps->Enable(false);
}
