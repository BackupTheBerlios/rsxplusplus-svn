/* 
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
#include "Resource.h"
#include "../client/DCPlusPlus.h"
#include "../client/FavoriteManager.h"
#include "../client/UploadManager.h"

#include "ChatCtrl.h"
#include "AGEmotionSetup.h"
#include "PrivateFrame.h"
//RSX++
#include "MainFrm.h"
//END

EmoticonSetup* g_pEmotionsSetup = NULL;

#define MAX_EMOTICONS 48

static const tstring protocols[] = { _T("http://"), _T("https://"), _T("www."), _T("ftp://"), 
	_T("magnet:?"), _T("dchub://"), _T("irc://"), _T("ed2k://"), _T("mms://"), _T("file://"),
	_T("adc://"), _T("adcs://"), _T("nmdcs://"), _T("svn://") };

ChatCtrl::ChatCtrl() : ccw(_T("edit"), this), client(NULL), m_bPopupMenu(false) {
	if(g_pEmotionsSetup == NULL) {
		g_pEmotionsSetup = new EmoticonSetup();
	}
	
	g_pEmotionsSetup->inc();
}

ChatCtrl::~ChatCtrl() {
	if(g_pEmotionsSetup->unique()) {
		g_pEmotionsSetup->dec();
		g_pEmotionsSetup = NULL;
	} else {
		g_pEmotionsSetup->dec();
	}
}
	
void ChatCtrl::AppendText(const Identity& i, const tstring& sMyNick, const tstring& sTime, tstring sMsg, CHARFORMAT2& cf, bool bUseEmo/* = true*/, bool useHL/* = true*/, tstring chatExtraInfo) {
	SetRedraw(FALSE);

	SCROLLINFO si = { 0 };
	POINT pt = { 0 };

	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	GetScrollInfo(SB_VERT, &si);
	GetScrollPos(&pt);

	LONG lSelBegin = 0, lSelEnd = 0, lTextLimit = 0, lNewTextLen = 0;
	LONG lSelBeginSaved, lSelEndSaved;

	// Unify line endings
	tstring::size_type j = 0; 
	while((j = sMsg.find(_T("\r"), j)) != tstring::npos)
		sMsg.erase(j, 1);

	GetSel(lSelBeginSaved, lSelEndSaved);
	lSelEnd = lSelBegin = GetTextLengthEx(GTL_NUMCHARS);

	bool isMyMessage = i.getUser() == ClientManager::getInstance()->getMe();
	bool isRealUser = i.getUser().get() != 0 && !i.isBot() && !i.isHub() && !i.isHidden(); //RSX++
	tstring sLine = sTime;
	if(isRealUser && chatExtraInfo.size() > 0) {
		chatExtraInfo += _T(" ");
		sLine += chatExtraInfo;
	}
	sLine += sMsg;

	// Remove old chat if size exceeds
	lNewTextLen = sLine.size();
	lTextLimit = GetLimitText();

	if(lSelEnd + lNewTextLen > lTextLimit) {
		LONG lRemoveChars = 0;
		int multiplier = 1;

		if(lNewTextLen >= lTextLimit) {
			lRemoveChars = lSelEnd;
		} else {
			while(lRemoveChars < lNewTextLen)
				lRemoveChars = LineIndex(LineFromChar(multiplier++ * lTextLimit / 10));
		}

		// Update selection ranges
		lSelEnd = lSelBegin -= lRemoveChars;
		lSelEndSaved -= lRemoveChars;
		lSelBeginSaved -= lRemoveChars;

		// ...and the scroll position
		pt.y -= PosFromChar(lRemoveChars).y;

		SetSel(0, lRemoveChars);
		ReplaceSel(_T(""));
	}

	// Add to the end
	SetSel(lSelBegin, lSelEnd);
	setText(sLine);

	// Format TimeStamp
	if(!sTime.empty()) {
		lSelEnd += sTime.size();
		SetSel(lSelBegin, lSelEnd - 1);
		SetSelectionCharFormat(WinUtil::m_TextStyleTimestamp);

		PARAFORMAT2 pf;
		memzero(&pf, sizeof(PARAFORMAT2));
		pf.dwMask = PFM_STARTINDENT; 
		pf.dxStartIndent = 0;
		SetParaFormat(pf);
	}
	//RSX++
	if(isRealUser && chatExtraInfo.size() > 1) {
		lSelBegin = lSelEnd;
		lSelEnd += chatExtraInfo.size();
		SetSel(lSelBegin, lSelEnd - 1);
		SetSelectionCharFormat(isMyMessage ? WinUtil::m_ChatTextMyOwn : WinUtil::m_ChatTextGeneral);

		PARAFORMAT2 pf;
		memzero(&pf, sizeof(PARAFORMAT2));
		pf.dwMask = PFM_STARTINDENT; 
		pf.dxStartIndent = 0;
		SetParaFormat(pf);
	}
	//END

	// Authors nick
	tstring sAuthor = Text::toT(i.getNick());
	if(!sAuthor.empty()) {
		LONG iLen = (sMsg[0] == _T('*')) ? 1 : 0;
		LONG iAuthorLen = sAuthor.size() + 1;
		sMsg.erase(0, iAuthorLen + iLen);
   		
		lSelBegin = lSelEnd;
		lSelEnd += iAuthorLen + iLen;
		
		if(isMyMessage) {
			SetSel(lSelBegin, lSelBegin + iLen + 1);
			SetSelectionCharFormat(WinUtil::m_ChatTextMyOwn);
			SetSel(lSelBegin + iLen + 1, lSelBegin + iLen + iAuthorLen);
			SetSelectionCharFormat(WinUtil::m_TextStyleMyNick);
		} else {
			bool isFavorite = FavoriteManager::getInstance()->isFavoriteUser(i.getUser());
			bool isProtected = i.isSet("PR"); //RSX++
			if(BOOLSETTING(BOLD_AUTHOR_MESS) || isFavorite || i.isOp() || isProtected) {
				SetSel(lSelBegin, lSelBegin + iLen + 1);
				SetSelectionCharFormat(cf);
				SetSel(lSelBegin + iLen + 1, lSelEnd);
				if(isFavorite){
					SetSelectionCharFormat(WinUtil::m_TextStyleFavUsers);
				} else if(i.isOp()) {
					SetSelectionCharFormat(WinUtil::m_TextStyleOPs);
				//RSX++
				} else if(isProtected) {
					SetSelectionCharFormat(WinUtil::m_TextStyleProtected); //RSX++
				//END
				} else {
					SetSelectionCharFormat(WinUtil::m_TextStyleBold);
				}
			} else {
				SetSel(lSelBegin, lSelEnd);
				SetSelectionCharFormat(cf);
            }
		}
	} else {
		bool thirdPerson = false;
        switch(sMsg[0]) {
			case _T('*'):
				if(sMsg[1] != _T(' ')) break;
				thirdPerson = true;
            case _T('<'):
				tstring::size_type iAuthorLen = sMsg.find(thirdPerson ? _T(' ') : _T('>'), thirdPerson ? 2 : 1);
				if(iAuthorLen != tstring::npos) {
                    bool isOp = false, isFavorite = false;

                    if(client != NULL) {
						tstring nick(sMsg.c_str() + 1);
						nick.erase(iAuthorLen - 1);
						
						const OnlineUserPtr ou = client->findUser(Text::fromT(nick));
						if(ou != NULL) {
							isFavorite = FavoriteManager::getInstance()->isFavoriteUser(ou->getUser());
							isOp = ou->getIdentity().isOp();
						}
                    }
                    
					lSelBegin = lSelEnd;
					lSelEnd += iAuthorLen;
					sMsg.erase(0, iAuthorLen);

        			if(BOOLSETTING(BOLD_AUTHOR_MESS) || isFavorite || isOp) {
        				SetSel(lSelBegin, lSelBegin + 1);
        				SetSelectionCharFormat(cf);
						SetSel(lSelBegin + 1, lSelEnd);
						if(isFavorite){
							SetSelectionCharFormat(WinUtil::m_TextStyleFavUsers);
						} else if(isOp) {
							SetSelectionCharFormat(WinUtil::m_TextStyleOPs);
						} else {
							SetSelectionCharFormat(WinUtil::m_TextStyleBold);
						}
        			} else {
        				SetSel(lSelBegin, lSelEnd);
        				SetSelectionCharFormat(cf);
                    }
				}
        }
	}

	// Format the message part
	FormatChatLine(sMyNick, sMsg, cf, isMyMessage, sAuthor, lSelEnd, bUseEmo);

	//RSX++ // Highlights
	tstring sMsgLower = Text::toLower(sMsg);
	lSelBegin = lSelEnd;

	if(client && useHL) {
		if(client->getUseHL() && RSXPP_BOOLSETTING(USE_HIGHLIGHT)) {
			tstring textToMatch = Util::emptyStringT;
			bool matched = false;
			CHARFORMAT2 hlcf;

			const HighLight::List& hll = FavoriteManager::getInstance()->getHLs();
			for(HighLight::List::const_iterator i = hll.begin(); i != hll.end(); ++i) {
				textToMatch = Text::toT((*i)->getHstring());
				try {
					const boost::wregex reg(Text::toT((*i)->getHstring()));

					memzero(&hlcf, sizeof(CHARFORMAT2));
					hlcf.cbSize = sizeof(hlcf);
					hlcf.dwMask = CFM_BACKCOLOR | CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
					hlcf.crBackColor = (*i)->getHasBgColor() ? (*i)->getBackColor() : SETTING(TEXT_GENERAL_BACK_COLOR);
					hlcf.crTextColor = (*i)->getHasFontColor() ? (*i)->getFontColor() : SETTING(TEXT_GENERAL_FORE_COLOR);
					if((*i)->getBoldFont())
						hlcf.dwEffects |= CFE_BOLD;
					if((*i)->getItalicFont())
						hlcf.dwEffects |= CFE_ITALIC;
					if((*i)->getUnderlineFont())
						hlcf.dwEffects |= CFM_UNDERLINE;
					if((*i)->getStrikeoutFont())
						 hlcf.dwEffects |= CFM_STRIKEOUT;

					boost::wsregex_iterator iter(sMsgLower.begin(), sMsgLower.end(), reg);
					boost::wsregex_iterator enditer;
					for(; iter != enditer; ++iter) {
						SetSel(lSelBegin + iter->position(), lSelBegin + iter->position() + iter->length());
						SetSelectionCharFormat(hlcf);
						struct hlAction curAction = {
							sMsgLower.substr(iter->position(), iter->length()),
							(*i)->getDisplayPopup(), 
							(*i)->getFlashWindow(), 
							(*i)->getPlaySound(), 
							(*i)->getSoundFilePath()
						};
						actions.push_back(curAction);
						matched = true;
					}
				} catch(...) {
					//...
				}
			}
			//avoid spam, show popup after scan all msg
			if(matched) {
				handleActions(sAuthor);
			}
		}
	}
	//END

	SetSel(lSelBeginSaved, lSelEndSaved);
	if(	isMyMessage || ((si.nPage == 0 || (size_t)si.nPos >= (size_t)si.nMax - si.nPage - 5) &&
		(lSelBeginSaved == lSelEndSaved || !sSelectedUser.empty() || !sSelectedIP.empty() || !sSelectedURL.empty())))
	{
		PostMessage(EM_SCROLL, SB_BOTTOM, 0);
	} else {
		SetScrollPos(&pt);
	}

	// Force window to redraw
	SetRedraw(TRUE);
	InvalidateRect(NULL);
}

void ChatCtrl::FormatChatLine(const tstring& sMyNick, const tstring& sText, CHARFORMAT2& cf, bool isMyMessage, const tstring& sAuthor, LONG lSelBegin, bool bUseEmo) {
	// Set text format
	tstring sMsgLower(sText.length(), NULL);
	std::transform(sText.begin(), sText.end(), sMsgLower.begin(), _totlower);

	LONG lSelEnd = lSelBegin + sText.size();
	SetSel(lSelBegin, lSelEnd);
	SetSelectionCharFormat(isMyMessage ? WinUtil::m_ChatTextMyOwn : cf);
	
	// highlight all occurences of my nick
	long lMyNickStart = -1, lMyNickEnd = -1;
	size_t lSearchFrom = 0;	
	tstring sNick(sMyNick.length(), NULL);
	std::transform(sMyNick.begin(), sMyNick.end(), sNick.begin(), _totlower);

	bool found = false;
	while((lMyNickStart = (long)sMsgLower.find(sNick, lSearchFrom)) != tstring::npos) {
		lMyNickEnd = lMyNickStart + (long)sNick.size();
		SetSel(lSelBegin + lMyNickStart, lSelBegin + lMyNickEnd);
		SetSelectionCharFormat(WinUtil::m_TextStyleMyNick);
		lSearchFrom = lMyNickEnd;
		found = true;
	}
	
	if(found) {
		if(	!SETTING(CHATNAMEFILE).empty() && !BOOLSETTING(SOUNDS_DISABLED) &&
			!sAuthor.empty() && (stricmp(sAuthor.c_str(), sNick) != 0)) {
				::PlaySound(Text::toT(SETTING(CHATNAMEFILE)).c_str(), NULL, SND_FILENAME | SND_ASYNC);	 	
        }	
	}

	// highlight all occurences of favourite users' nicks
	FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
	for(FavoriteManager::FavoriteMap::const_iterator i = ul.begin(); i != ul.end(); ++i) {
		const FavoriteUser& pUser = i->second;

		lSearchFrom = 0;
		sNick = Text::toT(pUser.getNick());
		std::transform(sNick.begin(), sNick.end(), sNick.begin(), _totlower);

		while((lMyNickStart = (long)sMsgLower.find(sNick, lSearchFrom)) != tstring::npos) {
			lMyNickEnd = lMyNickStart + (long)sNick.size();
			SetSel(lSelBegin + lMyNickStart, lSelBegin + lMyNickEnd);
			SetSelectionCharFormat(WinUtil::m_TextStyleFavUsers);
			lSearchFrom = lMyNickEnd;
		}
	}

	// Links and smilies
	FormatEmoticonsAndLinks(sText, sMsgLower, lSelBegin, bUseEmo);
}

void ChatCtrl::FormatEmoticonsAndLinks(const tstring& sMsg, const tstring& sMsgLower, LONG lSelBegin, bool bUseEmo) {
	LONG lSelEnd = lSelBegin + sMsg.size();

	// hightlight all URLs and make them clickable
	for(size_t i = 0; i < (sizeof(protocols) / sizeof(protocols[0])); ++i) {
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

				if(boost::regex_search(start + linkEnd, end, result, reg, boost::match_default)) {
					dcassert(!result.empty());
					
					linkEnd += result.length(0);
					SetSel(lSelBegin + linkStart, lSelBegin + linkEnd);
					SetSelectionCharFormat(WinUtil::m_TextStyleURL);
				}
			} catch(...) {
			}
			
			linkStart = sMsgLower.find(protocols[i], linkEnd);			
		}
	}

	// insert emoticons
	if(bUseEmo && g_pEmotionsSetup->getUseEmoticons()) {
		const Emoticon::List& emoticonsList = g_pEmotionsSetup->getEmoticonsList();
		tstring::size_type lastReplace = 0;
		uint8_t smiles = 0;

		while(true) {
			tstring::size_type curReplace = tstring::npos;
			Emoticon* foundEmoticon = NULL;

			for(Emoticon::Iter emoticon = emoticonsList.begin(); emoticon != emoticonsList.end(); ++emoticon) {
				tstring::size_type idxFound = sMsg.find((*emoticon)->getEmotionText(), lastReplace);
				if(idxFound < curReplace || curReplace == tstring::npos) {
					curReplace = idxFound;
					foundEmoticon = (*emoticon);
				}
			}

			if(curReplace != tstring::npos && smiles < MAX_EMOTICONS) {
				CHARFORMAT2 cfSel;
				cfSel.cbSize = sizeof(cfSel);

				lSelBegin += (curReplace - lastReplace);
				lSelEnd = lSelBegin + foundEmoticon->getEmotionText().size();
				SetSel(lSelBegin, lSelEnd);

				GetSelectionCharFormat(cfSel);
				if(!(cfSel.dwEffects & CFE_LINK)) {
					CImageDataObject::InsertBitmap(GetOleInterface(), 
						foundEmoticon->getEmotionBmp(cfSel.crBackColor));

					++smiles;
					++lSelBegin;
				} else lSelBegin = lSelEnd;
				lastReplace = curReplace + foundEmoticon->getEmotionText().size();
			} else break;
		}
	}

}

bool ChatCtrl::HitNick(const POINT& p, tstring& sNick, int& iBegin, int& iEnd) {
	if(client == NULL) return false;
	
	int iCharPos = CharFromPos(p), line = LineFromChar(iCharPos), len = LineLength(iCharPos) + 1;
	long lSelBegin = 0, lSelEnd = 0;
	if(len < 3)
		return false;

	// Metoda FindWordBreak nestaci, protoze v nicku mohou byt znaky povazovane za konec slova
	int iFindBegin = LineIndex(line), iEnd1 = LineIndex(line) + LineLength(iCharPos);

	for(lSelBegin = iCharPos; lSelBegin >= iFindBegin; lSelBegin--) {
		if(FindWordBreak(WB_ISDELIMITER, lSelBegin))
			break;
	}
	lSelBegin++;
	for(lSelEnd = iCharPos; lSelEnd < iEnd1; lSelEnd++) {
		if(FindWordBreak(WB_ISDELIMITER, lSelEnd))
			break;
	}

	len = lSelEnd - lSelBegin;
	if(len <= 0)
		return false;

	tstring sText;
	sText.resize(len);

	GetTextRange(lSelBegin, lSelEnd, &sText[0]);

	size_t iLeft = 0, iRight = 0, iCRLF = sText.size(), iPos = sText.find(_T('<'));
	if(iPos != tstring::npos) {
		iLeft = iPos + 1;
		iPos = sText.find(_T('>'), iLeft);
		if(iPos == tstring::npos) 
			return false;

		iRight = iPos - 1;
		iCRLF = iRight - iLeft + 1;
	} else {
		iLeft = 0;
	}

	tstring sN = sText.substr(iLeft, iCRLF);
	if(sN.size() == 0)
		return false;

	if(client->findUser(Text::fromT(sN)) != NULL) {
		sNick = sN;
		iBegin = lSelBegin + iLeft;
		iEnd = lSelBegin + iLeft + iCRLF;
		return true;
	}
    
	// Jeste pokus odmazat eventualni koncovou ':' nebo '>' 
	// Nebo pro obecnost posledni znak 
	// A taky prvni znak 
	// A pak prvni i posledni :-)
	if(iCRLF > 1) {
		sN = sText.substr(iLeft, iCRLF - 1);
		if(client->findUser(Text::fromT(sN)) != NULL) {
			sNick = sN;
   			iBegin = lSelBegin + iLeft;
   			iEnd = lSelBegin + iLeft + iCRLF - 1;
			return true;
		}

		sN = sText.substr(iLeft + 1, iCRLF - 1);
		if(client->findUser(Text::fromT(sN)) != NULL) {
        	sNick = sN;
			iBegin = lSelBegin + iLeft + 1;
			iEnd = lSelBegin + iLeft + iCRLF;
			return true;
		}

		sN = sText.substr(iLeft + 1, iCRLF - 2);
		if(client->findUser(Text::fromT(sN)) != NULL) {
			sNick = sN;
   			iBegin = lSelBegin + iLeft + 1;
			iEnd = lSelBegin + iLeft + iCRLF - 1;
			return true;
		}
	}	
	return false;
}

bool ChatCtrl::HitIP(const POINT& p, tstring& sIP, int& iBegin, int& iEnd) {
	int iCharPos = CharFromPos(p), len = LineLength(iCharPos) + 1;
	if(len < 3)
		return false;

	DWORD lPosBegin = FindWordBreak(WB_LEFT, iCharPos);
	DWORD lPosEnd = FindWordBreak(WB_RIGHTBREAK, iCharPos);
	len = lPosEnd - lPosBegin;

	tstring sText;
	sText.resize(len);
	GetTextRange(lPosBegin, lPosEnd, &sText[0]);

	for(int i = 0; i < len; i++) {
		if(!((sText[i] == 0) || (sText[i] == '.') || ((sText[i] >= '0') && (sText[i] <= '9')))) {
			return false;
		}
	}

	sText += _T('.');
	size_t iFindBegin = 0, iPos = tstring::npos, iEnd2 = 0;
	bool boOK = true;
	for(int i = 0; i < 4; i++) {
		iPos = sText.find(_T('.'), iFindBegin);
		if(iPos == tstring::npos) {
			boOK = false;
			break;
		}
		iEnd2 = atoi(Text::fromT(sText.substr(iFindBegin)).c_str());
		if((iEnd2 < 0) || (iEnd2 > 255)) {
			boOK = false;
			break;
		}
		iFindBegin = iPos + 1;
	}

	if(boOK) {
		sIP = sText.substr(0, iPos);
		iBegin = lPosBegin;
		iEnd = lPosEnd;
	}
	return boOK;
}

bool ChatCtrl::HitURL() {
	long lSelBegin = 0, lSelEnd = 0;
	GetSel(lSelBegin, lSelEnd);
	bool boOK = false;

	CHARFORMAT2 cfSel;
	cfSel.cbSize = sizeof(cfSel);
    
	GetSelectionCharFormat(cfSel);
	if(cfSel.dwEffects & CFE_LINK) {
		boOK = true;
	}
	return boOK;
}

tstring ChatCtrl::LineFromPos(const POINT& p) const {
	int iCharPos = CharFromPos(p), line = LineFromChar(iCharPos), len = LineLength(iCharPos);
	if(len < 3) {
		return Util::emptyStringT;
	}

	tstring tmp;
	tmp.resize(len);

	GetLine(line, &tmp[0], len);

	return tmp;
}

LRESULT ChatCtrl::OnRButtonDown(POINT pt) {
	long lSelBegin = 0, lSelEnd = 0; tstring sSel;

	sSelectedLine = LineFromPos(pt);
	sSelectedUser = Util::emptyStringT;
	sSelectedIP = Util::emptyStringT;

	// Po kliku dovnitr oznaceneho textu si zkusime poznamenat pripadnej nick ci ip...
	// jinak by nam to neuznalo napriklad druhej klik na uz oznaceny nick =)
	GetSel(lSelBegin, lSelEnd);
	int iCharPos = CharFromPos(pt), iBegin = 0, iEnd = 0;
	if((lSelEnd > lSelBegin) && (iCharPos >= lSelBegin) && (iCharPos <= lSelEnd)) {
		if(HitIP(pt, sSel, iBegin, iEnd)) {
			sSelectedIP = sSel;
		} else if(HitNick(pt, sSel, iBegin, iEnd)) {
			sSelectedUser = sSel;
		}
		return 1;
	}

	// Po kliku do IP oznacit IP
	if(HitIP(pt, sSel, iBegin, iEnd)) {
		sSelectedIP = sSel;
		SetSel(iBegin, iEnd);
		InvalidateRect(NULL);
	// Po kliku na Nick oznacit Nick
	} else if(HitNick(pt, sSel, iBegin, iEnd)) {
		sSelectedUser = sSel;
		SetSel(iBegin, iEnd);
		InvalidateRect(NULL);
	}
	return 1;
}

LRESULT ChatCtrl::onExitMenuLoop(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	m_bPopupMenu = false;
	bHandled = FALSE;
	return 0;
}

LRESULT ChatCtrl::onSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
    if(m_bPopupMenu)
    {
        SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW))) ;
		return 1;
    }
    bHandled = FALSE;
	return 0;
}

LRESULT ChatCtrl::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click

	if(pt.x == -1 && pt.y == -1) {
		CRect erc;
		GetRect(&erc);
		pt.x = erc.Width() / 2;
		pt.y = erc.Height() / 2;
		ClientToScreen(&pt);
	}

	POINT ptCl = pt;
	ScreenToClient(&ptCl); 
	OnRButtonDown(ptCl);

	bool boHitURL = HitURL();
	if (!boHitURL)
		sSelectedURL = Util::emptyStringT;

	OMenu menu;
	menu.CreatePopupMenu();

	if (copyMenu.m_hMenu != NULL) {
		// delete copy menu if it exists
		copyMenu.DestroyMenu();
		copyMenu.m_hMenu = NULL;
	}

	if(sSelectedUser.empty()) {

		if(!sSelectedIP.empty()) {
			menu.InsertSeparatorFirst(sSelectedIP);
			menu.AppendMenu(MF_STRING, IDC_WHOIS_IP, (TSTRING(WHO_IS) + _T(" ") + sSelectedIP).c_str() );
			if (client && client->isOp()) {
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, IDC_BAN_IP, (_T("!banip ") + sSelectedIP).c_str());
				menu.SetMenuDefaultItem(IDC_BAN_IP);
				menu.AppendMenu(MF_STRING, IDC_UNBAN_IP, (_T("!unban ") + sSelectedIP).c_str());
				menu.AppendMenu(MF_SEPARATOR);
			}
		} else {
			menu.InsertSeparatorFirst(_T("Text"));
		}

		menu.AppendMenu(MF_STRING, ID_EDIT_COPY, CTSTRING(COPY));
		menu.AppendMenu(MF_STRING, IDC_COPY_ACTUAL_LINE,  CTSTRING(COPY_LINE));

		if(!sSelectedURL.empty()) 
  			menu.AppendMenu(MF_STRING, IDC_COPY_URL, CTSTRING(COPY_URL));
	} else {
		bool isMe = (sSelectedUser == Text::toT(client->getMyNick()));

		// click on nick
		copyMenu.CreatePopupMenu();
		copyMenu.InsertSeparatorFirst(TSTRING(COPY));

		for(int j=0; j < OnlineUser::COLUMN_LAST; j++) {
			copyMenu.AppendMenu(MF_STRING, IDC_COPY + j, CTSTRING_I(HubFrame::columnNames[j]));
		}

		menu.InsertSeparatorFirst(sSelectedUser);

		if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
			menu.AppendMenu(MF_STRING, IDC_OPEN_USER_LOG,  CTSTRING(OPEN_USER_LOG));
			menu.AppendMenu(MF_SEPARATOR);
		}		

		menu.AppendMenu(MF_STRING, IDC_SELECT_USER, CTSTRING(SELECT_USER_LIST));
		menu.AppendMenu(MF_SEPARATOR);
		
		if(!isMe) {
			menu.AppendMenu(MF_STRING, IDC_PUBLIC_MESSAGE, CTSTRING(SEND_PUBLIC_MESSAGE));
			menu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CTSTRING(SEND_PRIVATE_MESSAGE));
			menu.AppendMenu(MF_SEPARATOR);
			
			const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
			if(!FavoriteManager::getInstance()->isIgnoredUser(ou->getUser()->getCID())) {
				menu.AppendMenu(MF_STRING, IDC_IGNORE, CTSTRING(IGNORE_USER));
			} else {    
				menu.AppendMenu(MF_STRING, IDC_UNIGNORE, CTSTRING(UNIGNORE_USER));
			}
			menu.AppendMenu(MF_SEPARATOR);
		}
		
		menu.AppendMenu(MF_POPUP, (UINT)(HMENU)copyMenu, CTSTRING(COPY));
		
		if(!isMe) {
			menu.AppendMenu(MF_POPUP, (UINT)(HMENU)WinUtil::grantMenu, CTSTRING(GRANT_SLOTS_MENU));
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
			menu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
			menu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
			
			// add user commands
			prepareMenu(menu, ::UserCommand::CONTEXT_CHAT, client->getHubUrl());
		}

		// default doubleclick action
		switch(SETTING(CHAT_DBLCLICK)) {
        case 0:
			menu.SetMenuDefaultItem(IDC_SELECT_USER);
			break;
        case 1:
			menu.SetMenuDefaultItem(IDC_PUBLIC_MESSAGE);
			break;
        case 2:
			menu.SetMenuDefaultItem(IDC_PRIVATEMESSAGE);
			break;
        case 3:
			menu.SetMenuDefaultItem(IDC_GETLIST);
			break;
        case 4:
			menu.SetMenuDefaultItem(IDC_MATCH_QUEUE);
			break;
        case 6:
			menu.SetMenuDefaultItem(IDC_ADD_TO_FAVORITES);
			break;
		} 
	}

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_EDIT_SELECT_ALL, CTSTRING(SELECT_ALL));
	menu.AppendMenu(MF_STRING, ID_EDIT_CLEAR_ALL, CTSTRING(CLEAR));
	
	//flag to indicate pop up menu.
    m_bPopupMenu = true;
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

	return 0;
}

LRESULT ChatCtrl::onSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if(wParam != SIZE_MINIMIZED && HIWORD(lParam) > 0) {
		scrollToEnd();
	}

	bHandled = FALSE;
	return 0;
}

LRESULT ChatCtrl::onLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	sSelectedLine = Util::emptyStringT;
	sSelectedIP = Util::emptyStringT;
	sSelectedUser = Util::emptyStringT;
	sSelectedURL = Util::emptyStringT;

	bHandled = FALSE;
	return 0;
}

LRESULT ChatCtrl::onClientEnLink(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	ENLINK* pEL = (ENLINK*)pnmh;

	if ( pEL->msg == WM_LBUTTONUP ) {
		long lBegin = pEL->chrg.cpMin, lEnd = pEL->chrg.cpMax;
		TCHAR* sURLTemp = new TCHAR[(lEnd - lBegin)+1];
		if(sURLTemp) {
			GetTextRange(lBegin, lEnd, sURLTemp);
			tstring sURL = sURLTemp;

			WinUtil::openLink(sURL);

			delete[] sURLTemp;
		}
	} else if(pEL->msg == WM_RBUTTONUP) {
		sSelectedURL = Util::emptyStringT;
		long lBegin = pEL->chrg.cpMin, lEnd = pEL->chrg.cpMax;
		TCHAR* sURLTemp = new TCHAR[(lEnd - lBegin)+1];
		if(sURLTemp) {
			GetTextRange(lBegin, lEnd, sURLTemp);
			sSelectedURL = sURLTemp;
			delete[] sURLTemp;
		}

		SetSel(lBegin, lEnd);
		InvalidateRect(NULL);
	}

	return 0;
}

LRESULT ChatCtrl::onEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	Copy();
	return 0;
}

LRESULT ChatCtrl::onEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	SetSelAll();
	return 0;
}

LRESULT ChatCtrl::onEditClearAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	SetWindowText(Util::emptyStringT.c_str());
	return 0;
}

LRESULT ChatCtrl::onCopyActualLine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!sSelectedLine.empty()) {
		WinUtil::setClipboard(sSelectedLine);
	}
	return 0;
}

LRESULT ChatCtrl::onBanIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!sSelectedIP.empty()) {
		tstring s = _T("!banip ") + sSelectedIP + _T(" ");
		client->hubMessage(Text::fromT(s));
	}
	return 0;
}

LRESULT ChatCtrl::onUnBanIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!sSelectedIP.empty()) {
		tstring s = _T("!unban ") + sSelectedIP + _T(" ");
		client->hubMessage(Text::fromT(s));
	}
	return 0;
}

LRESULT ChatCtrl::onCopyURL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!sSelectedURL.empty()) {
		WinUtil::setClipboard(sSelectedURL);
	}
	return 0;
}

LRESULT ChatCtrl::onWhoisIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!sSelectedIP.empty()) {
 		WinUtil::openLink(_T("http://www.ripe.net/perl/whois?form_type=simple&full_query_string=&searchtext=") + sSelectedIP);
 	}
	return 0;
}

LRESULT ChatCtrl::onOpenUserLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou) {
		StringMap params;

		params["userNI"] = ou->getIdentity().getNick();
		params["hubNI"] = client->getHubName();
		params["myNI"] = client->getMyNick();
		params["userCID"] = ou->getUser()->getCID().toBase32();
		params["hubURL"] = client->getHubUrl();

		tstring file = Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params, false)));
		if(Util::fileExists(Text::fromT(file))) {
			ShellExecute(NULL, NULL, file.c_str(), NULL, NULL, SW_SHOWNORMAL);
		} else {
			MessageBox(CTSTRING(NO_LOG_FOR_USER),CTSTRING(NO_LOG_FOR_USER), MB_OK );	  
		}
	}

	return 0;
}

LRESULT ChatCtrl::onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		PrivateFrame::openWindow(HintedUser(ou->getUser(), client->getHubUrl()), Util::emptyStringT, client);

	return 0;
}

LRESULT ChatCtrl::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		ou->getList(client->getHubUrl());

	return 0;
}

LRESULT ChatCtrl::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		ou->matchQueue(client->getHubUrl());

	return 0;
}

LRESULT ChatCtrl::onGrantSlot(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou) {
		uint64_t time = 0;
		switch(wID) {
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

	return 0;
}

LRESULT ChatCtrl::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		ou->addFav();

	return 0;
}

LRESULT ChatCtrl::onIgnore(UINT /*uMsg*/, WPARAM /*wParam*/, HWND /*lParam*/, BOOL& /*bHandled*/){
	OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		FavoriteManager::getInstance()->addIgnoredUser(ou->getUser()->getCID());

	return 0;
}

LRESULT ChatCtrl::onUnignore(UINT /*uMsg*/, WPARAM /*wParam*/, HWND /*lParam*/, BOOL& /*bHandled*/){
	OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		FavoriteManager::getInstance()->removeIgnoredUser(ou->getUser()->getCID());

	return 0;
}

LRESULT ChatCtrl::onCopyUserInfo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring sCopy;
	
	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou) {
		sCopy = ou->getText(static_cast<uint8_t>(wID - IDC_COPY));
	}

	if (!sCopy.empty())
		WinUtil::setClipboard(sCopy);

	return 0;
}

LRESULT ChatCtrl::onReport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));

	if(ou) {
		CHARFORMAT2 cf;
		memzero(&cf, sizeof(CHARFORMAT2));
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_BACKCOLOR | CFM_COLOR | CFM_BOLD;
		cf.crBackColor = SETTING(BACKGROUND_COLOR);
		cf.crTextColor = SETTING(ERROR_COLOR);

		AppendText(Identity(NULL, 0), Text::toT(client->getCurrentNick()), Text::toT("[" + Util::getShortTimeString() + "] "), Text::toT(WinUtil::getReport(ou->getIdentity(), m_hWnd)) + _T('\n'), cf, false);
	}
	return 0;
}

LRESULT ChatCtrl::onGetUserResponses(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou) {
		try {
			string fname = QueueManager::getInstance()->addClientCheck(HintedUser(ou->getUser(), client->getHubUrl()));
			if(!fname.empty())
				ou->getIdentity().setTestSURQueued(fname);
		} catch(const Exception& e) {
			LogManager::getInstance()->message(e.getError());		
		}
	}

	return 0;
}

LRESULT ChatCtrl::onCheckList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou) {
		try {
			string fname = QueueManager::getInstance()->addFileListCheck(HintedUser(ou->getUser(), client->getHubUrl()));
			if(!fname.empty())
				ou->getIdentity().setFileListQueued(fname);
		} catch(const Exception& e) {
			LogManager::getInstance()->message(e.getError());		
		}
	}
	return 0;
}
//RSX++
LRESULT ChatCtrl::onCleanUser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		ou->getIdentity().cleanUser();
	return 0;
}

LRESULT ChatCtrl::onSetProtected(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		ou->getUser()->setFlag(User::PROTECTED);
	return 0;
}

LRESULT ChatCtrl::onUnsetProtected(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		ou->getUser()->unsetFlag(User::PROTECTED);
	return 0;
}

LRESULT ChatCtrl::onCustomKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		ou->customKick();
	return 0;
}

LRESULT ChatCtrl::onMultihubKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou)
		ou->multiHubKick();
	return 0;
}
//END
void ChatCtrl::runUserCommand(UserCommand& uc) {
	StringMap ucParams;

	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return;

	client->getMyIdentity().getParams(ucParams, "my", true);
	client->getHubIdentity().getParams(ucParams, "hub", false);

	const OnlineUserPtr ou = client->findUser(Text::fromT(sSelectedUser));
	if(ou != NULL) {
		StringMap tmp = ucParams;
		ou->getIdentity().getParams(tmp, "user", true);
		client->escapeParams(tmp);
		client->sendUserCmd(uc, tmp);
	}
}

string ChatCtrl::escapeUnicode(tstring str) {
	TCHAR buf[8];
	memzero(buf, sizeof(buf));

	int dist = 0;
	tstring::iterator i;
	while((i = std::find_if(str.begin() + dist, str.end(), std::bind2nd(std::greater<TCHAR>(), 0x7f))) != str.end()) {
		dist = (i+1) - str.begin(); // Random Acess iterators FTW
		snwprintf(buf, sizeof(buf), _T("%hd"), int(*i));
		str.replace(i, i+1, _T("\\ud\\u") + tstring(buf) + _T("?"));
		memzero(buf, sizeof(buf));
	}
	return Text::fromT(str);
}

tstring ChatCtrl::rtfEscape(tstring str) {
	tstring::size_type i = 0;
	while((i = str.find_first_of(_T("{}\\\n"), i)) != tstring::npos) {
		switch(str[i]) {
			// no need to process \r handled elsewhere
			case '\n': str.replace(i, 1, _T("\\line\n")); i+=6; break;
			default: str.insert(i, _T("\\")); i+=2;
		}
	}
	return str;
}

void ChatCtrl::scrollToEnd() {
	SCROLLINFO si = { 0 };
	POINT pt = { 0 };

	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	GetScrollInfo(SB_VERT, &si);
	GetScrollPos(&pt);

	// this must be called twice to work properly :(
	PostMessage(EM_SCROLL, SB_BOTTOM, 0);
	PostMessage(EM_SCROLL, SB_BOTTOM, 0);

	SetScrollPos(&pt);
}
//RSX++ //handle highlights actions
void ChatCtrl::handleActions(const dcpp::tstring& msgAuthor) {
	tstring matchStrings = Util::emptyStringT;
	string soundFilePath = Util::emptyString;
	int matches = 0;
	bool flashWindow = false;

	for(ActionsList::const_iterator i = actions.begin(); i != actions.end(); ++i) {
		if((*i).actPopup) {
			if((matchStrings.find((*i).match + _T(", ")) == tstring::npos)) //probably string isn't here, add
				matchStrings += (*i).match + _T(", ");
			matches++;
		}
		if((*i).actFlash) {
			flashWindow = true;
		}
		if((*i).actSound && !(*i).soundPath.empty()) {
			soundFilePath = (*i).soundPath;
		}
	}
	if(matchStrings.length() >= 2)
		matchStrings = matchStrings.substr(0, matchStrings.length()-2);

	if(matches > 0) {
		const tstring& popupMsg = msgAuthor+ _T(" (") + Util::toStringW(matches) + _T(" match(es)) - ") +  matchStrings;
		const tstring& popupTitle = _T("Highlights");
		MainFrame::getMainFrame()->ShowBalloonTip(popupMsg.c_str(), popupTitle.c_str());
	}
	if(!soundFilePath.empty()) {
		::PlaySound(Text::toT(soundFilePath).c_str(), NULL, SND_FILENAME | SND_ASYNC);
	}
	if(flashWindow) {
		WinUtil::flashWindow();
	}
	//clear vector
	actions.clear();
}
//END