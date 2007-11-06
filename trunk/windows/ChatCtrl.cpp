/* 
 * 
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
#include "Resource.h"
#include "../client/DCPlusPlus.h"
#include "../client/FavoriteManager.h"

#include "ChatCtrl.h"
#include "AGEmotionSetup.h"
#include "atlstr.h"

#include "../client/pme.h" //RSX++
#include "MainFrm.h" //RSX++

CAGEmotionSetup* g_pEmotionsSetup = NULL;

#define MAX_EMOTICONS 48

tstring ChatCtrl::sSelectedLine = Util::emptyStringT;
tstring ChatCtrl::sSelectedIP = Util::emptyStringT;
tstring ChatCtrl::sSelectedUser = Util::emptyStringT;

static const TCHAR* Links[] = { _T("http://"), _T("https://"), _T("www."), _T("ftp://"), 
	_T("magnet:?"), _T("dchub://"), _T("irc://"), _T("ed2k://"), _T("mms://"), _T("file://"),
	_T("adc://"), _T("adcs://") };

ChatCtrl::ChatCtrl() : m_boAutoScroll(true), client(NULL) {
	if(g_pEmotionsSetup == NULL) {
		g_pEmotionsSetup = new CAGEmotionSetup();
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
	if(GetWindowTextLength() > 25000) {
		// We want to limit the buffer to 25000 characters...after that, w95 becomes sad...
		SetRedraw(FALSE);
		SetSel(0, LineIndex(LineFromChar(2000)));
		ReplaceSel(_T(""));
		SetRedraw(TRUE);
	}
}

void ChatCtrl::AppendText(const Identity& i, const tstring& sMyNick, const tstring& sTime, const LPCTSTR sMsg, CHARFORMAT2& cf, bool bUseEmo/* = true*/, bool useHL/* = true*/) {
	SetRedraw(FALSE);
	long lSelBeginSaved, lSelEndSaved;
	GetSel(lSelBeginSaved, lSelEndSaved);
	POINT cr;
	GetScrollPos(&cr);

	long lSelBegin = 0, lSelEnd = 0;

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

	CAtlString sText;
	tstring sAuthor = Text::toT(i.getNick());
	bool bMyMess = i.getUser() == ClientManager::getInstance()->getMe();
	if(!sAuthor.empty()) {
		size_t iLen = (sMsg[0] == _T('*')) ? 1 : 0;
		size_t iAuthorLen = _tcslen(sAuthor.c_str())+1;
   		sText = sMsg+iAuthorLen+iLen;
		lSelEnd = lSelBegin = GetTextLengthEx(GTL_NUMCHARS);
		SetSel(lSelEnd, lSelEnd);
		ReplaceSel(((tstring)sMsg).substr(0, iAuthorLen+iLen).c_str(), false);
		if(bMyMess) {
			SetSel(lSelBegin, lSelBegin+iLen+1);
			SetSelectionCharFormat(WinUtil::m_ChatTextMyOwn);
			SetSel(lSelBegin+iLen+1, lSelBegin+iLen+iAuthorLen);
			SetSelectionCharFormat(WinUtil::m_TextStyleMyNick);
		} else {
			bool isFavorite = FavoriteManager::getInstance()->isFavoriteUser(i.getUser());

			if(BOOLSETTING(BOLD_AUTHOR_MESS) || isFavorite || i.isOp()) {
				SetSel(lSelBegin, lSelBegin+iLen+1);
				SetSelectionCharFormat(cf);
				SetSel(lSelBegin+iLen+1, lSelBegin+iLen+iAuthorLen);
				if(isFavorite){
					SetSelectionCharFormat(WinUtil::m_TextStyleFavUsers);
				} else if(i.isOp()) {
					SetSelectionCharFormat(WinUtil::m_TextStyleOPs);
				} else {
					SetSelectionCharFormat(WinUtil::m_TextStyleBold);
				}
			} else {
				SetSel(lSelBegin, lSelBegin+iLen+iAuthorLen);
				SetSelectionCharFormat(cf);
            }
		}
	} else {
        switch(sMsg[0]) {
            case _T('<'): {
                if((_tcschr((TCHAR*)sMsg+1, _T('>'))) != NULL) {
                    size_t iAuthorLen = _tcslen(sMsg+1)+1;
                    sText = sMsg+iAuthorLen;
		            lSelEnd = lSelBegin = GetTextLengthEx(GTL_NUMCHARS);
		            SetSel(lSelEnd, lSelEnd);
            		ReplaceSel(((tstring)sMsg).substr(0, iAuthorLen).c_str(), false);
        			if(BOOLSETTING(BOLD_AUTHOR_MESS)) {
        				SetSel(lSelBegin, lSelBegin+1);
        				SetSelectionCharFormat(cf);
                        SetSel(lSelBegin+1, lSelBegin+iAuthorLen);
        				SetSelectionCharFormat(WinUtil::m_TextStyleBold);
        			} else {
        				SetSel(lSelBegin, lSelBegin+iAuthorLen);
        				SetSelectionCharFormat(cf);
                    }
				} else {
					sText = sMsg;
				}
                break;
            }
            case _T('*'): {
                if(sMsg[1] == _T(' ') && (_tcschr((wchar_t *)sMsg+2, _T(' '))) != NULL) {
                    size_t iAuthorLen = _tcslen(sMsg+2)+1;
                    sText = sMsg+iAuthorLen+1;
		            lSelEnd = lSelBegin = GetTextLengthEx(GTL_NUMCHARS);
		            SetSel(lSelEnd, lSelEnd);
            		ReplaceSel(((tstring)sMsg).substr(0, iAuthorLen+1).c_str(), false);
        			if(BOOLSETTING(BOLD_AUTHOR_MESS)) {
        				SetSel(lSelBegin, lSelBegin+2);
        				SetSelectionCharFormat(cf);
      					SetSel(lSelBegin+2, lSelBegin+1+iAuthorLen);
        				SetSelectionCharFormat(WinUtil::m_TextStyleBold);
        			} else {
        				SetSel(lSelBegin, lSelBegin+1+iAuthorLen);
        				SetSelectionCharFormat(cf);
                    }
				} else {
					sText = sMsg;
				}
                break;
            }
            default:
                sText = sMsg;
                break;
        }
	}

	sText.Remove(_T('\r'));
	sText += "\n";

	//RSX++ //remove emots from links; no setting to on/off it - in links shouldn't be emots anyway
	if(bUseEmo) {
		for(size_t i = 0; i < (sizeof(Links) / sizeof(Links[0])); i++) {
			if(sText.Find(Links[i]) != tstring::npos) {
				bUseEmo = false;
				break;
			}
		}
	}
	//END

	// Insert emoticons
	if(bUseEmo && g_pEmotionsSetup->getUseEmoticons()) {
		const CAGEmotion::List& Emoticons = g_pEmotionsSetup->getEmoticonsList();
		uint8_t smiles = 0; int nIdxFound = -1;
		while(true) {
			TCHAR *rpl = NULL;
			CAGEmotion* pFoundEmotion = NULL;
			int len = sText.GetLength();
			for(CAGEmotion::Iter pEmotion = Emoticons.begin(); pEmotion != Emoticons.end(); ++pEmotion) {
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
				AppendTextOnly(sMyNick, sText.Left(rpl - sText), cf, bMyMess, sAuthor, useHL);
				lSelEnd = GetTextLengthEx(GTL_NUMCHARS);
				SetSel(lSelEnd, lSelEnd);
				CImageDataObject::InsertBitmap(GetOleInterface(), 
					pFoundEmotion->getEmotionBmp(bMyMess ? WinUtil::m_ChatTextMyOwn.crBackColor : WinUtil::m_ChatTextGeneral.crBackColor));

				sText = rpl + pFoundEmotion->getEmotionText().size();
				smiles++;
			} else {
				if(_tcslen(sText) > 0) {
					AppendTextOnly(sMyNick, sText, cf, bMyMess, sAuthor, useHL);
				}
				break;
			}
		}
	} else {
		AppendTextOnly(sMyNick, sText, cf, bMyMess, sAuthor, useHL);
	}
	SetSel(lSelBeginSaved, lSelEndSaved);
	SetScrollPos(&cr);
	GoToEnd();

	// Force window to redraw
	SetRedraw(TRUE);
	InvalidateRect(NULL);
}

void ChatCtrl::AppendTextOnly(const tstring& sMyNick, const LPCTSTR sText, CHARFORMAT2& cf, bool bMyMess, const tstring& sAuthor, bool useHL/* = true*/) {
	// Insert text at the end
	long lSelEnd = GetTextLengthEx(GTL_NUMCHARS);
	long lSelBegin = lSelEnd;
	SetSel(lSelBegin, lSelEnd);
	ReplaceSel(sText, false);

	// Set text format
	CAtlString sMsgLower = sText;
	sMsgLower.MakeLower();

	lSelEnd = GetTextLengthEx(GTL_NUMCHARS);
	SetSel(lSelBegin, lSelEnd);
	SetSelectionCharFormat(bMyMess ? WinUtil::m_ChatTextMyOwn : cf);

	// Zvyrazneni vsech URL a nastaveni "klikatelnosti"
	long lSearchFrom = 0;
	for(size_t i = 0; i < (sizeof(Links) / sizeof(Links[0])); i++) {
		long linkStart = sMsgLower.Find(Links[i], lSearchFrom);
		while(linkStart > 0) {
			long linkEnd;
			long linkEndSpace = sMsgLower.Find(_T(" "), linkStart);
			long linkEndLine = sMsgLower.Find(_T("\n"), linkStart);
			if((linkEndSpace <= linkStart && linkEndLine > linkStart) || (linkEndSpace > linkEndLine && linkEndLine > linkStart)) {
				linkEnd = linkEndLine;
			} else if(linkEndSpace > linkStart) {
				linkEnd = linkEndSpace;
			} else {
				linkEnd = _tcslen(sMsgLower);
			}
			SetSel(lSelBegin + linkStart, lSelBegin + linkEnd);
			SetSelectionCharFormat(WinUtil::m_TextStyleURL);
			linkStart = sMsgLower.Find(Links[i], linkEnd);
		}
	}

	// Zvyrazneni vsech vyskytu vlastniho nicku
	long lMyNickStart = -1, lMyNickEnd = -1;	
	CAtlString sNick = sMyNick.c_str();
	sNick.MakeLower();

	while((lMyNickStart = sMsgLower.Find(sNick, lSearchFrom)) >= 0) {
		lMyNickEnd = lMyNickStart + sNick.GetLength();
		SetSel(lSelBegin + lMyNickStart, lSelBegin + lMyNickEnd);
		SetSelectionCharFormat(WinUtil::m_TextStyleMyNick);
		lSearchFrom = lMyNickEnd;

		if(	!SETTING(CHATNAMEFILE).empty() && !BOOLSETTING(SOUNDS_DISABLED) &&
			!sAuthor.empty() && (Util::stricmp(sAuthor.c_str(), sNick) != 0)) {
				::PlaySound(Text::toT(SETTING(CHATNAMEFILE)).c_str(), NULL, SND_FILENAME | SND_ASYNC);	 	
        }
	}

	// Zvyrazneni vsech vyskytu nicku Favorite useru
	FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
	for(FavoriteManager::FavoriteMap::const_iterator i = ul.begin(); i != ul.end(); ++i) {
		const FavoriteUser& pUser = i->second;

		lSearchFrom = 0;
		sNick = Text::toT(pUser.getNick()).c_str();
		sNick.MakeLower();

		while((lMyNickStart = sMsgLower.Find(sNick, lSearchFrom)) >= 0) {
			lMyNickEnd = lMyNickStart + sNick.GetLength();
			SetSel(lSelBegin + lMyNickStart, lSelBegin + lMyNickEnd);
			SetSelectionCharFormat(WinUtil::m_TextStyleFavUsers);
			lSearchFrom = lMyNickEnd;
		}
	}

	//RSX++ //crappy highlights implement
	if(client && useHL) {
		if(client->getUseHL() && RSXBOOLSETTING(USE_HIGHLIGHT)) {
			CAtlString sDat = "";
			long HLStart = 0, HLEnd = 0;

			tstring textToMatch = Util::emptyStringT;
			tstring msg = sMsgLower;

			tstring matchedStrings = Util::emptyStringT;
			bool matched = false;

			CHARFORMAT2 hlcf;

			HighLight::List hll = FavoriteManager::getInstance()->getHLs();
			for(HighLight::Iter i = hll.begin(); i != hll.end(); i++) {
				//if((*i)->getHstring().length() < 5)
				//	return;
				textToMatch = Text::toT((*i)->getHstring());

				memset(&hlcf, 0, sizeof(CHARFORMAT2));
				hlcf.cbSize = sizeof(hlcf);
				hlcf.dwReserved = 0;
				hlcf.dwMask = CFM_BACKCOLOR | CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
				if((*i)->getHasBgColor())
					hlcf.crBackColor = (*i)->getBackColor();
				else
					hlcf.crBackColor = SETTING(TEXT_GENERAL_BACK_COLOR);
				if((*i)->getHasFontColor())
					hlcf.crTextColor = (*i)->getFontColor();
				else
					hlcf.crTextColor = SETTING(TEXT_GENERAL_FORE_COLOR);
				if((*i)->getBoldFont())
					hlcf.dwEffects |= CFE_BOLD;
				if((*i)->getItalicFont())
					hlcf.dwEffects |= CFE_ITALIC;
				if((*i)->getUnderlineFont())
					hlcf.dwEffects |= CFM_UNDERLINE;
				if((*i)->getStrikeoutFont())
					 hlcf.dwEffects |= CFM_STRIKEOUT;

				PME regexp(Text::toLower(textToMatch), _T("gims"));
				if(regexp.IsValid()) {
					while(regexp.match(msg) > 0) {
						if(regexp.NumBackRefs() == 1) {
							HLStart = regexp.GetStartPos(0);
							HLEnd = HLStart + regexp.GetLength(0);

							SetSel(lSelBegin + HLStart, lSelBegin + HLEnd);
							SetSelectionCharFormat(hlcf);
							
							struct hlAction curAction = {
								msg.substr(HLStart, regexp.GetLength(0)),
								(*i)->getDisplayPopup(), 
								(*i)->getFlashWindow(), 
								(*i)->getPlaySound(), 
								(*i)->getSoundFilePath()
							};
							actions.push_back(curAction);
							matched = true;
						} else {
							for(int j = 1; j < regexp.NumBackRefs(); ++j) {
								HLStart = regexp.GetStartPos(j);
								HLEnd = HLStart + regexp.GetLength(j);

								SetSel(lSelBegin + HLStart, lSelBegin + HLEnd);
								SetSelectionCharFormat(hlcf);

								struct hlAction curAction = {
									msg.substr(HLStart, regexp.GetLength(j)),
									(*i)->getDisplayPopup(), 
									(*i)->getFlashWindow(), 
									(*i)->getPlaySound(), 
									(*i)->getSoundFilePath()
								};
								actions.push_back(curAction);
								matched = true;
							}
						}
					}
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

bool ChatCtrl::HitNick(POINT p, tstring& sNick, int& iBegin, int& iEnd) {
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

	AutoArray<TCHAR> buf(len+1);
	GetTextRange(lSelBegin, lSelEnd, buf);
	tstring sText(buf, len);

	int iLeft = 0, iRight = 0, iCRLF = sText.size(), iPos = sText.find(_T('<'));
	if(iPos >= 0) {
		iLeft = iPos + 1;
		iPos = sText.find(_T('>'), iLeft);
		if(iPos < 0) 
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

bool ChatCtrl::HitIP(POINT p, tstring& sIP, int& iBegin, int& iEnd) {
	int iCharPos = CharFromPos(p), len = LineLength(iCharPos) + 1;
	if(len < 3)
		return false;

	DWORD lPosBegin = FindWordBreak(WB_LEFT, iCharPos);
	DWORD lPosEnd = FindWordBreak(WB_RIGHTBREAK, iCharPos);
	len = lPosEnd - lPosBegin;
	
	AutoArray<TCHAR> buf(len+1);
	GetTextRange(lPosBegin, lPosEnd, buf);
	for(int i = 0; i < len; i++) {
		if(!((buf[i] == 0) || (buf[i] == '.') || ((buf[i] >= '0') && (buf[i] <= '9')))) {
			return false;
		}
	}
	tstring sText(buf, len);

	sText += _T('.');
	int iFindBegin = 0, iPos = -1, iEnd2 = 0;
	bool boOK = true;
	for(int i = 0; i < 4; i++) {
		iPos = sText.find(_T('.'), iFindBegin);
		if(iPos < 0) {
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

tstring ChatCtrl::LineFromPos(POINT p) const {
	int iCharPos = CharFromPos(p), line = LineFromChar(iCharPos), len = LineLength(iCharPos) + 1;
	if(len < 3) {
		return Util::emptyStringT;
	}
	AutoArray<TCHAR> buf(len+1);
	GetLine(line, buf, len);
	tstring x(buf, len-1);
	return x;
}

void ChatCtrl::GoToEnd() {
/** TODO make auto-scroll dependent on scrollbar position
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	GetScrollInfo(SB_VERT, &si);

	if (si.nPos >= si.nMax - si.nPage- 14)
*/
	if(m_boAutoScroll)
		PostMessage(EM_SCROLL, SB_BOTTOM, 0);
}

void ChatCtrl::SetAutoScroll(bool boAutoScroll) {
	m_boAutoScroll = boAutoScroll;
	 if(boAutoScroll)
		GoToEnd();
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
//RSX++ //handle highlights actions
void ChatCtrl::handleActions(const tstring& msgAuthor) {
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

	if(matches > 0) {
		//@todo
		tstring popupMsg = msgAuthor+ _T(" (") + Util::toStringW(matches) + _T(" match(es)) - ") +  matchStrings;
		popupMsg = popupMsg.substr(0, popupMsg.length()-2); //remove last ", "
		tstring popupTitle;
		popupTitle = _T("Highlights");

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