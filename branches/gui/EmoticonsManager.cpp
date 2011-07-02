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
#include "EmoticonsManager.h"

#include "WinUtil.h"

#include "../client/DCPlusPlus.h"
#include "../client/SimpleXML.h"

#include <math.h>

Emoticon::Emoticon(const tstring& _emoticonText, const string& _imagePath) : 
	emoticonText(_emoticonText), imagePath(_imagePath)
{
	emoticonBitmap = wxBitmap(Text::toT(imagePath), wxBITMAP_TYPE_BMP);
	if(!emoticonBitmap.IsOk())
		return;

	emoticonBitmap.UseAlpha();
}

wxBitmap Emoticon::getEmoticonBmp(const wxColour& clrBkColor) 
{
	wxMemoryDC DirectDC;
	
	// FIXME: use 24bit color depth here, because wxWidgets undo alpha premultiplication for 32bit in Richedit (bug or feature?)
	wxBitmap DirectBitmap(emoticonBitmap.GetWidth(), emoticonBitmap.GetHeight(), 24);

	DirectDC.SelectObject(DirectBitmap);
	DirectDC.SetBackground(clrBkColor);
	DirectDC.Clear();
	DirectDC.DrawBitmap(emoticonBitmap, 0, 0, true);

	return DirectBitmap;
}

void EmoticonsManager::Load() 
{
	setUseEmoticons(false);

	if((SETTING(EMOTICONS_FILE) == "Disabled") || !Util::fileExists(Util::getPath(Util::PATH_EMOPACKS) + SETTING(EMOTICONS_FILE) + ".xml" )) 
	{
		return;
	}

	try 
	{
		SimpleXML xml;
		xml.fromXML(File(Util::getPath(Util::PATH_EMOPACKS) + SETTING(EMOTICONS_FILE) + ".xml", File::READ, File::OPEN).read());
		
		if(xml.findChild("Emoticons")) 
		{
			xml.stepIn();

			while(xml.findChild("Emoticon")) 
			{
				tstring strEmotionText = Text::toT(xml.getChildAttrib("PasteText"));
				if (strEmotionText.empty()) 
				{
					strEmotionText = Text::toT(xml.getChildAttrib("Expression"));
				}
				
				string strEmotionBmpPath = xml.getChildAttrib("Bitmap");
				if (!strEmotionBmpPath.empty()) {
					if (strEmotionBmpPath[0] == '.') 
					{
						// change relative path
						strEmotionBmpPath = Util::getPath(Util::PATH_EMOPACKS) + strEmotionBmpPath;
					} 
					else 
					{
						strEmotionBmpPath = "EmoPacks\\" + strEmotionBmpPath;
					}
				}

				emoticons.push_back(new Emoticon(strEmotionText, strEmotionBmpPath));
			}
			xml.stepOut();
		}
	} 
	catch(const Exception& e) 
	{
		dcdebug("EmoticonsManager::Create: %s\n", e.getError().c_str());
		return;
	}
	
	setUseEmoticons(true);
}

void EmoticonsManager::Unload() 
{
	for_each(emoticons.begin(), emoticons.end(), DeleteFunction());
	emoticons.clear();
}
