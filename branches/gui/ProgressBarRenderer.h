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
 
#ifndef _PROGRESSBARRENDERER_H
#define _PROGRESSBARRENDERER_H

#include "TypedDataViewCtrl.h"

#include <wx/dataview.h>

#define MIN3(a, b, c) (((a) < (b)) ? ((((a) < (c)) ? (a) : (c))) : ((((b) < (c)) ? (b) : (c))))
#define MAX3(a, b, c) (((a) > (b)) ? ((((a) > (c)) ? (a) : (c))) : ((((b) > (c)) ? (b) : (c))))
#define CENTER(a, b, c) ((((a) < (b)) && ((a) < (c))) ? (((b) < (c)) ? (b) : (c)) : ((((b) < (a)) && ((b) < (c))) ? (((a) < (c)) ? (a) : (c)) : (((a) < (b)) ? (a) : (b))))

const int MAX_SHADE = 44;
const int SHADE_LEVEL = 90;
const int blend_vector[MAX_SHADE] = {0, 8, 16, 20, 10, 4, 0, -2, -4, -6, -10, -12, -14, -16, -14, -12, -10, -8, -6, -4, -2, 0, 
1, 2, 3, 8, 10, 12, 14, 16, 14, 12, 10, 6, 4, 2, 0, -4, -10, -20, -16, -8, 0};

template <class T>
class ProgressBarRenderer : public wxDataViewCustomRenderer
{
public:
	ProgressBarRenderer(void) { }
	~ProgressBarRenderer(void) { }

	wxSize GetSize() const { return wxSize(0, 0); } // <-- what is this for?
	bool GetValue(wxVariant&) const { return true; }
	bool SetValue(const wxVariant& _value) { value << _value; return true; }
	
	bool Render(wxRect rect, wxDC *dc, int state) 
	{	
		int64_t position = 0;
		int64_t size = 0;
		wxColour backColour, textColour;
		
		if(T::GetItemAttr(value.data, position, size, backColour, textColour))
		{
			HLSTRIPLE hls_bk = RGB2HLS(dc->GetBackground().GetColour());

			dc->SetPen(wxPen(WinUtil::blendColours(dc->GetBackground().GetColour(), backColour, (hls_bk.hlstLightness > 0.75) ? 0.6 : 0.4)));
			dc->SetBrush(wxBrush(WinUtil::blendColours(dc->GetBackground().GetColour(), backColour, (hls_bk.hlstLightness > 0.75) ? 0.85 : 0.70)));
			dc->DrawRectangle(rect);

			int textTop = ceil((double)(rect.GetHeight() - WinUtil::fontHeight - 2) / 2);
			dc->SetFont(WinUtil::font);
			dc->SetTextForeground(backColour);
			dc->DrawText(value.GetText(), rect.GetLeft() + 6, rect.GetTop() + textTop);

			wxColour a = backColour.ChangeLightness(65);
			wxColour b = backColour.ChangeLightness(135);

			rect.SetWidth((int) ((int64_t)rect.GetWidth() * position / size) + 1);

			int w = rect.GetWidth() - 1;
			int h = rect.GetHeight() - 2;

			for (int _x = 0; _x <= w; ++_x) 
			{
				wxColour cr = WinUtil::blendColours(b, a, (double)(_x) / (double)w);
				for (int _y = 0; _y < h; ++_y) 
				{
					double lightness = blend_vector[(size_t)floor(((double)_y / h) * MAX_SHADE - 1)] / (double)SHADE_LEVEL;

					wxColour tmp(cr);
					dc->SetPen(wxPen(tmp.ChangeLightness(100 * (1.0 + lightness))));
					dc->DrawPoint(rect.GetLeft() + _x + 1, rect.GetTop() + _y + 1);
				}
			}

			dc->SetTextForeground(TextFromBackground(backColour));
			dc->SetClippingRegion(rect);
			dc->DrawText(value.GetText(), rect.GetLeft() + 6, rect.GetTop() + textTop);
		}
		else
		{
			RenderText(value.GetText(), 6, rect, dc, state);
		}

		return true;
	}

private:

	wxDataViewIconTextData value;

	typedef struct tagHLSTRIPLE 
	{
		DOUBLE hlstHue;
		DOUBLE hlstLightness;
		DOUBLE hlstSaturation;
	} HLSTRIPLE;

	inline HLSTRIPLE RGB2HLS(const wxColour& colour) 
	{
		double r = (double)colour.Red() / 255;
		double g = (double)colour.Green() / 255;
		double b = (double)colour.Blue() / 255;
		double m0 = MIN3(r, g, b), m2 = MAX3(r, g, b), d;
		HLSTRIPLE hlst = {0, -1, -1 };

		hlst.hlstLightness = (m2 + m0) / 2;
		d = (m2 - m0) / 2;
		if (hlst.hlstLightness <= 0.5) 
		{
			if(hlst.hlstLightness == 0) hlst.hlstLightness = 0.1;
			hlst.hlstSaturation = d / hlst.hlstLightness;
		} 
		else 
		{
			if(hlst.hlstLightness == 1) hlst.hlstLightness = 0.99;
			hlst.hlstSaturation = d / (1 - hlst.hlstLightness);
		}
		if (hlst.hlstSaturation > 0 && hlst.hlstSaturation < 1)
			hlst.hlstHue = RGB2HUE(r, g, b);
		return hlst;
	}

	double RGB2HUE(double r, double g, double b)
	{
		double H;
		double m2 = MAX3(r, g, b);
		double m1 = CENTER(r, g, b);
		double m0 = MIN3(r, g, b);

		if (m2 == m1) {
			if (r == g) {
				H = 60;
				goto _RGB2HUE_END;
			}
			if (g == b) {
				H = 180;
				goto _RGB2HUE_END;
			}
			H = 60;
			goto _RGB2HUE_END;
		}
		double F = 60 * (m1 - m0) / (m2 - m0);
		if (r == m2) {
			H = 0 + F * (g - b);
			goto _RGB2HUE_END;
		}
		if (g == m2) {
			H = 120 + F * (b - r);
			goto _RGB2HUE_END;
		}
		H = 240 + F * (r - g);

	_RGB2HUE_END:
		if (H < 0)
			H = H + 360;
		return H;
	}

	const wxColour& TextFromBackground(const wxColour& bg) 
	{
		HLSTRIPLE hlst = RGB2HLS(bg);
		if (hlst.hlstLightness > 0.63)
			return *wxBLACK;
		else
			return *wxWHITE;
	}

};

#endif	// _PROGRESSBARRENDERER_H
