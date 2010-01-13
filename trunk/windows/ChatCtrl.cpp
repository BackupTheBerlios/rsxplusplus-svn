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

tstring ChatCtrl::sSelectedLine = Util::emptyStringT;
tstring ChatCtrl::sSelectedIP = Util::emptyStringT;
tstring ChatCtrl::sSelectedUser = Util::emptyStringT;
tstring ChatCtrl::sSelectedURL = Util::emptyStringT;

static const TCHAR* Links[] = { _T("http://"), _T("https://"), _T("www."), _T("ftp://"), 
	_T("magnet:?"), _T("dchub://"), _T("irc://"), _T("ed2k://"), _T("mms://"), _T("file://"),
	_T("adc://"), _T("adcs://"), _T("nmdcs://") };

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
	
void ChatCtrl::AdjustTextSize() {
	if(GetWindowTextLength() > RSXPP_SETTING(MAX_CHAT_BUFSIZE)) {
		SetRedraw(FALSE);
		SetSel(0, LineIndex(LineFromChar(2000)));
		ReplaceSel(_T(""));
		SetRedraw(TRUE);

		scrollToEnd();
	}
}

void ChatCtrl::AppendText(const Identity& i, const tstring& sMyNick, const tstring& sTime, const tstring& sMsg, CHARFORMAT2& cf, bool bUseEmo/* = true*/, bool useHL/* = true*/, tstring chatExtraInfo) {
	SetRedraw(FALSE);

	SCROLLINFO si = { 0 };
	POINT pt = { 0 };

	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	GetScrollInfo(SB_VERT, &si);
	GetScrollPos(&pt);

	LONG lSelBegin = 0, lSelEnd = 0;
	LONG lSelBeginSaved, lSelEndSaved;
	GetSel(lSelBeginSaved, lSelEndSaved);

	// Insert TimeStamp and format with default style
	if(!sTime.empty()) {
		lSelEnd = lSelBegin = GetTextLengthEx(GTL_NUMCHARS);
		SetSel(lSelEnd, lSelEnd);
		ReplaceSel(sTime.c_str(), false);
		lSelEnd = GetTextLengthEx(GTL_NUMCHARS);
		SetSel(lSelBegin, lSelEnd - 1);
		SetSelectionCharFormat(WinUtil::m_TextStyleTimestamp);

		PARAFORMAT2 pf;
		memzero(&pf, sizeof(PARAFORMAT2));
		pf.dwMask = PFM_STARTINDENT; 
		pf.dxStartIndent = 0;
		SetParaFormat(pf);
	}
	//RSX++
	bool isMyMessage = i.getUser() == ClientManager::getInstance()->getMe();
	bool isRealUser = i.getUser().get() != 0 && !i.isBot() && !i.isHub() && !i.isHidden();

	if(!chatExtraInfo.empty() && isRealUser) {
		chatExtraInfo += _T(" ");
		lSelEnd = lSelBegin = GetTextLengthEx(GTL_NUMCHARS);
		SetSel(lSelEnd, lSelEnd);
		ReplaceSel(chatExtraInfo.c_str(), false);
		lSelEnd = GetTextLengthEx(GTL_NUMCHARS);
		SetSel(lSelBegin, lSelEnd - 1);
		SetSelectionCharFormat(isMyMessage ? WinUtil::m_ChatTextMyOwn : WinUtil::m_ChatTextGeneral);

		PARAFORMAT2 pf;
		memzero(&pf, sizeof(PARAFORMAT2));
		pf.dwMask = PFM_STARTINDENT; 
		pf.dxStartIndent = 0;
		SetParaFormat(pf);
	}
	//END

	TCHAR* sText = (TCHAR*)sMsg.c_str();
	tstring sAuthor = Text::toT(i.getNick());
	//bool isMyMessage = i.getUser() == ClientManager::getInstance()->getMe();
	if(!sAuthor.empty()) {
		LONG iLen = (sMsg[0] == _T('*')) ? 1 : 0;
		LONG iAuthorLen = (LONG)_tcslen(sAuthor.c_str()) + 1;
   		sText += iAuthorLen + iLen;
   		
		lSelEnd = lSelBegin = GetTextLengthEx(GTL_NUMCHARS);
		SetSel(lSelEnd, lSelEnd);
		ReplaceSel(sMsg.substr(0, iAuthorLen + iLen).c_str(), false);
		
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
				SetSel(lSelBegin + iLen + 1, lSelBegin + iLen + iAuthorLen);
				if(isFavorite){
					SetSelectionCharFormat(WinUtil::m_TextStyleFavUsers);
				} else if(i.isOp()) {
					SetSelectionCharFormat(WinUtil::m_TextStyleOPs);
				} else if(isProtected) {
					SetSelectionCharFormat(WinUtil::m_TextStyleProtected); //RSX++
				} else {
					SetSelectionCharFormat(WinUtil::m_TextStyleBold);
				}
			} else {
				SetSel(lSelBegin, lSelBegin + iLen + iAuthorLen);
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
				sText = _tcschr(sText + 1 + (int)thirdPerson, thirdPerson ? _T(' ') : _T('>'));
                if(sText != NULL) {
                    LONG iAuthorLen = (LONG)(sText - sMsg.c_str());
                    
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
                    
		            lSelEnd = lSelBegin = GetTextLengthEx(GTL_NUMCHARS);
		            SetSel(lSelEnd, lSelEnd);
            		ReplaceSel(sMsg.substr(0, iAuthorLen).c_str(), false);
        			if(BOOLSETTING(BOLD_AUTHOR_MESS) || isFavorite || isOp) {
        				SetSel(lSelBegin, lSelBegin + 1);
        				SetSelectionCharFormat(cf);
                        SetSel(lSelBegin + 1, lSelBegin + iAuthorLen);
						if(isFavorite){
							SetSelectionCharFormat(WinUtil::m_TextStyleFavUsers);
						} else if(isOp) {
							SetSelectionCharFormat(WinUtil::m_TextStyleOPs);
						} else {
							SetSelectionCharFormat(WinUtil::m_TextStyleBold);
						}
        			} else {
        				SetSel(lSelBegin, lSelBegin + iAuthorLen);
        				SetSelectionCharFormat(cf);
                    }
				} else {
					sText = (TCHAR*)sMsg.c_str();
				}
        }
	}
				   			
	{
		 TCHAR *fsrc, *fdst;
		 fsrc = fdst = sText;
		 while(*fsrc) {
			  if (*fsrc != '\r') {
				   *fdst = *fsrc;
				   fdst++;
			  }
			  fsrc++;
		 }
		 *fdst = _T('\0');
	}
	//RSX++ //remove emots from links; no setting to on/off it - in links shouldn't be emots anyway
	if(bUseEmo) {
		for(size_t i = 0; i < (sizeof(Links) / sizeof(Links[0])); i++) {
			if(sMsg.find(Links[i]) != tstring::npos) {
				bUseEmo = false;
				break;
			}
		}
	}
	//END

	// Insert emoticons
	if(bUseEmo && g_pEmotionsSetup->getUseEmoticons()) {
		const Emoticon::List& Emoticons = g_pEmotionsSetup->getEmoticonsList();
		uint8_t smiles = 0; int nIdxFound = -1;
		while(true) {
			TCHAR *rpl = NULL;
			Emoticon* pFoundEmotion = NULL;
			int64_t len = _tcslen(sText);
			for(Emoticon::Iter pEmotion = Emoticons.begin(); pEmotion != Emoticons.end(); ++pEmotion) {
				nIdxFound = -1;
				TCHAR *txt = Util::strstr(sText, (*pEmotion)->getEmotionText().c_str(), &nIdxFound);
				if((txt < rpl && txt) || !rpl && txt) {
					if(len > nIdxFound) {
						rpl = txt;
						pFoundEmotion = (*pEmotion);
						len = nIdxFound;
					}
				}
			}

			if(rpl && (smiles < MAX_EMOTICONS)) {
				AppendTextOnly(sMyNick, tstring(sText, rpl - sText).c_str(), cf, isMyMessage, sAuthor, useHL);
				lSelEnd = GetTextLengthEx(GTL_NUMCHARS);
				SetSel(lSelEnd, lSelEnd);
				CImageDataObject::InsertBitmap(GetOleInterface(), 
					pFoundEmotion->getEmotionBmp(isMyMessage ? WinUtil::m_ChatTextMyOwn.crBackColor : WinUtil::m_ChatTextGeneral.crBackColor));

				sText = rpl + pFoundEmotion->getEmotionText().size();
				smiles++;
			} else {
				if(_tcslen(sText) > 0) {
					AppendTextOnly(sMyNick, sText, cf, isMyMessage, sAuthor, useHL);
				}
				break;
			}
		}
	} else {
		AppendTextOnly(sMyNick, sText, cf, isMyMessage, sAuthor, useHL);
	}
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

void ChatCtrl::AppendTextOnly(const tstring& sMyNick, const TCHAR* sText, CHARFORMAT2& cf, bool isMyMessage, const tstring& sAuthor, bool useHL/* = true*/) {
	// Insert text at the end
	long lSelEnd = GetTextLengthEx(GTL_NUMCHARS);
	long lSelBegin = lSelEnd;
	SetSel(lSelBegin, lSelEnd);
	ReplaceSel(sText, false);

	// Set text format
	tstring sMsgLower = sText;
	std::transform(sMsgLower.begin(), sMsgLower.end(), sMsgLower.begin(), _totlower);

	lSelEnd = GetTextLengthEx(GTL_NUMCHARS);
	SetSel(lSelBegin, lSelEnd);
	SetSelectionCharFormat(isMyMessage ? WinUtil::m_ChatTextMyOwn : cf);
	
	// Zvyrazneni vsech URL a nastaveni "klikatelnosti"
	for(size_t i = 0; i < (sizeof(Links) / sizeof(Links[0])); i++) {
		size_t linkStart = sMsgLower.find(Links[i]);
		bool isMagnet = _tcscmp(Links[i], _T("magnet:?")) == 0;
		while(linkStart != tstring::npos) {
			size_t linkEnd = linkStart + _tcslen(Links[i]);
			
			try {
				boost::match_results<tstring::const_iterator> result;
				//RSX++ //workaround for msvc std lib problems
				tstring::const_iterator start = sMsgLower.begin();
				tstring::const_iterator end = sMsgLower.end();

				// TODO: complete regexp for URLs
				boost::wregex reg;
				if(isMagnet) // magnet links have totally indeferent structure than classic URL // -/?%&=~#'\\w\\.\\+\\*\\(\\)
					reg =       _T("^(\\w)+=[:\\w]+(&(\\w)+=[\\S]*)*[^\\s<>.,;!(){}\"']+");
				else
					reg = _T("^([@\\w-]+(\\.)*)+(:[\\d]+)?(/[\\S]*)*[^\\s<>.,;!(){}\"']+");
					
				if(boost::regex_search(start + linkEnd, end, result, reg, boost::match_default)) {
					dcassert(!result.empty());

					linkEnd += result.length(0);
					SetSel(lSelBegin + linkStart, lSelBegin + linkEnd);
					SetSelectionCharFormat(WinUtil::m_TextStyleURL);
				}
			} catch(...) {
			}
			
			linkStart = sMsgLower.find(Links[i], linkEnd);			
		}
	}

	// Zvyrazneni vsech vyskytu vlastniho nicku
	long lMyNickStart = -1, lMyNickEnd = -1;
	size_t lSearchFrom = 0;	
	tstring sNick = sMyNick.c_str();
	std::transform(sNick.begin(), sNick.end(), sNick.begin(), _totlower);

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

	// Zvyrazneni vsech vyskytu nicku Favorite useru
	FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
	for(FavoriteManager::FavoriteMap::const_iterator i = ul.begin(); i != ul.end(); ++i) {
		const FavoriteUser& pUser = i->second;

		lSearchFrom = 0;
		sNick = Text::toT(pUser.getNick()).c_str();
		std::transform(sNick.begin(), sNick.end(), sNick.begin(), _totlower);

		while((lMyNickStart = (long)sMsgLower.find(sNick, lSearchFrom)) != tstring::npos) {
			lMyNickEnd = lMyNickStart + (long)sNick.size();
			SetSel(lSelBegin + lMyNickStart, lSelBegin + lMyNickEnd);
			SetSelectionCharFormat(WinUtil::m_TextStyleFavUsers);
			lSearchFrom = lMyNickEnd;
		}
	}

	//RSX++ // Highlights
	if(client && useHL) {
		if(client->getUseHL() && RSXPP_BOOLSETTING(USE_HIGHLIGHT)) {
			// decrease number of string allocs
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
		PrivateFrame::openWindow(ou->getUser(), Util::emptyStringT, client);

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
	if(ou)
		client->cheatMessage("*** Info on " + ou->getIdentity().getNick() + " ***" + "\r\n" + ou->getIdentity().getReport() + "\r\n");

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
		client->sendUserCmd(Util::formatParams(uc.getCommand(), tmp, false));
	}
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