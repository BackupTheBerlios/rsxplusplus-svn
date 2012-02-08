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

#ifndef RSXPLUSPLUS_LIBHUNSPELL_HPP
#define RSXPLUSPLUS_LIBHUNSPELL_HPP

class Hunspell;

class libhunspell {
public:
	libhunspell();
	~libhunspell();

	void open(const std::string& dic, const std::string& aff);
	void close();

	bool spell(const dcpp::tstring& word);
	void suggest(const dcpp::tstring& word, std::list<dcpp::tstring>& list);

	bool isRunning() {
		return dll && ptr && ok;
	}

private:
	std::wstring toWide(const char* str) const;
	std::string fromWide(const std::wstring& str) const;
	void prepare();

	Hunspell* ptr;
	HMODULE dll;

	bool ok;
	unsigned encoding;

	typedef void*	(__cdecl *dll_hunspell_initialize)			(char*, char*);
	typedef void	(__cdecl *dll_hunspell_uninitialize)		(Hunspell*);
	typedef int		(__cdecl *dll_hunspell_spell)				(Hunspell*, char*);
	typedef int		(__cdecl *dll_hunspell_suggest)				(Hunspell*, char*, char***);
	typedef void	(__cdecl *dll_hunspell_free_list)			(Hunspell*, char***, int);
	typedef char*	(__cdecl *dll_hunspell_get_dic_encoding)	(Hunspell*);

	dll_hunspell_initialize			hunspell_initialize;
	dll_hunspell_uninitialize		hunspell_uninitialize;
	dll_hunspell_spell				hunspell_spell;
	dll_hunspell_suggest			hunspell_suggest;
	dll_hunspell_free_list			hunspell_free_list;
	dll_hunspell_get_dic_encoding	hunspell_get_dic_encoding;
};

#endif

/**
 * @file
 * $Id: libhunspell.hpp 230 2011-01-30 18:15:33Z adrian_007 $
 */
