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
#include "TransferView.h"

#include "CountryFlags.h"
#include "ProgressBarRenderer.h"
#include "WinUtil.h"

#include "../client/ConnectionManager.h"
#include "../client/Download.h"
#include "../client/DownloadManager.h"
#include "../client/QueueManager.h"
#include "../client/Upload.h"
#include "../client/UploadManager.h"
#include "../client/User.h"
#include "../client/UserConnection.h"
#include "../client/SettingsManager.h"
#include "../client/version.h"

#include <wx/sound.h>
#include <wx/wx.h>

BEGIN_EVENT_TABLE(TransferView, wxWindow)
	EVT_SIZE(TransferView::OnSize)
	EVT_DATAVIEW_ITEM_ACTIVATED(ID_LIST_CTRL, TransferView::OnDblClick)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(ID_LIST_CTRL, TransferView::OnContextMenu)

	EVT_MENU(IDC_FORCE, TransferView::OnForce)
	EVT_MENU(ID_SEARCH_ALTERNATES, TransferView::OnSearchForAlternates)
	EVT_MENU(IDC_MENU_SLOWDISCONNECT, TransferView::OnSlowDisconnect)
	EVT_MENU(IDC_DISCONNECT_ALL, TransferView::OnDisconnectAll)
	EVT_MENU(IDC_EXPAND_ALL, TransferView::OnExpandAll)
	EVT_MENU(IDC_COLLAPSE_ALL, TransferView::OnCollapseAll)
	EVT_MENU(IDC_REMOVE, TransferView::OnRemove)
	EVT_MENU(IDC_REMOVEALL, TransferView::OnRemoveAll)
END_EVENT_TABLE()


int TransferView::columnIndexes[] = { COLUMN_USER, COLUMN_HUB, COLUMN_STATUS, COLUMN_TIMELEFT, COLUMN_SPEED, COLUMN_FILE, COLUMN_SIZE, COLUMN_PATH, COLUMN_CIPHER, COLUMN_IP, COLUMN_RATIO };
int TransferView::columnSizes[] = { 150, 150, 250, 75, 75, 175, 100, 200, 100, 150, 50 };

static ResourceManager::Strings columnNames[] = { ResourceManager::USER, ResourceManager::HUB_SEGMENTS, ResourceManager::STATUS,
	ResourceManager::TIME_LEFT, ResourceManager::SPEED, ResourceManager::FILENAME, ResourceManager::SIZE, ResourceManager::PATH,
	ResourceManager::CIPHER, ResourceManager::IP_BARE, ResourceManager::RATIO};

TransferView::TransferView(wxWindow* parent) : wxWindow(parent, ID_TRANSFER_VIEW, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN | wxCLIP_SIBLINGS)
{
	arrows.Create(16, 16, false);
	arrows.Add(wxBitmap(wxT("IDB_ARROWS")), wxBITMAP_TYPE_BMP_RESOURCE);

	ctrlTransfers = new ItemInfoList(this, ID_LIST_CTRL, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN | wxCLIP_SIBLINGS | wxVSCROLL | wxHSCROLL);

	WinUtil::splitTokens(columnIndexes, SETTING(MAINFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(MAINFRAME_WIDTHS), COLUMN_LAST);

	for(uint8_t j=0; j < COLUMN_LAST; ++j)
	{
		wxAlignment fmt = (j == COLUMN_SIZE || j == COLUMN_TIMELEFT || j == COLUMN_SPEED) ? wxALIGN_RIGHT : wxALIGN_LEFT;

		wxDataViewRenderer* rndr = NULL;
		switch(j)
		{
			case COLUMN_STATUS: rndr = new ProgressBarRenderer<TransferView>(); break;
			case COLUMN_IP: rndr = new CountryFlags::FlagRenderer; break;
		}
		ctrlTransfers->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], rndr);
	}
	
	ctrlTransfers->SetFont(WinUtil::font);
	ctrlTransfers->SetBackgroundColour(WinUtil::bgColor);
	ctrlTransfers->SetForegroundColour(WinUtil::textColor);
	/* TODO ctrlTransfers->setColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlTransfers->setVisible(SETTING(MAINFRAME_VISIBLE));*/
	ctrlTransfers->setImageList(&arrows);
	ctrlTransfers->setSortColumn(COLUMN_USER);

	ctrlTransfers->Connect(wxEVT_CHAR, wxKeyEventHandler(TransferView::OnChar), NULL, this);

	ConnectionManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);
	SettingsManager::getInstance()->addListener(this);
}

TransferView::~TransferView(void)
{
}

void TransferView::prepareClose() 
{
	/* TODO ctrlTransfers->saveHeaderOrder(SettingsManager::MAINFRAME_ORDER, SettingsManager::MAINFRAME_WIDTHS,
		SettingsManager::MAINFRAME_VISIBLE);*/

	ConnectionManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);
	QueueManager::getInstance()->removeListener(this);
	SettingsManager::getInstance()->removeListener(this);

	ctrlTransfers->deleteAllItems();
}

bool TransferView::GetItemAttr(void* data, int64_t& position, int64_t& size, wxColour& backColour, wxColour& textColour)
{
	ItemInfo* ii = (ItemInfo*)data;
	
	if(ii->status == ItemInfo::STATUS_RUNNING)
	{
		position = ii->pos;
		size = ii->size;
		backColour = SETTING(PROGRESS_OVERRIDE_COLORS) ? 
				(ii->download ? (ii->parent ? SETTING(PROGRESS_SEGMENT_COLOR) : SETTING(DOWNLOAD_BAR_COLOR)) :
				SETTING(UPLOAD_BAR_COLOR)) : wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		// TODO
		textColour = (ii->download ? SETTING(PROGRESS_TEXT_COLOR_DOWN) : SETTING(PROGRESS_TEXT_COLOR_UP));

		return true;
	}

	return false;
}

void TransferView::OnSize(wxSizeEvent& event)
{
	wxRect rc = this->GetClientRect();
	ctrlTransfers->SetSize(rc);

	event.Skip();
}

void TransferView::OnTasks()
{
	TaskQueue::List t;
	tasks.get(t);

	if(t.size() > 2) 
		ctrlTransfers->Freeze();

	for(TaskQueue::Iter i = t.begin(); i != t.end(); ++i) 
	{
		switch(i->first)
		{
			case ADD_ITEM: 
			{
				auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(i->second));
				ItemInfo* ii = new ItemInfo(ui->user, ui->download);
				ii->update(*ui);
				if(ii->download) 
				{
					ctrlTransfers->insertGroupedItem(ii, false);
				} 
				else 
				{
					ctrlTransfers->insertItem(ii);
				}

				break;
			}
			case REMOVE_ITEM:
			{
				auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(i->second));

				int pos = -1;
				ItemInfo* ii = findItem(*ui, pos);
				if(ii) 
				{
					if(ui->download) 
					{
						ctrlTransfers->removeGroupedItem(ii);
					} 
					else 
					{
						dcassert(pos != -1);
						ctrlTransfers->deleteItem(pos);
						delete ii;
					}
				}
				break;
			}
			case UPDATE_ITEM: 
			{
				auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(i->second));

				int pos = -1;
				ItemInfo* ii = findItem(*ui, pos);
				if(ii) 
				{
					if(ui->download) 
					{
						ItemInfo* parent = ii->parent ? ii->parent : ii;

						if(ui->type == Transfer::TYPE_FILE || ui->type == Transfer::TYPE_TREE)
						{
							/* parent item must be updated with correct info about whole file */
							if(ui->status == ItemInfo::STATUS_RUNNING && parent->status == ItemInfo::STATUS_RUNNING && parent->hits == -1)
							{
								ui->updateMask &= ~UpdateInfo::MASK_POS;
								ui->updateMask &= ~UpdateInfo::MASK_ACTUAL;
								ui->updateMask &= ~UpdateInfo::MASK_SIZE;
								ui->updateMask &= ~UpdateInfo::MASK_STATUS_STRING;
								ui->updateMask &= ~UpdateInfo::MASK_TIMELEFT;
							}
						}

						/* if target has changed, regroup the item */
						bool changeParent = (ui->updateMask & UpdateInfo::MASK_FILE) && (ui->target != ii->target);
						if(changeParent)
							ctrlTransfers->removeGroupedItem(ii, false);

						ii->update(*ui);

						if(changeParent) 
						{
							ctrlTransfers->insertGroupedItem(ii, false);
							parent = ii->parent ? ii->parent : ii;
						} 
						else if(ii == parent || !parent->collapsed) 
						{
							updateItem(ctrlTransfers->findItem(ii), ui->updateMask);
						}
						continue;
					}
					ii->update(*ui);
					dcassert(pos != -1);
					updateItem(pos, ui->updateMask);
				}
				break;
			}
			case UPDATE_PARENT: 
			{
				auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(i->second));
				ItemInfoList::ParentPair* pp = ctrlTransfers->findParentPair(ui->target);
			
				if(!pp) 
					continue;

				if(ui->user.user) 
				{
					int pos = -1;
					ItemInfo* ii = findItem(*ui, pos);
					if(ii) 
					{
						ii->status = ui->status;
						ii->statusString = ui->statusString;

						if(!pp->parent->collapsed) 
						{
							updateItem(ctrlTransfers->findItem(ii), ui->updateMask);
						}
					}
				}

				pp->parent->update(*ui);
				updateItem(ctrlTransfers->findItem(pp->parent), ui->updateMask);
				break;
			}
		}
	}

	if(!t.empty()) 
	{
		ctrlTransfers->resort();
		if(t.size() > 2) 
			ctrlTransfers->Thaw();
	}
}

void TransferView::OnDblClick(wxDataViewEvent& event)
{

	ItemInfo* i = ctrlTransfers->getItemData(event.GetItem());

	const vector<ItemInfo*>& children = ctrlTransfers->findChildren(i->getGroupCond());
	if(i->parent != NULL || children.size() <= 1) 
	{
		switch(SETTING(TRANSFERLIST_DBLCLICK)) 
		{
			case 0:
				i->pm(i->user.hint);
				break;
			case 1:
				i->getList(i->user.hint);
				break;
			case 2:
				i->matchQueue(i->user.hint);
			case 3:
				i->grant(i->user.hint);
				break;
			case 4:
				i->addFav();
				break;
			case 5:
				i->browseList(i->user.hint);
				break;
		}
	}
}

void TransferView::OnContextMenu(wxDataViewEvent& /*event*/)
{
	wxDataViewItemArray sel;
	ctrlTransfers->GetSelections(sel);

	const ItemInfo* ii = ctrlTransfers->getItemData(sel[0]);
	bool parent = !ii->parent && ctrlTransfers->findChildren(ii->getGroupCond()).size() > 1;

	wxMenu transferMenu;	
	wxMenu* previewMenu = WinUtil::getPreviewMenu(ii->target);

	if(!parent)
	{
		transferMenu.SetTitle(Text::toT(ClientManager::getInstance()->getNicks(ii->user)[0]));
		appendUserItems(transferMenu, ii->user.hint);
		transferMenu.AppendSeparator();
		transferMenu.Append(IDC_FORCE, CTSTRING(FORCE_ATTEMPT));
		
		if(ii->download) 
		{
			transferMenu.AppendSeparator();
			transferMenu.Append(ID_SEARCH_ALTERNATES, CTSTRING(SEARCH_FOR_ALTERNATES));
			transferMenu.AppendCheckItem(IDC_MENU_SLOWDISCONNECT, CTSTRING(SETSTRONGDC_DISCONNECTING_ENABLE));
			transferMenu.AppendSubMenu(previewMenu, CTSTRING(PREVIEW_MENU));
		}

		if(ii->user.user)
			prepareMenu(transferMenu, UserCommand::CONTEXT_CHAT, ClientManager::getInstance()->getHubs(ii->user.user->getCID(), ii->user.hint));

		transferMenu.AppendSeparator();
		transferMenu.Append(IDC_REMOVE, CTSTRING(CLOSE_CONNECTION));
	} 
	else 
	{
		transferMenu.SetTitle(TSTRING(SETTINGS_SEGMENT));
		transferMenu.Append(ID_SEARCH_ALTERNATES, CTSTRING(SEARCH_FOR_ALTERNATES));
		transferMenu.AppendSubMenu(previewMenu, CTSTRING(PREVIEW_MENU));
		transferMenu.AppendCheckItem(IDC_MENU_SLOWDISCONNECT, CTSTRING(SETSTRONGDC_DISCONNECTING_ENABLE));
		transferMenu.AppendSeparator();
		transferMenu.Append(IDC_FORCE, CTSTRING(CONNECT_ALL));
		transferMenu.Append(IDC_DISCONNECT_ALL, CTSTRING(DISCONNECT_ALL));
		transferMenu.AppendSeparator();
		transferMenu.Append(IDC_EXPAND_ALL, CTSTRING(EXPAND_ALL));
		transferMenu.Append(IDC_COLLAPSE_ALL, CTSTRING(COLLAPSE_ALL));
		transferMenu.AppendSeparator();
		transferMenu.Append(IDC_REMOVEALL, CTSTRING(REMOVE_ALL));
	}

	if(ii->download) 
	{
		if(!ii->target.empty()) 
		{
			string target = Text::fromT(ii->target);
			/* TODO 
			if(previewMenu.GetMenuItemCount() > 1) {
				transferMenu.EnableMenuItem((UINT)(HMENU)previewMenu, MFS_ENABLED);
			} else {
				transferMenu.EnableMenuItem((UINT)(HMENU)previewMenu, MFS_DISABLED);
			}*/

			const QueueItem::StringMap& queue = QueueManager::getInstance()->lockQueue();
			QueueItem::StringIter qi = queue.find(&target);

			bool slowDisconnect = false;
			if(qi != queue.end())
				slowDisconnect = qi->second->isSet(QueueItem::FLAG_AUTODROP);

			QueueManager::getInstance()->unlockQueue();

			if(slowDisconnect)
				transferMenu.Check(IDC_MENU_SLOWDISCONNECT, true);
		}
	}

	PopupMenu(&transferMenu);
}

void TransferView::OnChar(wxKeyEvent& event)
{
	if(event.GetKeyCode() == WXK_DELETE)
	{
		wxCommandEvent evt;
		OnRemove(evt);
	}
}

void TransferView::OnForce(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	ctrlTransfers->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		ItemInfo* ii = ctrlTransfers->getItemData(sel[i]);
		// TODO ctrlTransfers.SetItemText(i, COLUMN_STATUS, CTSTRING(CONNECTING_FORCED));

		if(ii->parent == NULL && ii->hits != -1) 
		{
			const vector<ItemInfo*>& children = ctrlTransfers->findChildren(ii->getGroupCond());
			for(vector<ItemInfo*>::const_iterator j = children.begin(); j != children.end(); ++j) 
			{
				ItemInfo* ii = *j;

				/* TODO int h = ctrlTransfers->findItem(ii);
				if(h != -1)
					ctrlTransfers->SetItemText(h, COLUMN_STATUS, CTSTRING(CONNECTING_FORCED));*/

				ii->transferFailed = false;
				ConnectionManager::getInstance()->force(ii->user);
			}
		} 
		else 
		{
			ii->transferFailed = false;
			ConnectionManager::getInstance()->force(ii->user);
		}
	}
}

void TransferView::OnSearchForAlternates(wxCommandEvent& /*event*/)
{
	// don't spam with too many searches, so search only for the first selected file
	const ItemInfo *ii = ctrlTransfers->getItemData(ctrlTransfers->getFirstSelectedItem());
	
	TTHValue tth;
	if(QueueManager::getInstance()->getTTH(Text::fromT(ii->target), tth)) 
	{
		WinUtil::searchHash(tth);
	}
}

void TransferView::OnSlowDisconnect(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	ctrlTransfers->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const ItemInfo *ii = ctrlTransfers->getItemData(sel[i]);
		const QueueItem::StringMap& queue = QueueManager::getInstance()->lockQueue();

		string tmp = Text::fromT(ii->target);
		QueueItem::StringIter qi = queue.find(&tmp);

		if(qi != queue.end()) 
		{
			if(qi->second->isSet(QueueItem::FLAG_AUTODROP)) 
			{
				qi->second->unsetFlag(QueueItem::FLAG_AUTODROP);
			} 
			else 
			{
				qi->second->setFlag(QueueItem::FLAG_AUTODROP);
			}
		}

		QueueManager::getInstance()->unlockQueue();
	}

}

void TransferView::OnDisconnectAll(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	ctrlTransfers->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const ItemInfo* ii = ctrlTransfers->getItemData(sel[i]);
		
		const vector<ItemInfo*>& children = ctrlTransfers->findChildren(ii->getGroupCond());
		for(vector<ItemInfo*>::const_iterator j = children.begin(); j != children.end(); ++j) 
		{
			ItemInfo* ii = *j;
			ii->disconnect();

			/* TODO int h = ctrlTransfers.findItem(ii);
			if(h != -1)
				ctrlTransfers.SetItemText(h, COLUMN_STATUS, CTSTRING(DISCONNECTED));*/
		}

		// TODO ctrlTransfers.SetItemText(i, COLUMN_STATUS, CTSTRING(DISCONNECTED));
	}
}

void TransferView::OnExpandAll(wxCommandEvent& /*event*/)
{
	for(ItemInfoList::ParentMap::const_iterator i = ctrlTransfers->getParents().begin(); i != ctrlTransfers->getParents().end(); ++i) 
	{
		ItemInfo* l = (*i).second.parent;
		if(l->collapsed) 
			ctrlTransfers->Expand(l, ctrlTransfers->findItem(l));
	}
}

void TransferView::OnCollapseAll(wxCommandEvent& /*event*/)
{
	for(int i = ctrlTransfers->getItemCount() - 1; i >= 0; --i) 
	{
		ItemInfo* ii = ctrlTransfers->getItemData(i);
		if(ii->download && ii->parent)
			ctrlTransfers->deleteItem(ii); 

		if(ii->download && !ii->parent && !ii->collapsed) 
		{
			ii->collapsed = true;
			// TODO ctrlTransfers.SetItemState(ctrlTransfers.findItem(m), INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
		 }
	}

}

TransferView::ItemInfo::ItemInfo(const HintedUser& u, bool aDownload) : user(u), download(aDownload), transferFailed(false),
	status(STATUS_WAITING), pos(0), size(0), actual(0), speed(0), timeLeft(0), ip(Util::emptyStringT), target(Util::emptyStringT),
	flagIndex(0), collapsed(true), parent(NULL), hits(-1), statusString(Util::emptyStringT), running(0) { }

void TransferView::ItemInfo::update(const UpdateInfo& ui) 
{
	if(ui.type != Transfer::TYPE_LAST)
		type = ui.type;
		
	if(ui.updateMask & UpdateInfo::MASK_STATUS) 
	{
		status = ui.status;
	}
	if(ui.updateMask & UpdateInfo::MASK_STATUS_STRING) 
	{
		// No slots etc from transfermanager better than disconnected from connectionmanager
		if(!transferFailed)
			statusString = ui.statusString;
		transferFailed = ui.transferFailed;
	}
	if(ui.updateMask & UpdateInfo::MASK_SIZE) 
	{
		size = ui.size;
	}
	if(ui.updateMask & UpdateInfo::MASK_POS) 
	{
		pos = ui.pos;
	}
	if(ui.updateMask & UpdateInfo::MASK_ACTUAL) 
	{
		actual = ui.actual;
	}
	if(ui.updateMask & UpdateInfo::MASK_SPEED) 
	{
		speed = ui.speed;
	}
	if(ui.updateMask & UpdateInfo::MASK_FILE) 
	{
		target = ui.target;
	}
	if(ui.updateMask & UpdateInfo::MASK_TIMELEFT) 
	{
		timeLeft = ui.timeLeft;
	}
	if(ui.updateMask & UpdateInfo::MASK_IP) 
	{
		flagIndex = ui.flagIndex;
		ip = ui.IP;
	}
	if(ui.updateMask & UpdateInfo::MASK_CIPHER) 
	{
		cipher = ui.cipher;
	}	
	if(ui.updateMask & UpdateInfo::MASK_SEGMENT) 
	{
		running = ui.running;
	}
}

int TransferView::ItemInfo::compareItems(const ItemInfo* a, const ItemInfo* b, uint8_t col) 
{
	if(a->status == b->status) 
	{
		if(a->download != b->download) 
		{
			return a->download ? -1 : 1;
		}
	} 
	else 
	{
		return (a->status == ItemInfo::STATUS_RUNNING) ? -1 : 1;
	}

	switch(col) 
	{
		case COLUMN_USER: 
		{
			if(a->hits == b->hits)
				return lstrcmpi(a->getText(COLUMN_USER).c_str(), b->getText(COLUMN_USER).c_str());
			return compare(a->hits, b->hits);						
		}
		case COLUMN_HUB: 
		{
			if(a->running == b->running)
				return lstrcmpi(a->getText(COLUMN_HUB).c_str(), b->getText(COLUMN_HUB).c_str());
			return compare(a->running, b->running);						
		}
		case COLUMN_STATUS: return 0;
		case COLUMN_TIMELEFT: return compare(a->timeLeft, b->timeLeft);
		case COLUMN_SPEED: return compare(a->speed, b->speed);
		case COLUMN_SIZE: return compare(a->size, b->size);
		case COLUMN_RATIO: return compare(a->getRatio(), b->getRatio());
		default: return lstrcmpi(a->getText(col).c_str(), b->getText(col).c_str());
	}
}

void TransferView::ItemInfo::disconnect() 
{
	ConnectionManager::getInstance()->disconnect(user, download);
}

void TransferView::ItemInfo::removeAll() 
{
	if(hits <= 1) 
	{
		QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
	} 
	else 
	{
		if(!BOOLSETTING(CONFIRM_DELETE) || wxMessageBox(wxT("Do you really want to remove this item?"), wxT(APPNAME) wxT(" ") wxT(VERSIONSTRING), wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT) == wxYES)
			QueueManager::getInstance()->remove(Text::fromT(target));
	}
}

static tstring getFile(const Transfer::Type& type, const tstring& fileName) 
{
	tstring file;

	if(type == Transfer::TYPE_TREE) 
	{
		file = _T("TTH: ") + fileName;
	} 
	else if(type == Transfer::TYPE_FULL_LIST || type == Transfer::TYPE_PARTIAL_LIST) 
	{
		file = TSTRING(FILE_LIST);
	} 
	else 
	{
		file = fileName;
	}
	return file;
}

wxString TransferView::ItemInfo::getText(uint8_t col) const 
{
	switch(col) 
	{
		case COLUMN_USER: return (hits == -1) ? WinUtil::getNicks(user) : (Util::toStringW(hits) + _T(' ') + TSTRING(USERS));
		case COLUMN_HUB: return (hits == -1) ? WinUtil::getHubNames(user).first : (Util::toStringW(running) + _T(' ') + TSTRING(NUMBER_OF_SEGMENTS));
		case COLUMN_STATUS: return statusString;
		case COLUMN_TIMELEFT: return (status == STATUS_RUNNING) ? Util::formatSeconds(timeLeft) : Util::emptyStringT;
		case COLUMN_SPEED: return (status == STATUS_RUNNING) ? (Util::formatBytesW(speed) + _T("/s")) : Util::emptyStringT;
		case COLUMN_FILE: return getFile(type, Util::getFileName(target));
		case COLUMN_SIZE: return Util::formatBytesW(size); 
		case COLUMN_PATH: return Util::getFilePath(target);
		case COLUMN_IP: return ip;
		case COLUMN_RATIO: return (status == STATUS_RUNNING) ? Util::toStringW(getRatio()) : Util::emptyStringT;
		case COLUMN_CIPHER: return cipher;
		default: return Util::emptyStringT;
	}
}

TransferView::ItemInfo* TransferView::findItem(const UpdateInfo& ui, int& pos) const 
{
	for(unsigned int j = 0; j < ctrlTransfers->getItemCount(); ++j) 
	{
		ItemInfo* ii = ctrlTransfers->getItemData(j);
		if(ui == *ii) 
		{
			pos = j;
			return ii;
		} 
		else if(ui.download && ii->download && ii->parent == NULL) 
		{
			const vector<ItemInfo*>& children = ctrlTransfers->findChildren(ii->getGroupCond());
			for(vector<ItemInfo*>::const_iterator k = children.begin(); k != children.end(); k++) 
			{
				ItemInfo* ii = *k;
				if(ui == *ii) 
				{
					return ii;
				}
			}
		}
	}
	return NULL;
}

void TransferView::updateItem(int ii, uint32_t updateMask) 
{
	if(	updateMask & UpdateInfo::MASK_STATUS || updateMask & UpdateInfo::MASK_STATUS_STRING ||
		updateMask & UpdateInfo::MASK_POS || updateMask & UpdateInfo::MASK_ACTUAL) {
		ctrlTransfers->updateItem(ii, COLUMN_STATUS);
	}
	if(updateMask & UpdateInfo::MASK_POS || updateMask & UpdateInfo::MASK_ACTUAL) {
		ctrlTransfers->updateItem(ii, COLUMN_RATIO);
	}
	if(updateMask & UpdateInfo::MASK_SIZE) {
		ctrlTransfers->updateItem(ii, COLUMN_SIZE);
	}
	if(updateMask & UpdateInfo::MASK_SPEED) {
		ctrlTransfers->updateItem(ii, COLUMN_SPEED);
	}
	if(updateMask & UpdateInfo::MASK_FILE) {
		ctrlTransfers->updateItem(ii, COLUMN_PATH);
		ctrlTransfers->updateItem(ii, COLUMN_FILE);
	}
	if(updateMask & UpdateInfo::MASK_TIMELEFT) {
		ctrlTransfers->updateItem(ii, COLUMN_TIMELEFT);
	}
	if(updateMask & UpdateInfo::MASK_IP) {
		ctrlTransfers->updateItem(ii, COLUMN_IP);
	}
	if(updateMask & UpdateInfo::MASK_SEGMENT) {
		ctrlTransfers->updateItem(ii, COLUMN_HUB);
	}
	if(updateMask & UpdateInfo::MASK_CIPHER) {
		ctrlTransfers->updateItem(ii, COLUMN_CIPHER);
	}
}

void TransferView::on(ConnectionManagerListener::Added, const ConnectionQueueItem* aCqi) throw()
{
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());

	if(ui->download) 
	{
		string aTarget; int64_t aSize; int aFlags;
		if(QueueManager::getInstance()->getQueueInfo(aCqi->getUser(), aTarget, aSize, aFlags)) 
		{
			Transfer::Type type = Transfer::TYPE_FILE;
			if(aFlags & QueueItem::FLAG_USER_LIST)
				type = Transfer::TYPE_FULL_LIST;
			else if(aFlags & QueueItem::FLAG_PARTIAL_LIST)
				type = Transfer::TYPE_PARTIAL_LIST;
			
			ui->setType(type);
			ui->setTarget(Text::toT(aTarget));
			ui->setSize(aSize);
		}
	}

	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setStatusString(TSTRING(CONNECTING));

	speak(ADD_ITEM, ui);
}

void TransferView::on(ConnectionManagerListener::Failed, const ConnectionQueueItem* aCqi, const string& aReason) throw()
{
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());
	if(aCqi->getUser().user->isSet(User::OLD_CLIENT)) 
	{
		ui->setStatusString(TSTRING(SOURCE_TOO_OLD));
	} 
	else 
	{
		ui->setStatusString(Text::toT(aReason));
	}
	ui->setStatus(ItemInfo::STATUS_WAITING);
	speak(UPDATE_ITEM, ui);
}

void TransferView::on(ConnectionManagerListener::Removed, const ConnectionQueueItem* aCqi) throw()
{
	speak(REMOVE_ITEM, new UpdateInfo(aCqi->getUser(), aCqi->getDownload()));
}

void TransferView::on(ConnectionManagerListener::StatusChanged, const ConnectionQueueItem* aCqi) throw()
{
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());
	string aTarget;	int64_t aSize; int aFlags = 0;

	if(QueueManager::getInstance()->getQueueInfo(aCqi->getUser(), aTarget, aSize, aFlags)) 
	{
		Transfer::Type type = Transfer::TYPE_FILE;
		if(aFlags & QueueItem::FLAG_USER_LIST)
			type = Transfer::TYPE_FULL_LIST;
		else if(aFlags & QueueItem::FLAG_PARTIAL_LIST)
			type = Transfer::TYPE_PARTIAL_LIST;
	
		ui->setType(type);
		ui->setTarget(Text::toT(aTarget));
		ui->setSize(aSize);
	}

	ui->setStatusString(TSTRING(CONNECTING));
	ui->setStatus(ItemInfo::STATUS_WAITING);

	speak(UPDATE_ITEM, ui);
}

void TransferView::starting(UpdateInfo* ui, const Transfer* t) {
	ui->setPos(t->getPos());
	ui->setTarget(Text::toT(t->getPath()));
	ui->setType(t->getType());
	const UserConnection& uc = t->getUserConnection();
	ui->setCipher(Text::toT(uc.getCipherName()));
	const string& ip = uc.getRemoteIp();
	const string& country = Util::getIpCountry(ip);
	if(country.empty()) 
	{
		ui->setIP(Text::toT(ip), 0);
	} 
	else 
	{
		ui->setIP(Text::toT(country + " (" + ip + ")"), 0 /* TODO WinUtil::getFlagIndexByCode(country.c_str())*/);
	}
}

void TransferView::on(DownloadManagerListener::Requesting, const Download* aDownload) throw()
{
	UpdateInfo* ui = new UpdateInfo(aDownload->getHintedUser(), true);
	
	starting(ui, aDownload);
	
	ui->setActual(aDownload->getActual());
	ui->setSize(aDownload->getSize());
	ui->setStatus(ItemInfo::STATUS_RUNNING);	ui->updateMask &= ~UpdateInfo::MASK_STATUS; // hack to avoid changing item status
	ui->setStatusString(TSTRING(REQUESTING) + _T(" ") + getFile(aDownload->getType(), Text::toT(Util::getFileName(aDownload->getPath()))) + _T("..."));

	speak(UPDATE_ITEM, ui);
}
	
void TransferView::on(DownloadManagerListener::Failed, const Download* aDownload, const string& aReason) throw()
{	
	UpdateInfo* ui = new UpdateInfo(aDownload->getHintedUser(), true, true);
	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setPos(0);
	ui->setSize(aDownload->getSize());
	ui->setTarget(Text::toT(aDownload->getPath()));
	ui->setType(aDownload->getType());

	tstring tmpReason = Text::toT(aReason);
	if(aDownload->isSet(Download::FLAG_SLOWUSER)) 
	{
		tmpReason += _T(": ") + TSTRING(SLOW_USER);
	} 
	else if(aDownload->getOverlapped() && !aDownload->isSet(Download::FLAG_OVERLAP)) 
	{
		tmpReason += _T(": ") + TSTRING(OVERLAPPED_SLOW_SEGMENT);
	}

	ui->setStatusString(tmpReason);

	if(BOOLSETTING(POPUP_DOWNLOAD_FAILED)) 
	{
		WinUtil::notifyUser(TSTRING(DOWNLOAD_FAILED),
			TSTRING(FILE) + _T(": ") + Util::getFileName(ui->target) + _T("\n")+
			TSTRING(USER) + _T(": ") + WinUtil::getNicks(ui->user) + _T("\n")+
			TSTRING(REASON) + _T(": ") + Text::toT(aReason), wxICON_WARNING);
	}

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(DownloadManagerListener::Starting, const Download* aDownload) throw()
{
	UpdateInfo* ui = new UpdateInfo(aDownload->getHintedUser(), true);
	
	ui->setStatus(ItemInfo::STATUS_RUNNING);
	ui->setStatusString(TSTRING(DOWNLOAD_STARTING));
	ui->setTarget(Text::toT(aDownload->getPath()));
	ui->setType(aDownload->getType());
	
	speak(UPDATE_ITEM, ui);
}

void TransferView::on(DownloadManagerListener::Tick, const DownloadList& dl) throw()
{
	for(DownloadList::const_iterator j = dl.begin(); j != dl.end(); ++j) 
	{
		Download* d = *j;
		
		UpdateInfo* ui = new UpdateInfo(d->getHintedUser(), true);
		ui->setStatus(ItemInfo::STATUS_RUNNING);
		ui->setActual(d->getActual());
		ui->setPos(d->getPos());
		ui->setSize(d->getSize());
		ui->setTimeLeft(d->getSecondsLeft());
		ui->setSpeed(static_cast<int64_t>(d->getAverageSpeed()));
		ui->setType(d->getType());

		tstring pos = Util::formatBytesW(d->getPos());
		double percent = (double)d->getPos()*100.0/(double)d->getSize();
		tstring elapsed = Util::formatSeconds((GET_TICK() - d->getStart())/1000);

		tstring statusString;

		if(d->isSet(Download::FLAG_PARTIAL)) 
		{
			statusString += _T("[P]");
		}
		if(d->getUserConnection().isSecure()) 
		{
			if(d->getUserConnection().isTrusted()) 
			{
				statusString += _T("[S]");
			} 
			else 
			{
				statusString += _T("[U]");
			}
		}
		if(d->isSet(Download::FLAG_TTH_CHECK)) 
		{
			statusString += _T("[T]");
		}
		if(d->isSet(Download::FLAG_ZDOWNLOAD)) 
		{
			statusString += _T("[Z]");
		}
		if(d->isSet(Download::FLAG_CHUNKED)) 
		{
			statusString += _T("[C]");
		}
		if(!statusString.empty()) 
		{
			statusString += _T(" ");
		}
		statusString += Text::tformat(TSTRING(DOWNLOADED_BYTES), pos.c_str(), percent, elapsed.c_str());
		ui->setStatusString(statusString);
			
		tasks.add(UPDATE_ITEM, ui);
	}

	callAsync(std::bind(&TransferView::OnTasks, this));
}

void TransferView::on(DownloadManagerListener::Status, const UserConnection* uc, const string& aReason) throw()
{
	UpdateInfo* ui = new UpdateInfo(uc->getHintedUser(), true);
	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setPos(0);
	ui->setStatusString(Text::toT(aReason));

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(UploadManagerListener::Starting, const Upload* aUpload) throw()
{
	UpdateInfo* ui = new UpdateInfo(aUpload->getHintedUser(), false);

	starting(ui, aUpload);
	
	ui->setStatus(ItemInfo::STATUS_RUNNING);
	ui->setActual(aUpload->getStartPos() + aUpload->getActual());
	ui->setSize(aUpload->getType() == Transfer::TYPE_TREE ? aUpload->getSize() : aUpload->getFileSize());
	ui->setRunning(1);
	
	if(!aUpload->isSet(Upload::FLAG_RESUMED)) 
	{
		ui->setStatusString(TSTRING(UPLOAD_STARTING));
	}

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(UploadManagerListener::Tick, const UploadList& ul) throw()
{
	for(UploadList::const_iterator j = ul.begin(); j != ul.end(); ++j) 
	{
		Upload* u = *j;

		if (u->getPos() == 0) 
			continue;

		UpdateInfo* ui = new UpdateInfo(u->getHintedUser(), false);
		ui->setActual(u->getStartPos() + u->getActual());
		ui->setPos(u->getStartPos() + u->getPos());
		ui->setTimeLeft(u->getSecondsLeft(true)); // we are interested when whole file is finished and not only one chunk
		ui->setSpeed(static_cast<int64_t>(u->getAverageSpeed()));

		tstring pos = Util::formatBytesW(ui->pos);
		double percent = (double)ui->pos*100.0/(double)(u->getType() == Transfer::TYPE_TREE ? u->getSize() : u->getFileSize());
		tstring elapsed = Util::formatSeconds((GET_TICK() - u->getStart())/1000); 
		
		tstring statusString;

		if(u->isSet(Upload::FLAG_PARTIAL)) 
		{
			statusString += _T("[P]");
		}
		if(u->getUserConnection().isSecure()) 
		{
			if(u->getUserConnection().isTrusted()) 
			{
				statusString += _T("[S]");
			} 
			else 
			{
				statusString += _T("[U]");
			}
		}
		if(u->isSet(Upload::FLAG_ZUPLOAD)) 
		{
			statusString += _T("[Z]");
		}
		if(u->isSet(Upload::FLAG_CHUNKED)) 
		{
			statusString += _T("[C]");
		}		
		if(!statusString.empty()) 
		{
			statusString += _T(" ");
		}			
		statusString += Text::tformat(TSTRING(UPLOADED_BYTES), pos.c_str(), percent, elapsed.c_str());

		ui->setStatusString(statusString);
					
		tasks.add(UPDATE_ITEM, ui);
	}

	callAsync(std::bind(&TransferView::OnTasks, this));
}

void TransferView::on(QueueManagerListener::StatusUpdated, const QueueItem* qi) throw()
{
	if(qi->getFlags() & QueueItem::FLAG_USER_LIST)
		return;

	UpdateInfo* ui = new UpdateInfo(const_cast<QueueItem*>(qi), true);
	ui->setTarget(Text::toT(qi->getTarget()));
	ui->setType(Transfer::TYPE_FILE);

	if(qi->isRunning()) 
	{
		double ratio = 0;
		int64_t totalSpeed = 0;
		int16_t segs = 0;
		
		bool partial = false, trusted = false, untrusted = false, tthcheck = false, zdownload = false, chunked = false;

		for(DownloadList::const_iterator i = qi->getDownloads().begin(); i != qi->getDownloads().end(); i++) 
		{
			Download *d = *i;

			if(d->getStart() > 0) 
			{
				segs++;

				if(d->isSet(Download::FLAG_PARTIAL)) 
				{
					partial = true;
				}
				if(d->getUserConnection().isSecure()) 
				{
					if(d->getUserConnection().isTrusted()) 
					{
						trusted = true;
					} 
					else 
					{
						untrusted = true;
					}
				}
				if(d->isSet(Download::FLAG_TTH_CHECK)) 
				{
					tthcheck = true;
				}
				if(d->isSet(Download::FLAG_ZDOWNLOAD)) 
				{
					zdownload = true;
				}
				if(d->isSet(Download::FLAG_CHUNKED)) 
				{
					chunked = true;
				}
		
				totalSpeed += static_cast<int64_t>(d->getAverageSpeed());
				ratio += d->getPos() > 0 ? (double)d->getActual() / (double)d->getPos() : 1.00;
			}
		}

		ui->setRunning(segs);
		if(segs > 0) 
		{
			ratio = ratio / segs;

			ui->setStatus(ItemInfo::STATUS_RUNNING);
			ui->setSize(qi->getSize());
			ui->setPos(qi->getDownloadedBytes());
			ui->setActual((int64_t)((double)ui->pos * (ratio == 0 ? 1.00 : ratio)));
			ui->setTimeLeft((totalSpeed > 0) ? ((ui->size - ui->pos) / totalSpeed) : 0);
			ui->setSpeed(totalSpeed);

			if(qi->getFileBegin() == 0) 
			{
				// file is starting
				const_cast<QueueItem*>(qi)->setFileBegin(GET_TICK());

				ui->setStatusString(TSTRING(DOWNLOAD_STARTING));
				if ((!SETTING(BEGINFILE).empty()) && (!BOOLSETTING(SOUNDS_DISABLED)))
					wxSound::Play(Text::toT(SETTING(BEGINFILE)), wxSOUND_ASYNC);
 
				if(BOOLSETTING(POPUP_DOWNLOAD_START)) 
				{
					WinUtil::notifyUser(TSTRING(DOWNLOAD_STARTING), TSTRING(FILE) + _T(": ") + Util::getFileName(ui->target));
				}
			} 
			else 
			{
				uint64_t time = GET_TICK() - qi->getFileBegin();
				if(time > 1000) 
				{
					tstring pos = Util::formatBytesW(ui->pos);
					double percent = (double)ui->pos*100.0/(double)ui->size;
					tstring elapsed = Util::formatSeconds(time/1000);
					tstring flag;
					
					if(partial) 
					{
						flag += _T("[P]");
					}
					if(trusted) 
					{
						flag += _T("[S]");
					}
					if(untrusted) 
					{
						flag += _T("[U]");
					}
					if(tthcheck) 
					{
						flag += _T("[T]");
					}
					if(zdownload) 
					{
						flag += _T("[Z]");
					}
					if(chunked) 
					{
						flag += _T("[C]");
					}					

					if(!flag.empty()) 
					{
						flag += _T(" ");
					}
					
					ui->setStatusString(flag + Text::tformat(TSTRING(DOWNLOADED_BYTES), pos.c_str(), percent, elapsed.c_str()));
				}
			}
		}
	} 
	else 
	{
		const_cast<QueueItem*>(qi)->setFileBegin(0);

		ui->setSize(qi->getSize());
		ui->setStatus(ItemInfo::STATUS_WAITING);
		ui->setRunning(0);
	}

	speak(UPDATE_PARENT, ui);

}

void TransferView::on(QueueManagerListener::Removed, const QueueItem* qi) throw()
{
	if(qi->getFlags() & QueueItem::FLAG_USER_LIST)
		return;
		
	UpdateInfo* ui = new UpdateInfo(const_cast<QueueItem*>(qi), true);
	ui->setTarget(Text::toT(qi->getTarget()));
	ui->setPos(0);
	ui->setActual(0);
	ui->setTimeLeft(0);
	ui->setStatusString(TSTRING(DISCONNECTED));
	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setRunning(0);

	speak(UPDATE_PARENT, ui);
}

void TransferView::on(QueueManagerListener::Finished, const QueueItem* qi, const string&, const Download* download) throw()
{
	if(qi->getFlags() & QueueItem::FLAG_USER_LIST)
		return;

	// update download item
	UpdateInfo* ui = new UpdateInfo(download->getHintedUser(), true);

	ui->setStatus(ItemInfo::STATUS_WAITING);	
	ui->setPos(0);
	ui->setStatusString( TSTRING(DOWNLOAD_FINISHED_IDLE));

	speak(UPDATE_ITEM, ui);

	// update file item
	ui = new UpdateInfo(const_cast<QueueItem*>(qi), true, true);
	ui->user = download->getHintedUser();

	ui->setTarget(Text::toT(qi->getTarget()));
	ui->setPos(0);
	ui->setActual(0);
	ui->setTimeLeft(0);
	ui->setStatusString(TSTRING(DOWNLOAD_FINISHED_IDLE));
	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setRunning(0);
	
	if(BOOLSETTING(POPUP_DOWNLOAD_FINISHED)) 
	{
		WinUtil::notifyUser(TSTRING(DOWNLOAD_FINISHED_IDLE), TSTRING(FILE) + _T(": ") + Util::getFileName(ui->target));
	}

	speak(UPDATE_PARENT, ui);
}

void TransferView::on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw()
{
}

void TransferView::onTransferComplete(const Transfer* aTransfer, bool isUpload, const string& aFileName, bool isTree) 
{
	UpdateInfo* ui = new UpdateInfo(aTransfer->getHintedUser(), !isUpload);

	ui->setStatus(ItemInfo::STATUS_WAITING);	
	ui->setPos(0);
	ui->setStatusString(isUpload ? TSTRING(UPLOAD_FINISHED_IDLE) : TSTRING(DOWNLOAD_FINISHED_IDLE));
	ui->setRunning(0);

	if(isUpload && BOOLSETTING(POPUP_UPLOAD_FINISHED) && !isTree) 
	{
		WinUtil::notifyUser(TSTRING(UPLOAD_FINISHED_IDLE),
			TSTRING(FILE) + _T(": ") + Text::toT(aFileName) + _T("\n")+
			TSTRING(USER) + _T(": ") + WinUtil::getNicks(aTransfer->getHintedUser()));
	}

	speak(UPDATE_ITEM, ui);
}

void TransferView::runUserCommand(const UserCommand& uc) 
{
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	wxDataViewItemArray sel;
	ctrlTransfers->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const ItemInfo* ii = ctrlTransfers->getItemData(sel[i]);
		if(!ii->user.user || !ii->user.user->isOnline())
			continue;

		StringMap tmp = ucParams;
		ucParams["fileFN"] = Text::fromT(ii->target);

		// compatibility with 0.674 and earlier
		ucParams["file"] = ucParams["fileFN"];
		
		ClientManager::getInstance()->userCommand(ii->user, uc, tmp, true);
	}
}