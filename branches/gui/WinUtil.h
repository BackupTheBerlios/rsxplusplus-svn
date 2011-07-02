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

#ifndef _WINUTIL_H
#define _WINUTIL_H

#include "../client/CID.h"
#include "../client/MerkleTree.h"
#include "../client/SettingsManager.h"
#include "../client/User.h"

#include <wx/aui/aui.h>
#include <wx/listctrl.h>
#include <wx/richtext/richtextbuffer.h>
#include <wx/utils.h> 
#include <wx/wx.h>

class MainWindow;

class WinUtil
{
public:
	
	static MainWindow* mainWindow;

	// appearance variables
	static wxBrush	bgBrush;
	static wxColour textColor;
	static wxColour bgColor;
	static wxFont	font;
	static int		fontHeight;
	static wxFont	boldFont;
	static wxFont	systemFont;
	static wxFont	smallBoldFont;

	// imagelists
	static wxImageList userImages;
	static wxImageList fileImages;

	static int dirIconIndex;
	static int dirMaskedIndex;
	static int fileImageCount;
	
	typedef unordered_map<tstring, int> ImageMap;
	typedef ImageMap::const_iterator ImageIter;
	static ImageMap fileIndexes;

	// chat styles
	static wxRichTextAttr m_TextStyleTimestamp;
	static wxRichTextAttr m_ChatTextGeneral;
	static wxRichTextAttr m_TextStyleMyNick;
	static wxRichTextAttr m_ChatTextMyOwn;
	static wxRichTextAttr m_ChatTextServer;
	static wxRichTextAttr m_ChatTextSystem;
	static wxRichTextAttr m_TextStyleBold;
	static wxRichTextAttr m_TextStyleFavUsers;
	static wxRichTextAttr m_TextStyleOPs;
	static wxRichTextAttr m_TextStyleURL;
	static wxRichTextAttr m_ChatTextPrivate;
	static wxRichTextAttr m_ChatTextLog;

	static tstring	tth;

	static TStringList lastDirs;

	static void init(MainWindow* _mainWindow);
	static void uninit();

	static void initColors();
	static wxColour blendColours(const wxColour& fg, const wxColour& bg, double alpha);

	static string encodeFont(const wxFont& font);
	static void decodeFont(const string& setting, wxFont &dest);

	static void openFile(const tstring& file) { wxLaunchDefaultApplication(file); }
	static void openLink(const tstring& url);
	static void openMagnetLink(const tstring& magnetLink);

	static bool parseMagnetUri(const tstring& magnetLink, tstring& fileHash, tstring& fileName, int64_t& fileSize);

	static void addLastDir(const tstring& dir);
	static bool browseFile(tstring& target, wxWindow* owner = NULL, bool save = true, const tstring& initialDir = Util::emptyStringW, const TCHAR* types = NULL, const TCHAR* defExt = NULL);
	static bool browseDirectory(tstring& target, wxWindow* owner = NULL);

	static void setButtonPressed(int nID, bool pressed = true);
	static void setClipboard(const wxString& str);

	static void bitziLink(const TTHValue& /*aHash*/);
	static void searchHash(const TTHValue& /*aHash*/);

	static bool getUCParams(wxWindow* parent, const UserCommand& cmd, StringMap& sm) throw();

	static tstring getNicks(const CID& cid, const string& hintUrl);
	static tstring getNicks(const UserPtr& u, const string& hintUrl);
	static tstring getNicks(const CID& cid, const string& hintUrl, bool priv);
	static tstring getNicks(const HintedUser& user) { return getNicks(user.user->getCID(), user.hint); }

	/** @return Pair of hubnames as a string and a bool representing the user's online status */
	static pair<tstring, bool> getHubNames(const CID& cid, const string& hintUrl);
	static pair<tstring, bool> getHubNames(const UserPtr& u, const string& hintUrl);
	static pair<tstring, bool> getHubNames(const CID& cid, const string& hintUrl, bool priv);
	static pair<tstring, bool> getHubNames(const HintedUser& user) { return getHubNames(user.user->getCID(), user.hint); }

	static int getTextWidth(const tstring& str, wxWindow* wnd);
	static int getTextHeight(wxWindow* wnd, const wxFont& fnt);

	static int getIconIndex(const tstring& aFileName);
	static int getDirIconIndex() { return dirIconIndex; }
	static int getDirMaskedIndex() { return dirMaskedIndex; }

	static void splitTokens(int* array, const string& tokens, int maxItems = -1) throw();
	static void saveHeaderOrder(wxListView& ctrl, SettingsManager::StrSetting order, 
		SettingsManager::StrSetting widths, int n, int* indexes, int* sizes) throw();

	/* Check if this is a common /-command.
	 * @param cmd The whole text string, will be updated to contain only the command.
	 * @param param Set to any parameters.
	 * @param message Message that should be sent to the chat.
	 * @param status Message that should be shown in the status line.
	 * @return True if the command was processed, false otherwise.
	 */
	static tstring commands;
	static bool checkCommand(tstring& cmd, tstring& param, tstring& message, tstring& status, bool& thirdPerson);
	static string formatTime(uint64_t rest);
	static tstring generateStats();

	static wxMenu* getPreviewMenu(const tstring& fileName);

	static void asyncEvent(wxEvtHandler* evtHandler, int message, int command = 0, void* data = NULL);

	static void premultiplyAlpha(wxBitmap& bmp);
	static void notifyUser(const tstring& title, const tstring& text, int icon = wxICON_INFORMATION);

};

#endif	// _WINUTIL_H