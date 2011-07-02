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
#include "SettingsPage.h"

#include "../client/SettingsManager.h"

SettingsPage::SettingsPage(wxWindow* parent, SettingsManager* s, wxWindowID id) : wxPanel(parent, id), settings(s)
{
}

SettingsPage::~SettingsPage(void)
{
}

void SettingsPage::read(Item const* items, ListItem* listItems /* = NULL */, wxCheckListBox* list /* = 0 */)
{
	bool const useDef = true;
	for(Item const* i = items; i->type != T_END; i++)
	{
		wxWindow* item = FindWindow(i->itemID);
		if (item == NULL)
		{
			// Control not exist ? Why ??
			throw;
		}

		switch(i->type)
		{
		case T_STR:
			dynamic_cast<wxTextEntry*>(item)->SetValue(Text::toT(settings->get((SettingsManager::StrSetting)i->setting, useDef)));
			break;
		case T_INT:
			((wxSpinCtrl*)item)->SetValue(settings->get((SettingsManager::IntSetting)i->setting, useDef));
			break;
		case T_INT64:
			if(!SettingsManager::getInstance()->isDefault(i->setting))
			{
				((wxSpinCtrl*)item)->SetValue(settings->get((SettingsManager::Int64Setting)i->setting, useDef));
			}
			break;
		case T_BOOL:
			((wxCheckBox*)item)->SetValue(settings->getBool((SettingsManager::IntSetting)i->setting, useDef));
		}
	}

	if(listItems != NULL)
	{
		for(int i = 0; listItems[i].setting != 0; i++)
		{
			list->Append(CTSTRING_I(listItems[i].desc));
			list->Check(i, SettingsManager::getInstance()->getBool(SettingsManager::IntSetting(listItems[i].setting), true));
		}
	}
}

void SettingsPage::write(Item const* items, ListItem* listItems /* = NULL */, wxCheckListBox* list /* = NULL */)
{
	tstring buf;

	for(Item const* i = items; i->type != T_END; i++)
	{
		wxWindow* item = FindWindow(i->itemID);
		if (item == NULL)
		{
			// Control not exist ? Why ??
			throw;
		}

		switch(i->type)
		{
			case T_STR:
				settings->set((SettingsManager::StrSetting)i->setting, Text::fromT(dynamic_cast<wxTextEntry*>(item)->GetValue()));
				break;

			case T_INT:
				settings->set((SettingsManager::IntSetting)i->setting, ((wxSpinCtrl*)item)->GetValue());
				break;

			case T_INT64:
				settings->set((SettingsManager::Int64Setting)i->setting, ((wxSpinCtrl*)item)->GetValue());
				break;

			case T_BOOL:
				settings->set((SettingsManager::IntSetting)i->setting, ((wxCheckBox*)item)->IsChecked());
		}
	}

	if(listItems != NULL)
	{
		int i;
		for(i = 0; listItems[i].setting != 0; i++)
		{
			SettingsManager::getInstance()->set(SettingsManager::IntSetting(listItems[i].setting), list->IsChecked(i));
		}
	}
}

void SettingsPage::translate(TextItem* textItems)
{
	if (textItems != NULL) {
		for(int i = 0; textItems[i].itemID != 0; i++)
		{
			FindWindow(textItems[i].itemID)->SetLabel(CTSTRING_I(textItems[i].translatedString));
		}
	}
}
