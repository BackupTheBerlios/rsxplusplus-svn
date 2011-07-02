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
#include "SearchFrame.h"

#include "CountryFlags.h"
#include "MainWindow.h"
#include "WinUtil.h"

#include "../client/QueueManager.h"
#include "../client/StringTokenizer.h"
#include "../client/Text.h"

#include <wx/checklst.h>
#include <wx/dcbuffer.h>

BEGIN_EVENT_TABLE(SearchFrame, MDIChildFrame)
	EVT_CLOSE(SearchFrame::OnClose)
	EVT_SET_FOCUS(SearchFrame::OnSetFocus)
	EVT_TEXT_ENTER(IDC_SEARCH_EDIT, SearchFrame::OnEnter)
	EVT_BUTTON(IDC_SEARCH, SearchFrame::OnEnter)
	EVT_BUTTON(IDC_SEARCH_PAUSE, SearchFrame::OnPause)
	EVT_BUTTON(IDC_PURGE, SearchFrame::OnPurge)
	EVT_CHECKBOX(ID_SHOWLIST_CHECKBOX, SearchFrame::OnShowUI)
	EVT_CHECKBOX(IDC_FREESLOTS, SearchFrame::OnFreeSlots)
	EVT_CHECKBOX(IDC_COLLAPSED, SearchFrame::OnCollapsed)
	EVT_COMBOBOX(ID_FILTER_COMBOBOX, SearchFrame::OnFilterSelection)
	EVT_TEXT(ID_FILTER_TEXTBOX, SearchFrame::OnFilterChange)
	EVT_TEXT_ENTER(ID_FILTER_TEXTBOX, SearchFrame::OnFilterEnter)
	EVT_DATAVIEW_ITEM_ACTIVATED(IDC_RESULTS, SearchFrame::OnDblClick)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(IDC_RESULTS, SearchFrame::OnContextMenu)
	EVT_CHECKLISTBOX(wxID_ANY, SearchFrame::OnCheckListBox)

	EVT_MENU(ID_DOWNLOAD, SearchFrame::OnDownload)
	EVT_MENU(ID_DOWNLOADDIR, SearchFrame::OnDownloadDirectory)
	EVT_MENU(ID_DOWNLOADTO, SearchFrame::OnDownloadTo)
	EVT_MENU(ID_DOWNLOADDIRTO, SearchFrame::OnDownloadDirectoryTo)
	EVT_MENU(ID_VIEW_AS_TEXT, SearchFrame::OnViewAsText)
	EVT_MENU(ID_SEARCH_ALTERNATES, SearchFrame::OnSearchForAlternates)
	EVT_MENU(ID_BITZI_LOOKUP, SearchFrame::OnBitziLookup)
	EVT_MENU(ID_REMOVE, SearchFrame::OnRemove)
	EVT_MENU_RANGE(IDC_COPY, IDC_COPY + SearchFrame::COLUMN_LAST, SearchFrame::OnCopy)

END_EVENT_TABLE()

std::set<tstring> SearchFrame::lastSearches;

int SearchFrame::columnIndexes[] = { COLUMN_FILENAME, COLUMN_HITS, COLUMN_NICK, COLUMN_TYPE, COLUMN_SIZE,
	COLUMN_PATH, COLUMN_SLOTS, COLUMN_CONNECTION, COLUMN_HUB, COLUMN_EXACT_SIZE, COLUMN_IP, COLUMN_TTH };
int SearchFrame::columnSizes[] = { 210, 80, 100, 50, 80, 100, 40, 70, 150, 80, 100, 150 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILE,  ResourceManager::HIT_COUNT, ResourceManager::USER, ResourceManager::TYPE, ResourceManager::SIZE,
	ResourceManager::PATH, ResourceManager::SLOTS, ResourceManager::CONNECTION, 
	ResourceManager::HUB, ResourceManager::EXACT_SIZE, ResourceManager::IP_BARE, ResourceManager::TTH_ROOT };

SearchFrame::SearchFrame(const tstring& str, int64_t size, SearchManager::SizeModes mode, SearchManager::TypeModes type) : 
	MDIChildFrame(wxID_ANY, CTSTRING(SEARCH), wxColour(127, 127, 255), wxT("IDI_SEARCH")), 
	droppedResults(0), onlyFree(false), isHash(false), expandSR(false), running(false), useGrouping(BOOLSETTING(GROUP_SEARCH_RESULTS)), waiting(false),
	exactSize1(false), exactSize2(0), resultsCount(0)
{
	CreateStatusBar(5, wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE);
	progress.SetAlignment(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
		
	ctrlShowUI = new wxCheckBox(GetStatusBar(), ID_SHOWLIST_CHECKBOX, wxEmptyString, wxPoint(6, 5), wxSize(16, 16), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlShowUI->SetValue(1);

	splitter = new wxSplitterWindow(this, ID_SPLITTER, wxDefaultPosition, wxDefaultSize,  wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxSP_3D | wxSP_LIVE_UPDATE);

	controlPanel = new wxPanel(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,  wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxTAB_TRAVERSAL | wxNO_BORDER);

	// create sizers
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer* box1 = new wxStaticBoxSizer(wxVERTICAL, controlPanel, TSTRING(SEARCH_FOR));
	wxStaticBoxSizer* box2 = new wxStaticBoxSizer(wxHORIZONTAL, controlPanel, TSTRING(SIZE));
	wxStaticBoxSizer* box3 = new wxStaticBoxSizer(wxHORIZONTAL, controlPanel, TSTRING(FILE_TYPE));
	wxStaticBoxSizer* box4 = new wxStaticBoxSizer(wxHORIZONTAL, controlPanel, TSTRING(SEARCH_IN_RESULTS));
	wxStaticBoxSizer* box5 = new wxStaticBoxSizer(wxVERTICAL, controlPanel, TSTRING(SEARCH_OPTIONS));
	wxStaticBoxSizer* box6 = new wxStaticBoxSizer(wxVERTICAL, controlPanel, TSTRING(HUBS));

	// Search for group
	searchBox = new wxComboBox(box1->GetStaticBox(), IDC_SEARCH_EDIT, wxEmptyString, wxDefaultPosition, wxSize(0, -1), 0, NULL, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxTE_PROCESS_ENTER);
	searchBox->SetFont(WinUtil::font);
	searchBox->SetBackgroundColour(WinUtil::bgColor);
	searchBox->SetForegroundColour(WinUtil::textColor);

	for(std::set<tstring>::const_iterator i = lastSearches.begin(); i != lastSearches.end(); ++i) 
	{
		searchBox->Insert(*i, 0);
	}

	wxButton* searchButton = new wxButton(box1->GetStaticBox(), IDC_SEARCH, CTSTRING(SEARCH), wxDefaultPosition, wxSize(0, 21),  wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	searchButton->SetDefault();

	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonsSizer->Add(new wxButton(box1->GetStaticBox(), IDC_PURGE, CTSTRING(PURGE), wxDefaultPosition, wxSize(0, 21), wxCLIP_SIBLINGS | wxCLIP_CHILDREN), 1, wxEXPAND | wxALL, 0);
	buttonsSizer->Add(searchButton, 1, wxEXPAND | wxALL, 0);
	buttonsSizer->Add(ctrlPauseSearch = new wxButton(box1->GetStaticBox(), IDC_SEARCH_PAUSE, CTSTRING(PAUSE), wxDefaultPosition, wxSize(0, 21),  wxCLIP_SIBLINGS | wxCLIP_CHILDREN), 1, wxEXPAND | wxALL, 0);
	
	box1->Add(searchBox, 0, wxBOTTOM | wxEXPAND, 1);
	box1->Add(buttonsSizer, 0, wxBOTTOM | wxEXPAND, 1);
	
	// Size group
	wxString modes[4] = { CTSTRING(NORMAL), CTSTRING(AT_LEAST), CTSTRING(AT_MOST), CTSTRING(EXACT_SIZE) };
	wxString sizes[4] = { CTSTRING(B), CTSTRING(KB), CTSTRING(MB), CTSTRING(GB) };
	wxString types[9] = { CTSTRING(ANY), CTSTRING(AUDIO), CTSTRING(COMPRESSED), CTSTRING(DOCUMENT), CTSTRING(EXECUTABLE), 
							CTSTRING(PICTURE), CTSTRING(VIDEO), CTSTRING(DIRECTORY), _T("TTH")};

	ctrlMode = new wxComboBox(box2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(0, -1), 4, modes, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxCB_READONLY);
	ctrlMode->SetFont(WinUtil::font);
	ctrlMode->SetBackgroundColour(WinUtil::bgColor);
	ctrlMode->SetForegroundColour(WinUtil::textColor);
	ctrlMode->SetSelection(0);
		
	ctrlSize = new wxTextCtrl(box2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(0, -1), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlSize->SetFont(WinUtil::font);
	ctrlSize->SetBackgroundColour(WinUtil::bgColor);
	ctrlSize->SetForegroundColour(WinUtil::textColor);

	ctrlSizeMode = new wxComboBox(box2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(0, -1), 4, sizes, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxCB_READONLY);
	ctrlSizeMode->SetFont(WinUtil::font);
	ctrlSizeMode->SetBackgroundColour(WinUtil::bgColor);
	ctrlSizeMode->SetForegroundColour(WinUtil::textColor);
	ctrlSizeMode->SetSelection(0);

	box2->Add(ctrlMode, 1, wxRIGHT | wxBOTTOM | wxEXPAND, 1);
	box2->Add(ctrlSize, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 1);
	box2->Add(ctrlSizeMode, 1, wxLEFT | wxBOTTOM | wxEXPAND, 1);

	// File type group
	ctrlFiletype = new wxComboBox(box3->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(0, -1), 9, types, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxCB_READONLY);
	ctrlFiletype->SetFont(WinUtil::font);
	ctrlFiletype->SetBackgroundColour(WinUtil::bgColor);
	ctrlFiletype->SetForegroundColour(WinUtil::textColor);
	ctrlFiletype->SetSelection(0);

	box3->Add(ctrlFiletype, 3, wxRIGHT | wxBOTTOM | wxEXPAND, 1);
	box3->Add(new wxButton(box3->GetStaticBox(), ID_CMD_SETTINGS, _("..."), wxDefaultPosition, wxSize(0, 21), wxCLIP_SIBLINGS | wxCLIP_CHILDREN), 1, wxBOTTOM, 1);
	
	// Search in results group
	ctrlFilter = new wxTextCtrl(box4->GetStaticBox(), ID_FILTER_TEXTBOX, wxEmptyString, wxDefaultPosition, wxSize(0, -1), wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxTE_PROCESS_ENTER);
	ctrlFilter->SetFont(WinUtil::font);
	ctrlFilter->SetBackgroundColour(WinUtil::bgColor);
	ctrlFilter->SetForegroundColour(WinUtil::textColor);

	ctrlFilterSel = new wxComboBox(box4->GetStaticBox(), ID_FILTER_COMBOBOX, wxEmptyString, wxDefaultPosition, wxSize(0, -1), 0, NULL, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxCB_READONLY);
	ctrlFilterSel->SetFont(WinUtil::font);
	ctrlFilterSel->SetBackgroundColour(WinUtil::bgColor);
	ctrlFilterSel->SetForegroundColour(WinUtil::textColor);

	for(int j = 0; j < COLUMN_LAST; j++) 
	{
		ctrlFilterSel->Append(CTSTRING_I(columnNames[j]));
	}
	ctrlFilterSel->SetSelection(0);

	box4->Add(ctrlFilter, 1, wxRIGHT | wxBOTTOM | wxEXPAND, 1);
	box4->Add(ctrlFilterSel, 1, wxLEFT | wxBOTTOM | wxEXPAND, 1);
	
	// Search options group
	box5->Add(ctrlSlots = new wxCheckBox(box5->GetStaticBox(), IDC_FREESLOTS, CTSTRING(ONLY_FREE_SLOTS), wxDefaultPosition, wxSize(0, -1), wxCLIP_SIBLINGS | wxCLIP_CHILDREN), 0, wxLEFT | wxBOTTOM | wxEXPAND, 2);
	box5->Add(ctrlCollapsed = new wxCheckBox(box5->GetStaticBox(), IDC_COLLAPSED, CTSTRING(EXPANDED_RESULTS), wxDefaultPosition, wxSize(0, -1), wxCLIP_SIBLINGS | wxCLIP_CHILDREN), 0, wxLEFT | wxBOTTOM | wxEXPAND, 2);

	// Hubs group
	ctrlHubs = new wxCheckListBox(box6->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize(0, -1), 0, NULL, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlHubs->SetFont(WinUtil::font);
	ctrlHubs->SetBackgroundColour(WinUtil::bgColor);
	ctrlHubs->SetForegroundColour(WinUtil::textColor);

	box6->Add(ctrlHubs, 1, wxBOTTOM | wxEXPAND, 1);

	// position sizers
	sizer->Add(box1, 0, wxLEFT | wxTOP | wxEXPAND, 4);
	sizer->Add(box2, 0, wxLEFT | wxTOP | wxEXPAND, 4);
	sizer->Add(box3, 0, wxLEFT | wxTOP | wxEXPAND, 4);
	sizer->Add(box4, 0, wxLEFT | wxTOP | wxEXPAND, 4);
	sizer->Add(box5, 0, wxLEFT | wxTOP | wxEXPAND, 4);
	sizer->Add(box6, 1, wxLEFT | wxTOP | wxEXPAND, 4);
	
	controlPanel->SetSizerAndFit(sizer);

	if(BOOLSETTING(FREE_SLOTS_DEFAULT)) 
	{
		ctrlSlots->SetValue(true);
		onlyFree = true;
	}

	ctrlResults = new SearchInfoList(splitter, IDC_RESULTS, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxHSCROLL);
	ctrlResults->SetFont(WinUtil::font);
	ctrlResults->SetBackgroundColour(WinUtil::bgColor);
	ctrlResults->SetForegroundColour(WinUtil::textColor);
	ctrlResults->setImageList(&WinUtil::fileImages);
	ctrlResults->Connect(wxEVT_CHAR, wxKeyEventHandler(SearchFrame::OnKeyDown), NULL, this);

	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(SEARCHFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(SEARCHFRAME_WIDTHS), COLUMN_LAST);

	for(uint8_t j = 0; j < COLUMN_LAST; j++) 
	{
		wxAlignment fmt = (j == COLUMN_SIZE || j == COLUMN_EXACT_SIZE) ? wxALIGN_RIGHT : wxALIGN_LEFT;
		wxDataViewRenderer* rndr = (j == COLUMN_IP) ? new CountryFlags::FlagRenderer : NULL;
		ctrlResults->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], rndr);
	}

	/* TODO ctrlResults->setColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlResults->setVisible(SETTING(SEARCHFRAME_VISIBLE));*/
	ctrlResults->setSortColumn(COLUMN_HITS, false);

	splitter->SplitVertically(controlPanel, ctrlResults, 170); // TODO: save settings
	splitter->SetMinimumPaneSize(1);

	wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
	frameSizer->Add(splitter, 1, wxEXPAND);
	SetSizerAndFit(frameSizer);

	initHubs();

	SearchManager::getInstance()->addListener(this);
	SettingsManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);

	if(!str.empty()) 
	{
		// update layout first, because OnEnter event access statusbar which hasn't been prepared yet
		UpdateLayout();

		searchBox->SetValue(str);
		ctrlMode->SetSelection(mode);
		ctrlSize->SetValue(Util::toStringW(size));
		ctrlFiletype->SetSelection(type);
		
		// simulate Search button click
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, IDC_SEARCH);
		ProcessEvent(evt);
	} 
	else 
	{
		//ctrlPauseSearch->Enable(false);
	}
}

SearchFrame::~SearchFrame(void)
{
}

void SearchFrame::openWindow(const tstring& str, int64_t size, SearchManager::SizeModes mode, SearchManager::TypeModes type) 
{
	MDIFreezer freezer;

	SearchFrame* frm = new SearchFrame(str, size, mode, type);
	frm->Show();

	//frames.insert( FramePair(pChild->m_hWnd, pChild) );
}

void SearchFrame::OnClose(wxCloseEvent& event)
{
	ClientManager::getInstance()->cancelSearch((void*)this);
	SettingsManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	SearchManager::getInstance()->removeListener(this);
	ClientManager::getInstance()->removeListener(this);

	ctrlResults->Freeze();
	ctrlResults->deleteAllItems();
	ctrlResults->Thaw();

	// delete all results which came in paused state
	for_each(pausedResults.begin(), pausedResults.end(), DeleteFunction());
	for_each(hubs.begin(), hubs.end(), DeleteFunction());

	event.Skip();
}

void SearchFrame::UpdateLayout()
{
	if(GetStatusBar())
	{
		int widths[5] = { 16, -1, 100, 100, 100 };
		GetStatusBar()->SetFieldsCount(5, widths);
	}
}

bool SearchFrame::GetItemAttr(void* data, int64_t& position, int64_t& size, wxColour& backColour, wxColour& textColour)
{
	SearchFrame* sf = (SearchFrame*)data;
	if(sf->searchStartTime > 0)
	{
		position = (GET_TICK() - sf->searchStartTime);
		size = (sf->searchEndTime - sf->searchStartTime);

		if(position > size)
			return false;

		backColour = wxColour(128,128,128);
		textColour = wxColour(255, 255, 255);

		return true;
	}

	return false;
}

void SearchFrame::onFilterResult()
{
	GetStatusBar()->SetStatusText(Util::toStringW(droppedResults) + _T(" ") + TSTRING(FILTERED), 4);
}

void SearchFrame::onQueueStats(tstring* str)
{
	wxRect rect;
	GetStatusBar()->GetFieldRect(2, rect);

	if(str != NULL)
	{
		std::unique_ptr<tstring> t(str);

		wxVariant var;
		var << wxDataViewIconTextData(*t, wxNullIcon, this);
		progress.SetValue(var);

		wxMemoryDC memDC;
		memDC.SelectObject(progressBitmap.IsOk() ? progressBitmap : (progressBitmap = wxBitmap(rect.GetSize())));
		memDC.SetBackground(GetStatusBar()->GetBackgroundColour());

		wxRect rect2 = rect;
		rect2.SetPosition(wxPoint(0, 0));
		progress.Render(rect2, &memDC, 0);

		wxClientDC dc(GetStatusBar());
		dc.Blit(rect.GetPosition(), rect.GetSize(), &memDC, wxPoint(0, 0));
	}
	else
		GetStatusBar()->Refresh(true, &rect);
}

void SearchFrame::OnEnter(wxCommandEvent& /*event*/)
{
	StringList clients;
	
	// Change Default Settings If Changed
	if (onlyFree != BOOLSETTING(FREE_SLOTS_DEFAULT))
		SettingsManager::getInstance()->set(SettingsManager::FREE_SLOTS_DEFAULT, onlyFree);

	for(unsigned int i = 0; i < hubs.size(); ++i) 
	{
		if(ctrlHubs->IsChecked(i + 1))
		{
			clients.push_back(Text::fromT(hubs[i]->url));
		}
	}

	if(clients.empty())
		return;

	tstring s(searchBox->GetValue());

	tstring size = ctrlSize->GetValue();
		
	double lsize = Util::toDouble(Text::fromT(size));
	switch(ctrlSizeMode->GetSelection()) 
	{
		case 1:
			lsize*=1024.0; break;
		case 2:
			lsize*=1024.0*1024.0; break;
		case 3:
			lsize*=1024.0*1024.0*1024.0; break;
	}

	int64_t llsize = (int64_t)lsize;
	
	{
		Lock l(cs);
		search = StringTokenizer<tstring>(s, ' ').getTokens();
		s.clear();
		//strip out terms beginning with -
		for(TStringList::iterator si = search.begin(); si != search.end(); ) 
		{
			if(si->empty()) 
			{
				si = search.erase(si);
				continue;
			}
			if ((*si)[0] != _T('-')) 
				s += *si + _T(' ');	
			++si;
		}

		s = s.substr(0, max(s.size(), static_cast<tstring::size_type>(1)) - 1);
		if(s.empty())
			return;

		token = Util::toString(Util::rand());
	}
	
	// delete all results which came in paused state
	for_each(pausedResults.begin(), pausedResults.end(), DeleteFunction());
	pausedResults.clear();

	ctrlResults->deleteAllItems();	
	
	ctrlPauseSearch->Enable(true);
	ctrlPauseSearch->SetLabel(CTSTRING(PAUSE));

	SearchManager::SizeModes mode((SearchManager::SizeModes)ctrlMode->GetSelection());
	if(llsize == 0)
		mode = SearchManager::SIZE_DONTCARE;

	int ftype = ctrlFiletype->GetSelection();
	exactSize1 = (mode == SearchManager::SIZE_EXACT);
	exactSize2 = llsize;

	GetStatusBar()->SetStatusText(wxT(""), 3);
	GetStatusBar()->SetStatusText(wxT(""), 4);

	target = s;

	droppedResults = 0;
	resultsCount = 0;
	running = true;

	isHash = (ftype == SearchManager::TYPE_TTH);
	
	// Add new searches to the last-search dropdown list
	if(find(lastSearches.begin(), lastSearches.end(), s) == lastSearches.end()) 
	{
		unsigned int i = max(SETTING(SEARCH_HISTORY) - 1, 0);

		if(searchBox->GetCount() > i) 
			searchBox->Delete(i);
		searchBox->Insert(s, 0);

		while(lastSearches.size() > (TStringList::size_type)i) 
		{
			lastSearches.erase(lastSearches.begin());
		}
		lastSearches.insert(s);
		
		// update history in quick search box
		WinUtil::mainWindow->updateQuickSearches();
	}
	
	SetTitle(TSTRING(SEARCH) + _T(" - ") + s);
	
	// stop old search
	ClientManager::getInstance()->cancelSearch((void*)this);	

	// TODO: Get ADC searchtype extensions if any is selected
	StringList extList;
	{
		Lock l(cs);
		
		searchStartTime = GET_TICK();
		// more 5 seconds for transfering results
		searchEndTime = searchStartTime + SearchManager::getInstance()->search(clients, Text::fromT(s), llsize, 
			(SearchManager::TypeModes)ftype, mode, token, extList, (void*)this) + 5000;

		waiting = true;
	}
	
	GetStatusBar()->SetStatusText(TSTRING(SEARCHING_FOR) + _T(" ") + target + _T(" ... "), 1);

	if(BOOLSETTING(CLEAR_SEARCH)) // Only clear if the search was sent
		searchBox->SetValue(wxT(""));

}

void SearchFrame::OnPause(wxCommandEvent& /*event*/)
{
	if(!running) 
	{
		running = true;

		// readd all results which came during pause state
		while(!pausedResults.empty()) 
		{
			// start from the end because erasing front elements from vector is not efficient
			addSearchResult(pausedResults.back());
			pausedResults.pop_back();
		}

		// update controls texts
		GetStatusBar()->SetStatusText(Util::toStringW(ctrlResults->getItemCount()) + _T(" ") + TSTRING(FILES), 3);
		ctrlPauseSearch->SetLabel(CTSTRING(PAUSE));
	} 
	else 
	{
		running = false;
		ctrlPauseSearch->SetLabel(CTSTRING(CONTINUE));
	}
}

void SearchFrame::OnPurge(wxCommandEvent& /*event*/)
{
	searchBox->Clear();
	lastSearches.clear();
}

void SearchFrame::OnShowUI(wxCommandEvent& event)
{
	if(event.IsChecked()) 
	{	
		splitter->SplitVertically(controlPanel, ctrlResults, 200);

	}
	else 
	{
		splitter->Unsplit(controlPanel);
	}

	UpdateLayout();
}

void SearchFrame::OnFilterSelection(wxCommandEvent& /*event*/)
{
	filter = ctrlFilter->GetValue();

	updateSearchList();
}

void SearchFrame::OnFilterChange(wxCommandEvent& /*event*/)
{
	if(!BOOLSETTING(FILTER_ENTER))
	{
		filter = ctrlFilter->GetValue();

		updateSearchList();
	}
}

void SearchFrame::OnFilterEnter(wxCommandEvent& /*event*/)
{
	if(BOOLSETTING(FILTER_ENTER))
	{
		filter = ctrlFilter->GetValue();

		updateSearchList();
	}
}

void SearchFrame::OnDblClick(wxDataViewEvent& /*event*/)
{
	ctrlResults->forEachSelectedT(SearchInfo::Download(Text::toT(SETTING(DOWNLOAD_DIRECTORY)), this));
}

void SearchFrame::OnContextMenu(wxDataViewEvent& /*event*/)
{
	wxMenu resultsMenu;
	wxMenu* targetMenu = new wxMenu();
	wxMenu* targetDirMenu = new wxMenu();

	// TODO copyMenu.Append(IDC_COPY_LINK, CTSTRING(COPY_MAGNET_LINK));

	wxDataViewItemArray sel;
	ctrlResults->GetSelections(sel);

	SearchInfo::CheckTTH cs = ctrlResults->forEachSelectedT(SearchInfo::CheckTTH());

	if(sel.size() > 1)
		resultsMenu.SetTitle(Util::toStringW(sel.size()) + _T(" ") + TSTRING(FILES));
	else
		resultsMenu.SetTitle(Text::toT(ctrlResults->getItemData(sel[0])->sr->getFileName()));
				
	resultsMenu.Append(ID_DOWNLOAD, CTSTRING(DOWNLOAD));
	resultsMenu.AppendSubMenu(targetMenu, CTSTRING(DOWNLOAD_TO));
	resultsMenu.Append(ID_DOWNLOADDIR, CTSTRING(DOWNLOAD_WHOLE_DIR));
	resultsMenu.AppendSubMenu(targetDirMenu, CTSTRING(DOWNLOAD_WHOLE_DIR_TO));
	resultsMenu.Append(ID_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT));
	resultsMenu.AppendSeparator();
	resultsMenu.Append(ID_SEARCH_ALTERNATES, CTSTRING(SEARCH_FOR_ALTERNATES));
	resultsMenu.Append(ID_BITZI_LOOKUP, CTSTRING(BITZI_LOOKUP));
	resultsMenu.AppendSubMenu(ctrlResults->getCopyMenu(), CTSTRING(COPY));
	resultsMenu.AppendSeparator();
	appendUserItems(resultsMenu, Util::emptyString); // TODO: hubhint
	prepareMenu(resultsMenu, UserCommand::CONTEXT_SEARCH, cs.hubs);
	resultsMenu.AppendSeparator();
	resultsMenu.Append(ID_REMOVE, CTSTRING(REMOVE));

	int n = 0;

	targetMenu->SetTitle(TSTRING(DOWNLOAD_TO));
	//Append favorite download dirs
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if (spl.size() > 0) 
	{
		for(StringPairIter i = spl.begin(); i != spl.end(); i++) 
		{
			targetMenu->Append(ID_DOWNLOAD_FAVORITE_DIRS + n, Text::toT(i->second).c_str());
			n++;
		}
		Bind(wxEVT_COMMAND_MENU_SELECTED, &SearchFrame::OnDownloadFavoriteDirs, this, ID_DOWNLOAD_FAVORITE_DIRS, ID_DOWNLOAD_FAVORITE_DIRS + n - 1);
		targetMenu->AppendSeparator();
	}

	n = 0;
	targetMenu->Append(ID_DOWNLOADTO, CTSTRING(BROWSE));
	if(WinUtil::lastDirs.size() > 0) 
	{
		// TODO targetMenu.InsertSeparatorLast(TSTRING(PREVIOUS_FOLDERS));
		targetMenu->AppendSeparator();
		for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) 
		{
			targetMenu->Append(ID_DOWNLOAD_TARGET + n, i->c_str());
			n++;
		}
	}

	if(cs.hasTTH) 
	{
		targets.clear();
		QueueManager::getInstance()->getTargets(TTHValue(Text::fromT(cs.tth)), targets);

		if(targets.size() > 0) 
		{
			// TODO targetMenu->InsertSeparatorLast(TSTRING(ADD_AS_SOURCE));
			targetMenu->AppendSeparator();
			for(StringIter i = targets.begin(); i != targets.end(); ++i) 
			{
				targetMenu->Append(ID_DOWNLOAD_TARGET + n, Text::toT(Util::getFileName(*i)).c_str());
				n++;
			}
		}
	}

	if(n > 0)
		Bind(wxEVT_COMMAND_MENU_SELECTED, &SearchFrame::OnDownloadTarget, this, ID_DOWNLOAD_TARGET, ID_DOWNLOAD_TARGET + n - 1);

	n = 0;
	targetDirMenu->SetTitle(TSTRING(DOWNLOAD_WHOLE_DIR_TO));
	//Append favorite download dirs
	if (spl.size() > 0) 
	{
		for(StringPairIter i = spl.begin(); i != spl.end(); ++i) 
		{
			targetDirMenu->Append(ID_DOWNLOAD_WHOLE_FAVORITE_DIRS + n, Text::toT(i->second).c_str());
			n++;
		}
		Bind(wxEVT_COMMAND_MENU_SELECTED, &SearchFrame::OnDownloadWholeFavoriteDirs, this, ID_DOWNLOAD_WHOLE_FAVORITE_DIRS, ID_DOWNLOAD_WHOLE_FAVORITE_DIRS + n - 1);
		targetDirMenu->AppendSeparator();
	}

	n = 0;
	targetDirMenu->Append(ID_DOWNLOADDIRTO, CTSTRING(BROWSE));
	if(WinUtil::lastDirs.size() > 0) 
	{
		targetDirMenu->AppendSeparator();
		for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) 
		{
			targetDirMenu->Append(ID_DOWNLOAD_WHOLE_TARGET + n, i->c_str());
			n++;
		}
		Bind(wxEVT_COMMAND_MENU_SELECTED, &SearchFrame::OnDownloadWholeTarget, this, ID_DOWNLOAD_WHOLE_TARGET, ID_DOWNLOAD_WHOLE_TARGET + n - 1);
	}		

	PopupMenu(&resultsMenu);
}

void SearchFrame::OnKeyDown(wxKeyEvent& event)
{
	if(event.GetKeyCode() == WXK_DELETE)
	{
		wxCommandEvent evt;
		OnRemove(evt);
	}
}

void SearchFrame::OnCheckListBox(wxCommandEvent& event)
{
	if(event.GetInt() == 0)
	{
		for(unsigned int i = 0; i < hubs.size(); ++i)
		{
			const HubInfo* client = hubs[i];
			if (!client->op)
				ctrlHubs->Check(i + 1, false);
		}		
	}
}

void SearchFrame::OnDownloadTo(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	ctrlResults->GetSelections(sel);

	if(sel.size() == 1) 
	{
		const SearchInfo* si = ctrlResults->getItemData(sel[0]);
		const SearchResultPtr& sr = si->sr;
	
		if(sr->getType() == SearchResult::TYPE_FILE) 
		{
			tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY)) + si->getText(COLUMN_FILENAME);
			if(WinUtil::browseFile(target, this))
			{
				WinUtil::addLastDir(Util::getFilePath(target));
				ctrlResults->forEachSelectedT(SearchInfo::DownloadTarget(target));
			}
		} 
		else 
		{
			tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
			if(WinUtil::browseDirectory(target, this)) 
			{
				WinUtil::addLastDir(target);
				ctrlResults->forEachSelectedT(SearchInfo::Download(target, this));
			}
		}
	} 
	else 
	{
		tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
		if(WinUtil::browseDirectory(target, this)) 
		{
			WinUtil::addLastDir(target);
			ctrlResults->forEachSelectedT(SearchInfo::Download(target, this));
		}
	}
}

void SearchFrame::OnDownloadDirectoryTo(wxCommandEvent& /*event*/)
{
	tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
	if(WinUtil::browseDirectory(target, this)) 
	{
		WinUtil::addLastDir(target);
		ctrlResults->forEachSelectedT(SearchInfo::DownloadWhole(target));
	}
}

void SearchFrame::OnDownloadFavoriteDirs(wxCommandEvent& event) 
{
	dcassert(event.GetId() >= ID_DOWNLOAD_FAVORITE_DIRS);
	size_t newId = (size_t)event.GetId() - ID_DOWNLOAD_FAVORITE_DIRS;

	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if(newId < spl.size()) 
	{
		ctrlResults->forEachSelectedT(SearchInfo::Download(Text::toT(spl[newId].first), this));
	} 
	else 
	{
		dcassert((newId - spl.size()) < targets.size());
		ctrlResults->forEachSelectedT(SearchInfo::DownloadTarget(Text::toT(targets[newId - spl.size()])));
	}
}

void SearchFrame::OnDownloadWholeFavoriteDirs(wxCommandEvent& event) 
{
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	dcassert((event.GetId() - ID_DOWNLOAD_WHOLE_FAVORITE_DIRS) < (int)spl.size());
	ctrlResults->forEachSelectedT(SearchInfo::DownloadWhole(Text::toT(spl[event.GetId() - ID_DOWNLOAD_WHOLE_FAVORITE_DIRS].first)));
}

void SearchFrame::OnDownloadTarget(wxCommandEvent& event) 
{
	dcassert(event.GetId() >= ID_DOWNLOAD_TARGET);
	size_t newId = (size_t)event.GetId() - ID_DOWNLOAD_TARGET;

	if(newId < WinUtil::lastDirs.size()) 
	{
		ctrlResults->forEachSelectedT(SearchInfo::Download(WinUtil::lastDirs[newId], this));
	} 
	else 
	{
		dcassert((newId - WinUtil::lastDirs.size()) < targets.size());
		ctrlResults->forEachSelectedT(SearchInfo::DownloadTarget(Text::toT(targets[newId - WinUtil::lastDirs.size()])));
	}
}

void SearchFrame::OnDownloadWholeTarget(wxCommandEvent& event) 
{
	dcassert((event.GetId() - ID_DOWNLOAD_WHOLE_TARGET) < (int)WinUtil::lastDirs.size());
	ctrlResults->forEachSelectedT(SearchInfo::DownloadWhole(WinUtil::lastDirs[event.GetId() - ID_DOWNLOAD_WHOLE_TARGET]));
}

void SearchFrame::OnSearchForAlternates(wxCommandEvent& /*event*/)
{
	// don't spam with too many searches, so search only for the first selected file
	const SearchResultPtr& sr = ctrlResults->getItemData(ctrlResults->getFirstSelectedItem())->sr;
	if(sr->getType() == SearchResult::TYPE_FILE) {
		WinUtil::searchHash(sr->getTTH());
	}
}

void SearchFrame::OnBitziLookup(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	ctrlResults->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const SearchResultPtr& sr = ctrlResults->getItemData(sel[i])->sr;
		if(sr->getType() == SearchResult::TYPE_FILE) {
			WinUtil::bitziLink(sr->getTTH());
		}		
	}
}

void SearchFrame::OnRemove(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	ctrlResults->GetSelections(sel);

	// remove from end to avoid changing selection indexes
	for(int i = sel.size() - 1; i >= 0; --i)
	{
		ctrlResults->removeGroupedItem(ctrlResults->getItemData(sel[i]));
	}

	// select next item
	ctrlResults->Select(sel[0]);
}

void SearchFrame::OnCopy(wxCommandEvent& event)
{
	tstring sCopy;

	wxDataViewItemArray sel;
	ctrlResults->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const SearchInfo* si = ctrlResults->getItemData(sel[i]);
		
		if(!sCopy.empty())
			sCopy += _T("\r\n");

		sCopy += si->getText(static_cast<uint8_t>(event.GetId() - IDC_COPY));
	}
	
	if (!sCopy.empty())
		WinUtil::setClipboard(sCopy);

}

bool SearchFrame::parseFilter(FilterModes& mode, int64_t& size) 
{
	tstring::size_type start = (tstring::size_type)tstring::npos;
	tstring::size_type end = (tstring::size_type)tstring::npos;
	int64_t multiplier = 1;
	
	if(filter.compare(0, 2, _T(">=")) == 0) {
		mode = GREATER_EQUAL;
		start = 2;
	} else if(filter.compare(0, 2, _T("<=")) == 0) {
		mode = LESS_EQUAL;
		start = 2;
	} else if(filter.compare(0, 2, _T("==")) == 0) {
		mode = EQUAL;
		start = 2;
	} else if(filter.compare(0, 2, _T("!=")) == 0) {
		mode = NOT_EQUAL;
		start = 2;
	} else if(filter[0] == _T('<')) {
		mode = LESS;
		start = 1;
	} else if(filter[0] == _T('>')) {
		mode = GREATER;
		start = 1;
	} else if(filter[0] == _T('=')) {
		mode = EQUAL;
		start = 1;
	}

	if(start == tstring::npos)
		return false;
	if(filter.length() <= start)
		return false;

	if((end = Util::findSubString(filter, _T("TiB"))) != tstring::npos) {
		multiplier = 1024LL * 1024LL * 1024LL * 1024LL;
	} else if((end = Util::findSubString(filter, _T("GiB"))) != tstring::npos) {
		multiplier = 1024*1024*1024;
	} else if((end = Util::findSubString(filter, _T("MiB"))) != tstring::npos) {
		multiplier = 1024*1024;
	} else if((end = Util::findSubString(filter, _T("KiB"))) != tstring::npos) {
		multiplier = 1024;
	} else if((end = Util::findSubString(filter, _T("TB"))) != tstring::npos) {
		multiplier = 1000LL * 1000LL * 1000LL * 1000LL;
	} else if((end = Util::findSubString(filter, _T("GB"))) != tstring::npos) {
		multiplier = 1000*1000*1000;
	} else if((end = Util::findSubString(filter, _T("MB"))) != tstring::npos) {
		multiplier = 1000*1000;
	} else if((end = Util::findSubString(filter, _T("kB"))) != tstring::npos) {
		multiplier = 1000;
	} else if((end = Util::findSubString(filter, _T("B"))) != tstring::npos) {
		multiplier = 1;
	}


	if(end == tstring::npos) 
	{
		end = filter.length();
	}
	
	tstring tmpSize = filter.substr(start, end-start);
	size = static_cast<int64_t>(Util::toDouble(Text::fromT(tmpSize)) * multiplier);
	
	return true;
}

bool SearchFrame::matchFilter(SearchInfo* si, int sel, bool doSizeCompare, FilterModes mode, int64_t size) 
{
	bool insert = false;

	if(filter.empty()) 
	{
		insert = true;
	} 
	else if(doSizeCompare) 
	{
		switch(mode) 
		{
			case EQUAL: insert = (size == si->sr->getSize()); break;
			case GREATER_EQUAL: insert = (size <=  si->sr->getSize()); break;
			case LESS_EQUAL: insert = (size >=  si->sr->getSize()); break;
			case GREATER: insert = (size < si->sr->getSize()); break;
			case LESS: insert = (size > si->sr->getSize()); break;
			case NOT_EQUAL: insert = (size != si->sr->getSize()); break;
		}
	} 
	else 
	{
		try 
		{
			boost::wregex reg(filter, boost::regex_constants::icase);
			tstring s = si->getText(static_cast<uint8_t>(sel));

			insert = boost::regex_search(s.begin(), s.end(), reg);
		} 
		catch(...) 
		{
			insert = true;
		}
	}
	return insert;
}
	

void SearchFrame::updateSearchList(SearchInfo* si) 
{
	int64_t size = -1;
	FilterModes mode = NONE;

	int sel = ctrlFilterSel->GetSelection();
	bool doSizeCompare = sel == COLUMN_SIZE && parseFilter(mode, size);

	if(si != NULL) 
	{
//		if(!matchFilter(si, sel, doSizeCompare, mode, size))
//			ctrlResults.deleteItem(si);
	} 
	else 
	{
		ctrlResults->Freeze();
		ctrlResults->deleteAllItems(false);

		for(SearchInfoList::ParentMap::const_iterator i = ctrlResults->getParents().begin(); i != ctrlResults->getParents().end(); ++i) 
		{
			SearchInfo* si = (*i).second.parent;
			si->collapsed = true;
			if(matchFilter(si, sel, doSizeCompare, mode, size)) 
			{
				dcassert(ctrlResults->findItem(si) == -1);
			/*	int k = ctrlResults->insertItem(si);

			 	const vector<SearchInfo*>& children = ctrlResults->findChildren(si->getGroupCond());
			TODO	if(!children.empty()) 
				{
					if(si->collapsed) 
					{
						ctrlResults->SetItemState(k, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);	
					} 
					else 
					{
						ctrlResults->SetItemState(k, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);	
					}
				} else {
					ctrlResults->SetItemState(k, INDEXTOSTATEIMAGEMASK(0), LVIS_STATEIMAGEMASK);	
				}*/
			}
		}
		ctrlResults->Thaw();
	}
}

void SearchFrame::on(SearchManagerListener::SR, const SearchResultPtr& aResult) throw()
{
	// Check that this is really a relevant search result...
	{
		Lock l(cs);

		if(search.empty()) 
		{
			return;
		}

		if(!aResult->getToken().empty() && token != aResult->getToken()) 
		{
			droppedResults++;
			callAsync(std::bind(&SearchFrame::onFilterResult, this));
			return;
		}
		
		if(isHash) 
		{
			if(aResult->getType() != SearchResult::TYPE_FILE || TTHValue(Text::fromT(search[0])) != aResult->getTTH()) 
			{
				droppedResults++;
				callAsync(std::bind(&SearchFrame::onFilterResult, this));
				return;
			}
		} 
		else 
		{
			// match all here
			for(TStringIter j = search.begin(); j != search.end(); ++j) 
			{
				if((*j->begin() != _T('-') && Util::findSubString(aResult->getFile(), Text::fromT(*j)) == -1) ||
					(*j->begin() == _T('-') && j->size() != 1 && Util::findSubString(aResult->getFile(), Text::fromT(j->substr(1))) != -1)
					) 
				{
					droppedResults++;
					callAsync(std::bind(&SearchFrame::onFilterResult, this));
					return;
				}
			}
		}
	}

	// Reject results without free slots
	if((onlyFree && aResult->getFreeSlots() < 1) ||
	   (exactSize1 && (aResult->getSize() != exactSize2)))
	{
		droppedResults++;
		callAsync(std::bind(&SearchFrame::onFilterResult, this));
		return;
	}
	
	SearchInfo* i = new SearchInfo(aResult);
	callAsync(std::bind(&SearchFrame::addSearchResult, this, new SearchInfo(aResult)));
}

void SearchFrame::on(TimerManagerListener::Second, uint64_t aTick) throw()
{
	Lock l(cs);
	
	if(waiting) 
	{
		if(aTick <= searchEndTime)
		{
			TCHAR buf[64];
			_stprintf(buf, _T("%s %s"), CTSTRING(TIME_LEFT), Util::formatSeconds(searchEndTime > aTick ? (searchEndTime - aTick) / 1000 : 0).c_str());
			callAsync(std::bind(&SearchFrame::onQueueStats, this, new tstring(buf)));
		}
		else
		{
			waiting = false;
			callAsync(std::bind(&SearchFrame::onQueueStats, this, (tstring*)NULL));
		}
	}
}

void SearchFrame::addSearchResult(SearchInfo* si) 
{
	const SearchResultPtr& sr = si->sr;
    // Check previous search results for dupes
	if(!si->getText(COLUMN_TTH).empty() && useGrouping) {
		SearchInfoList::ParentPair* pp = ctrlResults->findParentPair(sr->getTTH());
		if(pp) 
		{
			if((sr->getUser()->getCID() == pp->parent->getUser()->getCID()) && (sr->getFile() == pp->parent->sr->getFile())) 
			{
				delete si;
				return;	 	
			} 	
			for(vector<SearchInfo*>::const_iterator k = pp->children.begin(); k != pp->children.end(); k++)
			{
				if((sr->getUser()->getCID() == (*k)->getUser()->getCID()) && (sr->getFile() == (*k)->sr->getFile())) 
				{
					delete si;
					return;	 	
				} 	
			}	 	
		}
	} else {
		for(SearchInfoList::ParentMap::const_iterator s = ctrlResults->getParents().begin(); s != ctrlResults->getParents().end(); ++s) 
		{
			SearchInfo* si2 = (*s).second.parent;
	        const SearchResultPtr& sr2 = si2->sr;
			if((sr->getUser()->getCID() == sr2->getUser()->getCID()) && (sr->getFile() == sr2->getFile())) 
			{
				delete si;	 	
				return;	 	
			}
		}	 	
    }

	if(running) 
	{
		bool resort = false;
		resultsCount++;

		if(ctrlResults->getSortColumn() == COLUMN_HITS && resultsCount % 15 == 0) 
		{
			resort = true;
		}

		if(!si->getText(COLUMN_TTH).empty() && useGrouping) 
		{
			ctrlResults->insertGroupedItem(si, expandSR);
		} 
		else 
		{
			SearchInfoList::ParentPair pp = { si, SearchInfoList::emptyVector };
			ctrlResults->insertItem(si);
			ctrlResults->getParents().insert(make_pair(const_cast<TTHValue*>(&sr->getTTH()), pp));
		}

		if(!filter.empty())
			updateSearchList(si);

		if (BOOLSETTING(BOLD_SEARCH)) 
		{
			setDirty();
		}
		GetStatusBar()->SetStatusText(Util::toStringW(resultsCount) + _T(" ") + TSTRING(FILES), 3);

		if(resort) 
		{
			ctrlResults->resort();
		}
	} 
	else 
	{
		// searching is paused, so store the result but don't show it in the GUI (show only information: visible/all results)
		pausedResults.push_back(si);
		GetStatusBar()->SetStatusText(Util::toStringW(resultsCount) + _T("/") + Util::toStringW(pausedResults.size() + resultsCount) + _T(" ") + WSTRING(FILES), 3);
	}
}

void SearchFrame::initHubs() 
{
	ctrlHubs->Insert(TSTRING(ONLY_WHERE_OP), 0);
		
	ClientManager* clientMgr = ClientManager::getInstance();
	clientMgr->lock();
	clientMgr->addListener(this);

	const Client::List& clients = clientMgr->getClients();

	Client::Iter it;
	Client::Iter endIt = clients.end();
	for(it = clients.begin(); it != endIt; ++it) 
	{
		Client* client = it->second;
		if (!client->isConnected())
			continue;

		onHubAdded(new HubInfo(client));
	}

	clientMgr->unlock();

}

void SearchFrame::onHubAdded(HubInfo* info) 
{
	hubs.push_back(info);
	
	int item = ctrlHubs->Append(info->name);
	ctrlHubs->Check(item, ctrlHubs->IsChecked(0) ? info->op : true);
}

void SearchFrame::onHubChanged(HubInfo* info) 
{
	int nItem = 0;
	int n = hubs.size();
	for(; nItem < n; ++nItem)
	{
		if(hubs[nItem]->url == info->url)
			break;
	}
	if (nItem == n)
		return;

	delete hubs[nItem];
	hubs[nItem] = info;
	ctrlHubs->SetString(nItem + 1, info->name);

	if (ctrlHubs->IsChecked(0))
		ctrlHubs->Check(nItem, info->op);
}

void SearchFrame::onHubRemoved(HubInfo* info) 
{
	int nItem = 0;
	int n = hubs.size();
	for(; nItem < n; ++nItem)
	{
		if(hubs[nItem]->url == info->url)
			break;
	}

	delete info;

	if (nItem == n)
		return;

	delete hubs[nItem];
	hubs.erase(hubs.begin() + nItem);

	ctrlHubs->Delete(nItem + 1);
}

void SearchFrame::SearchInfo::view() 
{
	try 
	{
		if(sr->getType() == SearchResult::TYPE_FILE) 
		{
			QueueManager::getInstance()->add(Util::getTempPath() + sr->getFileName(),
				sr->getSize(), sr->getTTH(), HintedUser(sr->getUser(), sr->getHubURL()),
				QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_TEXT);
		}
	} catch(const Exception&) {
	}
}

void SearchFrame::SearchInfo::Download::operator()(SearchInfo* si) 
{
	try 
	{
		if(si->sr->getType() == SearchResult::TYPE_FILE) {
			string target = Text::fromT(tgt + tstring(si->getText(COLUMN_FILENAME)));
			QueueManager::getInstance()->add(target, si->sr->getSize(), 
				si->sr->getTTH(), HintedUser(si->sr->getUser(), si->sr->getHubURL()));
			
			const vector<SearchInfo*>& children = sf->getUserList().findChildren(si->getGroupCond());
			for(SearchInfo::Iter i = children.begin(); i != children.end(); i++) 
			{
				SearchInfo* j = *i;
				try 
				{
					QueueManager::getInstance()->add(Text::fromT(tgt + tstring(si->getText(COLUMN_FILENAME))), j->sr->getSize(), j->sr->getTTH(), HintedUser(j->getUser(), j->sr->getHubURL()));
				} 
				catch(const Exception&) 
				{
				}
			}
			if(wxGetKeyState(WXK_SHIFT))
				QueueManager::getInstance()->setPriority(target, QueueItem::HIGHEST);
		} 
		else 
		{
			QueueManager::getInstance()->addDirectory(si->sr->getFile(), HintedUser(si->sr->getUser(), si->sr->getHubURL()), Text::fromT(tgt),
				wxGetKeyState(WXK_SHIFT) ? QueueItem::HIGHEST : QueueItem::DEFAULT);
		}
	} 
	catch(const Exception&) 
	{
	}
}

void SearchFrame::SearchInfo::DownloadWhole::operator()(SearchInfo* si) 
{
	try 
	{
		QueueItem::Priority prio = wxGetKeyState(WXK_SHIFT) ? QueueItem::HIGHEST : QueueItem::DEFAULT;
		if(si->sr->getType() == SearchResult::TYPE_FILE) 
		{
			QueueManager::getInstance()->addDirectory(Text::fromT(tstring(si->getText(COLUMN_PATH))),
				HintedUser(si->sr->getUser(), si->sr->getHubURL()), Text::fromT(tgt), prio);
		} 
		else 
		{
			QueueManager::getInstance()->addDirectory(si->sr->getFile(), HintedUser(si->sr->getUser(), si->sr->getHubURL()), 
				Text::fromT(tgt), prio);
		}
	} 
	catch(const Exception&) 
	{
	}
}

void SearchFrame::SearchInfo::DownloadTarget::operator()(SearchInfo* si) 
{
	try 
	{
		if(si->sr->getType() == SearchResult::TYPE_FILE) 
		{
			string target = Text::fromT(tgt);
			QueueManager::getInstance()->add(target, si->sr->getSize(), 
				si->sr->getTTH(), HintedUser(si->sr->getUser(), si->sr->getHubURL()));

			if(wxGetKeyState(WXK_SHIFT))
				QueueManager::getInstance()->setPriority(target, QueueItem::HIGHEST);
		} 
		else 
		{
			QueueManager::getInstance()->addDirectory(si->sr->getFile(), HintedUser(si->sr->getUser(), si->sr->getHubURL()), Text::fromT(tgt),
				wxGetKeyState(WXK_SHIFT) ? QueueItem::HIGHEST : QueueItem::DEFAULT);
		}
	} 
	catch(const Exception&) 
	{
	}
}

void SearchFrame::SearchInfo::getList() 
{
	try 
	{
		QueueManager::getInstance()->addList(HintedUser(sr->getUser(), sr->getHubURL()), QueueItem::FLAG_CLIENT_VIEW, Text::fromT(tstring(getText(COLUMN_PATH))));
	} 
	catch(const Exception&) 
	{
		// Ignore for now...
	}
}

void SearchFrame::SearchInfo::browseList() 
{
	try 
	{
		QueueManager::getInstance()->addList(HintedUser(sr->getUser(), sr->getHubURL()), QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_PARTIAL_LIST, Text::fromT(tstring(getText(COLUMN_PATH))));
	} 
	catch(const Exception&) 
	{
		// Ignore for now...
	}
}

void SearchFrame::SearchInfo::CheckTTH::operator()(SearchInfo* si) 
{
	if(firstTTH) 
	{
		tth = si->getText(COLUMN_TTH);
		hasTTH = true;
		firstTTH = false;
	} 
	else if(hasTTH) 
	{
		if(tth != si->getText(COLUMN_TTH)) 
		{
			hasTTH = false;
		}
	} 

	if(firstHubs && hubs.empty()) 
	{
		hubs = ClientManager::getInstance()->getHubs(si->sr->getUser()->getCID(), si->sr->getHubURL());
		firstHubs = false;
	} 
	else if(!hubs.empty()) 
	{
		// we will merge hubs of all users to ensure we can use OP commands in all hubs
		StringList sl = ClientManager::getInstance()->getHubs(si->sr->getUser()->getCID(), si->sr->getHubURL());
		hubs.insert( hubs.end(), sl.begin(), sl.end() );
		//Util::intersect(hubs, ClientManager::getInstance()->getHubs(si->sr->getUser()->getCID()));
	}
}

void SearchFrame::runUserCommand(const UserCommand& uc) 
{
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	set<CID> users;

	wxDataViewItemArray sel;
	ctrlResults->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const SearchResultPtr& sr = ctrlResults->getItemData(sel[i])->sr;

		if(!sr->getUser()->isOnline())
			continue;

		if(uc.once()) {
			if(users.find(sr->getUser()->getCID()) != users.end())
				continue;
			users.insert(sr->getUser()->getCID());
		}

		ucParams["fileFN"] = sr->getFile();
		ucParams["fileSI"] = Util::toString(sr->getSize());
		ucParams["fileSIshort"] = Util::formatBytes(sr->getSize());
		if(sr->getType() == SearchResult::TYPE_FILE) {
			ucParams["fileTR"] = sr->getTTH().toBase32();
		}

		// compatibility with 0.674 and earlier
		ucParams["file"] = ucParams["fileFN"];
		ucParams["filesize"] = ucParams["fileSI"];
		ucParams["filesizeshort"] = ucParams["fileSIshort"];
		ucParams["tth"] = ucParams["fileTR"];

		StringMap tmp = ucParams;
		ClientManager::getInstance()->userCommand(HintedUser(sr->getUser(), sr->getHubURL()), uc, tmp, true);
	}
}