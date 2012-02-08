/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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
#include "../client/Util.h"
#include "../client/Codepages.h"

#include "libhunspell.hpp"

#ifdef _WIN64
#define HUNSPELL_DLL_FILENAME _T("libhunspell64.dll")
#else
#define HUNSPELL_DLL_FILENAME _T("libhunspell32.dll")
#endif

libhunspell::libhunspell() : ok(false), ptr(0) {

}

libhunspell::~libhunspell() {
	if(ok) {
		close();
	}
}

void libhunspell::open(const std::string& dic, const std::string& aff) {
	prepare();
	if(!ok) return;

	ptr = (Hunspell*)hunspell_initialize((char*)aff.c_str(), (char*)dic.c_str());

	std::string dicEncoding(hunspell_get_dic_encoding(ptr));
			
	int size = sizeof(codepages) / sizeof(codepages[0]);
	encoding = CP_UTF8;
	for(int i = 0; i < size; ++i) {
		if(strcmp(codepages[i].name, dicEncoding.c_str()) == 0) {
			encoding = codepages[i].codepage;
			break;
		}
	}
	ok = true;
}

void libhunspell::close() {
	if(ok) {
		ok = false;
	}
}

bool libhunspell::spell(const dcpp::tstring& word) {
	if(!isRunning()) return false;

	int x = hunspell_spell(ptr, (char*)fromWide(word).c_str());
	return x == 0 ? true : false;
}

void libhunspell::suggest(const dcpp::tstring& word, std::list<dcpp::tstring>& list) {
	if(!isRunning()) return;
	char** sl = 0;
	int size = hunspell_suggest(ptr, (char*)fromWide(word).c_str(), &sl);
	for(int i = 0; i < size; ++i) {
		list.push_back(toWide(sl[i]));
	}
	hunspell_free_list(ptr, &sl, size);
}

std::wstring libhunspell::toWide(const char* str) const {
	std::wstring ret;
	ret.resize(MultiByteToWideChar(encoding, 0, str, -1, NULL, 0)+1);

	MultiByteToWideChar(encoding, 0, str, -1, &ret[0], ret.size());
	return ret;
}

std::string libhunspell::fromWide(const std::wstring& str) const {
	std::string ret;
	ret.resize(str.length() + 1);

	WideCharToMultiByte(encoding, 0, str.c_str(), str.length(), &ret[0], ret.size(), NULL, NULL);
	return ret;
}

void libhunspell::prepare() {
	ok = false;
	dcpp::tstring path(Text::toT(Util::getPath(Util::PATH_RESOURCES)));
	path += HUNSPELL_DLL_FILENAME;

	dll = LoadLibrary(path.c_str());
	if(dll) {
		hunspell_initialize =			reinterpret_cast<dll_hunspell_initialize>(GetProcAddress(dll,		"hunspell_initialize"));
		hunspell_uninitialize =			reinterpret_cast<dll_hunspell_uninitialize>(GetProcAddress(dll,		"hunspell_uninitialize"));
		hunspell_spell =				reinterpret_cast<dll_hunspell_spell>(GetProcAddress(dll,			"hunspell_spell"));
		hunspell_suggest =				reinterpret_cast<dll_hunspell_suggest>(GetProcAddress(dll,			"hunspell_suggest"));
		hunspell_free_list =			reinterpret_cast<dll_hunspell_free_list>(GetProcAddress(dll,		"hunspell_free_list"));
		hunspell_get_dic_encoding =		reinterpret_cast<dll_hunspell_get_dic_encoding>(GetProcAddress(dll, "hunspell_get_dic_encoding"));

		if(
			!hunspell_initialize || 
			!hunspell_uninitialize || 
			!hunspell_spell ||
			!hunspell_suggest || 
			!hunspell_free_list || 
			!hunspell_get_dic_encoding
			) {
				FreeLibrary(dll);
				dll = 0;
				return;
		} else {
			ok = true;
		}
	}
}

/**
 * @file
 * $Id: libhunspell.cpp 230 2011-01-30 18:15:33Z adrian_007 $
 */
