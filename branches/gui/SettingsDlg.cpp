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
#include "SettingsDlg.h"

#include "SettingsPage.h"

#include "GeneralPage.h"
#include "NetworkPage.h"
#include "ProxyPage.h"
#include "DownloadPage.h"
#include "FavoriteDirsPage.h"
#include "PreviewPage.h"
#include "QueuePage.h"
#include "SharePage.h"
#include "UploadPage.h"
#include "AppearancePage.h"
#include "TextStylesPage.h"
//#include "ProgressColorsPage.h"
#include "UserListPage.h"
#include "PopupsPage.h"
#include "SoundsPage.h"
//#include "ToolbarPage.h"
#include "WindowsPage.h"
#include "AdvancedPage.h"
#include "ExpertsPage.h"
#include "LogsPage.h"
//#include "CommandsPage.h"
#include "LimitsPage.h"
#include "FakePage.h"
//#include "ClientsPage.h"
#include "SecurityPage.h"

#include <wx/bookctrl.h>
#include <wx/treebook.h>

BEGIN_EVENT_TABLE(SettingsDlg, wxPropertySheetDialog)
	EVT_BUTTON(wxID_OK, SettingsDlg::OnOK)
END_EVENT_TABLE()

SettingsDlg::SettingsDlg(wxWindow* parent, SettingsManager* s)
{
	SetSheetStyle(wxPROPSHEET_TREEBOOK);
	Create(parent, wxID_ANY, TSTRING(SETTINGS)/*, wxDefaultPosition, wxSize(660, 520)*/);
	CreateButtons();

	wxTreebook* tree = (wxTreebook*)GetBookCtrl();

	wxImageList* images = new wxImageList(16, 16);
	images->Add(wxBitmap(wxT("IDB_SETTINGS")), wxBITMAP_TYPE_BMP_RESOURCE);
	tree->AssignImageList(images);

	tree->AddPage(new GeneralPage(tree, s), CTSTRING(SETTINGS_GENERAL), false, 0);
	tree->AddPage(new NetworkPage(tree, s), CTSTRING(SETTINGS_NETWORK), false, 1);
	tree->InsertSubPage(SP_NETWORK, new ProxyPage(tree, s), _("Proxy"), false, 1);	// TODO: some icon for proxy
	tree->InsertSubPage(SP_NETWORK, new LimitsPage(tree, s), CTSTRING(SETTINGS_LIMIT), false, 20);
	tree->AddPage(new DownloadPage(tree, s), CTSTRING(SETTINGS_DOWNLOADS), false, 2);
	tree->InsertSubPage(SP_DOWNLOAD, new FavoriteDirsPage(tree, s), CTSTRING(SETTINGS_FAVORITE_DIRS_PAGE), false, 17);
	tree->InsertSubPage(SP_DOWNLOAD, new PreviewPage(tree, s), CTSTRING(SETTINGS_AVIPREVIEW), false, 18);
	tree->InsertSubPage(SP_DOWNLOAD, new QueuePage(tree, s), CTSTRING(SETTINGS_QUEUE), false, 19);
	tree->AddPage(new SharePage(tree, s), CTSTRING(SETTINGS_UPLOADS), false, 3);
	tree->AddPage(new UploadPage(tree, s), CTSTRING(SETTINGS_UPLOADS_SLOTS), false, 4);
	tree->AddPage(new AppearancePage(tree, s), CTSTRING(SETTINGS_APPEARANCE), false, 5);
	tree->InsertSubPage(SP_APPEARANCE, new TextStylesPage(tree, s), CTSTRING(SETTINGS_TEXT_STYLES), false, 6);
	//tree->InsertSubPage(SP_TEXT_STYLES, new ProgressColorsPage(tree, s), CTSTRING(SETTINGS_OPERACOLORS), false, 8);
	tree->InsertSubPage(SP_TEXT_STYLES, new UserListPage(tree, s), CTSTRING(SETTINGS_USER_COLORS), false, 11);
	tree->InsertSubPage(SP_APPEARANCE, new PopupsPage(tree, s), CTSTRING(BALLOON_POPUPS), false, 7);
	tree->InsertSubPage(SP_APPEARANCE, new SoundsPage(tree, s), CTSTRING(SETTINGS_SOUNDS), false, 9);
	//tree->InsertSubPage(SP_APPEARANCE, new ToolbarPage(tree, s), CTSTRING(SETTINGS_TOOLBAR), false, 10);
	tree->InsertSubPage(SP_APPEARANCE, new WindowsPage(tree, s), CTSTRING(SETTINGS_WINDOWS), false, 12);
	tree->AddPage(new AdvancedPage(tree, s), CTSTRING(SETTINGS_ADVANCED), false, 13);
	tree->InsertSubPage(SP_ADVANCED, new ExpertsPage(tree, s), CTSTRING(SETTINGS_ADVANCED3), false, 14);
	tree->InsertSubPage(SP_ADVANCED, new LogsPage(tree, s), CTSTRING(SETTINGS_LOGS), false, 15);
	//tree->InsertSubPage(SP_ADVANCED, new CommandsPage(tree, s), CTSTRING(SETTINGS_USER_COMMANDS), false, 16);
	tree->InsertSubPage(SP_ADVANCED, new FakePage(tree, s), CTSTRING(SETTINGS_FAKEDETECT), false, 21);
	//tree->InsertSubPage(SP_FAKEDETECT, new ClientsPage(tree, s), CTSTRING(SETTINGS_ADVANCED3), false, 22);
	tree->InsertSubPage(SP_ADVANCED, new SecurityPage(tree, s), CTSTRING(SETTINGS_CERTIFICATES), false, 23);

	LayoutDialog();
	Center();
}

SettingsDlg::~SettingsDlg(void)
{
}

void SettingsDlg::OnOK(wxCommandEvent& event)
{
	for(unsigned int i = 0; i < GetBookCtrl()->GetPageCount(); ++i)
	{
		SettingsPage* page = (SettingsPage*)GetBookCtrl()->GetPage(i);
		page->write();
	}

	event.Skip();
}
