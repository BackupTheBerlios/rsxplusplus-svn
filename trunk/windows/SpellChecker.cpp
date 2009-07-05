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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/Text.h"
#include "../client/Util.h"
#include "../client/Codepages.h"
#include "../client/rsxppSettingsManager.h"

#include "SpellChecker.hpp"

#ifdef near
#undef near
#endif
#define HUNSPELL_STATIC 1
#include <hunspell/hunspell.hxx>
#include <hunspell/csutil.hxx>

void SpellChecker::reload() {
	string fname_t = Util::getConfigPath() + RSXPP_SETTING(DICTIONARY);
	if(!Util::fileExists(string(fname_t + ".dic")) || !Util::fileExists(string(fname_t + ".aff"))) {
		RSXPP_SET(DICTIONARY, Util::emptyString);
	} else {
		try {
			dic = new Hunspell(string(fname_t + ".aff").c_str(), string(fname_t + ".dic").c_str());

			int size = sizeof(codepages) / sizeof(codepages[0]);
			encoding = CP_UTF8;
			for(int i = 0; i < size; ++i) {
				if(strcmp(codepages[i].name, dic->get_dic_encoding()) == 0) {
					encoding = codepages[i].codepage;
					break;
				}
			}
			dcdebug("Loaded dictionary: %s\n", fname_t.c_str());
		} catch(...) {
			if(dic) delete dic;
			dic = 0;
			RSXPP_SET(DICTIONARY, Util::emptyString);
		}
	}
}

SpellChecker::SpellChecker() : dic(0) {

}

SpellChecker::~SpellChecker() {
	if(dic) {
		delete dic;
		dic = 0;
	}
}

bool SpellChecker::isWordOk(const dcpp::tstring& word) {
	if(!dic) return true;
	bool ret = true;
	try {
		ret = dic->spell(fromWide(word).c_str()) != 0;
	} catch(...) { }
	return ret;
}

void SpellChecker::suggest(const dcpp::tstring& word, std::list<dcpp::tstring>& sl) {
	if(dic) {
		sl.clear();
		try {
			char** list;
			int cnt = dic->suggest(&list, fromWide(word).c_str());
			for(int i = 0; i < cnt; ++i) {
				sl.push_back(toWide(list[i]));
			}
			dic->free_list(&list, cnt);
		} catch(...) { }
	}
}

void SpellChecker::addWord(const dcpp::tstring& word) {
	try {
		dic->add(Text::fromT(word).c_str());
	} catch(...) { }
	//@todo make own dic
}

bool SpellChecker::isRunning() {
	return dic != 0;
}

std::wstring SpellChecker::toWide(const char* str) const {
	std::wstring ret;
	ret.resize(MultiByteToWideChar(encoding, 0, str, -1, NULL, 0)+1);

	MultiByteToWideChar(encoding, 0, str, -1, &ret[0], ret.size());
	return ret;
}

std::string SpellChecker::fromWide(const std::wstring& str) const {
	std::string ret;
	ret.resize(str.length() + 1);

	WideCharToMultiByte(encoding, 0, str.c_str(), str.length(), &ret[0], ret.size(), NULL, NULL);
	return ret;
}

/**
 * @file
 * $Id$
 */
