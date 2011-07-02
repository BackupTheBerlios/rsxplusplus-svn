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

#ifndef _FINISHEDFRAME_H
#define _FINISHEDFRAME_H

#include "StaticFrame.h"

#include "TypedDataViewCtrl.h"

#include "../client/FinishedManager.h"

#include <wx/wx.h>

template<bool D>
class FinishedFrame :
	public StaticFrame<FinishedFrame<D>>,
	protected FinishedManagerListener
{
public:

	typedef StaticFrame<FinishedFrame<D>> BaseType;

	FinishedFrame(void) : 
		BaseType(	D ? ID_CMD_FINISHED_DL : ID_CMD_FINISHED_UL, 
					D ? _("Finished downloads") : _("Finished uploads"), wxColour(0, 0, 0), 
					D ? wxT("IDI_FINISHED_DL") : wxT("IDI_FINISHED_UL")),
		totalSpeed(0), totalBytes(0)
	{
		int widths[4] = { -1, 100, 100, 100 };
		CreateStatusBar(4, wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE);
		SetStatusWidths(4, widths);

		ctrlList = new TypedDataViewCtrl<FinishedItem>(this, ID_LIST_CTRL, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxHSCROLL);
		ctrlList->SetFont(WinUtil::font);
		ctrlList->SetBackgroundColour(WinUtil::bgColor);
		ctrlList->SetForegroundColour(WinUtil::textColor);
		ctrlList->setImageList(&WinUtil::fileImages);
		ctrlList->Connect(wxEVT_CHAR, wxKeyEventHandler(FinishedFrame<D>::OnKeyDown), NULL, this);

		for(uint8_t j = 0; j < FinishedItem::COLUMN_LAST; j++) 
		{
			wxAlignment fmt = (j == FinishedItem::COLUMN_SIZE || j == FinishedItem::COLUMN_SPEED) ? wxALIGN_RIGHT : wxALIGN_LEFT;
			ctrlList->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j]);
		}

		ctrlList->setSortColumn(FinishedItem::COLUMN_DONE);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(ctrlList, 1, wxEXPAND);
		SetSizerAndFit(sizer);

		// bind events
		Bind(wxEVT_CLOSE_WINDOW, &FinishedFrame<D>::OnClose, this);

		FinishedManager::getInstance()->addListener(this);
		const FinishedItemList& fl = FinishedManager::getInstance()->lockList(!D);
		for(FinishedItemList::const_iterator i = fl.begin(); i != fl.end(); ++i) 
		{
			addEntry(*i, false);
		}
		FinishedManager::getInstance()->unlockList();

		updateStatus();
	}

	~FinishedFrame(void) { }

private:

	// events
	void OnClose(wxCloseEvent& event)
	{
		FinishedManager::getInstance()->removeListener(this);
		
		ctrlList->deleteAllItems();

		WinUtil::setButtonPressed(GetId(), false);
		event.Skip();
	}

	void OnKeyDown(wxKeyEvent& event)
	{
		if(event.GetKeyCode() == WXK_DELETE)
		{
			wxDataViewItemArray sel;
			ctrlList->GetSelections(sel);
			for(unsigned int i = 0; i < sel.size(); ++i)
			{
				FinishedItem *ii = ctrlList->getItemData(sel[i]);
				FinishedManager::getInstance()->remove(ii, !D);
				ctrlList->deleteItem(ii);
					
				totalBytes -= ii->getSize();
				totalSpeed -= ii->getAvgSpeed();
					
				delete ii;
			}
		}
	}

	// methods
	void UpdateLayout() { }

	void addEntry(FinishedItem* entry, bool updateView = true) 
	{
		totalBytes += entry->getSize();
		totalSpeed += entry->getAvgSpeed();

		int loc = ctrlList->insertItem(entry);

		if(updateView)
		{
			ctrlList->ensureVisible(loc);

			if(SettingsManager::getInstance()->get(D ? SettingsManager::BOLD_FINISHED_DOWNLOADS : SettingsManager::BOLD_FINISHED_UPLOADS))
				setDirty();
			updateStatus();
		}
	}


	void updateStatus() 
	{
		int count = ctrlList->getItemCount();
		SetStatusText(Util::toStringW(count) + _T(" ") + TSTRING(ITEMS), 1);
		SetStatusText(Util::formatBytesW(totalBytes), 2);
		SetStatusText(Util::formatBytesW(count > 0 ? totalSpeed / count : 0) + _T("/s"), 3);
	}

	// attributes
	TypedDataViewCtrl<FinishedItem>* ctrlList;

	int64_t totalBytes;
	int64_t totalSpeed;

	static int columnSizes[FinishedItem::COLUMN_LAST];
	static int columnIndexes[FinishedItem::COLUMN_LAST];

	// listeners
	void on(AddedDl, FinishedItem* entry) throw() 
	{
		if(D)
			callAsync(std::bind(&FinishedFrame<D>::addEntry, this, entry, true));
	}

	void on(AddedUl, FinishedItem* entry) throw() 
	{
		if(!D)
			callAsync(std::bind(&FinishedFrame<D>::addEntry, this, entry, true));
	}

};

template<bool D>
int FinishedFrame<D>::columnIndexes[] = { FinishedItem::COLUMN_DONE, FinishedItem::COLUMN_FILE,
	FinishedItem::COLUMN_PATH, FinishedItem::COLUMN_NICK, FinishedItem::COLUMN_HUB, FinishedItem::COLUMN_SIZE, FinishedItem::COLUMN_SPEED };

template<bool D>
int FinishedFrame<D>::columnSizes[] = { 100, 110, 290, 125, 80, 80 };
static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::TIME, ResourceManager::PATH, 
	ResourceManager::NICK, ResourceManager::HUB, ResourceManager::SIZE, ResourceManager::SPEED };

int FinishedItem::getImageIndex() const { return WinUtil::getIconIndex(Text::toT(getTarget())); }

#endif	// _FINISHEDFRAMEBASE_H