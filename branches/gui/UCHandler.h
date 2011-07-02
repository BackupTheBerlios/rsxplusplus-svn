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

#ifndef _UCHANDLER_H
#define _UCHANDLER_H

#include "../client/FavoriteManager.h"

#include <wx/wx.h>

template<class T>
class UCHandler 
{
public:

	void prepareMenu(wxMenu& menu, int ctx, const string& hubUrl) 
	{
		prepareMenu(menu, ctx, StringList(1, hubUrl));
	}

	void prepareMenu(wxMenu& menu, int ctx, const StringList& hubs) 
	{
		bool isOp = false;
		userCommands = FavoriteManager::getInstance()->getUserCommands(ctx, hubs, isOp);
		isOp = isOp && (ctx != UserCommand::CONTEXT_HUB);
		int n = 0;
		int m = 0;
		
		if(!userCommands.empty() || isOp) 
		{
			wxMenu *cur, *subMenu = NULL;

			menu.AppendSeparator();
			
			if(BOOLSETTING(UC_SUBMENU)) 
			{
				subMenu = new wxMenu();
				subMenu->SetTitle(TSTRING(SETTINGS_USER_COMMANDS));
				
				menu.AppendSubMenu(subMenu, TSTRING(SETTINGS_USER_COMMANDS));
				cur = subMenu;
			}
			else
			{
				cur = &menu;
			}

			if(isOp) 
			{
				cur->Append(IDC_REPORT, CTSTRING(REPORT));
				cur->Append(IDC_CHECKLIST, CTSTRING(CHECK_FILELIST));
				cur->AppendSeparator();
			} 

			for(UserCommand::List::iterator ui = userCommands.begin(); ui != userCommands.end(); ++ui) 
			{
				UserCommand& uc = *ui;
				if(uc.getType() == UserCommand::TYPE_SEPARATOR) 
				{
					// Avoid double separators...
					if( (cur->GetMenuItemCount() >= 1) &&
						!(cur->FindItemByPosition(cur->GetMenuItemCount() - 1)->IsSeparator()))
					{
						cur->AppendSeparator();
						m++;
					}
				} 
				else if(uc.isRaw() || uc.isChat()) 
				{
					cur = BOOLSETTING(UC_SUBMENU) ? subMenu : &menu;
					for(StringList::const_iterator i = uc.getDisplayName().begin(), iend = uc.getDisplayName().end(); i != iend; ++i) 
					{
						tstring name = Text::toT(*i);
						if(i + 1 == iend) 
						{
							cur->Append(IDC_USER_COMMAND + n, name);
#pragma message(__FILE__ ": fix Bind method call")
							//((T*)this)->Bind(wxEVT_COMMAND_MENU_SELECTED, std::bind(&T::runUserCommand, static_cast<T*>(this), std::cref(uc)), IDC_USER_COMMAND + n);
							m++;
						} 
						else 
						{
							bool found = false;
							TCHAR buf[1024];
							// Let's see if we find an existing item...
							for(int k = 0; k < cur->GetMenuItemCount(); k++) 
							{
								wxMenuItem* item = cur->FindItemByPosition(k);
								if(item->IsSubMenu()) 
								{
									if(item->GetText() == name)
									{
										found = true;
										cur = item->GetSubMenu();
									}
								}
							}
							if(!found) 
							{
								wxMenu* m = new wxMenu();
								cur->AppendSubMenu(m, name);
								cur = m;
							}
						}
					}
				}
				n++;
			}
		}
	}

private:
	
	UserCommand::List	userCommands;

};
 
 #endif	// _UCHANDLER_H