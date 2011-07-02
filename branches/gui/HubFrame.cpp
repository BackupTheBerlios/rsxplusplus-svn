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
#include "HubFrame.h"

#include "CountryFlags.h"
#include "MainWindow.h"
#include "PrivateFrame.h"
#include "TypedDataViewCtrl.h"
#include "WinUtil.h"

#include "../client/ChatMessage.h"
#include "../client/Client.h"
#include "../client/ClientManager.h"
#include "../client/ConnectionManager.h"
#include "../client/LogManager.h"
#include "../client/QueueManager.h"
#include "../client/UploadManager.h"

#include <wx/sound.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>
#include <wx/tooltip.h>

BEGIN_EVENT_TABLE(HubFrame, MDIChildFrame)
	EVT_CLOSE(HubFrame::OnClose)
	EVT_AUINOTEBOOK_TAB_RIGHT_UP(wxID_ANY, HubFrame::OnTabContextMenu)
	EVT_COMMAND(IDC_USERS, GET_ATTRIBUTES, HubFrame::OnGetAttributes)
	EVT_COMMAND(wxID_ANY, SEND_MESSAGE, HubFrame::OnSendMessage)
	EVT_CHECKBOX(ID_SHOWLIST_CHECKBOX, HubFrame::OnShowUsers)
	EVT_SPLITTER_UNSPLIT(ID_SPLITTER, HubFrame::OnUnsplit)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(IDC_USERS, HubFrame::OnUserListContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(IDC_USERS, HubFrame::OnUserListDblClick)
	EVT_COMBOBOX(ID_FILTER_COMBOBOX, HubFrame::OnFilterSelection)
	EVT_TEXT(ID_FILTER_TEXTBOX, HubFrame::OnFilterChange)
	EVT_TEXT_ENTER(ID_FILTER_TEXTBOX, HubFrame::OnFilterEnter)
	EVT_SET_FOCUS(HubFrame::OnSetFocus)

	EVT_MENU(ID_CMD_FOLLOW, HubFrame::OnFollow)
	EVT_MENU(IDC_COPY_HUBNAME, HubFrame::OnCopyHubInfo)
	EVT_MENU(IDC_COPY_HUBADDRESS, HubFrame::OnCopyHubInfo)
	EVT_MENU(IDC_OPEN_HUB_LOG, HubFrame::OnOpenHubLog)
	EVT_MENU(ID_EDIT_CLEAR_ALL, HubFrame::OnEditClearAll)
	EVT_MENU(IDC_ADD_AS_FAVORITE, HubFrame::OnAddFavorite)
	EVT_MENU(ID_CMD_RECONNECT, HubFrame::OnReconnect)
	EVT_MENU(IDC_CLOSE_WINDOW, HubFrame::OnCloseWindow)

	EVT_MENU_RANGE(IDC_COPY, IDC_COPY + OnlineUser::COLUMN_LAST, HubFrame::OnCopyUserInfo)
	EVT_MENU(IDC_OPEN_USER_LOG,  HubFrame::OnOpenUserLog)
	EVT_MENU(IDC_PUBLIC_MESSAGE, HubFrame::OnPublicMessage)
	EVT_MENU(IDC_IGNORE, HubFrame::OnIgnore)
	EVT_MENU(IDC_UNIGNORE, HubFrame::OnUnignore)
	EVT_MENU(IDC_REFRESH, HubFrame::OnRefresh)
END_EVENT_TABLE()

HubFrame::FrameMap HubFrame::frames;

int HubFrame::columnSizes[] = { 100, 75, 75, 75, 100, 75, 100, 100, 50, 40, 40, 40, 300 };

int HubFrame::columnIndexes[] = { OnlineUser::COLUMN_NICK, OnlineUser::COLUMN_SHARED, OnlineUser::COLUMN_EXACT_SHARED,
	OnlineUser::COLUMN_DESCRIPTION, OnlineUser::COLUMN_TAG,	OnlineUser::COLUMN_CONNECTION, OnlineUser::COLUMN_IP, OnlineUser::COLUMN_EMAIL,
	OnlineUser::COLUMN_VERSION, OnlineUser::COLUMN_MODE, OnlineUser::COLUMN_HUBS, OnlineUser::COLUMN_SLOTS, OnlineUser::COLUMN_CID };

ResourceManager::Strings HubFrame::columnNames[] = { ResourceManager::NICK, ResourceManager::SHARED, ResourceManager::EXACT_SHARED, 
	ResourceManager::DESCRIPTION, ResourceManager::TAG, ResourceManager::CONNECTION, ResourceManager::IP_BARE, ResourceManager::EMAIL,
	ResourceManager::VERSION, ResourceManager::MODE, ResourceManager::HUBS, ResourceManager::SLOTS, ResourceManager::CID };

HubFrame::HubFrame(const tstring& aServer, const string& aRawOne, const string& aRawTwo, const string& aRawThree, const string& aRawFour,
	const string& aRawFive, int chatUserSplit, bool userListState) : 
	MDIChildFrame(wxID_ANY, _T("HubFrame"), wxColour(255, 0, 0), wxT("IDI_HUB"), wxT("IDI_HUB_OFF")), server(aServer), updateUsers(false),
	timeStamps(BOOLSETTING(TIME_STAMPS)), resort(false), chatUserSplitSize(chatUserSplit), waitingForPW(false)
{
	client = ClientManager::getInstance()->getClient(Text::fromT(aServer));
	client->addListener(this);
		
	client->setRawOne(aRawOne);
	client->setRawTwo(aRawTwo);
	client->setRawThree(aRawThree);
	client->setRawFour(aRawFour);
	client->setRawFive(aRawFive);

	memset(statusSizes, 0, sizeof(statusSizes));
	statusSizes[0] = -1;
	statusSizes[5] = 16;
	CreateStatusBar(6, wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE);

	chatUsersSplitter = new wxSplitterWindow(this, ID_SPLITTER, wxDefaultPosition, wxDefaultSize,  wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxSP_3D | wxSP_LIVE_UPDATE);

	ctrlChat = new ChatPanel(chatUsersSplitter, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlChat->SetFont(WinUtil::font);
	ctrlChat->setClient(client);
	ctrlChat->readLog(getLogFile(), SETTING(HUB_LAST_LINES_LOG));

	userListPanel = new wxPanel(chatUsersSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxTAB_TRAVERSAL | wxNO_BORDER);

	ctrlUsers = new UserList(userListPanel, IDC_USERS, wxDefaultPosition, wxSize(0, 0), wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxVSCROLL | wxHSCROLL);
	ctrlUsers->SetFont(WinUtil::font);
	ctrlUsers->SetBackgroundColour(WinUtil::bgColor);
	ctrlUsers->SetForegroundColour(WinUtil::textColor);

	ctrlFilter = new wxTextCtrl(userListPanel, ID_FILTER_TEXTBOX, wxEmptyString, wxDefaultPosition, wxSize(0, -1), wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxTE_PROCESS_ENTER);
	ctrlFilter->SetFont(WinUtil::font);
	ctrlFilter->SetBackgroundColour(WinUtil::bgColor);
	ctrlFilter->SetForegroundColour(WinUtil::textColor);

	ctrlFilterSel = new wxComboBox(userListPanel, ID_FILTER_COMBOBOX, wxEmptyString, wxDefaultPosition, wxSize(0, -1), 0, NULL, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxCB_READONLY);
	ctrlFilterSel->SetFont(WinUtil::font);
	ctrlFilterSel->SetBackgroundColour(WinUtil::bgColor);
	ctrlFilterSel->SetForegroundColour(WinUtil::textColor);

	wxBoxSizer* verticalSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* horizontalSizer = new wxBoxSizer(wxHORIZONTAL);

	// set filterbox and filtercombobox proportions to 2:1
	horizontalSizer->Add(ctrlFilter, 2, wxALL | wxEXPAND, 0);
	horizontalSizer->Add(ctrlFilterSel, 1, wxLEFT | wxRIGHT | wxEXPAND, 2);

	verticalSizer->Add(ctrlUsers, 1, wxBOTTOM | wxEXPAND, 5);
	verticalSizer->Add(horizontalSizer, 0, wxEXPAND);

	userListPanel->SetSizerAndFit(verticalSizer);
	
	const FavoriteHubEntry *fhe = FavoriteManager::getInstance()->getFavoriteHubEntry(Text::fromT(server));
	if(fhe) 
	{
		WinUtil::splitTokens(columnIndexes, fhe->getHeaderOrder(), OnlineUser::COLUMN_LAST);
		WinUtil::splitTokens(columnSizes, fhe->getHeaderWidths(), OnlineUser::COLUMN_LAST);
	} 
	else 
	{
		WinUtil::splitTokens(columnIndexes, SETTING(HUBFRAME_ORDER), OnlineUser::COLUMN_LAST);
		WinUtil::splitTokens(columnSizes, SETTING(HUBFRAME_WIDTHS), OnlineUser::COLUMN_LAST);                           
	}
	  	
	ctrlFilterSel->Append(CTSTRING(ANY));
	ctrlFilterSel->SetSelection(0);

	for(uint8_t j = 0; j < OnlineUser::COLUMN_LAST; ++j) 
	{
		wxAlignment fmt = (j == OnlineUser::COLUMN_SHARED || j == OnlineUser::COLUMN_EXACT_SHARED || j == OnlineUser::COLUMN_SLOTS) ? wxALIGN_RIGHT : wxALIGN_LEFT;
		wxDataViewRenderer* rndr = (j == OnlineUser::COLUMN_IP) ? new CountryFlags::FlagRenderer : NULL;
		ctrlUsers->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], rndr);
		ctrlFilterSel->Append(CTSTRING_I(columnNames[j]));
	}
	
	/* TODO ctrlUsers->setColumnOrderArray(OnlineUser::COLUMN_LAST, columnIndexes);*/
	ctrlUsers->setSortColumn(OnlineUser::COLUMN_NICK);
	ctrlUsers->setImageList(&WinUtil::userImages);

	if(fhe) 
	{
		// TODO ctrlUsers->setVisible(fhe->getHeaderVisible());
    }
	else
	{
	    // TODO ctrlUsers->setVisible(SETTING(HUBFRAME_VISIBLE));
		userListState = BOOLSETTING(GET_USER_INFO);
    }

	ctrlShowUsers = new wxCheckBox(GetStatusBar(), ID_SHOWLIST_CHECKBOX, wxEmptyString, wxDefaultPosition, wxSize(16, 16), wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlShowUsers->SetValue(userListState);

	wxBoxSizer* windowSizer = new wxBoxSizer(wxVERTICAL);
	windowSizer->Add(chatUsersSplitter, 1, wxEXPAND | wxBOTTOM, 4);
	SetSizerAndFit(windowSizer);

	if(userListState)
		chatUsersSplitter->SplitVertically(ctrlChat, userListPanel);
	
	// set default splitter positions
	wxRect rc = this->GetClientRect();

	// when windows is created, chatUserSplitSize contains proportional size which is now updated to absolute one
	chatUserSplitSize = (rc.GetWidth() * (chatUserSplitSize ? chatUserSplitSize : 7500)) / 10000; // <-- be compatible with WTL version
	chatUsersSplitter->SetSashPosition(chatUserSplitSize, true);
	chatUsersSplitter->SetSashGravity(0.75);

	ctrlChat->SetSashPosition(-(WinUtil::fontHeight + 12), true);

	showJoins = BOOLSETTING(SHOW_JOINS);
	favShowJoins = BOOLSETTING(FAV_SHOW_JOINS);

	client->connect();

	FavoriteManager::getInstance()->addListener(this);
    TimerManager::getInstance()->addListener(this);
	SettingsManager::getInstance()->addListener(this);
}

HubFrame::~HubFrame(void)
{
	ClientManager::getInstance()->putClient(client);

	dcassert(frames.find(server) != frames.end());
	dcassert(frames[server] == this);
	frames.erase(server);

	clearTaskList();
}

void HubFrame::openWindow(const tstring& server, const string& rawOne, const string& rawTwo, const string& rawThree, const string& rawFour, 
	const string& rawFive, int chatusersplit, bool userliststate, string sColumsOrder, string sColumsWidth, string sColumsVisible)
{
	FrameIter i = frames.find(server);
	if(i == frames.end()) 
	{
		MDIFreezer freezer;
		
		HubFrame* frm = new HubFrame(server, rawOne, rawTwo, rawThree, rawFour, rawFive, chatusersplit, userliststate);
		frm->Show();
		frames[server] = frm;
	}
	else
	{
		if(i->second->IsIconized())
			i->second->Restore();

		i->second->Activate();
	}
}

void HubFrame::resortUsers() 
{
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		i->second->resortForFavsFirst(true);
}

void HubFrame::OnClose(wxCloseEvent& event)
{
	RecentHubEntry* r = FavoriteManager::getInstance()->getRecentHubEntry(Text::fromT(server));
	if(r) 
	{
		r->setName(Text::fromT((const TCHAR*)this->GetTitle().c_str()));
		r->setUsers(Util::toString(client->getUserCount()));
		r->setShared(Util::toString(client->getAvailable()));
		FavoriteManager::getInstance()->updateRecent(r);
	}

	SettingsManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	FavoriteManager::getInstance()->removeListener(this);

	client->removeListener(this);
	client->disconnect(true);

	SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, ctrlShowUsers->IsChecked());
	FavoriteManager::getInstance()->removeUserCommand(Text::fromT(server));

	clearUserList();
	clearTaskList();

	string tmp, tmp2, tmp3;
	// TODO ctrlUsers->saveHeaderOrder(tmp, tmp2, tmp3);

	FavoriteHubEntry *fhe = FavoriteManager::getInstance()->getFavoriteHubEntry(Text::fromT(server));
	if(fhe != NULL) 
	{
		int split = (chatUsersSplitter->GetSashPosition() * 10000) / max(1, GetClientRect().GetWidth());
		dcassert(split < 10000);
		fhe->setChatUserSplit(split); // <-- be compatible with WTL version
		fhe->setUserListState(ctrlShowUsers->IsChecked());
		fhe->setHeaderOrder(tmp);
		fhe->setHeaderWidths(tmp2);
		fhe->setHeaderVisible(tmp3);
			
		FavoriteManager::getInstance()->save();
	} 
	else 
	{
		SettingsManager::getInstance()->set(SettingsManager::HUBFRAME_ORDER, tmp);
		SettingsManager::getInstance()->set(SettingsManager::HUBFRAME_WIDTHS, tmp2);
		SettingsManager::getInstance()->set(SettingsManager::HUBFRAME_VISIBLE, tmp3);
	}

	event.Skip();
}

void HubFrame::UpdateLayout()
{
	GetStatusBar()->SetFieldsCount(6, statusSizes);

	// position checkbox in the last field
	wxRect sr;
	GetStatusBar()->GetFieldRect(5, sr);
	sr.Offset(2, 0);
	sr.SetWidth(16);
	ctrlShowUsers->SetSize(sr);
}

void HubFrame::clearUserList() 
{
	for(UserList::iterator i = ctrlUsers->begin(); i != ctrlUsers->end(); ++i) 
	{
		(*i).dec();
	}
	ctrlUsers->deleteAllItems();
}

void HubFrame::clearTaskList() 
{
	tasks.clear();
}

void HubFrame::OnTasks()
{
	TaskQueue::List t;
	tasks.get(t);

	if(t.size() > 2) ctrlUsers->Freeze();

	for(TaskQueue::Iter i = t.begin(); i != t.end(); ++i) 
	{
		switch(i->first)
		{
			case UPDATE_USER_JOIN:
			{
				UserTask& u = *static_cast<UserTask*>(i->second);
				if(updateUser(u)) 
				{
					bool isFavorite = FavoriteManager::getInstance()->isFavoriteUser(u.onlineUser->getUser());
					if (isFavorite && (!SETTING(SOUND_FAVUSER).empty()) && (!BOOLSETTING(SOUNDS_DISABLED)))
						wxSound::Play(Text::toT(SETTING(SOUND_FAVUSER)), wxSOUND_ASYNC);

					if(isFavorite && BOOLSETTING(POPUP_FAVORITE_CONNECTED)) 
					{
						WinUtil::notifyUser(TSTRING(FAVUSER_ONLINE), Text::toT(u.onlineUser->getIdentity().getNick() + " - " + client->getHubName()));
					}

					if (showJoins || (favShowJoins && isFavorite)) 
					{
				 		addLine(_T("*** ") + TSTRING(JOINS) + _T(" ") + Text::toT(u.onlineUser->getIdentity().getNick()), WinUtil::m_ChatTextSystem);
					}	

					if(client->isOp() && !u.onlineUser->getIdentity().isBot() && !u.onlineUser->getIdentity().isHub()) 
					{			
						if(BOOLSETTING(CHECK_NEW_USERS)) 
						{
							if(u.onlineUser->getIdentity().isTcpActive(client) || client->isActive()) 
							{
								try 
								{
									QueueManager::getInstance()->addList(HintedUser(u.onlineUser->getUser(), client->getHubUrl()), QueueItem::FLAG_USER_CHECK);
								} 
								catch(const Exception&) 
								{
								}
							}
						}
					}
				}
				break;
			}
			case UPDATE_USER: 
				updateUser(*static_cast<UserTask*>(i->second));
				break;
			case REMOVE_USER: 
			{
				const UserTask& u = *static_cast<UserTask*>(i->second);
				removeUser(u.onlineUser);

				if (showJoins || (favShowJoins && FavoriteManager::getInstance()->isFavoriteUser(u.onlineUser->getUser()))) 
				{
					addLine(Text::toT("*** " + STRING(PARTS) + " " + u.onlineUser->getIdentity().getNick()), WinUtil::m_ChatTextSystem);
				}
				break;
			}
		}

		delete i->second;
	}

	if(resort && ctrlShowUsers->IsChecked()) 
	{
		ctrlUsers->resort();
		resort = false;
	}

	if(t.size() > 2) ctrlUsers->Thaw();

	// update stats
	size_t AllUsers = client->getUserCount();
	size_t ShownUsers = ctrlUsers->getItemCount();
			
	tstring text[3];
			
	if(AllUsers != ShownUsers) 
	{
		text[0] = Util::toStringW(ShownUsers) + _T("/") + Util::toStringW(AllUsers) + _T(" ") + TSTRING(HUB_USERS);
	} 
	else 
	{
		text[0] = Util::toStringW(AllUsers) + _T(" ") + TSTRING(HUB_USERS);
	}
			
	int64_t available = client->getAvailable();
	text[1] = Util::formatBytesW(available);
			
	if(AllUsers > 0)
		text[2] = Util::formatBytesW(available / AllUsers) + _T("/") + TSTRING(USER);

	bool update = false;
	for(int i = 0; i < 3; i++) 
	{
		int size = WinUtil::getTextWidth(text[i], GetStatusBar());
		if(size != statusSizes[i + 2]) 
		{
			statusSizes[i + 2] = size;
			update = true;
		}
		GetStatusBar()->SetStatusText(text[i], i + 2);
	}
			
	if(update)
		UpdateLayout();
}

void HubFrame::OnConnected()
{
	addStatus(TSTRING(CONNECTED), WinUtil::m_ChatTextServer);
	setTabColor(RGB(0, 255, 0));
	unsetIconState();
			
	tstring text = Text::toT(client->getCipherName());
	GetStatusBar()->SetStatusText(text, 1);
	statusSizes[1] = WinUtil::getTextWidth(text, GetStatusBar());

	if(BOOLSETTING(POPUP_HUB_CONNECTED)) 
	{
		WinUtil::notifyUser(TSTRING(CONNECTED), Text::toT(client->getAddress()));
	}

	if ((!SETTING(SOUND_HUBCON).empty()) && (!BOOLSETTING(SOUNDS_DISABLED)))
		wxSound::Play(Text::toT(SETTING(SOUND_HUBCON)), wxSOUND_ASYNC);

}

void HubFrame::OnDisconnected()
{
	clearUserList();
	setTabColor(RGB(255, 0, 0));
	setIconState();

	// reset statusbar info
	updateUsers = true;

	if ((!SETTING(SOUND_HUBDISCON).empty()) && (!BOOLSETTING(SOUNDS_DISABLED)))
		wxSound::Play(Text::toT(SETTING(SOUND_HUBDISCON)), wxSOUND_ASYNC);

	if(BOOLSETTING(POPUP_HUB_DISCONNECTED)) 
	{
		WinUtil::notifyUser(TSTRING(DISCONNECTED), Text::toT(client->getAddress()));
	}
}

void HubFrame::OnGetPassword()
{
	if(client->getPassword().size() > 0) 
	{
		client->password(client->getPassword());
		addStatus(TSTRING(STORED_PASSWORD_SENT), WinUtil::m_ChatTextSystem);
	} 
	else 
	{
		if(!BOOLSETTING(PROMPT_PASSWORD)) 
		{
			ctrlChat->getCtrlMessage()->SetValue(wxT("/password "));
			ctrlChat->getCtrlMessage()->SetFocus();
			ctrlChat->getCtrlMessage()->SetSelection(10, 10);
			waitingForPW = true;
		} 
		else
		{
			wxPasswordEntryDialog linePwd(this, CTSTRING(ENTER_PASSWORD), CTSTRING(ENTER_PASSWORD));

			if(linePwd.ShowModal() == wxID_OK) 
			{
				client->setPassword(Text::fromT(tstring(linePwd.GetValue())));
				client->password(Text::fromT(tstring(linePwd.GetValue())));
				waitingForPW = false;
			} else {
				client->disconnect(true);
			}
		}
	}
}

void HubFrame::OnChatMessage(const Identity& from, const tstring& msg)
{
	if(!from.getUser() || (!FavoriteManager::getInstance()->isIgnoredUser(from.getUser())))
		addLine(from, msg, WinUtil::m_ChatTextGeneral);
}

void HubFrame::OnPrivateMessage(const Identity& from, const UserPtr& to, const UserPtr& replyTo, bool hub, bool bot, const tstring& msg)
{
	tstring nick = Text::toT(from.getNick());
	if(!from.getUser() || (!FavoriteManager::getInstance()->isIgnoredUser(from.getUser()))) 
	{
		bool myPM = replyTo == ClientManager::getInstance()->getMe();
		const UserPtr& user = myPM ? to : replyTo;
		if(hub) 
		{
			if(BOOLSETTING(IGNORE_HUB_PMS)) 
			{
				addStatus(TSTRING(IGNORED_MESSAGE) + _T(" ") + msg, WinUtil::m_ChatTextSystem, false);
			} 
			else if(BOOLSETTING(POPUP_HUB_PMS) || PrivateFrame::isOpen(user)) 
			{
				PrivateFrame::gotMessage(from, to, replyTo, msg, client);
			} 
			else 
			{
				addLine(TSTRING(PRIVATE_MESSAGE_FROM) + _T(" ") + nick + _T(": ") + msg, WinUtil::m_ChatTextPrivate);
			}
		} 
		else if(bot) 
		{
			if(BOOLSETTING(IGNORE_BOT_PMS)) 
			{
				addStatus(TSTRING(IGNORED_MESSAGE) + _T(" ") + msg, WinUtil::m_ChatTextPrivate, false);
			} 
			else if(BOOLSETTING(POPUP_BOT_PMS) || PrivateFrame::isOpen(user)) 
			{
				PrivateFrame::gotMessage(from, to, replyTo, msg, client);
			} 
			else 
			{
				addLine(TSTRING(PRIVATE_MESSAGE_FROM) + _T(" ") + nick + _T(": ") + msg, WinUtil::m_ChatTextPrivate);
			}
		} 
		else 
		{
			if(BOOLSETTING(POPUP_PMS) || PrivateFrame::isOpen(user)) 
			{
				PrivateFrame::gotMessage(from, to, replyTo, msg, client);
			} 
			else 
			{
				addLine(TSTRING(PRIVATE_MESSAGE_FROM) + _T(" ") + nick + _T(": ") + msg, WinUtil::m_ChatTextPrivate);
			}

			WinUtil::mainWindow->setPMIcon();
		}
	}
}

void HubFrame::OnTabContextMenu(wxAuiNotebookEvent& /*event*/)
{
	tabMenuShown = true;
	wxMenu tabMenu;

	wxMenu* copyHubMenu = new wxMenu();
	copyHubMenu->SetTitle(TSTRING(COPY));
	copyHubMenu->Append(IDC_COPY_HUBNAME, CTSTRING(HUB_NAME));
	copyHubMenu->Append(IDC_COPY_HUBADDRESS, CTSTRING(HUB_ADDRESS));

	tabMenu.SetTitle(Text::toT(!client->getHubName().empty() ? (client->getHubName().size() > 50 ? (client->getHubName().substr(0, 50) + "...") : client->getHubName()) : client->getHubUrl()));	
	if(BOOLSETTING(LOG_MAIN_CHAT)) 
	{
		tabMenu.Append(IDC_OPEN_HUB_LOG, CTSTRING(OPEN_HUB_LOG));
		tabMenu.AppendSeparator();
	}
	tabMenu.Append(ID_EDIT_CLEAR_ALL, CTSTRING(CLEAR));
	tabMenu.AppendSeparator();
	tabMenu.Append(IDC_ADD_AS_FAVORITE, CTSTRING(ADD_TO_FAVORITES));
	tabMenu.Append(ID_CMD_RECONNECT, CTSTRING(MENU_RECONNECT));
	wxMenuItem* copyItem = tabMenu.AppendSubMenu(copyHubMenu, CTSTRING(COPY));
	prepareMenu(tabMenu, ::UserCommand::CONTEXT_HUB, client->getHubUrl());
	tabMenu.AppendSeparator();
	tabMenu.Append(IDC_CLOSE_WINDOW, CTSTRING(CLOSE));
	
	if(!client->isConnected())
		copyItem->Enable(false);
	else
		copyItem->Enable(true);
	
	PopupMenu(&tabMenu);
}

void HubFrame::OnSendMessage(wxCommandEvent& event)
{
	tstring s(event.GetString());

	// Special command
	if(s[0] == _T('/')) 
	{
		tstring cmd = s;
		tstring param;
		tstring message;
		tstring status;
		bool thirdPerson = false;
		if(WinUtil::checkCommand(cmd, param, message, status, thirdPerson)) 
		{
			if(!message.empty())
				client->hubMessage(Text::fromT(message), thirdPerson);

			if(!status.empty())
				addStatus(status, WinUtil::m_ChatTextSystem);

		} 
		else if(stricmp(cmd.c_str(), _T("join")) ==0 )
		{
			if(!param.empty()) {
				redirect = param;
				if(BOOLSETTING(JOIN_OPEN_NEW_WINDOW)) 
				{
					HubFrame::openWindow(param);
				} 
				else 
				{
					wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, ID_CMD_FOLLOW);
					ProcessEvent(evt);
				}
			} 
			else 
			{
				addStatus(TSTRING(SPECIFY_SERVER), WinUtil::m_ChatTextSystem);
			}
		} 
		else if((stricmp(cmd.c_str(), _T("clear")) == 0) || (stricmp(cmd.c_str(), _T("cls")) == 0)) 
		{
			ctrlChat->Clear();
		} 
		else if(stricmp(cmd.c_str(), _T("ts")) == 0) 
		{
			timeStamps = !timeStamps;
			if(timeStamps) 
			{
				addStatus(TSTRING(TIMESTAMPS_ENABLED), WinUtil::m_ChatTextSystem);
			} 
			else 
			{
				addStatus(TSTRING(TIMESTAMPS_DISABLED), WinUtil::m_ChatTextSystem);
			}
		} 
		else if((stricmp(cmd.c_str(), _T("password")) == 0) && waitingForPW) 
		{
			client->setPassword(Text::fromT(param));
			client->password(Text::fromT(param));
			waitingForPW = false;
		} 
		else if(stricmp(cmd.c_str(), _T("showjoins")) == 0) 
		{
			showJoins = !showJoins;
			if(showJoins) 
			{
				addStatus(TSTRING(JOIN_SHOWING_ON), WinUtil::m_ChatTextSystem);
			} 
			else 
			{
				addStatus(TSTRING(JOIN_SHOWING_OFF), WinUtil::m_ChatTextSystem);
			}
		} 
		else if(stricmp(cmd.c_str(), _T("favshowjoins")) == 0) 
		{
			favShowJoins = !favShowJoins;
			if(favShowJoins) 
			{
				addStatus(TSTRING(FAV_JOIN_SHOWING_ON), WinUtil::m_ChatTextSystem);
			} 
			else 
			{
				addStatus(TSTRING(FAV_JOIN_SHOWING_OFF), WinUtil::m_ChatTextSystem);
			}
		} else if(stricmp(cmd.c_str(), _T("close")) == 0) 
		{
			Close();
		} 
		else if(stricmp(cmd.c_str(), _T("userlist")) == 0) 
		{
			ctrlShowUsers->SetValue(!ctrlShowUsers->IsChecked());
		} 
		else if(stricmp(cmd.c_str(), _T("connection")) == 0) 
		{
			addStatus(Text::toT((STRING(IP) + " " + client->getLocalIp() + ", " + 
				STRING(PORT) + " " +
				Util::toString(ConnectionManager::getInstance()->getPort()) + "/" + 
				Util::toString(SearchManager::getInstance()->getPort()) + "/" +
				Util::toString(ConnectionManager::getInstance()->getSecurePort())))
				, WinUtil::m_ChatTextSystem);
		} 
		else if((stricmp(cmd.c_str(), _T("favorite")) == 0) || (stricmp(cmd.c_str(), _T("fav")) == 0)) 
		{
			addAsFavorite();
		} 
		else if((stricmp(cmd.c_str(), _T("removefavorite")) == 0) || (stricmp(cmd.c_str(), _T("removefav")) == 0)) 
		{
			removeFavoriteHub();
		} 
		else if(stricmp(cmd.c_str(), _T("getlist")) == 0)
		{
			if(!param.empty())
			{
				OnlineUserPtr ui = client->findUser(Text::fromT(param));
				if(ui)
					ui->getList(client->getHubUrl());
			}
		} 
		else if(stricmp(cmd.c_str(), _T("f")) == 0) 
		{
			// TODO if(param.empty())
			//	param = findTextPopup();

			// findText(param);
		} 
		else if(stricmp(cmd.c_str(), _T("extraslots"))==0) 
		{
			int j = Util::toInt(Text::fromT(param));
			if(j > 0) 
			{
				SettingsManager::getInstance()->set(SettingsManager::EXTRA_SLOTS, j);
				addStatus(TSTRING(EXTRA_SLOTS_SET), WinUtil::m_ChatTextSystem );
			} 
			else 
			{
				addStatus(TSTRING(INVALID_NUMBER_OF_SLOTS), WinUtil::m_ChatTextSystem );
			}
		} 
		else if(stricmp(cmd.c_str(), _T("smallfilesize")) == 0) 
		{
			int j = Util::toInt(Text::fromT(param));
			if(j >= 64) 
			{
				SettingsManager::getInstance()->set(SettingsManager::SET_MINISLOT_SIZE, j);
				addStatus(TSTRING(SMALL_FILE_SIZE_SET), WinUtil::m_ChatTextSystem );
			} 
			else 
			{
				addStatus(TSTRING(INVALID_SIZE), WinUtil::m_ChatTextSystem );
			}
		} 
		else if(stricmp(cmd.c_str(), _T("savequeue")) == 0) 
		{
			QueueManager::getInstance()->saveQueue();
			addStatus(_T("Queue saved."), WinUtil::m_ChatTextSystem );
		} 
		else if(stricmp(cmd.c_str(), _T("whois")) == 0) 
		{
			WinUtil::openLink(_T("http://www.ripe.net/perl/whois?form_type=simple&full_query_string=&searchtext=") + Text::toT(Util::encodeURI(Text::fromT(param))));
		} 
		else if(stricmp(cmd.c_str(), _T("ignorelist")) == 0) 
		{
			tstring ignorelist = _T("Ignored users:");
			FavoriteManager::FavoriteMap ignoredUsers = FavoriteManager::getInstance()->getIgnoredUsers();
			for(FavoriteManager::FavoriteMap::const_iterator i = ignoredUsers.begin(); i != ignoredUsers.end(); ++i)
				ignorelist += _T(" ") + Text::toT(ClientManager::getInstance()->getNicks(i->first, client->getHubUrl())[0]);
			addLine(ignorelist, WinUtil::m_ChatTextSystem);
		} 
		else if(stricmp(cmd.c_str(), _T("log")) == 0) 
		{
			StringMap params;
			params["hubNI"] = client->getHubName();
			params["hubURL"] = client->getHubUrl();
			params["myNI"] = client->getMyNick(); 
			if(param.empty()) 
			{
				WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_MAIN_CHAT), params, false))));
			} 
			else if(stricmp(param.c_str(), _T("status")) == 0) 
			{
				WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_STATUS), params, false))));
			}
		} 
		else if(stricmp(cmd.c_str(), _T("help")) == 0) 
		{
			addLine(_T("*** ") + WinUtil::commands + _T(", /smallfilesize #, /extraslots #, /savequeue, /join <hub-ip>, /clear, /ts, /showjoins, /favshowjoins, /close, /userlist, /connection, /favorite, /pm <user> [message], /getlist <user>, /winamp, /whois [IP], /ignorelist, /removefavorite"), WinUtil::m_ChatTextSystem);
		} 
		else if(stricmp(cmd.c_str(), _T("pm")) == 0) 
		{
			string::size_type j = param.find(_T(' '));
			if(j != string::npos) 
			{
				tstring nick = param.substr(0, j);
				const OnlineUserPtr ui = client->findUser(Text::fromT(nick));

				if(ui) 
				{
					if(param.size() > j + 1)
						PrivateFrame::openWindow(HintedUser(ui->getUser(), client->getHubUrl()), param.substr(j+1), client);
					else
						PrivateFrame::openWindow(HintedUser(ui->getUser(), client->getHubUrl()), Util::emptyStringT, client);
						
				}
			} 
			else if(!param.empty()) 
			{
				const OnlineUserPtr ui = client->findUser(Text::fromT(param));
				if(ui)
					PrivateFrame::openWindow(HintedUser(ui->getUser(), client->getHubUrl()), Util::emptyStringT, client);
			}
		} 
		else if(stricmp(cmd.c_str(), _T("stats")) == 0) 
		{
			if(client->isOp())
				client->hubMessage(Text::fromT(WinUtil::generateStats()));
			else
				addLine(WinUtil::generateStats());
		} 
		else 
		{
			if (BOOLSETTING(SEND_UNKNOWN_COMMANDS)) 
			{
				client->hubMessage(Text::fromT(s));
			} 
			else 
			{
				addStatus(TSTRING(UNKNOWN_COMMAND) + _T(" ") + cmd);
			}
		}
	} 
	else if(waitingForPW) 
	{
		addStatus(TSTRING(DONT_REMOVE_SLASH_PASSWORD));
		ctrlChat->getCtrlMessage()->SetValue(_T("/password "));
		ctrlChat->getCtrlMessage()->SetFocus();
		ctrlChat->getCtrlMessage()->SetSelection(10, 10);

		event.Skip(); // don't clear message box
	} 
	else 
	{
		// TODO if(BOOLSETTING(CZCHARS_DISABLE))
		//	s = WinUtil::disableCzChars(s);

		client->hubMessage(Text::fromT(s));
	}
}

void HubFrame::OnTab(wxKeyEvent& event)
{
	if(ctrlChat->getCtrlMessage()->GetValue().IsEmpty()) 
	{
		event.Skip();
		return;
	}
		
	if(!event.ShiftDown()) 
	{
		tstring text = ctrlChat->getCtrlMessage()->GetValue();

		string::size_type textStart = text.find_last_of(_T(" \n\t"));

		if(complete.empty()) 
		{
			if(textStart != string::npos) 
			{
				complete = text.substr(textStart + 1);
			} 
			else 
			{
				complete = text;
			}
			if(complete.empty()) 
			{
				// Still empty, no text entered...
				ctrlUsers->SetFocus();
				return;
			}
			
			ctrlUsers->UnselectAll();
			
		}

		if(textStart == string::npos)
			textStart = 0;
		else
			textStart++;

		int start = ctrlUsers->getFirstSelectedItem() + 1;
		int i = start;
		int j = ctrlUsers->getItemCount();

		bool firstPass = i < j;
		if(!firstPass)
			i = 0;
		while(firstPass || (!firstPass && i < start)) 
		{
			const OnlineUserPtr ui = ctrlUsers->getItemData(i);
			const tstring& nick = ui->getText(OnlineUser::COLUMN_NICK);
			bool found = (strnicmp(nick, complete, complete.length()) == 0);
			tstring::size_type x = 0;
			if(!found) 
			{
				// Check if there's one or more [ISP] tags to ignore...
				tstring::size_type y = 0;
				while(nick[y] == _T('[')) 
				{
					x = nick.find(_T(']'), y);
					if(x != string::npos) 
					{
						if(strnicmp(nick.c_str() + x + 1, complete.c_str(), complete.length()) == 0) 
						{
							found = true;
							break;
						}
					} 
					else 
					{
						break;
					}
					y = x + 1; // assuming that nick[y] == '\0' is legal
				}
			}
			if(found) 
			{
				if((start - 1) != -1) 
				{
					ctrlUsers->unselect(start - 1);
				}
				ctrlUsers->select(i);
				ctrlUsers->ensureVisible(i);
				ctrlChat->getCtrlMessage()->SetSelection(textStart, ctrlChat->getCtrlMessage()->GetValue().Length());
				ctrlChat->getCtrlMessage()->Replace(textStart, ctrlChat->getCtrlMessage()->GetValue().Length(), nick.c_str());
				return;
			}
			i++;
			if(i == j) 
			{
				firstPass = false;
				i = 0;
			}
		}
	}
}

void HubFrame::OnShowUsers(wxCommandEvent& event)
{
	if(event.IsChecked()) 
	{	
		chatUsersSplitter->Freeze();
		chatUsersSplitter->SplitVertically(ctrlChat, userListPanel);
		chatUsersSplitter->SetSashPosition(chatUserSplitSize, true);
		client->refreshUserList(true);
		chatUsersSplitter->Thaw();
	}
	else 
	{
		clearUserList();

		// save sash position and unsplit
		chatUserSplitSize = chatUsersSplitter->GetSashPosition();
		chatUsersSplitter->Unsplit(userListPanel);
	}

	SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, event.IsChecked());

	UpdateLayout();
}

void HubFrame::OnUnsplit(wxSplitterEvent& /*event*/)
{
	ctrlShowUsers->SetValue(false);
}

void HubFrame::OnUserListContextMenu(wxDataViewEvent& /*event*/)
{
	tabMenuShown = false;

	wxMenu menu;

	wxDataViewItemArray sel;
	ctrlUsers->GetSelections(sel);

	bool isMe = false;

	if(sel.size() == 1) 
	{
		tstring sNick = Text::toT(ctrlUsers->getItemData(sel[0])->getIdentity().getNick());
	    isMe = (sNick == Text::toT(client->getMyNick()));

		menu.SetTitle(sNick);

		if(BOOLSETTING(LOG_PRIVATE_CHAT)) 
		{
			menu.Append(IDC_OPEN_USER_LOG,  CTSTRING(OPEN_USER_LOG));
			menu.AppendSeparator();
		}
	}
	else 
	{
		menu.SetTitle(Util::toStringW(sel.size()) + _T(" ") + TSTRING(HUB_USERS));
	}

	if(!isMe) 
	{
		menu.Append(IDC_PUBLIC_MESSAGE, CTSTRING(SEND_PUBLIC_MESSAGE));
		appendUserItems(menu, client->getHubUrl());
		menu.AppendSeparator();

		if(sel.size() == 1) 
		{
			const OnlineUserPtr ou = ctrlUsers->getItemData(sel[0]);
			if(!FavoriteManager::getInstance()->isIgnoredUser(ou->getUser())) 
			{
				menu.Append(IDC_IGNORE, CTSTRING(IGNORE_USER));
			} 
			else 
			{    
				menu.Append(IDC_UNIGNORE, CTSTRING(UNIGNORE_USER));
			}
			menu.AppendSeparator();
		}
	}

	menu.AppendSubMenu(ctrlUsers->getCopyMenu(), CTSTRING(COPY));

	prepareMenu(menu, ::UserCommand::CONTEXT_CHAT, client->getHubUrl());
	menu.AppendSeparator();
	menu.Append(IDC_REFRESH, CTSTRING(REFRESH_USER_LIST));
	ctrlUsers->PopupMenu(&menu);
}

void HubFrame::OnUserListDblClick(wxDataViewEvent& event)
{
	OnlineUserPtr ou = ctrlUsers->getItemData(event.GetItem());

	if(ou->getUser() != ClientManager::getInstance()->getMe())
	{
		switch(SETTING(USERLIST_DBLCLICK)) 
		{
		    case 0:
				ou->getList(client->getHubUrl());
		        break;
		    case 1:
			{
				wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, IDC_PUBLIC_MESSAGE);
				ProcessEvent(evt);
			}
		    case 2:
				ou->pm(client->getHubUrl());
		        break;
		    case 3:
		        ou->matchQueue(client->getHubUrl());
		        break;
		    case 4:
		        ou->grant(client->getHubUrl());
		        break;
		    case 5:
		        ou->addFav();
		        break;
			case 6:
				ou->browseList(client->getHubUrl());
				break;
		}	
	}
}

void HubFrame::OnFilterSelection(wxCommandEvent& /*event*/)
{
	filter = ctrlFilter->GetValue();

	updateUserList();
	updateUsers = true;
}

void HubFrame::OnFilterChange(wxCommandEvent& /*event*/)
{
	if(!BOOLSETTING(FILTER_ENTER))
	{
		filter = ctrlFilter->GetValue();

		updateUserList();
		updateUsers = true;
	}
}

void HubFrame::OnFilterEnter(wxCommandEvent& /*event*/)
{
	if(BOOLSETTING(FILTER_ENTER))
	{
		filter = ctrlFilter->GetValue();

		updateUserList();
		updateUsers = true;
	}
}

void HubFrame::OnFollow(wxCommandEvent& /*event*/)
{
	if(!redirect.empty())
	{
		if(ClientManager::getInstance()->isConnected(Text::fromT(redirect))) 
		{
			addStatus(TSTRING(REDIRECT_ALREADY_CONNECTED), WinUtil::m_ChatTextServer);
			return;
		}
		
		dcassert(frames.find(server) != frames.end());
		dcassert(frames[server] == this);
		frames.erase(server);
		server = redirect;
		frames[server] = this;

		// the client is dead, long live the client!
		client->removeListener(this);
		clearUserList();
		ClientManager::getInstance()->putClient(client);
		clearTaskList();
		client = ClientManager::getInstance()->getClient(Text::fromT(server));

		ctrlChat->setClient(client);

		RecentHubEntry r;
		r.setName("*");
		r.setDescription("***");
		r.setUsers("*");
		r.setShared("*");
		r.setServer(Text::fromT(redirect));
		FavoriteManager::getInstance()->addRecent(r);

		client->addListener(this);
		client->connect();
	}
}

void HubFrame::OnCopyHubInfo(wxCommandEvent& event)
{
    if(client->isConnected()) 
	{
        string sCopy;

		switch (event.GetId())
		{
			case IDC_COPY_HUBNAME:
				sCopy += client->getHubName();
				break;
			case IDC_COPY_HUBADDRESS:
				sCopy += client->getHubUrl();
				break;
		}

		if(!sCopy.empty())
			WinUtil::setClipboard(Text::toT(sCopy));
    }
}

void HubFrame::OnOpenHubLog(wxCommandEvent& /*event*/)
{
	string logFile = getLogFile();
	if(Util::fileExists(logFile))
	{
		WinUtil::openFile(Text::toT(logFile));
	} 
	else 
	{
		wxMessageBox(CTSTRING(NO_LOG_FOR_HUB), CTSTRING(NO_LOG_FOR_HUB), wxOK);
	}
}

void HubFrame::OnEditClearAll(wxCommandEvent& /*event*/)
{
	ctrlChat->Clear();
}

void HubFrame::OnAddFavorite(wxCommandEvent& /*event*/)
{
	addAsFavorite();
}

void HubFrame::addAsFavorite() 
{
	const FavoriteHubEntry* existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());
	if(!existingHub) 
	{
		FavoriteHubEntry aEntry;
		aEntry.setServer(Text::fromT(server));
		aEntry.setName(client->getHubName());
		aEntry.setDescription(client->getHubDescription());
		aEntry.setConnect(false);
		if(!client->getPassword().empty()) 
		{
			aEntry.setNick(client->getMyNick());
			aEntry.setPassword(client->getPassword());
		}
		aEntry.setConnect(false);
		FavoriteManager::getInstance()->addFavorite(aEntry);
		addStatus(TSTRING(FAVORITE_HUB_ADDED), WinUtil::m_ChatTextSystem);
	} 
	else 
	{
		addStatus(TSTRING(FAVORITE_HUB_ALREADY_EXISTS), WinUtil::m_ChatTextSystem);
	}
}

void HubFrame::removeFavoriteHub() 
{
	const FavoriteHubEntry* removeHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());
	if(removeHub) 
	{
		FavoriteManager::getInstance()->removeFavorite(removeHub);
		addStatus(TSTRING(FAVORITE_HUB_REMOVED), WinUtil::m_ChatTextSystem);
	} 
	else 
	{
		addStatus(TSTRING(FAVORITE_HUB_DOES_NOT_EXIST), WinUtil::m_ChatTextSystem);
	}
}

void HubFrame::OnReconnect(wxCommandEvent& /*event*/)
{
	client->reconnect();
}

void HubFrame::OnCopyUserInfo(wxCommandEvent& event)
{
	tstring sCopy;

	wxDataViewItemArray sel;
	ctrlUsers->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const OnlineUserPtr ou = ctrlUsers->getItemData(sel[i]);
	
		if(!sCopy.empty())
			sCopy += _T("\r\n");

		sCopy += ou->getText(static_cast<uint8_t>(event.GetId() - IDC_COPY));
	}

	if (!sCopy.empty())
		WinUtil::setClipboard(sCopy);

}

void HubFrame::OnOpenUserLog(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	ctrlUsers->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		const OnlineUserPtr ou = ctrlUsers->getItemData(sel[i]);
		StringMap params;

		params["userNI"] = ou->getIdentity().getNick();
		params["hubNI"] = client->getHubName();
		params["myNI"] = client->getMyNick();
		params["userCID"] = ou->getUser()->getCID().toBase32();
		params["hubURL"] = client->getHubUrl();

		tstring file = Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params, false)));
		if(Util::fileExists(Text::fromT(file))) 
		{
			WinUtil::openFile(file);
		} 
		else 
		{
			wxMessageBox(CTSTRING(NO_LOG_FOR_USER),CTSTRING(NO_LOG_FOR_USER), wxOK);	  
		}
	}
}

void HubFrame::OnPublicMessage(wxCommandEvent& /*event*/)
{
	if(!client->isConnected())
		return;

	tstring sUsers = Util::emptyStringT;

	if(ctrlChat->getSelectedUser().empty()) 
	{
		wxDataViewItemArray sel;
		ctrlUsers->GetSelections(sel);

		for(unsigned int i = 0; i < sel.size(); ++i)
		{
			if (!sUsers.empty())
				sUsers += _T(", ");

			sUsers += Text::toT((ctrlUsers->getItemData(sel[i]))->getIdentity().getNick());
		}
	}
	else
	{
		sUsers = ctrlChat->getSelectedUser();
	}

	long selBegin, selEnd;
	ctrlChat->getCtrlMessage()->GetSelection(&selBegin, &selEnd);

	if(selBegin == 0  && selEnd == 0)
	{
		sUsers += _T(": ");
		if (ctrlChat->getCtrlMessage()->GetValue().IsEmpty()) 
		{	
			ctrlChat->getCtrlMessage()->SetValue(sUsers.c_str());
			ctrlChat->getCtrlMessage()->SetFocus();
			ctrlChat->getCtrlMessage()->SetSelection(ctrlChat->getCtrlMessage()->GetValue().Length(), ctrlChat->getCtrlMessage()->GetValue().Length());
		} 
		else 
		{
			ctrlChat->getCtrlMessage()->Replace(selBegin, selEnd, sUsers.c_str());
			ctrlChat->getCtrlMessage()->SetFocus();
		}
	} 
	else 
	{
		sUsers += _T(" ");
		ctrlChat->getCtrlMessage()->Replace(selBegin, selEnd, sUsers.c_str());
		ctrlChat->getCtrlMessage()->SetFocus();
	}

}

void HubFrame::OnIgnore(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	ctrlUsers->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		FavoriteManager::getInstance()->addIgnoredUser((ctrlUsers->getItemData(sel[i]))->getUser());
	}
}

void HubFrame::OnUnignore(wxCommandEvent& /*event*/)
{
	wxDataViewItemArray sel;
	ctrlUsers->GetSelections(sel);

	for(unsigned int i = 0; i < sel.size(); ++i)
	{
		FavoriteManager::getInstance()->removeIgnoredUser((ctrlUsers->getItemData(sel[i]))->getUser());
	}
}

void HubFrame::OnRefresh(wxCommandEvent& /*event*/)
{
	if(client->isConnected()) 
	{
		clearUserList();
		client->refreshUserList(false);
	}
}

void HubFrame::OnGetAttributes(wxCommandEvent& event)
{
	wxDataViewItemAttr* attr = (wxDataViewItemAttr*)event.GetClientData();

	OnlineUser* ui = ctrlUsers->getItemData(event.GetInt());

	if (FavoriteManager::getInstance()->isFavoriteUser(ui->getUser())) 
	{
		attr->SetColour(SETTING(FAVORITE_COLOR));
	} 
	else if (UploadManager::getInstance()->hasReservedSlot(ui->getUser())) 
	{
		attr->SetColour(SETTING(RESERVED_SLOT_COLOR));
	} 
	else if (FavoriteManager::getInstance()->isIgnoredUser(ui->getUser())) 
	{
		attr->SetColour(SETTING(IGNORED_COLOR));
	} 
	else if(ui->getIdentity().getStatus() & Identity::FIREBALL) 
	{
		attr->SetColour(SETTING(FIREBALL_COLOR));
	} 
	else if(ui->getIdentity().getStatus() & Identity::SERVER) 
	{
		attr->SetColour(SETTING(SERVER_COLOR));
	} 
	else if(ui->getIdentity().isOp()) 
	{
		attr->SetColour(SETTING(OP_COLOR));
	} 
	else if(!ui->getIdentity().isTcpActive(client)) 
	{
		attr->SetColour(SETTING(PASIVE_COLOR));
	} 
	else 
	{
		attr->SetColour(SETTING(NORMAL_COLOUR));
	}

	if (client->isOp()) 
	{
		if(Util::toInt(ui->getIdentity().get("FC")) & Identity::BAD_CLIENT) 
		{
			attr->SetColour(SETTING(BAD_CLIENT_COLOUR));
		} 
		else if(Util::toInt(ui->getIdentity().get("FC")) & Identity::BAD_LIST) 
		{
			attr->SetColour(SETTING(BAD_FILELIST_COLOUR));
		} 
		else if(BOOLSETTING(SHOW_SHARE_CHECKED_USERS) && (Util::toInt(ui->getIdentity().get("FC")) & Identity::CHECKED) == Identity::CHECKED) 
		{
			attr->SetColour(SETTING(FULL_CHECKED_COLOUR));
		}
	}
}

void HubFrame::addLine(const tstring& aLine) 
{
	addLine(Identity(NULL, 0), aLine, WinUtil::m_ChatTextGeneral);
}


void HubFrame::addLine(const tstring& aLine, const wxRichTextAttr& cf, bool bUseEmo) 
{
    addLine(Identity(NULL, 0), aLine, cf, bUseEmo);
}

void HubFrame::addLine(const Identity& i, const tstring& aLine, const wxRichTextAttr& cf, bool bUseEmo) 
{
	if(BOOLSETTING(LOG_MAIN_CHAT)) 
	{
		StringMap params;
		params["message"] = Text::fromT(aLine);
		client->getHubIdentity().getParams(params, "hub", false);
		params["hubURL"] = client->getHubUrl();
		client->getMyIdentity().getParams(params, "my", true);
		LOG(LogManager::CHAT, params);
	}

	if(timeStamps) 
	{
		ctrlChat->AppendText(i, Text::toT(client->getCurrentNick()), Text::toT("[" + Util::getShortTimeString() + "] "), aLine + _T('\n'), cf, bUseEmo);
	} 
	else 
	{
		ctrlChat->AppendText(i, Text::toT(client->getCurrentNick()), Util::emptyStringT, aLine + _T('\n'), cf, bUseEmo);
	}
	if (BOOLSETTING(BOLD_HUB)) 
	{
		setDirty();
	}
}

void HubFrame::addStatus(const tstring& aLine, const wxRichTextAttr& cf, bool inChat /* = true */) 
{
	tstring line = _T("[") + Text::toT(Util::getShortTimeString()) + _T("] ") + aLine;

   	if(line.size() > 512) 
	{
		line.resize(512);
	}

	GetStatusBar()->SetStatusText(line, 0);
	while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
		lastLinesList.erase(lastLinesList.begin());
	lastLinesList.push_back(line);

	wxString lastLines;
	for(TStringIter i = lastLinesList.begin(); i != lastLinesList.end(); ++i) 
	{
		lastLines += *i;
		lastLines += _T("\r\n");
	}
	if(lastLines.size() > 2) 
	{
		lastLines.erase(lastLines.size() - 2);
	}
	this->GetStatusBar()->SetToolTip(new wxToolTip(lastLines));

	if (BOOLSETTING(BOLD_HUB)) 
	{
		setDirty();
	}
	
	if(BOOLSETTING(STATUS_IN_CHAT) && inChat) 
	{
		addLine(_T("*** ") + aLine, cf);
	}
	if(BOOLSETTING(LOG_STATUS_MESSAGES)) 
	{
		StringMap params;
		client->getHubIdentity().getParams(params, "hub", false);
		params["hubURL"] = client->getHubUrl();
		client->getMyIdentity().getParams(params, "my", true);
		params["message"] = Text::fromT(aLine);
		LOG(LogManager::STATUS, params);
	}
}

string HubFrame::getLogFile() const
{
	StringMap params;
	params["hubNI"] = client->getHubName();
	params["hubURL"] = client->getHubUrl();
	params["myNI"] = client->getMyNick(); 

	return Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_MAIN_CHAT), params, false));
}

bool HubFrame::updateUser(const UserTask& u)
{
	if(!ctrlShowUsers->IsChecked()) 
		return false;
	
	if(!u.onlineUser->isInList) 
	{
		u.onlineUser->update(-1);

		if(!u.onlineUser->isHidden()) 
		{
			u.onlineUser->inc();
			ctrlUsers->insertItem(u.onlineUser.get());
		}

		if(!filter.empty())
			updateUserList(u.onlineUser);
		return true;
	} 
	else 
	{
		int pos = ctrlUsers->findItem(u.onlineUser.get());

		if(pos != -1) 
		{
			/*resort = */u.onlineUser->update(ctrlUsers->getSortColumn(), _T("") /* FIXME: put something better here*/); //|| resort;
			if(u.onlineUser->isHidden()) 
			{
				ctrlUsers->deleteItem(pos);
				u.onlineUser->dec();				
			} 
			else 
			{
				ctrlUsers->updateItem(pos);
			}
		}

		u.onlineUser->getIdentity().set("WO", u.onlineUser->getIdentity().isOp() ? "1" : Util::emptyString);
		updateUserList(u.onlineUser);
		return false;
	}
}

void HubFrame::removeUser(const OnlineUserPtr& aUser)
{
	if(!ctrlShowUsers->IsChecked()) 
		return;
	
	if(!aUser->isHidden()) 
	{
		int i = ctrlUsers->findItem(aUser.get());
		if(i != -1) 
		{
			ctrlUsers->deleteItem(i);
			aUser->dec();
		}
	}
}

void HubFrame::updateUserList(OnlineUserPtr ui) 
{
	int64_t size = -1;
	FilterModes mode = NONE;
	
	//single update?
	//avoid refreshing the whole list and just update the current item
	//instead
	if(ui != NULL) 
	{
		if(ui->isHidden()) 
		{
			return;
		}
		if(filter.empty()) 
		{
			if(ctrlUsers->findItem(ui.get()) == -1) 
			{
				ui->inc();
				ctrlUsers->insertItem(ui.get());
			}
		} 
		else 
		{
			int sel = ctrlFilterSel->GetSelection();
			bool doSizeCompare = sel == OnlineUser::COLUMN_SHARED && parseFilter(mode, size);

			if(matchFilter(*ui, sel, doSizeCompare, mode, size)) 
			{
				if(ctrlUsers->findItem(ui.get()) == -1) 
				{
					ui->inc();
					ctrlUsers->insertItem(ui.get());
				}
			} 
			else 
			{
				int i = ctrlUsers->findItem(ui.get());
				if(i != -1) 
				{
					ctrlUsers->deleteItem(i);
					ui->dec();
				}
			}
		}
	} 
	else 
	{
		ctrlUsers->Freeze();
		clearUserList();

		OnlineUserList l;
		client->getUserList(l);

		if(filter.empty()) 
		{
			for(OnlineUserList::const_iterator i = l.begin(); i != l.end(); ++i){
				const OnlineUserPtr& ui = *i;
				if(!ui->isHidden()) 
				{
					ui->inc();
					ctrlUsers->insertItem(ui.get());
				}
			}
		} 
		else 
		{
			int sel = ctrlFilterSel->GetSelection();
			bool doSizeCompare = sel == OnlineUser::COLUMN_SHARED && parseFilter(mode, size);

			for(OnlineUserList::const_iterator i = l.begin(); i != l.end(); ++i) 
			{
				const OnlineUserPtr& ui = *i;
				if(!ui->isHidden() && matchFilter(*ui, sel, doSizeCompare, mode, size)) 
				{
					ui->inc();
					ctrlUsers->insertItem(ui.get());
				}
			}
		}
		ctrlUsers->Thaw();
	}

}

bool HubFrame::parseFilter(FilterModes& mode, int64_t& size) 
{
	tstring::size_type start = (tstring::size_type)tstring::npos;
	tstring::size_type end = (tstring::size_type)tstring::npos;
	int64_t multiplier = 1;
	
	if(filter.empty()) 
	{
		return false;
	}

	if(filter.compare(0, 2, _T(">=")) == 0) 
	{
		mode = GREATER_EQUAL;
		start = 2;
	} 
	else if(filter.compare(0, 2, _T("<=")) == 0) 
	{
		mode = LESS_EQUAL;
		start = 2;
	} 
	else if(filter.compare(0, 2, _T("==")) == 0) 
	{
		mode = EQUAL;
		start = 2;
	} 
	else if(filter.compare(0, 2, _T("!=")) == 0) 
	{
		mode = NOT_EQUAL;
		start = 2;
	} 
	else if(filter[0] == _T('<')) 
	{
		mode = LESS;
		start = 1;
	} 
	else if(filter[0] == _T('>')) 
	{
		mode = GREATER;
		start = 1;
	} 
	else if(filter[0] == _T('=')) 
	{
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

bool HubFrame::matchFilter(const OnlineUser& ui, int sel, bool doSizeCompare, FilterModes mode, int64_t size) 
{
	if(filter.empty())
		return true;

	bool insert = false;
	if(doSizeCompare) 
	{
		switch(mode) 
		{
			case EQUAL: insert = (size == ui.getIdentity().getBytesShared()); break;
			case GREATER_EQUAL: insert = (size <=  ui.getIdentity().getBytesShared()); break;
			case LESS_EQUAL: insert = (size >=  ui.getIdentity().getBytesShared()); break;
			case GREATER: insert = (size < ui.getIdentity().getBytesShared()); break;
			case LESS: insert = (size > ui.getIdentity().getBytesShared()); break;
			case NOT_EQUAL: insert = (size != ui.getIdentity().getBytesShared()); break;
			case NONE: ; break;
		}
	} 
	else
	{
		try 
		{
			boost::wregex reg(filter, boost::regex_constants::icase);
			if(sel >= OnlineUser::COLUMN_LAST) {
				for(uint8_t i = OnlineUser::COLUMN_FIRST; i < OnlineUser::COLUMN_LAST; ++i) {
					tstring s = ui.getText(i);
					if(boost::regex_search(s.begin(), s.end(), reg)) {
						insert = true;
						break;
					}
				}
			} else {
				tstring s = ui.getText(static_cast<uint8_t>(sel));
				if(boost::regex_search(s.begin(), s.end(), reg))
					insert = true;
			}
		} catch(...) {
			insert = true;
		}
	}

	return insert;
}

// FavoriteManagerListener
void HubFrame::on(FavoriteManagerListener::UserAdded, const FavoriteUser& /*aUser*/) throw()
{
	resortForFavsFirst();
}

void HubFrame::on(FavoriteManagerListener::UserRemoved, const FavoriteUser& /*aUser*/) throw()
{
	resortForFavsFirst();
}

void HubFrame::resortForFavsFirst(bool justDoIt /* = false */) 
{
	if(justDoIt || BOOLSETTING(SORT_FAVUSERS_FIRST)) 
	{
		resort = true;
		callAsync(std::bind(&HubFrame::OnTasks, this));
	}
}

// TimerManagerListener
void HubFrame::on(TimerManagerListener::Second, uint64_t /*aTick*/) throw()
{
	if(updateUsers) 
	{
		updateUsers = false;
		callAsync(std::bind(&HubFrame::OnTasks, this));
	}
}

void HubFrame::on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw()
{
}

// ClientListener
void HubFrame::on(Connecting, const Client*) throw()
{
	if(BOOLSETTING(SEARCH_PASSIVE) && ClientManager::getInstance()->isActive(client->getHubUrl())) 
	{
		callAsync([this]() { addLine(TSTRING(ANTI_PASSIVE_SEARCH), WinUtil::m_ChatTextSystem); });
	}

	tstring hubUrl = Text::toT(client->getHubUrl());
	callAsync([this, hubUrl]() { addStatus(TSTRING(CONNECTING_TO) + " " + hubUrl + "...", WinUtil::m_ChatTextServer, true); });
	callAsync([this, hubUrl]() { SetTitle(hubUrl); });
}

void HubFrame::on(Connected, const Client*) throw()
{
	callAsync(std::bind(&HubFrame::OnConnected, this));
}

void HubFrame::on(UserUpdated, const Client*, const OnlineUserPtr& user) throw()
{
	speak(UPDATE_USER_JOIN, user);
}

void HubFrame::on(UsersUpdated, const Client*, const OnlineUserList& aList) throw()
{
	for(OnlineUserList::const_iterator i = aList.begin(); i != aList.end(); ++i) 
	{
		tasks.add(UPDATE_USER, new UserTask(*i));
	}
	updateUsers = true;
}

void HubFrame::on(ClientListener::UserRemoved, const Client*, const OnlineUserPtr& user) throw()
{
	speak(REMOVE_USER, user);
}

void HubFrame::on(Redirect, const Client*, const string& line) throw()
{
	if(ClientManager::getInstance()->isConnected(line)) 
	{
		callAsync(std::bind(&HubFrame::addStatus, this, TSTRING(REDIRECT_ALREADY_CONNECTED), WinUtil::m_ChatTextServer, true));
		return;
	}

	redirect = Text::toT(line);
	if(BOOLSETTING(AUTO_FOLLOW)) 
	{
		callAsync(std::bind(&HubFrame::OnFollow, this, wxCommandEvent()));
	} 
	else 
	{
		callAsync(std::bind(&HubFrame::addStatus, this, TSTRING(PRESS_FOLLOW) + _T(" ") + Text::toT(line), WinUtil::m_ChatTextServer, true));
	}
}

void HubFrame::on(Failed, const Client*, const string& line) throw()
{
	callAsync(std::bind(&HubFrame::addStatus, this, Text::toT(line), WinUtil::m_ChatTextServer, true));
	callAsync(std::bind(&HubFrame::OnDisconnected, this));
}

void HubFrame::on(GetPassword, const Client*) throw()
{
	callAsync(std::bind(&HubFrame::OnGetPassword, this));
}

void HubFrame::on(HubUpdated, const Client*) throw()
{
	string hubName;
	if(client->isTrusted()) {
		hubName = "[S] ";
	} else if(client->isSecure()) {
		hubName = "[U] ";
	}
	
	hubName += client->getHubName();
	if(!client->getHubDescription().empty()) {
		hubName += " - " + client->getHubDescription();
	}
	hubName += " (" + client->getHubUrl() + ")";
#ifdef _DEBUG
	string version = client->getHubIdentity().get("VE");
	if(!version.empty()) {
		hubName += " - " + version;
	}
#endif

	tstring hubNameT = Text::toT(hubName);
	callAsync([this, hubNameT]() { SetTitle(hubNameT); });
}

void HubFrame::on(Message, const Client*, const ChatMessage& message) throw()
{
	if(message.to && message.replyTo) 
	{
		callAsync(std::bind(&HubFrame::OnPrivateMessage, this, message.from->getIdentity(), message.to->getUser(), 
			message.replyTo->getUser(), message.replyTo->getIdentity().isHub(), message.replyTo->getIdentity().isBot(), Text::toT(message.format())));
	} 
	else 
	{
		callAsync(std::bind(&HubFrame::OnChatMessage, this, message.from->getIdentity(), Text::toT(message.format())));
	}
}

void HubFrame::on(StatusMessage, const Client*, const string& line, int statusFlags) throw()
{
	callAsync(std::bind(&HubFrame::addStatus, this, Text::toT(line), WinUtil::m_ChatTextServer, !BOOLSETTING(FILTER_MESSAGES) || !(statusFlags & ClientListener::FLAG_IS_SPAM)));
}

void HubFrame::on(NickTaken, const Client*) throw()
{
	callAsync(std::bind(&HubFrame::addStatus, this, TSTRING(NICK_TAKEN), WinUtil::m_ChatTextServer, true));
}

void HubFrame::on(SearchFlood, const Client*, const string& line) throw()
{
	callAsync(std::bind(&HubFrame::addStatus, this, TSTRING(SEARCH_SPAM_FROM) + _T(" ") + Text::toT(line), WinUtil::m_ChatTextServer, true));
}

void HubFrame::on(CheatMessage, const Client*, const string& line) throw()
{
	// TODO speak(CHEATING_USER, line);
}

void HubFrame::on(HubTopic, const Client*, const string& line) throw()
{
	callAsync(std::bind(&HubFrame::addStatus, this, TSTRING(HUB_TOPIC) + _T("\t") + Text::toT(line), WinUtil::m_ChatTextServer, true));
}

void HubFrame::on(UserReport, const Client*, const Identity&) throw()
{
	/* TODO
	string report = WinUtil::getReport(i, ctrlChat.m_hWnd);
	speak(CHEATING_USER, report);
	*/
}

void HubFrame::speak(Tasks s, const OnlineUserPtr& u) 
{ 
	tasks.add(static_cast<uint8_t>(s), new UserTask(u)); 
	updateUsers = true; 
}

void HubFrame::runUserCommand(const UserCommand& uc) 
{
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	client->getMyIdentity().getParams(ucParams, "my", true);
	client->getHubIdentity().getParams(ucParams, "hub", false);

	if(tabMenuShown) 
	{
		client->escapeParams(ucParams);
		client->sendUserCmd(uc, ucParams);
	} 
	else 
	{
		wxDataViewItemArray sel;
		ctrlUsers->GetSelections(sel);

		for(unsigned int i = 0; i < sel.size(); ++i)
		{
			const OnlineUserPtr ou = ctrlUsers->getItemData(sel[i]);
			if(ou->getUser()->isOnline()) 
			{
				StringMap tmp = ucParams;
				ou->getIdentity().getParams(tmp, "user", true);
				client->escapeParams(tmp);
				client->sendUserCmd(uc, tmp);
			}
		}
	}
}