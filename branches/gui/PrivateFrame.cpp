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
#include "PrivateFrame.h"

#include "../client/ClientManager.h"
#include "../client/FavoriteManager.h"
#include "../client/LogManager.h"

#include <wx/sound.h>
#include <wx/splitter.h>

BEGIN_EVENT_TABLE(PrivateFrame, MDIChildFrame)
	EVT_CLOSE(PrivateFrame::OnClose)
	EVT_SET_FOCUS(PrivateFrame::OnSetFocus)
	EVT_COMMAND(wxID_ANY, SEND_MESSAGE, PrivateFrame::OnSendMessage)
END_EVENT_TABLE()

PrivateFrame::FrameMap PrivateFrame::frames;

PrivateFrame::PrivateFrame(const HintedUser& replyTo_, Client* client) : 
	MDIChildFrame(wxID_ANY, "PrivateFrame", wxColour(0, 255, 255), wxT("IDI_PRIVATE"), wxT("IDI_PRIVATE_OFF")),
	replyTo(replyTo_), priv(FavoriteManager::getInstance()->isPrivate(replyTo_.hint)), isOffline(false)
{
	CreateStatusBar(1, wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE);

	ctrlChat = new ChatPanel(this, wxCLIP_SIBLINGS | wxCLIP_CHILDREN);
	ctrlChat->SetFont(WinUtil::font);
	ctrlChat->setClient(client);
	ctrlChat->readLog(getLogFile(), SETTING(PM_LAST_LINES_LOG));

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(ctrlChat, 1, wxEXPAND | wxBOTTOM, 4);
	SetSizerAndFit(sizer);

	updateTitle();

	ClientManager::getInstance()->addListener(this);
}

PrivateFrame::~PrivateFrame(void)
{
	frames.erase(replyTo);
}

void PrivateFrame::gotMessage(const Identity& from, const UserPtr& to, const UserPtr& replyTo, const tstring& aMessage, Client* c)
{
	PrivateFrame* p = NULL;
	bool myPM = replyTo == ClientManager::getInstance()->getMe();
	const UserPtr& user = myPM ? to : replyTo;
	
	FrameIter i = frames.find(user);
	if(i == frames.end()) {
		if(frames.size() > 200) return;

		MDIFreezer freezer;

		p = new PrivateFrame(HintedUser(user, c->getHubUrl()), c);
		p->Show();
		frames[user] = p;
		
		p->addLine(from, aMessage);

		if(Util::getAway()) 
		{
			if(!(BOOLSETTING(NO_AWAYMSG_TO_BOTS) && user->isSet(User::BOT))) 
			{
				StringMap params;
				from.getParams(params, "user", false);
				p->sendMessage(Text::toT(Util::getAwayMessage(params)));
			}
		}

		
		if(BOOLSETTING(POPUP_NEW_PM)) 
		{
			pair<tstring, bool> hubs = WinUtil::getHubNames(replyTo, c->getHubUrl());
			WinUtil::notifyUser(TSTRING(PRIVATE_MESSAGE), WinUtil::getNicks(replyTo, c->getHubUrl()) + _T(" - ") + hubs.first);
		}
				
		if(!SETTING(BEEPFILE).empty() && !BOOLSETTING(SOUNDS_DISABLED)) 
		{
			wxSound::Play(Text::toT(SETTING(BEEPFILE)), wxSOUND_ASYNC);
		}
	} 
	else 
	{

		if(!myPM) 
		{
			if(BOOLSETTING(POPUP_PM)) 
			{
				pair<tstring, bool> hubs = WinUtil::getHubNames(replyTo, c->getHubUrl());
				WinUtil::notifyUser(TSTRING(PRIVATE_MESSAGE), WinUtil::getNicks(replyTo, c->getHubUrl()) + _T(" - ") + hubs.first);
			}

			if(!SETTING(BEEPFILE).empty() && !BOOLSETTING(SOUNDS_DISABLED)) 
			{
				wxSound::Play(Text::toT(SETTING(BEEPFILE)), wxSOUND_ASYNC);
			}
		}
		i->second->addLine(from, aMessage);
	}
}

void PrivateFrame::openWindow(const HintedUser& replyTo, const tstring& msg, Client* c)
{
	PrivateFrame* p = NULL;
	FrameIter i = frames.find(replyTo);
	if(i == frames.end()) 
	{
		// check DDoS
		if(frames.size() > 200) 
			return;

		MDIFreezer freezer;
		p = new PrivateFrame(replyTo, c);
		p->Show();
		frames[replyTo] = p;
	} 
	else 
	{
		p = i->second;
		if(p->IsIconized())
			p->Restore();

		p->Activate();
	}

	if(!msg.empty())
		p->sendMessage(msg);
}

void PrivateFrame::OnClose(wxCloseEvent& event)
{
	ClientManager::getInstance()->removeListener(this);
	event.Skip();
}

void PrivateFrame::updateTitle() 
{
	pair<tstring, bool> hubs = WinUtil::getHubNames(replyTo.user->getCID(), replyTo.hint, priv);
	if(hubs.second) 
	{	
		unsetIconState();
		setTabColor(RGB(0, 255,	255));
		if(isOffline) 
		{
			tstring status = _T(" *** ") + TSTRING(USER_WENT_ONLINE) + _T(" [") + WinUtil::getNicks(replyTo.user->getCID(), replyTo.hint, priv) + _T(" - ") + hubs.first + _T("] ***");
			if(BOOLSETTING(STATUS_IN_CHAT)) 
			{
				addLine(status, WinUtil::m_ChatTextServer);
			} 
			else 
			{
				addStatusLine(status);
			}
		}

		isOffline = false;
	} 
	else 
	{
		setIconState();
		setTabColor(RGB(255, 0, 0));
		tstring status = _T(" *** ") + TSTRING(USER_WENT_OFFLINE);
		if(BOOLSETTING(STATUS_IN_CHAT)) 
		{
			addLine(status, WinUtil::m_ChatTextServer);
		} 
		else 
		{
			addStatusLine(status);
		}
		
		isOffline = true;
		ctrlChat->setClient(NULL);
	}

	SetTitle(WinUtil::getNicks(replyTo.user->getCID(), replyTo.hint, priv) + _T(" - ") + hubs.first);
}

void PrivateFrame::fillLogParams(StringMap& params) const
{
	const CID& cid = replyTo.user->getCID();
	const string& hint = replyTo.hint;	
	params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(cid, hint, priv));
	params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(cid, hint, priv));
	params["userCID"] = cid.toBase32(); 
	params["userNI"] = ClientManager::getInstance()->getNicks(cid, hint, priv)[0];
	params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
}

string PrivateFrame::getLogFile() const
{
	StringMap params;
	fillLogParams(params);
	return Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params, false));
}

void PrivateFrame::addLine(const tstring& aLine, const wxRichTextAttr& cf)
{
    addLine(Identity(NULL, 0), aLine, cf);
}

void PrivateFrame::addLine(const Identity& from, const tstring& aLine)
{
	addLine(from, aLine, WinUtil::m_ChatTextGeneral);
}

void PrivateFrame::addLine(const Identity& from, const tstring& aLine, const wxRichTextAttr& cf)
{
	if(BOOLSETTING(LOG_PRIVATE_CHAT)) 
	{
		StringMap params;
		params["message"] = Text::fromT(aLine);
		fillLogParams(params);
		LOG(LogManager::PM, params);
	}

	if(BOOLSETTING(TIME_STAMPS)) 
	{
		ctrlChat->AppendText(from, Text::toT(SETTING(NICK)), Text::toT("[" + Util::getShortTimeString() + "] "), aLine + _T('\n'), cf);
	} 
	else 
	{
		ctrlChat->AppendText(from, Text::toT(SETTING(NICK)), _T(""), aLine + _T('\n'), cf);
	}

	addStatusLine(TSTRING(LAST_CHANGE) + _T(" ") + Text::toT(Util::getTimeString()));

	if (BOOLSETTING(BOLD_PM)) 
	{
		setDirty();
	}
}

void PrivateFrame::addStatusLine(const tstring& aLine)
{
	GetStatusBar()->SetStatusText(_T("[") + Text::toT(Util::getShortTimeString()) + _T("] ") + aLine, 0);

	if (BOOLSETTING(BOLD_PM)) 
	{
		setDirty();
	}
}

void PrivateFrame::OnSendMessage(wxCommandEvent& event)
{
	tstring s(event.GetString());

	if(replyTo.user->isOnline()) 
	{
	/* TODO	if(BOOLSETTING(CZCHARS_DISABLE))
			s = WinUtil::disableCzChars(s);*/

		sendMessage(s);
	} 
	else 
	{
		GetStatusBar()->SetStatusText(CTSTRING(USER_WENT_OFFLINE), 0);
		event.Skip();
	}
}

void PrivateFrame::sendMessage(const tstring& msg, bool thirdPerson) 
{
	ClientManager::getInstance()->privateMessage(replyTo, Text::fromT(msg), thirdPerson);
}

	// ClientManagerListener
void PrivateFrame::on(ClientManagerListener::UserUpdated, const OnlineUser& aUser) throw() 
{
	if(aUser.getUser() == replyTo.user) 
	{
		ctrlChat->setClient(const_cast<Client*>(&aUser.getClient()));
		callAsync(std::bind(&PrivateFrame::updateTitle, this));
	}
}

void PrivateFrame::on(ClientManagerListener::UserConnected, const UserPtr& aUser) throw() 
{
	if(aUser == replyTo.user)
		callAsync(std::bind(&PrivateFrame::updateTitle, this));
}

void PrivateFrame::on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) throw() 
{
	if(aUser == replyTo.user) 
	{
		ctrlChat->setClient(NULL);
		callAsync(std::bind(&PrivateFrame::updateTitle, this));
	}
}