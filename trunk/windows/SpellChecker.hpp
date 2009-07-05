/*
 * Copyright (C) 2007-2009 adrian_007, adrian-007 on o2 point pl
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

#ifndef SPELL_CHECKER_HPP
#define SPELL_CHECKER_HPP

#include "../client/Singleton.h"

class Hunspell;

class SpellChecker : public Singleton<SpellChecker> {
public:
	SpellChecker();
	~SpellChecker();

	void reload();

	bool isWordOk(const dcpp::tstring& word);
	void suggest(const dcpp::tstring& word, std::list<dcpp::tstring>& sl);
	void addWord(const dcpp::tstring& word);
	bool isRunning();
private:
	std::wstring toWide(const char* str) const;
	std::string fromWide(const std::wstring& str) const;

	unsigned encoding;
	Hunspell* dic;
};
#endif

/**
 * @file
 * $Id$
 */
