/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
 * Copyright (C) 2010 Oyashiro-sama, oyashirosama dot hnnkni at gmail dot com
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
#include "../client/DCPlusPlus.h"
#include "WinUtil.h"
#include <math.h>

#include "EmoticonsDlg.h"

#define EMOTICONS_ICONMARGIN 8

extern EmoticonsManager* emoticonsManager;

BEGIN_EVENT_TABLE(EmoticonsDlg, wxDialog)
	EVT_LEFT_UP(EmoticonsDlg::onMouseUp)
	EVT_MOTION(EmoticonsDlg::onMouseMove)
	EVT_KEY_UP(EmoticonsDlg::onKeyUp)
	EVT_PAINT(EmoticonsDlg::onPaint)
	EVT_MOUSE_CAPTURE_LOST(EmoticonsDlg::onCaptureLost)
END_EVENT_TABLE()

EmoticonsDlg::EmoticonsDlg(wxWindow* parent, wxString *result, const wxRect pos) : wxDialog(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
	_result = result;
	_count = 0;

	if(emoticonsManager->getUseEmoticons() && SETTING(EMOTICONS_FILE) != "Disabled")
	{
		Emoticons = emoticonsManager->getEmoticonsList();

		if(Emoticons.empty())
		{
			Destroy();
		}
		else
		{
			unsigned int pocet = 0;
			string lastEmotionPath = "";
			Emoticon::Iter pEmotion;
			for (pEmotion = Emoticons.begin(); pEmotion != Emoticons.end(); pEmotion++)
			{
				if ((*pEmotion)->getEmoticonBmpPath() != lastEmotionPath)
					pocet++;
				lastEmotionPath = (*pEmotion)->getEmoticonBmpPath();
			}

			unsigned int i = (unsigned int)sqrt((double)Emoticons.size());
			unsigned int nXfor = i;
			unsigned int nYfor = i;
			if ((i*i) == (int)Emoticons.size())
			{
				nXfor, nYfor = i;
			}
			else
			{
				nXfor = i+1;
				if ((i*nXfor) < Emoticons.size())
					nYfor = i+1;
				else nYfor = i;
			}

			i = (unsigned int)sqrt((double)pocet);
			_nX = i;
			_nY = i;
			if ((i*i) == pocet)
			{
				_nX, _nY = i;
			}
			else
			{
				_nX = i+1;
				if ((i*_nX) < pocet) _nY = i+1;
				else _nY = i;
			}
			//----- Getting the size of emoticon from the first image -----
			wxBitmap bm((*Emoticons.begin())->getEmoticonBmp());
			_iconWidth = bm.GetWidth();
			_iconHeight = bm.GetHeight();
			_list.Create(_iconWidth, _iconHeight);
			//----- Loading bitmaps to list -----
			pEmotion = Emoticons.begin();
			lastEmotionPath = "";
			
			for (unsigned int iY = 0; iY < nYfor; iY++)
			{
				for (unsigned int iX = 0; iX < nXfor; iX++)
				{
					if ((iY * nXfor)+ iX + 1 > Emoticons.size())
						break;

					if ((*pEmotion)->getEmoticonBmpPath() != lastEmotionPath)
					{
						try
						{
							_list.Add((*pEmotion)->getEmoticonBmp());
							_emotText.Add((*pEmotion)->getEmoticonText());
							lastEmotionPath = (*pEmotion)->getEmoticonBmpPath();
							_count++;
						}
						catch (...) {}
					}
					pEmotion++;
				}
			}
			//----- End loading bitmaps ------
			_width = _nX * (_iconWidth + EMOTICONS_ICONMARGIN) + 2;
			_height = _nY * (_iconHeight + EMOTICONS_ICONMARGIN) + 2;
			Move(pos.GetRight() - _width < 0 ? 0 : pos.GetRight() - _width, pos.GetTop() - _height < 0 ? 0 : pos.GetTop() - _height);
			SetSize(_width, _height);
			
			CaptureMouse();
			isCaptured = true;
			ShowModal();
			if (isCaptured)
				ReleaseMouse();
		}
	}
	else
		Destroy();
}

EmoticonsDlg::~EmoticonsDlg(void)
{
}

void EmoticonsDlg::onMouseUp(wxMouseEvent &event)
{
	wxPoint mousePos = wxGetMousePosition();
	mousePos = ScreenToClient(mousePos);
	if (mousePos.x < 0 || mousePos.x > _width || mousePos.y < 0 || mousePos.y > _height)
		EndModal(wxID_CANCEL);
	else
	{
		// for testing purposes
		if(event.ControlDown() && event.ShiftDown())
		{
			const Emoticon::List& emoticons = emoticonsManager->getEmoticonsList();
			*_result = wxT("");
			string lastEmoticonPath = "";
			for(Emoticon::Iter icon = emoticons.begin(); icon != emoticons.end(); ++icon)
			{
				if (lastEmoticonPath != (*icon)->getEmoticonBmpPath())
					*_result += (*icon)->getEmoticonText() + _T(" ");
				
				lastEmoticonPath = (*icon)->getEmoticonBmpPath();
			}			
		}
		else
		{
			unsigned int pos = (event.GetY() - 2) / (_iconHeight + EMOTICONS_ICONMARGIN) * _nX + (event.GetX() - 2) / (_iconWidth + EMOTICONS_ICONMARGIN);
			if (pos < _count)
				*_result = _emotText[pos];
		}

		EndModal(wxID_OK);
	}
}

void EmoticonsDlg::onMouseMove(wxMouseEvent &event)
{
	unsigned int pos = (event.GetY() - 2) / (_iconHeight + EMOTICONS_ICONMARGIN) * _nX + (event.GetX() - 2) / (_iconWidth + EMOTICONS_ICONMARGIN);
	if (_oldPos != pos)
	{
		_oldPos = pos;
		if (pos < _count)
			SetToolTip(_emotText[pos]);
		else
			SetToolTip(wxEmptyString);
	}
}

void EmoticonsDlg::onKeyUp(wxKeyEvent &event)
{
	if (event.GetKeyCode() == VK_ESCAPE)
	{
		EndModal(wxID_CANCEL);
	}
}

void EmoticonsDlg::onCaptureLost(wxMouseCaptureLostEvent &WXUNUSED(event))
{
	isCaptured = false;
	EndModal(wxID_CANCEL);
}

void EmoticonsDlg::onPaint(wxPaintEvent &WXUNUSED(event))
{
	wxMemoryDC temp;
	wxBitmap bm(_width, _height);
	temp.SelectObject(bm);
	temp.SetBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	temp.Clear();

	for (unsigned int i = 0; i < _count; ++i)
	{
		temp.DrawRectangle((i % _nX) * (_iconWidth + EMOTICONS_ICONMARGIN) + 1, (i / _nX) * (_iconHeight + EMOTICONS_ICONMARGIN) + 1,
			(_iconWidth + EMOTICONS_ICONMARGIN), (_iconHeight + EMOTICONS_ICONMARGIN));
		_list.Draw(i, temp, (i % _nX) * (_iconWidth + EMOTICONS_ICONMARGIN) + EMOTICONS_ICONMARGIN / 2 + 1,
			(i / _nX) * (_iconHeight + EMOTICONS_ICONMARGIN) + EMOTICONS_ICONMARGIN / 2 + 1, wxIMAGELIST_DRAW_TRANSPARENT, true);
	}
	
	wxPaintDC dc(this);
	dc.Blit(0, 0, _width, _height, &temp, 0, 0);

	temp.SelectObject(wxNullBitmap);
}