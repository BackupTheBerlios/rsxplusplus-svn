// $Id$
//
// Copyright © 2009 Phil Cairns
// This code is published under the Code Project Open Licence. See
// http://www.codeproject.com/info/cpol10.aspx for licence details.
//
// A large amount of the code in this file was drawn from Matt Gullett's work
// in the Code Project article here: http://www.codeproject.com/KB/recipes/spellchecker_mg.aspx
// so his file header has been reproduced below.
//

/*

  Copyright:		2000
  Author:			Matthew T Gullett
  Email:			gullettm@yahoo.com
  Name:				CFPSSpellingEditCtrl
  Part of:			Spell Checking Engine 
  Requires:			

  DESCRIPTION
  ----------------------------------------------------------
  This class is designed to implement a self spell-checking
  edit control.  It utilized the FPS Spell Checking Engine
  to provide dictionary and suggestion support.


  INFO:
  ----------------------------------------------------------
  This class is provided -as is-.  No warranty as to the
  function or performance of this class is provided either 
  written or implied.  
  
  You may freely use this code and modify it as necessary,
  as long as this header is unmodified and credit is given
  to the author in the application(s) in which it is
  incorporated.

  26/06/2009 adrian_007
  - made unicode compatible
  - got rid of CString and replaced it with plain std::string/std::wstring
  - some small improvements
*/

#ifndef spellcheckedit_h_
#define spellcheckedit_h_

#include "SpellChecker.hpp"

enum
{
	ID_SPELLCHECK_OPT0=0x8000,
	ID_SPELLCHECK_OPT1,
	ID_SPELLCHECK_OPT2,
	ID_SPELLCHECK_OPT3,
	ID_SPELLCHECK_OPT4,
	ID_SPELLCHECK_OPT5,
	ID_SPELLCHECK_OPT6,
	ID_SPELLCHECK_OPT7,
	ID_SPELLCHECK_OPT8,
	ID_SPELLCHECK_OPT9,
	ID_SPELLCHECK_ADD
};

class CSpellCheckEdit : public CWindowImpl<CSpellCheckEdit, CEdit>
{
	typedef CWindowImpl<CSpellCheckEdit, CEdit> baseClass;
#ifdef UNICODE
	typedef std::wstring string_t;
#else
	typedef std::string string_t;
#endif
	struct SpError
	{
		CRect rcArea;
		string_t word;
		int posn;
	};
	typedef std::list<SpError> SPERRLIST;

private:
	bool _errorsDrawn;
	bool _hasErrors;
	UINT _timerId;
	DWORD _lastTick;
	SPERRLIST _errors;

protected:
	void ClearErrors()
	{
		_errors.clear();
	}

	void RedrawErrors()
	{
		ClearErrors();
		if(!SpellChecker::getInstance()->isRunning()) return;

		CClientDC dc(m_hWnd);
		string_t strText;
		int iLine = GetFirstVisibleLine();
		int iChar = LineIndex(iLine);
		int iLineLen = LineLength(iChar);
		int iLineCount = GetLineCount();
		CRect clientRect;
		BOOL bContinue = TRUE;
		{
			int len = GetWindowTextLength();
			strText.resize(len+1);
			strText.resize(GetWindowText(&strText[0], strText.size()));
		}
		GetClientRect(clientRect);

		while (bContinue)
		{
			if (iLineLen > 0 && iChar != -1)
			{
				RedrawErrors(iLine, iLineLen, iChar, strText.c_str(), dc);
			}

			iLine++;

			if (iLine > iLineCount)
			{
				bContinue = FALSE;
			}
			else
			{
				iChar = LineIndex(iLine);
				iLineLen = LineLength(iChar);

				POINT pt = PosFromChar(iChar);
				if (pt.y > clientRect.Height() || iLine >= iLineCount) {
					bContinue = FALSE;
				}
			}
		}
		_errorsDrawn = true;
	}

	BOOL IsWordBreak(TCHAR cThisChar)
	{
		BOOL bReturn = FALSE;

		if (!_istalnum(cThisChar))
		{
			bReturn = TRUE;

			switch (cThisChar)
			{
			case _T('_'):
			case _T('\''):
				bReturn = FALSE;
				break;

			default:
				break;
			}
		}

		return bReturn;
	}

	void DrawError(LPCTSTR lpszWord, int iChar, CClientDC& dc)
	{
		ATLASSERT(lpszWord);
		ATLASSERT(iChar >= 0);

		if (SpellChecker::getInstance()->isWordOk(lpszWord))
		{
			return;
		}

		_hasErrors = true;

		CFontHandle font = GetFont();
		CFontHandle oldFont;
		CBrush newBrush;
		CBrushHandle oldBrush;
		CPen newPen;
		CPenHandle oldPen = NULL;
		CSize szWord;
		int iY = 0;
		int strLen = _tcslen(lpszWord);

		oldFont = dc.SelectFont(font);
		newBrush.CreateSolidBrush(RGB(255,0,0));
		newPen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

		dc.GetTextExtent(lpszWord, strLen, &szWord);
		oldBrush = dc.SelectBrush(newBrush);
		oldPen = dc.SelectPen(newPen);

		CPoint pt = PosFromChar(iChar);
		iY = pt.y + szWord.cy;

		// Only draw the squiggly if the whole squiggly is going
		// to be visible.
		CRect rc;
		GetClientRect(rc);
		if (iY < rc.Height())
		{
			DrawSquiggly(dc, pt.x, szWord.cx, iY);
		}

		dc.SelectFont(oldFont);
		dc.SelectBrush(oldBrush);
		dc.SelectPen(oldPen);

		// create a new spelling error record
		SpError err;
		err.word = lpszWord;
		err.rcArea.SetRect(pt.x, pt.y, pt.x + szWord.cx, iY);
		err.posn = iChar;
		_errors.push_back(err);
	}

	void DrawSquiggly(CDC &dc, int iLeftX, int iWidth, int iY)
	{
		int iCurrentX = iLeftX;

		CBrush br;
		CPen pen;
		CPenHandle oldPen;
		LOGBRUSH lb;

		br.CreateSolidBrush(RGB(255, 0, 0));
		br.GetLogBrush(&lb);
		pen.CreatePen(PS_ALTERNATE, 1, &lb);
		oldPen = dc.SelectPen(pen);

		dc.MoveTo(iCurrentX - 1, iY);
		dc.LineTo(iCurrentX + iWidth, iY);

		dc.SelectPen(oldPen);
	}

	void RedrawErrors(int iLine, int iLineLen, int iChar, LPCTSTR lpszText, CClientDC& dc)
	{
		ATLASSERT(lpszText);
		ATLASSERT(iLine >= 0);
		ATLASSERT(iLineLen >= 0);
		ATLASSERT(iChar >= 0);

		string_t strWord;
		int iLineBegins = -1;
		TCHAR cThisChar = 0;
		int iCharPos = iChar;

		// extract words from line
		while (lpszText[iCharPos] != 0
			&& lpszText[iCharPos] != _T('\r')
			&& lpszText[iCharPos] != _T('\n')
			&& iCharPos < iChar + iLineLen)
		{
			cThisChar = lpszText[iCharPos];

			if (IsWordBreak(cThisChar))
			{
				while(strWord.size() && IsWordBreak(strWord[0]))
					strWord.erase((string_t::size_type)0);

				while(strWord.size() && IsWordBreak(strWord[strWord.size()-1]))
					strWord.erase(strWord.size()-1);

				if (strWord.size())
				{
					DrawError(strWord.c_str(), iLineBegins, dc);
				}

				strWord.clear();
				iLineBegins = -1;
			}
			else
			{
				strWord += lpszText[iCharPos];
				if (iLineBegins == -1 && strWord.size())
				{
					iLineBegins = iCharPos;
				}
			}

			iCharPos++;
		}

		if (strWord.size())
		{
			while(strWord.size() && IsWordBreak(strWord[0]))
				strWord.erase((string_t::size_type)0);

			while(strWord.size() && IsWordBreak(strWord[strWord.size()-1]))
				strWord.erase(strWord.size()-1);

			if (strWord.size())
			{
				DrawError(strWord.c_str(), iLineBegins, dc);
			}
		}
	}

	void InvalidateCheck()
	{
		if (_errorsDrawn)
		{
			_errorsDrawn = false;
		}
		RedrawWindow();
		_lastTick = ::GetTickCount();
	}
public:
	CSpellCheckEdit()
	{
		_errorsDrawn = false;
		_lastTick = GetTickCount();
		_timerId = (UINT)(-1);
		_hasErrors = false;
	}

	BEGIN_MSG_MAP(CSpellCheckEdit)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButton)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButton)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_SETTEXT, OnSetText)
		REFLECTED_COMMAND_CODE_HANDLER(EN_CHANGE, OnChange)
	END_MSG_MAP()

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		if (IsWindow() && _timerId == (UINT)(-1))
		{
			_timerId = SetTimer(1, 100, 0);
			ATLTRACE("Timer started\n");
		}
		_errorsDrawn = false;
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		bHandled = FALSE;
		if (_timerId != (UINT)(-1))
		{
			KillTimer(_timerId);
			_timerId = (UINT)(-1);
		}
		return 0;
	}

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (_timerId != (UINT)(-1)) {
			KillTimer(_timerId);
			_timerId = (UINT)(-1);
		}

		if(!_errorsDrawn) {
			if ((GetTickCount() - _lastTick) > 1000) {
				RedrawErrors();
			}
		}
		_timerId = SetTimer(1, 100, 0);
		return 0;
	}

	LRESULT OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
	{
		DefWindowProc(uMsg, wParam, lParam);
		InvalidateCheck();
		return 0;
	}

	LRESULT OnChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		InvalidateCheck();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
	{
		DefWindowProc(uMsg, wParam, lParam);
		if (_hasErrors)
		{
			RedrawErrors();
		}
		return 0;
	}

	LRESULT OnScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (_errorsDrawn)
		{
			_errorsDrawn = false;
			DefWindowProc(uMsg, wParam, lParam);
			RedrawWindow();
		}
		else
		{
			bHandled = FALSE;
		}
		return 0;
	}

	LRESULT OnLButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
	{
		DefWindowProc(uMsg, wParam, lParam);
		if (_errorsDrawn)
		{
			RedrawErrors();
		}
		return 0;
	}

	LRESULT OnKeyDown(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		bHandled = FALSE;
		InvalidateCheck();
		return 0;
	}

	LRESULT OnContextMenu(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
	{
		CPoint mousePt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		CPoint clientPt(mousePt);
		ScreenToClient(&clientPt);

		// Find out if we're over any errors
		SPERRLIST::iterator split;
		SpError thisError;
		bool bFound = false;
		for (split = _errors.begin(); !bFound && split != _errors.end(); split++)
		{
			if (split->rcArea.PtInRect(clientPt))
			{
				thisError = *split;
				bFound = true;
			}
		}
		if (!bFound)
		{
			bHandled = FALSE;
			return 0;
		}

		try
		{
			CMenu popup;
			popup.CreatePopupMenu();

			// Append the suggestions, if there are any
			std::list<string_t> options;
			SpellChecker::getInstance()->suggest(thisError.word, options);

			UINT itemID = ID_SPELLCHECK_OPT0;
			std::map<UINT, string_t> optMap;
			for(std::list<string_t>::iterator it = options.begin(); it != options.end() && itemID <= ID_SPELLCHECK_OPT9; ++it, ++itemID) {
				string_t s = *it;
				string_t::size_type pos = string_t::npos;
				while((pos = s.find(_T("&"), pos)) != string_t::npos)
					s.replace(pos, 1, _T("&&"));
				popup.AppendMenu(MF_ENABLED, itemID, s.c_str());
				optMap[itemID] = s;
			}

			if (options.size() > 0)
			{
				popup.AppendMenu(MF_SEPARATOR);
			}

			// Now the editing commands (cut, copy, paste, undo, etc)
			if (CanUndo())
			{
				popup.AppendMenu(MF_ENABLED, EM_UNDO, _T("Undo"));
				popup.AppendMenu(MF_SEPARATOR);
			}
			popup.AppendMenu(MF_ENABLED, WM_CUT, _T("Cut"));
			popup.AppendMenu(MF_ENABLED, WM_COPY, _T("Copy"));
			popup.AppendMenu(MF_ENABLED, WM_PASTE, _T("Paste"));
			popup.AppendMenu(MF_ENABLED, WM_CLEAR, _T("Delete"));
			popup.AppendMenu(MF_SEPARATOR);
			popup.AppendMenu(MF_ENABLED, EM_SETSEL, _T("Select All"));

			UINT cmdId = popup.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
				mousePt.x,
				mousePt.y,
				m_hWnd,
				0);
			switch (cmdId)
			{
			case EM_UNDO:
			case WM_CUT:
			case WM_COPY:
			case WM_PASTE:
			case WM_CLEAR:
			case EM_SETSEL:
				SendMessage(cmdId, 0, -1);
				break;

			case ID_SPELLCHECK_ADD:
				SpellChecker::getInstance()->addWord(thisError.word);
				InvalidateCheck();
				break;

			default:
				if (optMap.find(cmdId) != optMap.end())
				{
					SetSel(thisError.posn, thisError.posn + thisError.word.length());
					ReplaceSel(optMap[cmdId].c_str(), TRUE);
					InvalidateCheck();
				}
				break;
			}
		}
		catch (int)
		{
			bHandled = FALSE;
		}
		return 0;
	}
};

#endif
