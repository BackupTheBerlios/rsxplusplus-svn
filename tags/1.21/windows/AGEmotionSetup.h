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

#if !defined(AGEMOTIONSETUP_H__)
#define AGEMOTIONSETUP_H__

#include "../client/FastAlloc.h"
#include "../client/Util.h"
#include "ResourceLoader.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Emoticon

class Emoticon : public FastAlloc<Emoticon> {
public:
	typedef list<Emoticon*> List;
	typedef List::const_iterator Iter;

	Emoticon(const tstring& strEmotionText, const string& strEmotionBmpPath);
	~Emoticon() {	}
	
	const tstring& getEmotionText() const { return m_EmotionText; }
	HBITMAP getEmotionBmp() const {	return m_EmotionBmp; }
	HBITMAP getEmotionBmp(const COLORREF &clrBkColor);
	const string& getEmotionBmpPath() const { return m_EmotionBmpPath; }

private:
	tstring		m_EmotionText;
	string		m_EmotionBmpPath;
	ExCImage	m_EmotionBmp;
};

// EmoticonSetup

class EmoticonSetup : public intrusive_ptr_base<EmoticonSetup> {
public:
	EmoticonSetup() { Load(); }
	~EmoticonSetup() { Unload(); }

	// Variables
	GETSET(bool, useEmoticons, UseEmoticons);

	const Emoticon::List& getEmoticonsList() const { return EmotionsList; }

	void Load();
	void Unload();
	
private:
	Emoticon::List EmotionsList;
};

#endif // AGEMOTIONSETUP_H__