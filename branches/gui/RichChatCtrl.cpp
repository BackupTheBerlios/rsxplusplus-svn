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
#include "RichChatCtrl.h"

#include "EmoticonsManager.h"
#include "HubFrame.h"
#include "PrivateFrame.h"
#include "WinUtil.h"

#include "../client/ClientManager.h"
#include "../client/LogManager.h"
#include "../client/QueueManager.h"
#include "../client/UploadManager.h"
#include "../client/User.h"

#include <wx/sound.h>
#include <wx/wx.h>

#define MAX_EMOTICONS 48

EmoticonsManager* emoticonsManager = NULL;

static const tstring protocols[] = { _T("http://"), _T("https://"), _T("www."), _T("ftp://"), 
	_T("magnet:?"), _T("dchub://"), _T("irc://"), _T("ed2k://"), _T("mms://"), _T("file://"),
	_T("adc://"), _T("adcs://"), _T("nmdcs://"), _T("svn://") };

BEGIN_EVENT_TABLE(RichChatCtrl, wxRichTextCtrl)
	EVT_SIZE(RichChatCtrl::OnSize)
	EVT_CONTEXT_MENU(RichChatCtrl::OnContextMenu)
	EVT_LEFT_DOWN(RichChatCtrl::OnLeftDown)
	EVT_LEFT_DCLICK(RichChatCtrl::OnDblClick)
	EVT_TEXT_URL(wxID_ANY, RichChatCtrl::OnURL)

	EVT_MENU(IDC_COPY_ACTUAL_LINE, RichChatCtrl::OnCopyActualLine)
	EVT_MENU(ID_EDIT_COPY, RichChatCtrl::OnEditCopy)
	EVT_MENU(ID_EDIT_SELECT_ALL, RichChatCtrl::OnEditSelectAll)
	EVT_MENU(ID_EDIT_CLEAR_ALL, RichChatCtrl::OnEditClearAll)
	EVT_MENU(IDC_BAN_IP, RichChatCtrl::OnBanIP)
	EVT_MENU(IDC_UNBAN_IP, RichChatCtrl::OnUnBanIP)
	EVT_MENU(IDC_COPY_URL, RichChatCtrl::OnCopyURL)
	EVT_MENU(IDC_WHOIS_IP, RichChatCtrl::OnWhoisIP)

	EVT_MENU(IDC_OPEN_USER_LOG, RichChatCtrl::OnOpenUserLog)
	EVT_MENU(IDC_PRIVATEMESSAGE, RichChatCtrl::OnPrivateMessage)
	EVT_MENU(IDC_GETLIST, RichChatCtrl::OnGetList)
	EVT_MENU(IDC_MATCH_QUEUE, RichChatCtrl::OnMatchQueue)
	EVT_MENU(IDC_GRANTSLOT, RichChatCtrl::OnGrantSlot)
	EVT_MENU(IDC_GRANTSLOT_HOUR, RichChatCtrl::OnGrantSlot)
	EVT_MENU(IDC_GRANTSLOT_DAY, RichChatCtrl::OnGrantSlot)
	EVT_MENU(IDC_GRANTSLOT_WEEK, RichChatCtrl::OnGrantSlot)
	EVT_MENU(IDC_UNGRANTSLOT, RichChatCtrl::OnGrantSlot)
	EVT_MENU(IDC_ADD_TO_FAVORITES, RichChatCtrl::OnAddToFavorites)
	EVT_MENU(IDC_IGNORE, RichChatCtrl::OnIgnore)
	EVT_MENU(IDC_UNIGNORE, RichChatCtrl::OnUnignore)

	EVT_MENU_RANGE(IDC_COPY, IDC_COPY + OnlineUser::COLUMN_LAST, OnCopyUserInfo)

	EVT_MENU(IDC_REPORT, OnReport)
	EVT_MENU(IDC_CHECKLIST, OnCheckList)
END_EVENT_TABLE() 

#define STYLE_FLAGS wxRICHTEXT_SETSTYLE_RESET

RichChatCtrl::RichChatCtrl(wxWindow* parent, long style) : wxRichTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, style), client(NULL)
{
	if(emoticonsManager == NULL) 
	{
		emoticonsManager = new EmoticonsManager();
	}
	
	emoticonsManager->inc();

	// set no spacing between lines
	wxTextAttrEx basicStyle = GetBasicStyle(); 
	basicStyle.SetParagraphSpacingAfter(0); 
	SetBasicStyle(basicStyle);
}

RichChatCtrl::~RichChatCtrl(void)
{
	if(emoticonsManager->unique()) 
	{
		emoticonsManager->dec();
		emoticonsManager = NULL;
	} 
	else 
	{
		emoticonsManager->dec();
	}
}

void RichChatCtrl::AppendText(const Identity& i, const tstring& myNick, const tstring& time, tstring msg, const wxRichTextAttr& attr, bool useEmoticons)
{
	Freeze();

	int scrollPos = GetScrollPos(wxVERTICAL);
	int pageSize = GetScrollPageSize(wxVERTICAL);
	int scrollMax = GetScrollRange(wxVERTICAL);

	long lSelBegin = 0, lSelEnd = 0;
	long lSelBeginSaved, lSelEndSaved;
	
	// Unify line endings
	tstring::size_type j = 0; 
	while((j = msg.find(_T("\r"), j)) != tstring::npos)
		msg.erase(j, 1);

	GetSelection(&lSelBeginSaved, &lSelEndSaved);
	lSelEnd = lSelBegin = GetLastPosition();

	bool isMyMessage = i.getUser() == ClientManager::getInstance()->getMe();
	tstring sLine = time + msg;
	
	// TODO: Remove old chat if size exceeds
	
	wxRichTextCtrl::AppendText(sLine);

	if(!time.empty())
	{
		lSelEnd += time.size();
		SetStyleEx(wxRichTextRange(lSelBegin, lSelEnd - 1), WinUtil::m_TextStyleTimestamp, STYLE_FLAGS);
	}

	// Authors nick
	tstring author = Text::toT(i.getNick());
	if(!author.empty()) 
	{
		LONG iLen = (msg[0] == _T('*')) ? 1 : 0;
		LONG iAuthorLen = author.size() + 2;
		msg.erase(0, iAuthorLen + iLen);
   		
		lSelBegin = lSelEnd;
		lSelEnd += iAuthorLen + iLen;
		
		if(isMyMessage) 
		{
			SetStyleEx(wxRichTextRange(lSelBegin, lSelBegin + iLen + 1), WinUtil::m_ChatTextMyOwn, STYLE_FLAGS);
			SetStyleEx(wxRichTextRange(lSelBegin + iLen + 1, lSelEnd), WinUtil::m_TextStyleMyNick, STYLE_FLAGS);
		}
		else 
		{
			bool isFavorite = FavoriteManager::getInstance()->isFavoriteUser(i.getUser());

			if(BOOLSETTING(BOLD_AUTHOR_MESS) || isFavorite || i.isOp()) 
			{
				if(isFavorite)
				{
					SetStyleEx(wxRichTextRange(lSelBegin, lSelEnd), WinUtil::m_TextStyleFavUsers, STYLE_FLAGS);
				} 
				else if(i.isOp()) 
				{
					SetStyleEx(wxRichTextRange(lSelBegin, lSelEnd), WinUtil::m_TextStyleOPs, STYLE_FLAGS);
				} 
				else 
				{
					SetStyleEx(wxRichTextRange(lSelBegin, lSelEnd), WinUtil::m_TextStyleBold, STYLE_FLAGS);
				}
			} 
			else 
			{
				SetStyleEx(wxRichTextRange(lSelBegin, lSelEnd), attr, STYLE_FLAGS);
            }
		}
	} 
	else 
	{
		bool thirdPerson = false;
        switch(msg[0]) 
		{
			case _T('*'):
				if(msg[1] != _T(' ')) 
					break;
				thirdPerson = true;
            case _T('<'):
			{
				tstring::size_type iAuthorLen = msg.find(thirdPerson ? _T(' ') : _T('>'), thirdPerson ? 2 : 1);
				if(iAuthorLen != tstring::npos) 
				{
                    bool isOp = false, isFavorite = false;

                    if(client != NULL) 
					{
						tstring nick(msg.c_str() + 1);
						nick.erase(iAuthorLen - 1);
						
						const OnlineUserPtr ou = client->findUser(Text::fromT(nick));
						if(ou != NULL) {
							isFavorite = FavoriteManager::getInstance()->isFavoriteUser(ou->getUser());
							isOp = ou->getIdentity().isOp();
						}
                    }
                    
					lSelBegin = lSelEnd;
					lSelEnd += iAuthorLen + 1;
					msg.erase(0, iAuthorLen);

        			if(BOOLSETTING(BOLD_AUTHOR_MESS) || isFavorite || isOp) 
					{
						if(isFavorite)
						{
							SetStyleEx(wxRichTextRange(lSelBegin, lSelEnd), WinUtil::m_TextStyleFavUsers, STYLE_FLAGS);
						} 
						else if(isOp) 
						{
							SetStyleEx(wxRichTextRange(lSelBegin, lSelEnd), WinUtil::m_TextStyleOPs, STYLE_FLAGS);
						} 
						else 
						{
							SetStyleEx(wxRichTextRange(lSelBegin, lSelEnd), WinUtil::m_TextStyleBold, STYLE_FLAGS);
						}
        			} 
					else 
					{
        				SetStyleEx(wxRichTextRange(lSelBegin, lSelEnd), attr, STYLE_FLAGS);
                    }
				}
			}
        }
	}

	// Format the message part
	FormatChatLine(myNick, msg, attr, isMyMessage, author, lSelEnd, useEmoticons);

	SetSelection(lSelBeginSaved, lSelEndSaved);

	if(	isMyMessage || (((pageSize == 0) || (scrollPos >= scrollMax - pageSize - 5)) &&
		(lSelBeginSaved == lSelEndSaved || !selectedUser.empty() || !selectedIP.empty() || !selectedURL.empty())))
	{
		Thaw();
		ScrollIntoView(GetLastPosition(), WXK_END);
		Refresh();
	} 
	else
	{
		SetScrollPos(wxVERTICAL, scrollPos);
		Thaw();		
	}
}


void RichChatCtrl::FormatChatLine(const tstring& myNick, const tstring& msg, const wxRichTextAttr& attr, bool isMyMessage, const tstring& author, LONG lSelBegin, bool useEmoticons) 
{
	// Set text format
	tstring sMsgLower(msg.length(), NULL);
	std::transform(msg.begin(), msg.end(), sMsgLower.begin(), _totlower);

	long lSelEnd = lSelBegin + msg.size();
	SetStyleEx(wxRichTextRange(lSelBegin, lSelEnd), isMyMessage ? WinUtil::m_ChatTextMyOwn : attr, STYLE_FLAGS);
	
	// highlight all occurences of my nick
	long lMyNickStart = -1, lMyNickEnd = -1;
	size_t lSearchFrom = 0;	
	tstring sNick(myNick.length(), NULL);
	std::transform(myNick.begin(), myNick.end(), sNick.begin(), _totlower);

	bool found = false;
	while((lMyNickStart = (long)sMsgLower.find(sNick, lSearchFrom)) != tstring::npos) 
	{
		lMyNickEnd = lMyNickStart + (long)sNick.size();
		SetStyleEx(wxRichTextRange(lSelBegin + lMyNickStart, lSelBegin + lMyNickEnd), WinUtil::m_TextStyleMyNick, STYLE_FLAGS);
		lSearchFrom = lMyNickEnd;
		found = true;
	}
	
	if(found) 
	{
		if(	!SETTING(CHATNAMEFILE).empty() && !BOOLSETTING(SOUNDS_DISABLED) &&
			!author.empty() && (_tcsicmp(author.c_str(), sNick.c_str()) != 0)) {
				wxSound::Play(Text::toT(SETTING(CHATNAMEFILE)), wxSOUND_ASYNC);	 	
        }	
	}

	// highlight all occurences of favourite users' nicks
	FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
	for(FavoriteManager::FavoriteMap::const_iterator i = ul.begin(); i != ul.end(); ++i) 
	{
		const FavoriteUser& pUser = i->second;

		lSearchFrom = 0;
		sNick = Text::toT(pUser.getNick());
		std::transform(sNick.begin(), sNick.end(), sNick.begin(), _totlower);

		while((lMyNickStart = (long)sMsgLower.find(sNick, lSearchFrom)) != tstring::npos) 
		{
			lMyNickEnd = lMyNickStart + (long)sNick.size();
			SetStyleEx(wxRichTextRange(lSelBegin + lMyNickStart, lSelBegin + lMyNickEnd), WinUtil::m_TextStyleFavUsers, STYLE_FLAGS);
			lSearchFrom = lMyNickEnd;
		}
	}

	// Links and smilies
	FormatEmoticonsAndLinks(msg, sMsgLower, lSelBegin, useEmoticons);
}

void RichChatCtrl::FormatEmoticonsAndLinks(const tstring& sMsg, const tstring& sMsgLower, long lSelBegin, bool bUseEmo) 
{
	long lSelEnd = lSelBegin + sMsg.size();

	// hightlight all URLs and make them clickable
	for(size_t i = 0; i < (sizeof(protocols) / sizeof(protocols[0])); ++i) 
	{
		size_t linkStart = sMsgLower.find(protocols[i]);
		bool isMagnet = (protocols[i] == _T("magnet:?"));
		while(linkStart != tstring::npos) {
			size_t linkEnd = linkStart + protocols[i].size();
			
			try {
				// TODO: complete regexp for URLs
				boost::wregex reg;
				if(isMagnet) // magnet links have totally indifferent structure than classic URL // -/?%&=~#'\\w\\.\\+\\*\\(\\)
					reg.assign(_T("^(\\w)+=[:\\w]+(&(\\w)+=[\\S]*)*[^\\s<>.,;!(){}\"']+"), boost::regex_constants::icase);
				else
					reg.assign(_T("^([@\\w-]+(\\.)*)+(:[\\d]+)?(/[\\S]*)*[^\\s<>.,;!(){}\"']+"), boost::regex_constants::icase);
					
				tstring::const_iterator start = sMsg.begin();
				tstring::const_iterator end = sMsg.end();
				boost::match_results<tstring::const_iterator> result;

				if(boost::regex_search(start + linkEnd, end, result, reg, boost::match_default)) 
				{
					dcassert(!result.empty());
					
					wxRichTextAttr style = WinUtil::m_TextStyleURL;
					style.SetURL(protocols[i] + result.str(0));

					linkEnd += result.length(0);

					if(isMagnet)
					{
						// parse magnet link
						tstring fileName, fileHash;
						int64_t fileSize = 0;
						if(WinUtil::parseMagnetUri(tstring(style.GetURL()), fileHash, fileName, fileSize))
						{
							tstring replacementString = fileName + _T(" (") + Util::formatBytesW(fileSize) + _T(")");
							SetSelection(lSelBegin + linkStart, lSelBegin + linkEnd);
							Replace(lSelBegin + linkStart, lSelBegin + linkEnd, replacementString);

							// correct style end point
							linkEnd = linkEnd - result.length(0) + replacementString.size();
						}
					}
					
					SetStyleEx(wxRichTextRange(lSelBegin + linkStart, lSelBegin + linkEnd), style, STYLE_FLAGS);
				}
			} catch(...) 
			{
				// invalid expression
			}
			
			linkStart = sMsgLower.find(protocols[i], linkEnd);			
		}
	}

	// insert emoticons
	if(bUseEmo && emoticonsManager->getUseEmoticons()) 
	{
		const Emoticon::List& emoticonsList = emoticonsManager->getEmoticonsList();
		tstring::size_type lastReplace = 0;
		uint8_t smiles = 0;

		while(true) 
		{
			tstring::size_type curReplace = tstring::npos;
			Emoticon* foundEmoticon = NULL;

			for(Emoticon::Iter emoticon = emoticonsList.begin(); emoticon != emoticonsList.end(); ++emoticon) 
			{
				tstring::size_type idxFound = sMsg.find((*emoticon)->getEmoticonText(), lastReplace);
				if(idxFound < curReplace || curReplace == tstring::npos) 
				{
					curReplace = idxFound;
					foundEmoticon = (*emoticon);
				}
			}

			if(curReplace != tstring::npos && smiles < MAX_EMOTICONS) 
			{
				wxRichTextAttr cfSel;

				lSelBegin += (curReplace - lastReplace);
				lSelEnd = lSelBegin + foundEmoticon->getEmoticonText().size();

				GetStyleForRange(wxRichTextRange(lSelBegin, lSelEnd), cfSel);
				if(cfSel.GetURL().IsEmpty()) 
				{
					Delete(wxRichTextRange(lSelBegin, lSelEnd));
					WriteImage(foundEmoticon->getEmoticonBmp(cfSel.GetBackgroundColour()), wxBITMAP_TYPE_BMP);

					++smiles;
					++lSelBegin;
				} 
				else 
					lSelBegin = lSelEnd;

				lastReplace = curReplace + foundEmoticon->getEmoticonText().size();
			} 
			else 
				break;
		}
	}

}

void RichChatCtrl::OnSize(wxSizeEvent& event)
{
	ScrollIntoView(GetLastPosition(), WXK_END);
	event.Skip();
}

void RichChatCtrl::OnContextMenu(wxContextMenuEvent& event)
{
	handleSelection(ScreenToClient(event.GetPosition()));

	wxMenu menu;
	if(selectedUser.empty()) 
	{
		if(!selectedIP.empty()) 
		{
			menu.SetTitle(selectedIP);
			menu.Append(IDC_WHOIS_IP, (TSTRING(WHO_IS) + _T(" ") + selectedIP).c_str() );
			if (client && client->isOp()) 
			{
				menu.AppendSeparator();
				menu.Append(IDC_BAN_IP, (_T("!banip ") + selectedIP).c_str());
				menu.Append(IDC_UNBAN_IP, (_T("!unban ") + selectedIP).c_str());
				menu.AppendSeparator();
			}
		} 
		else 
		{
			menu.SetTitle(_T("Text"));
		}

		menu.Append(ID_EDIT_COPY, CTSTRING(COPY));
		menu.Append(IDC_COPY_ACTUAL_LINE,  CTSTRING(COPY_LINE));

		if(!selectedURL.empty()) 
  			menu.Append(IDC_COPY_URL, CTSTRING(COPY_URL));
	} 
	else 
	{
		bool isMe = (selectedUser == Text::toT(client->getMyNick()));

		// click on nick
		wxMenu* copyMenu = new wxMenu();
		copyMenu->SetTitle(TSTRING(COPY));

		for(int j=0; j < OnlineUser::COLUMN_LAST; j++) 
		{
			copyMenu->Append(IDC_COPY + j, CTSTRING_I(HubFrame::columnNames[j]));
		}

		menu.SetTitle(selectedUser);

		if(BOOLSETTING(LOG_PRIVATE_CHAT)) 
		{
			menu.Append(IDC_OPEN_USER_LOG,  CTSTRING(OPEN_USER_LOG));
			menu.AppendSeparator();
		}		

		menu.Append(IDC_SELECT_USER, CTSTRING(SELECT_USER_LIST));
		menu.AppendSeparator();
		
		if(!isMe) 
		{
			menu.Append(IDC_PUBLIC_MESSAGE, CTSTRING(SEND_PUBLIC_MESSAGE));
			menu.Append(IDC_PRIVATEMESSAGE, CTSTRING(SEND_PRIVATE_MESSAGE));
			menu.AppendSeparator();
			
			const OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
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
		
		menu.AppendSubMenu(copyMenu, CTSTRING(COPY));
		
		if(!isMe) 
		{
			// TODO menu.AppendMenu(MF_POPUP, (UINT)(HMENU)WinUtil::grantMenu, CTSTRING(GRANT_SLOTS_MENU));
			menu.AppendSeparator();
			menu.Append(IDC_GETLIST, CTSTRING(GET_FILE_LIST));
			menu.Append(IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
			menu.Append(IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
			
			// add user commands
			prepareMenu(menu, ::UserCommand::CONTEXT_CHAT, client->getHubUrl());
		}
	}

	menu.AppendSeparator();
	menu.Append(ID_EDIT_SELECT_ALL, CTSTRING(SELECT_ALL));
	menu.Append(ID_EDIT_CLEAR_ALL, CTSTRING(CLEAR));
	
	//flag to indicate pop up menu.
    // TODO m_bPopupMenu = true;
	PopupMenu(&menu);

	SelectNone();
}

void RichChatCtrl::OnLeftDown(wxMouseEvent& event)
{
	selectedLine.clear();
	selectedIP.clear();
	selectedUser.clear();
	selectedURL.clear();

	event.Skip();
}

void RichChatCtrl::OnDblClick(wxMouseEvent& event)
{
	// detect nick
	handleSelection(event.GetPosition());

	if(client != NULL && !selectedUser.empty())
	{
		OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
		if(ou->getUser() != ClientManager::getInstance()->getMe())
		{
			switch(SETTING(CHAT_DBLCLICK)) 
			{
				case 0: 
				{
					/* TODO int items = ctrlUsers->GetItemCount();
					int pos = -1;
					ctrlUsers.SetRedraw(FALSE);
					for(int i = 0; i < items; ++i) {
						if(ctrlUsers.getItemData(i) == ui)
							pos = i;
						ctrlUsers.SetItemState(i, (i == pos) ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED);
					}
					ctrlUsers.SetRedraw(TRUE);
					ctrlUsers.EnsureVisible(pos, FALSE);*/
					break;
				}    
				case 1: 
				{
					/* TODO tstring sUser = ui->getText(OnlineUser::COLUMN_NICK);
					int iSelBegin, iSelEnd;
					ctrlMessage.GetSel(iSelBegin, iSelEnd);

					if((iSelBegin == 0) && (iSelEnd == 0)) {
					sUser += _T(": ");
					if(ctrlMessage.GetWindowTextLength() == 0) {   
			            ctrlMessage.SetWindowText(sUser.c_str());
			            ctrlMessage.SetFocus();
			            ctrlMessage.SetSel(ctrlMessage.GetWindowTextLength(), ctrlMessage.GetWindowTextLength());
					} else {
			            ctrlMessage.ReplaceSel(sUser.c_str());
						ctrlMessage.SetFocus();
					}
					} else {
					    sUser += _T(" ");
					    ctrlMessage.ReplaceSel(sUser.c_str());
					    ctrlMessage.SetFocus();
					}*/
					break;
				}
				case 2:
					ou->pm(client->getHubUrl());
					break;
				case 3:
					ou->getList(client->getHubUrl());
					break;
				case 4:
					ou->matchQueue(client->getHubUrl());
					break;
				case 5:
					ou->grant(client->getHubUrl());
					break;
				case 6:
					ou->addFav();
					break;
			}
		}
	}

	event.Skip();
}

void RichChatCtrl::OnURL(wxTextUrlEvent& event)
{
	WinUtil::openLink((tstring(event.GetString().c_str())));
}

void RichChatCtrl::OnCopyActualLine(wxCommandEvent& /*event*/)
{
	if(!selectedLine.empty()) 
	{
		WinUtil::setClipboard(selectedLine);
	}	
}

void RichChatCtrl::OnEditCopy(wxCommandEvent& /*event*/)
{
	Copy();
}

void RichChatCtrl::OnEditSelectAll(wxCommandEvent& /*event*/)
{
	SelectAll();
}

void RichChatCtrl::OnEditClearAll(wxCommandEvent& /*event*/)
{
	Clear();
}

void RichChatCtrl::OnBanIP(wxCommandEvent& /*event*/)
{
	if(!selectedIP.empty()) 
	{
		tstring s = _T("!banip ") + selectedIP;
		client->hubMessage(Text::fromT(s));
	}	
}

void RichChatCtrl::OnUnBanIP(wxCommandEvent& /*event*/)
{
	if(!selectedIP.empty()) 
	{
		tstring s = _T("!unban ") + selectedIP;
		client->hubMessage(Text::fromT(s));
	}
}

void RichChatCtrl::OnCopyURL(wxCommandEvent& /*event*/)
{
	if(!selectedURL.empty()) 
	{
		WinUtil::setClipboard(selectedURL);
	}
}

void RichChatCtrl::OnWhoisIP(wxCommandEvent& /*event*/)
{
	if(!selectedIP.empty()) 
	{
 		WinUtil::openLink(_T("http://www.ripe.net/perl/whois?form_type=simple&full_query_string=&searchtext=") + selectedIP);
 	}
}

void RichChatCtrl::OnOpenUserLog(wxCommandEvent& /*event*/)
{
	OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou) 
	{
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
			wxMessageBox(CTSTRING(NO_LOG_FOR_USER), CTSTRING(NO_LOG_FOR_USER), wxOK);	  
		}
	}
}

void RichChatCtrl::OnPrivateMessage(wxCommandEvent& /*event*/)
{
	OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou)
		PrivateFrame::openWindow(HintedUser(ou->getUser(), client->getHubUrl()), Util::emptyStringT, client);
}

void RichChatCtrl::OnGetList(wxCommandEvent& /*event*/)
{
	OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou)
		ou->getList(client->getHubUrl());
}

void RichChatCtrl::OnMatchQueue(wxCommandEvent& /*event*/)
{
	OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou)
		ou->matchQueue(client->getHubUrl());
}

void RichChatCtrl::OnGrantSlot(wxCommandEvent& event)
{
	const OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou) 
	{
		uint64_t time = 0;
		switch(event.GetId()) 
		{
			case IDC_GRANTSLOT:			time = 600; break;
			case IDC_GRANTSLOT_DAY:		time = 3600; break;
			case IDC_GRANTSLOT_HOUR:	time = 24*3600; break;
			case IDC_GRANTSLOT_WEEK:	time = 7*24*3600; break;
			case IDC_UNGRANTSLOT:		time = 0; break;
		}
		
		if(time > 0)
			UploadManager::getInstance()->reserveSlot(HintedUser(ou->getUser(), client->getHubUrl()), time);
		else
			UploadManager::getInstance()->unreserveSlot(ou->getUser());
	}
}

void RichChatCtrl::OnAddToFavorites(wxCommandEvent& /*event*/)
{
	OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou)
		ou->addFav();
}

void RichChatCtrl::OnIgnore(wxCommandEvent& /*event*/)
{
	OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou)
		FavoriteManager::getInstance()->addIgnoredUser(ou->getUser());
}

void RichChatCtrl::OnUnignore(wxCommandEvent& /*event*/)
{
	OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou)
		FavoriteManager::getInstance()->removeIgnoredUser(ou->getUser());
}


void RichChatCtrl::OnCopyUserInfo(wxCommandEvent& event)
{
	tstring sCopy;
	
	const OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou) 
	{
		sCopy = ou->getText(static_cast<uint8_t>(event.GetId() - IDC_COPY));
	}

	if(!sCopy.empty())
		WinUtil::setClipboard(sCopy);
}

void RichChatCtrl::OnReport(wxCommandEvent& /*event*/)
{
	const OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
		
	if(ou) 
	{
		wxRichTextAttr attr;
		attr.SetBackgroundColour(SETTING(BACKGROUND_COLOR));
		attr.SetTextColour(SETTING(ERROR_COLOR));

		// TODO AppendText(Identity(NULL, 0), Text::toT(client->getCurrentNick()), Text::toT("[" + Util::getShortTimeString() + "] "), Text::toT(WinUtil::getReport(ou->getIdentity(), m_hWnd)) + _T('\n'), attr, false);
	}
}

void RichChatCtrl::OnCheckList(wxCommandEvent& /*event*/)
{
	const OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou) 
	{
		try 
		{
			QueueManager::getInstance()->addList(HintedUser(ou->getUser(), client->getHubUrl()), QueueItem::FLAG_USER_CHECK, client->getHubUrl());
		} 
		catch(const Exception& e) 
		{
			LogManager::getInstance()->message(e.getError(), LogManager::LOG_ERROR);		
		}
	}
}

void RichChatCtrl::handleSelection(const wxPoint& pt)
{
	selectedUser.clear();
	selectedIP.clear();
	selectedURL.clear();

	long charPos, x = 0, y = 0;
	HitTest(pt, &charPos);
	PositionToXY(charPos, &x, &y);

	selectedLine = GetLineText(y);

	long start = x;
	while(start > 0 && selectedLine[start - 1] != _T(' ') && selectedLine[start - 1] != _T('\t'))
		start--;

	long end = x;
	while(end < selectedLine.Length() && selectedLine[end] != _T(' ') && selectedLine[end] != _T('\t'))
		end++;
	
	wxString selectedWord;
	selectedWord = selectedLine.substr(start);
	selectedWord.resize(end - start);

	long wordPos = XYToPosition(start, y);

	if(GetSelectionRange().GetLength() <= 1) // select word only when there's no selected text
	{
		SetSelection(wordPos, wordPos + selectedWord.Length());
		SetCaretPosition(wordPos - 1);
		Refresh();
	}

	// is selected word URL?
	wxRichTextAttr selection;
    GetStyleForRange(wxRichTextRange(wordPos, wordPos + selectedWord.Length()), selection);

	if((selection.GetFlags() & wxTEXT_ATTR_URL) == wxTEXT_ATTR_URL)
	{
		selectedURL = selection.GetURL();
		return;
	}

	// is selected word IP address?
	const boost::wregex e(_T("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}"));	// FIXME: rewrite to something better
	if(boost::regex_match((const TCHAR*)selectedWord.c_str(), e))
	{
		selectedIP = selectedWord;
		return;
	}

	// is selected word user's nick?
	if(client == NULL)
		return;

	if(selectedWord.StartsWith(wxT("<")) && (selectedWord.EndsWith(wxT(">")) || selectedWord.EndsWith(wxT(":"))))
	{
		selectedWord = selectedWord.SubString(1, selectedWord.Length() - 2);
	}

	if(client->findUser(Text::fromT((const TCHAR*)selectedWord.c_str())) != NULL)
	{
		selectedUser = selectedWord;
		return;
	}
}

void RichChatCtrl::runUserCommand(const UserCommand& uc) 
{
	StringMap ucParams;

	if(!WinUtil::getUCParams(this, uc, ucParams))
		return;

	client->getMyIdentity().getParams(ucParams, "my", true);
	client->getHubIdentity().getParams(ucParams, "hub", false);

	const OnlineUserPtr ou = client->findUser(Text::fromT(selectedUser));
	if(ou != NULL) {
		StringMap tmp = ucParams;
		ou->getIdentity().getParams(tmp, "user", true);
		client->escapeParams(tmp);
		client->sendUserCmd(uc, tmp);
	}
}
