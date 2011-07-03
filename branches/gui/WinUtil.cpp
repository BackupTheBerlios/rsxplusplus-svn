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
#include "WinUtil.h"

#include "CountryFlags.h"
#include "MainWindow.h"
#include "SearchFrame.h"
#include "MagnetDlg.h"

//RSX++
#include "ImageManager.h"
//END

#include "../client/QueueManager.h"
#include "../client/ShareManager.h"
#include "../client/StringTokenizer.h"
#include "../client/version.h"

#include <wx/clipbrd.h>
#include <wx/file.h>
#include <wx/mimetype.h>
#include <wx/notifmsg.h>
#include <wx/rawbmp.h>

MainWindow* WinUtil::mainWindow = NULL;

wxBrush WinUtil::bgBrush;
wxColour WinUtil::textColor;
wxColour WinUtil::bgColor;
wxFont WinUtil::font;
int	WinUtil::fontHeight;
wxFont WinUtil::boldFont;
wxFont WinUtil::systemFont;
wxFont WinUtil::smallBoldFont;

wxImageList WinUtil::userImages;
wxImageList WinUtil::fileImages;

WinUtil::ImageMap WinUtil::fileIndexes;
int WinUtil::dirIconIndex = 0;
int WinUtil::dirMaskedIndex = 0;
int WinUtil::fileImageCount;

wxRichTextAttr WinUtil::m_TextStyleTimestamp;
wxRichTextAttr WinUtil::m_ChatTextGeneral;
wxRichTextAttr WinUtil::m_TextStyleMyNick;
wxRichTextAttr WinUtil::m_ChatTextMyOwn;
wxRichTextAttr WinUtil::m_ChatTextServer;
wxRichTextAttr WinUtil::m_ChatTextSystem;
wxRichTextAttr WinUtil::m_TextStyleBold;
wxRichTextAttr WinUtil::m_TextStyleFavUsers;
wxRichTextAttr WinUtil::m_TextStyleOPs;
wxRichTextAttr WinUtil::m_TextStyleURL;
wxRichTextAttr WinUtil::m_ChatTextPrivate;
wxRichTextAttr WinUtil::m_ChatTextLog;

tstring WinUtil::tth;

TStringList WinUtil::lastDirs;

void WinUtil::init(MainWindow* _mainWindow)
{
	mainWindow = _mainWindow;

	userImages.Create(16, 16, false);
	if(SETTING(USERLIST_IMAGE) == "")
		userImages.Add(ImageManager::getInstance()->users);
	else
		userImages.Add(wxBitmap(Text::toT(SETTING(USERLIST_IMAGE)).c_str(), wxBITMAP_TYPE_BMP));

	fileImages.Create(16, 16, true);
	fileImages.Add(ImageManager::getInstance()->folders);
	dirIconIndex = fileImageCount++;
	dirMaskedIndex = fileImageCount++;

	fileImageCount++;

	// set default font
	systemFont = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	//SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_FONT, encodeFont(systemFont));

	// load set font
	decodeFont(SETTING(TEXT_FONT), font);
	fontHeight = WinUtil::getTextHeight(mainWindow, font);

	// create bold font
	boldFont = font;
	boldFont.SetWeight(wxFONTWEIGHT_BOLD);
	
	// create small bold font
	smallBoldFont = boldFont;
	smallBoldFont.SetPixelSize(wxSize(0, (smallBoldFont.GetPixelSize().GetY() * 5) / 6));

	initColors();

	CountryFlags::init();
}

void WinUtil::uninit()
{
}

void WinUtil::initColors() 
{
	bgBrush = wxBrush(SETTING(BACKGROUND_COLOR), wxSOLID);
	textColor = SETTING(TEXT_COLOR);
	bgColor = SETTING(BACKGROUND_COLOR);

	wxRichTextAttr cf;
	cf.SetFont(font);
	cf.SetBackgroundColour(SETTING(BACKGROUND_COLOR));
	cf.SetTextColour(SETTING(TEXT_COLOR));

	m_TextStyleTimestamp = cf;
	m_TextStyleTimestamp.SetBackgroundColour(SETTING(TEXT_TIMESTAMP_BACK_COLOR));
	m_TextStyleTimestamp.SetTextColour(SETTING(TEXT_TIMESTAMP_FORE_COLOR));
	if(SETTING(TEXT_TIMESTAMP_BOLD))
		m_TextStyleTimestamp.SetFontWeight(wxFONTWEIGHT_BOLD);
	if(SETTING(TEXT_TIMESTAMP_ITALIC))
		m_TextStyleTimestamp.SetFontStyle(wxFONTSTYLE_ITALIC);

	m_ChatTextGeneral = cf;
	m_ChatTextGeneral.SetBackgroundColour(SETTING(TEXT_GENERAL_BACK_COLOR));
	m_ChatTextGeneral.SetTextColour(SETTING(TEXT_GENERAL_FORE_COLOR));
	if(SETTING(TEXT_GENERAL_BOLD))
		m_ChatTextGeneral.SetFontWeight(wxFONTWEIGHT_BOLD);
	if(SETTING(TEXT_GENERAL_ITALIC))
		m_ChatTextGeneral.SetFontStyle(wxFONTSTYLE_ITALIC);

	m_TextStyleBold = m_ChatTextGeneral;
	m_TextStyleBold.SetFontWeight(wxFONTWEIGHT_BOLD);
	
	m_TextStyleMyNick = cf;
	m_TextStyleMyNick.SetBackgroundColour(SETTING(TEXT_MYNICK_BACK_COLOR));
	m_TextStyleMyNick.SetTextColour(SETTING(TEXT_MYNICK_FORE_COLOR));
	if(SETTING(TEXT_MYNICK_BOLD))
		m_TextStyleMyNick.SetFontWeight(wxFONTWEIGHT_BOLD);
	if(SETTING(TEXT_MYNICK_ITALIC))
		m_TextStyleMyNick.SetFontStyle(wxFONTSTYLE_ITALIC);

	m_ChatTextMyOwn = cf;
	m_ChatTextMyOwn.SetBackgroundColour(SETTING(TEXT_MYOWN_BACK_COLOR));
	m_ChatTextMyOwn.SetTextColour(SETTING(TEXT_MYOWN_FORE_COLOR));
	if(SETTING(TEXT_MYOWN_BOLD))
		m_ChatTextMyOwn.SetFontWeight(wxFONTWEIGHT_BOLD);
	if(SETTING(TEXT_MYOWN_ITALIC))
		m_ChatTextMyOwn.SetFontStyle(wxFONTSTYLE_ITALIC);

	m_ChatTextPrivate = cf;
	m_ChatTextPrivate.SetBackgroundColour(SETTING(TEXT_PRIVATE_BACK_COLOR));
	m_ChatTextPrivate.SetTextColour(SETTING(TEXT_PRIVATE_FORE_COLOR));
	if(SETTING(TEXT_PRIVATE_BOLD))
		m_ChatTextPrivate.SetFontWeight(wxFONTWEIGHT_BOLD);
	if(SETTING(TEXT_PRIVATE_ITALIC))
		m_ChatTextPrivate.SetFontStyle(wxFONTSTYLE_ITALIC);

	m_ChatTextSystem = cf;
	m_ChatTextSystem.SetBackgroundColour(SETTING(TEXT_SYSTEM_BACK_COLOR));
	m_ChatTextSystem.SetTextColour(SETTING(TEXT_SYSTEM_FORE_COLOR));
	if(SETTING(TEXT_SYSTEM_BOLD))
		m_ChatTextSystem.SetFontWeight(wxFONTWEIGHT_BOLD);
	if(SETTING(TEXT_SYSTEM_ITALIC))
		m_ChatTextSystem.SetFontStyle(wxFONTSTYLE_ITALIC);

	m_ChatTextServer = cf;
	m_ChatTextServer.SetBackgroundColour(SETTING(TEXT_SERVER_BACK_COLOR));
	m_ChatTextServer.SetTextColour(SETTING(TEXT_SERVER_FORE_COLOR));
	if(SETTING(TEXT_SERVER_BOLD))
		m_ChatTextServer.SetFontWeight(wxFONTWEIGHT_BOLD);
	if(SETTING(TEXT_SERVER_ITALIC))
		m_ChatTextServer.SetFontStyle(wxFONTSTYLE_ITALIC);

	m_ChatTextLog = m_ChatTextGeneral;
	m_ChatTextLog.SetTextColour(blendColours(SETTING(TEXT_GENERAL_FORE_COLOR), SETTING(TEXT_GENERAL_BACK_COLOR), 0.4));

	m_TextStyleFavUsers = cf;
	m_TextStyleFavUsers.SetBackgroundColour(SETTING(TEXT_FAV_BACK_COLOR));
	m_TextStyleFavUsers.SetTextColour(SETTING(TEXT_FAV_FORE_COLOR));
	if(SETTING(TEXT_FAV_BOLD))
		m_TextStyleFavUsers.SetFontWeight(wxFONTWEIGHT_BOLD);
	if(SETTING(TEXT_FAV_ITALIC))
		m_TextStyleFavUsers.SetFontStyle(wxFONTSTYLE_ITALIC);

	m_TextStyleOPs = cf;
	m_TextStyleOPs.SetBackgroundColour(SETTING(TEXT_OP_BACK_COLOR));
	m_TextStyleOPs.SetTextColour(SETTING(TEXT_OP_FORE_COLOR));
	if(SETTING(TEXT_OP_BOLD))
		m_TextStyleOPs.SetFontWeight(wxFONTWEIGHT_BOLD);
	if(SETTING(TEXT_OP_ITALIC))
		m_TextStyleOPs.SetFontStyle(wxFONTSTYLE_ITALIC);

	m_TextStyleURL = cf;
	m_TextStyleURL.SetBackgroundColour(SETTING(TEXT_URL_BACK_COLOR));
	m_TextStyleURL.SetTextColour(SETTING(TEXT_URL_FORE_COLOR));
	m_TextStyleURL.SetFontUnderlined(true);
	m_TextStyleURL.SetURL(wxEmptyString);
	if(SETTING(TEXT_URL_BOLD))
		m_TextStyleURL.SetFontWeight(wxFONTWEIGHT_BOLD);
	if(SETTING(TEXT_URL_ITALIC))
		m_TextStyleURL.SetFontStyle(wxFONTSTYLE_ITALIC);
}

wxColour WinUtil::blendColours(const wxColour& fg, const wxColour& bg, double alpha)
{
	unsigned char r = wxColour::AlphaBlend(fg.Red(), bg.Red(), alpha);
	unsigned char g = wxColour::AlphaBlend(fg.Green(), bg.Green(), alpha);
	unsigned char b = wxColour::AlphaBlend(fg.Blue(), bg.Blue(), alpha);
	
	return wxColour(r, g, b);
}

string WinUtil::encodeFont(const wxFont& font)
{
	string res(Text::fromT((const TCHAR*)font.GetFaceName().c_str()));
	res += L',';
	res += Util::toString(font.GetPixelSize().GetY());
	res += L',';
	res += Util::toString(font.GetWeight());
	res += L',';
	res += (font.GetStyle() == wxFONTSTYLE_ITALIC) ? '1' : '0';
	return res;
}

void WinUtil::decodeFont(const string& setting, wxFont &dest) 
{
	StringTokenizer<string> st(setting, _T(','));
	StringList &sl = st.getTokens();
	
	dest = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

	if(sl.size() == 4)
	{
		dest.SetFaceName(Text::toT(sl[0]));
		dest.SetPixelSize(wxSize(0, Util::toInt(sl[1])));

		// convert weight from old WinAPI values
		int weight = Util::toInt(sl[2]);
		switch(weight) 
		{
			case 0:		weight = wxFONTWEIGHT_NORMAL; break;
			case 300:	weight = wxFONTWEIGHT_LIGHT; break;
			case 400:	weight = wxFONTWEIGHT_NORMAL; break;
			case 700:	weight = wxFONTWEIGHT_BOLD; break;
		}

		dest.SetWeight(weight);
		dest.SetStyle((sl[3] == "1") ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
	}
}

void WinUtil::openLink(const tstring& url) 
{
	if(_strnicmp(Text::fromT(url).c_str(), "magnet:?", 8) == 0) 
	{
		openMagnetLink(url);
		return;
	}
	if(_strnicmp(Text::fromT(url).c_str(), "dchub://", 8) == 0) 
	{
		// TODO parseDchubUrl(url, false);
		return;
	}
	if(_strnicmp(Text::fromT(url).c_str(), "nmdcs://", 8) == 0) 
	{
		// TODO parseDchubUrl(url, true);
		return;
	}	
	if(_strnicmp(Text::fromT(url).c_str(), "adc://", 6) == 0) 
	{
		// TODO parseADChubUrl(url, false);
		return;
	}
	if(_strnicmp(Text::fromT(url).c_str(), "adcs://", 7) == 0)
	{
		// TODO parseADChubUrl(url, true);
		return;
	}	

	wxLaunchDefaultBrowser(url);
}

void WinUtil::openMagnetLink(const tstring& magnetLink)
{
	tstring fhash, fname;
	int64_t fsize = 0;

	if(parseMagnetUri(magnetLink, fhash, fname, fsize))
	{
		// ok, we have a hash, and maybe a filename.
		if(!BOOLSETTING(MAGNET_ASK) && fsize > 0 && fname.length() > 0) 
		{
			switch(SETTING(MAGNET_ACTION)) 
			{
				case SettingsManager::MAGNET_AUTO_DOWNLOAD:
					try 
					{
						QueueManager::getInstance()->add(SETTING(DOWNLOAD_DIRECTORY) + Text::fromT(fname), fsize, TTHValue(Text::fromT(fhash)), HintedUser(UserPtr(), Util::emptyString));
					} 
					catch(const Exception& e) 
					{
						LogManager::getInstance()->message(e.getError(), LogManager::LOG_ERROR);
					}
					break;
				case SettingsManager::MAGNET_AUTO_SEARCH:
					SearchFrame::openWindow(fhash, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
					break;
			};
		}
		else 
		{
			MagnetDlg dlg(NULL);
			dlg.setValues(fhash, fname, fsize);
			dlg.ShowModal();
		}
	}
	else
	{
		wxMessageBox(TSTRING(MAGNET_DLG_TEXT_BAD), TSTRING(MAGNET_DLG_TITLE), wxOK | wxICON_EXCLAMATION);
	}
}

bool WinUtil::parseMagnetUri(const tstring& magnetLink, tstring& fileHash, tstring& fileName, int64_t& fileSize) 
{
	// official types that are of interest to us
	//  xt = exact topic
	//  xs = exact substitute
	//  as = acceptable substitute
	//  dn = display name
	//  xl = exact length
	if (strnicmp(magnetLink.c_str(), _T("magnet:?"), 8) == 0) 
	{
		LogManager::getInstance()->message(STRING(MAGNET_DLG_TITLE) + ": " + Text::fromT(magnetLink));
		StringTokenizer<tstring> mag(magnetLink.substr(8), _T('&'));
		typedef map<tstring, tstring> MagMap;
		MagMap hashes;
		tstring type, param;
		for(TStringList::const_iterator idx = mag.getTokens().begin(); idx != mag.getTokens().end(); ++idx) 
		{
			// break into pairs
			string::size_type pos = idx->find(_T('='));
			if(pos != string::npos) 
			{
				type = Text::toT(Text::toLower(Util::encodeURI(Text::fromT(idx->substr(0, pos)), true)));
				param = Text::toT(Util::encodeURI(Text::fromT(idx->substr(pos+1)), true));
			} 
			else 
			{
				type = Text::toT(Util::encodeURI(Text::fromT(*idx), true));
				param.clear();
			}
			// extract what is of value
			if(param.length() == 85 && strnicmp(param.c_str(), _T("urn:bitprint:"), 13) == 0) {
				hashes[type] = param.substr(46);
			} else if(param.length() == 54 && strnicmp(param.c_str(), _T("urn:tree:tiger:"), 15) == 0) {
				hashes[type] = param.substr(15);
			} else if(param.length() == 55 && strnicmp(param.c_str(), _T("urn:tree:tiger/:"), 16) == 0) {
				hashes[type] = param.substr(16);
			} else if(param.length() == 59 && strnicmp(param.c_str(), _T("urn:tree:tiger/1024:"), 20) == 0) {
				hashes[type] = param.substr(20);
			} else if(type.length() == 2 && strnicmp(type.c_str(), _T("dn"), 2) == 0) {
				fileName = param;
			} else if(type.length() == 2 && strnicmp(type.c_str(), _T("xl"), 2) == 0) {
				fileSize = _tstoi64(param.c_str());
			}
		}
		// pick the most authoritative hash out of all of them.
		if(hashes.find(_T("as")) != hashes.end()) 
		{
			fileHash = hashes[_T("as")];
		}
		if(hashes.find(_T("xs")) != hashes.end()) 
		{
			fileHash = hashes[_T("xs")];
		}
		if(hashes.find(_T("xt")) != hashes.end()) 
		{
			fileHash = hashes[_T("xt")];
		}
		
		if(!fileHash.empty() && Encoder::isBase32(Text::fromT(fileHash).c_str()))
		{
			return true;
		} 
	}

	return false;
}

void WinUtil::addLastDir(const tstring& dir) 
{
	if(find(lastDirs.begin(), lastDirs.end(), dir) != lastDirs.end()) 
	{
		return;
	}
	if(lastDirs.size() == 10) 
	{
		lastDirs.erase(lastDirs.begin());
	}
	lastDirs.push_back(dir);
}

bool WinUtil::browseFile(tstring& target, wxWindow* owner, bool save, const tstring& initialDir, const TCHAR* types, const TCHAR* defExt)
{
	long style;
	if(save)
	{
		style = wxFD_SAVE | wxFD_OVERWRITE_PROMPT;
	}
	else
	{
		style = wxFD_OPEN | wxFD_FILE_MUST_EXIST;
	}

	wxFileDialog dlg(owner, wxFileSelectorPromptStr, initialDir, target, defExt, style | wxFD_CHANGE_DIR);
	if(dlg.ShowModal() == wxID_OK)
	{
		target = dlg.GetPath();
		return true;
	}

	return false;
}

bool WinUtil::browseDirectory(tstring& target, wxWindow* owner)
{
	wxDirDialog dlg(owner, TSTRING(CHOOSE_FOLDER), target, wxDD_CHANGE_DIR);
	if (dlg.ShowModal() == wxID_OK)
	{
		target = dlg.GetPath();
		
		if(target.size() > 0 && target[target.size()-1] != _T('\\'))
			target+=_T('\\');
			
		return true;	
	}

	return false;
}

void WinUtil::setButtonPressed(int nID, bool pressed /* = true */) 
{
	if (nID == -1)
		return;

	mainWindow->getToolBar()->ToggleTool(nID, pressed);
	mainWindow->getToolBar()->Refresh();
}

void WinUtil::setClipboard(const wxString& str)
{
    if (wxTheClipboard->Open())
    {
        // This data objects are held by the clipboard,
        // so do not delete them in the app.
        wxTheClipboard->SetData(new wxTextDataObject(str));
        wxTheClipboard->Close();
    }
}

void  WinUtil::bitziLink(const TTHValue& aHash)
{
	// to use this free service by bitzi, we must not hammer or request information from bitzi
	// except when the user requests it (a mass lookup isn't acceptable), and (if we ever fetch
	// this data within DC++, we must identify the client/mod in the user agent, so abuse can be 
	// tracked down and the code can be fixed
	openLink(_T("http://bitzi.com/lookup/tree:tiger:") + Text::toT(aHash.toBase32()));
}

void  WinUtil::searchHash(const TTHValue& aHash)
{
	SearchFrame::openWindow(Text::toT(aHash.toBase32()), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
}

bool WinUtil::getUCParams(wxWindow* parent, const UserCommand& uc, StringMap& sm) throw() 
{
	string::size_type i = 0;
	StringMap done;

	while( (i = uc.getCommand().find("%[line:", i)) != string::npos) 
	{
		i += 7;
		string::size_type j = uc.getCommand().find(']', i);
		if(j == string::npos)
			break;

		string name = uc.getCommand().substr(i, j-i);
		if(done.find(name) == done.end()) 
		{
			/* TODO LineDlg dlg;
			dlg.title = Text::toT(Util::toString(" > ", uc.getDisplayName()));
			dlg.description = Text::toT(name);
			dlg.line = Text::toT(sm["line:" + name]);

			if(uc.adc()) 
			{
				Util::replace(_T("\\\\"), _T("\\"), dlg.description);
				Util::replace(_T("\\s"), _T(" "), dlg.description);
			}

			if(dlg.DoModal(parent) == IDOK) 
			{
				sm["line:" + name] = Text::fromT(dlg.line);
				done[name] = Text::fromT(dlg.line);
			} 
			else */
			{
				return false;
			}
		}
		i = j + 1;
	}
	i = 0;
	while( (i = uc.getCommand().find("%[kickline:", i)) != string::npos) 
	{
		i += 11;
		string::size_type j = uc.getCommand().find(']', i);
		if(j == string::npos)
			break;

		string name = uc.getCommand().substr(i, j-i);
		if(done.find(name) == done.end()) 
		{
			/* TODO KickDlg dlg;
			dlg.title = Text::toT(Util::toString(" > ", uc.getDisplayName()));
			dlg.description = Text::toT(name);

			if(uc.adc()) 
			{
				Util::replace(_T("\\\\"), _T("\\"), dlg.description);
				Util::replace(_T("\\s"), _T(" "), dlg.description);
			}

			if(dlg.DoModal(parent) == IDOK) 
			{
				sm["kickline:" + name] = Text::fromT(dlg.line);
				done[name] = Text::fromT(dlg.line);
			} 
			else */
			{
				return false;
			}
		}
		i = j + 1;
	}
	return true;
}

tstring WinUtil::getNicks(const CID& cid, const string& hintUrl) 
{
	return Text::toT(Util::toString(ClientManager::getInstance()->getNicks(cid, hintUrl)));
}

tstring WinUtil::getNicks(const UserPtr& u, const string& hintUrl) 
{
	return getNicks(u->getCID(), hintUrl);
}

tstring WinUtil::getNicks(const CID& cid, const string& hintUrl, bool priv) 
{
	return Text::toT(Util::toString(ClientManager::getInstance()->getNicks(cid, hintUrl, priv)));
}

static pair<tstring, bool> formatHubNames(const StringList& hubs) 
{
	if(hubs.empty())
	{
		return make_pair(CTSTRING(OFFLINE), false);
	} 
	else 
	{
		return make_pair(Text::toT(Util::toString(hubs)), true);
	}
}

pair<tstring, bool> WinUtil::getHubNames(const CID& cid, const string& hintUrl) 
{
	return formatHubNames(ClientManager::getInstance()->getHubNames(cid, hintUrl));
}

pair<tstring, bool> WinUtil::getHubNames(const UserPtr& u, const string& hintUrl) 
{
	return getHubNames(u->getCID(), hintUrl);
}

pair<tstring, bool> WinUtil::getHubNames(const CID& cid, const string& hintUrl, bool priv) 
{
	return formatHubNames(ClientManager::getInstance()->getHubNames(cid, hintUrl, priv));
}

int WinUtil::getTextWidth(const tstring& str, wxWindow* wnd) 
{
	wxClientDC dc(wnd);
	dc.SetFont(wnd->GetFont());
	wxSize sz = dc.GetTextExtent(str);

	return sz.x;
}

int WinUtil::getTextHeight(wxWindow* wnd, const wxFont& fnt) 
{
	wxClientDC dc(wnd);
	dc.SetFont(fnt);
	return dc.GetCharHeight();
}

int WinUtil::getIconIndex(const tstring& aFileName) 
{
	if(BOOLSETTING(USE_SYSTEM_ICONS)) 
	{
		tstring ext = Text::toLower(Util::getFileExt(aFileName));
		if(!ext.empty()) 
		{
			ImageIter j = fileIndexes.find(ext);
			if(j != fileIndexes.end())
				return j->second;

			wxFileType* fileType = wxTheMimeTypesManager->GetFileTypeFromExtension(ext);
			if(fileType != NULL)
			{
				wxIconLocation iconLoc;
				if(fileType->GetIcon(&iconLoc))
				{
					delete fileType;

					wxString fullname = iconLoc.GetFileName();
					if(wxFile::Exists(iconLoc.GetFileName()))
					{
						// wxWidgets can load wxIcon from wxIconLocation but doesn't allow specifying icon dimensions
						if(iconLoc.GetIndex())
						{
							fullname << wxT(';') << iconLoc.GetIndex();
						}

						fileImages.Add(wxIcon(fullname, wxBITMAP_TYPE_ICO, 16, 16));
						fileIndexes[ext] = fileImageCount++;
						return fileImageCount - 1;
					}
				}
			}
		}
	}

	return 2;
}

void WinUtil::splitTokens(int* array, const string& tokens, int maxItems /* = -1 */) throw() 
{
	StringTokenizer<string> t(tokens, _T(','));
	StringList& l = t.getTokens();
	if(maxItems == -1)
		maxItems = l.size();
	
	int k = 0;
	for(StringList::const_iterator i = l.begin(); i != l.end() && k < maxItems; ++i, ++k) 
	{
		array[k] = Util::toInt(*i);
	}
}

void WinUtil::saveHeaderOrder(wxListView& ctrl, SettingsManager::StrSetting order, 
	SettingsManager::StrSetting widths, int n, int* indexes, int* sizes) throw() 
{
/* TODO	string tmp;

	ctrl.GetColumnOrderArray(n, indexes);
	int i;
	for(i = 0; i < n; ++i) {
		tmp += Util::toString(indexes[i]);
		tmp += ',';
	}
	tmp.erase(tmp.size()-1, 1);
	SettingsManager::getInstance()->set(order, tmp);
	tmp.clear();
	int nHeaderItemsCount = ctrl.GetHeader().GetItemCount();
	for(i = 0; i < n; ++i) {
		sizes[i] = ctrl.GetColumnWidth(i);
		if (i >= nHeaderItemsCount) // Not exist column
			sizes[i] = 0;
		tmp += Util::toString(sizes[i]);
		tmp += ',';
	}
	tmp.erase(tmp.size()-1, 1);
	SettingsManager::getInstance()->set(widths, tmp);*/
}


#define LINE2 _T("-- http://strongdc.sf.net  <") COMPLETEVERSIONSTRING _T(">")

TCHAR *msgs[] = { _T("\r\n-- I'm a happy StrongDC++ user. You could be happy too.\r\n") LINE2,
_T("\r\n-- CZ-...what? Nope...never heard of it...\r\n") LINE2,
_T("\r\n-- Evolution of species: Ape --> Man\r\n-- Evolution of science: \"The Earth is Flat\" --> \"The Earth is Round\"\r\n-- Evolution of sharing: DC++ --> StrongDC++\r\n") LINE2,
_T("\r\n-- I share, therefore I am.\r\n") LINE2,
_T("\r\n-- I came, I searched, I found...\r\n") LINE2,
_T("\r\n-- I came, I shared, I sent...\r\n") LINE2,
_T("\r\n-- I can add multiple users to the same file and download from them simultaneously :)\r\n") LINE2,
_T("\r\n-- Ja jsem byl prvni klient, ktery nemuze ztratit slot pri segmentovem stahovani :-P\r\n") LINE2,
_T("\r\n-- Ja jsem byl prvni klient, ktery umi seskupovat prenosy se stejnym nazvem souboru :-P\r\n") LINE2,
};

#define MSGS 15

tstring WinUtil::commands = _T("\t\t\t\t\t HELP \t\t\t\t\t\t\t\t\n------------------------------------------------------------------------------------------------------------------------------------------------------------\n/refresh \t\t\t\t(obnoveni share) \t\t\t\t\t\t\n/savequeue \t\t\t\t(ulozi Download Queue) \t\t\t\t\t\t\n------------------------------------------------------------------------------------------------------------------------------------------------------------\n/search <string> \t\t\t(hledat neco...) \t\t\t\t\t\t\t\n/g <searchstring> \t\t\t(hledat Googlem) \t\t\t\t\t\t\n/imdb <imdbquery> \t\t\t(hledat film v IMDB databazi) \t\t\t\t\t\n/whois [IP] \t\t\t\t(hledat podrobnosti o IP) \t\t\t\t\t\n------------------------------------------------------------------------------------------------------------------------------------------------------------\n/slots # \t\t\t\t(upload sloty) \t\t\t\t\t\t\t\n/extraslots # \t\t\t\t(extra sloty pro male soubory) \t\t\t\t\t\n/smallfilesize # \t\t\t\t(maximalni velikost malych souboru) \t\t\t\t\n/ts \t\t\t\t\t(zobrazi datum a cas u zprav v mainchatu) \t\t\t\n/connection \t\t\t\t(zobrazi IP a port prez ktery jste pripojen) \t\t\t\t\n/showjoins \t\t\t\t(zapne/vypne zobrazovani a odpojovani useru v mainchatu) \t\n/showblockedipports \t\t\t(zobrazi zablokovane porty-mozna:)) \t\t\t\t\n/shutdown \t\t\t\t(vypne pocitac po urcitem timeoutu) \t\t\t\t\n------------------------------------------------------------------------------------------------------------------------------------------------------------\n/dc++ \t\t\t\t\t(zobrazi verzi DC++ v mainchatu) \t\t\t\t\t\n/strongdc++ \t\t\t\t(zobrazi verzi StrongDC++ v mainchatu) \t\t\t\t\n------------------------------------------------------------------------------------------------------------------------------------------------------------\n/away <msg> \t\t\t\t(zapne/vypne away mod) \t\t\t\t\t\n/winamp \t\t\t\t(Winamp spam v mainchatu) \t\t\t\t\t\n/w \t\t\t\t\t(Winamp spam v mainchatu) \t\t\t\t\t\n/clear,/c \t\t\t\t(smazat obsah mainchatu) \t\t\t\t\t\n/ignorelist \t\t\t\t(zobrazi ignorelist v mainchatu) \t\t\t\t\t\n");

bool WinUtil::checkCommand(tstring& cmd, tstring& param, tstring& message, tstring& status, bool& thirdPerson) 
{
	string::size_type i = cmd.find(' ');
	if(i != string::npos) 
	{
		param = cmd.substr(i+1);
		cmd = cmd.substr(1, i - 1);
	}
	else 
	{
		cmd = cmd.substr(1);
	}

	if(stricmp(cmd.c_str(), _T("log")) == 0) 
	{
		if(stricmp(param.c_str(), _T("system")) == 0) 
		{
			WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + "system.log")));
		} 
		else if(stricmp(param.c_str(), _T("downloads")) == 0) 
		{
			WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatTime(SETTING(LOG_FILE_DOWNLOAD), time(NULL)))));
		} 
		else if(stricmp(param.c_str(), _T("uploads")) == 0) 
		{
			WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatTime(SETTING(LOG_FILE_UPLOAD), time(NULL)))));
		} 
		else 
		{
			return false;
		}
	} 
	else if(stricmp(cmd.c_str(), _T("me")) == 0) 
	{
		message = param;
		thirdPerson = true;
	} 
	else if(stricmp(cmd.c_str(), _T("refresh"))==0) 
	{
		try 
		{
			ShareManager::getInstance()->setDirty();
			ShareManager::getInstance()->refresh(true);
		} 
		catch(const ShareException& e) 
		{
			status = Text::toT(e.getError());
		}
	} 
	else if(stricmp(cmd.c_str(), _T("slots"))==0) 
	{
		int j = Util::toInt(Text::fromT(param));
		if(j > 0) 
		{
			SettingsManager::getInstance()->set(SettingsManager::SLOTS, j);
			status = TSTRING(SLOTS_SET);
			ClientManager::getInstance()->infoUpdated();
		} 
		else 
		{
			status = TSTRING(INVALID_NUMBER_OF_SLOTS);
		}
	} 
	else if(stricmp(cmd.c_str(), _T("search")) == 0) 
	{
		if(!param.empty()) 
		{
			SearchFrame::openWindow(param);
		} 
		else 
		{
			status = TSTRING(SPECIFY_SEARCH_STRING);
		}
	} 
	else if(stricmp(cmd.c_str(), _T("strongdc++")) == 0) 
	{
		message = msgs[GET_TICK() % MSGS];
	} 
	else if(stricmp(cmd.c_str(), _T("away")) == 0) 
	{
		if(Util::getAway() && param.empty()) 
		{
			Util::setAway(false);
			mainWindow->getToolBar()->ToggleTool(ID_CMD_AWAY, false);
			status = TSTRING(AWAY_MODE_OFF);
		} 
		else 
		{
			Util::setAway(true);
			mainWindow->getToolBar()->ToggleTool(ID_CMD_AWAY, true);
			Util::setAwayMessage(Text::fromT(param));
			
			StringMap sm;
			status = TSTRING(AWAY_MODE_ON) + _T(" ") + Text::toT(Util::getAwayMessage(sm));
		}
		ClientManager::getInstance()->infoUpdated();
	} 
	else if(stricmp(cmd.c_str(), _T("g")) == 0) 
	{
		if(param.empty()) 
		{
			status = TSTRING(SPECIFY_SEARCH_STRING);
		} 
		else 
		{
			WinUtil::openLink(_T("http://www.google.com/search?q=") + Text::toT(Util::encodeURI(Text::fromT(param))));
		}
	} 
	else if(stricmp(cmd.c_str(), _T("imdb")) == 0) 
	{
		if(param.empty()) 
		{
			status = TSTRING(SPECIFY_SEARCH_STRING);
		} 
		else 
		{
			WinUtil::openLink(_T("http://www.imdb.com/find?q=") + Text::toT(Util::encodeURI(Text::fromT(param))));
		}
	} 
	else if(stricmp(cmd.c_str(), _T("u")) == 0)
	{
		if (!param.empty())
			WinUtil::openLink(Text::toT(Util::encodeURI(Text::fromT(param))));
	} 
	else if(stricmp(cmd.c_str(), _T("rebuild")) == 0)
	{
		HashManager::getInstance()->rebuild();
	} 
	else if(stricmp(cmd.c_str(), _T("shutdown")) == 0) 
	{
		mainWindow->setShutDown(!mainWindow->getShutDown());
		status = mainWindow->getShutDown() ? TSTRING(SHUTDOWN_ON) : TSTRING(SHUTDOWN_OFF);
	} 
	else if(stricmp(cmd.c_str(), _T("tvtome")) == 0) 
	{
		if(param.empty()) 
		{
			status = TSTRING(SPECIFY_SEARCH_STRING);
		} 
		else
		{
			WinUtil::openLink(_T("http://www.tvtome.com/tvtome/servlet/Search?searchType=all&searchString=") + Text::toT(Util::encodeURI(Text::fromT(param))));
		}
	} 
	else if(stricmp(cmd.c_str(), _T("csfd")) == 0) 
	{
		if(param.empty()) 
		{
			status = TSTRING(SPECIFY_SEARCH_STRING);
		} 
		else 
		{
			WinUtil::openLink(_T("http://www.csfd.cz/search.php?search=") + Text::toT(Util::encodeURI(Text::fromT(param))));
		}
	} 
	else 
	{
		return false;
	}

	return true;
}

string WinUtil::formatTime(uint64_t rest) 
{
	char buf[128];
	string formatedTime;
	uint64_t n, i;
	i = 0;
	n = rest / (24*3600*7);
	rest %= (24*3600*7);
	if(n) 
	{
		if(n >= 2)
			snprintf(buf, sizeof(buf), "%d weeks ", n);
		else
			snprintf(buf, sizeof(buf), "%d week ", n);
		formatedTime += (string)buf;
		i++;
	}
	n = rest / (24*3600);
	rest %= (24*3600);
	if(n) 
	{
		if(n >= 2)
			snprintf(buf, sizeof(buf), "%d days ", n); 
		else
			snprintf(buf, sizeof(buf), "%d day ", n);
		formatedTime += (string)buf;
		i++;
	}
	n = rest / (3600);
	rest %= (3600);
	if(n) 
	{
		if(n >= 2)
			snprintf(buf, sizeof(buf), "%d hours ", n);
		else
			snprintf(buf, sizeof(buf), "%d hour ", n);
		formatedTime += (string)buf;
		i++;
	}
	n = rest / (60);
	rest %= (60);
	if(n) 
	{
		snprintf(buf, sizeof(buf), "%d min ", n);
		formatedTime += (string)buf;
		i++;
	}
	n = rest;
	if(++i <= 3) 
	{
		snprintf(buf, sizeof(buf),"%d sec ", n); 
		formatedTime += (string)buf;
	}
	return formatedTime;
}

tstring WinUtil::generateStats() 
{
	TCHAR buf[1024];
	_sntprintf(buf, sizeof(buf), _T("\n-=[ wx%s ]=-\r\n-=[ Uptime: %s ]=-\r\n-=[ Downloaded: %s ][ Uploaded: %s ]=-\r\n-=[ Total download: %s ][ Total upload: %s ]=-\r\n-=[ System Uptime: %s ]=-"),
		COMPLETEVERSIONSTRING, formatTime(time(NULL) - Util::getStartTime()).c_str(),
		Util::formatBytesW(Socket::getTotalDown()).c_str(), Util::formatBytesW(Socket::getTotalUp()).c_str(), 
		Util::formatBytesW(SETTING(TOTAL_DOWNLOAD)).c_str(), Util::formatBytesW(SETTING(TOTAL_UPLOAD)).c_str(), 
		formatTime(GET_TICK() / 1000).c_str());
	return buf;
}

wxMenu* WinUtil::getPreviewMenu(const tstring& fileName)
{
	string fileExt = Text::fromT(Util::getFileExt(fileName));
	
	wxMenu* menu = new wxMenu();
	menu->SetTitle(TSTRING(PREVIEW_MENU));

	int j = 0;
	PreviewApplication::List lst = FavoriteManager::getInstance()->getPreviewApps();
	for(PreviewApplication::Iter i = lst.begin(); i != lst.end(); ++i, ++j)
	{
		StringList tok = StringTokenizer<string>((*i)->getExtension(), ';').getTokens();
		bool add = tok.empty();

		for(StringIter si = tok.begin(); si != tok.end(); ++si) 
		{
			if(_stricmp(fileExt.c_str(), si->c_str()) == 0)
			{
				add = true;
				break;
			}
		}
							
		if(add) 
			menu->Append(IDC_PREVIEW_APP + j, Text::toT(((*i)->getName())));
	}

	return menu;
}

void WinUtil::premultiplyAlpha(wxBitmap& bmp)
{
	bmp.UseAlpha();

#ifdef _WIN32	// hope that this is really needed for Windows only

	// wxWidgets always undo premultiplication of alpha channel before drawing
	// but it's wrong in case when image wasn't loaded via wxImage constructor
	BITMAP bm = { 0 };
	GetObject(bmp.GetHBITMAP(), sizeof(bm), &bm);
	if(bm.bmBitsPixel == 32) {
		BYTE *pBits = new BYTE[bm.bmWidth * bm.bmHeight * 4];
		GetBitmapBits(bmp.GetHBITMAP(), bm.bmWidth * bm.bmHeight * 4, pBits);
		
		// fix alpha channel	
		for (int y = 0; y < bm.bmHeight; y++) {
			BYTE * pPixel = (BYTE *) pBits + bm.bmWidth * 4 * y;

			for (int x = 0; x < bm.bmWidth; x++) {
				pPixel[0] = pPixel[0] * pPixel[3] / 255; 
				pPixel[1] = pPixel[1] * pPixel[3] / 255; 
				pPixel[2] = pPixel[2] * pPixel[3] / 255; 

				pPixel += 4;
			}
		}
		SetBitmapBits(bmp.GetHBITMAP(), bm.bmWidth * bm.bmHeight * 4, pBits);
	    
		delete[] pBits;
	}
#endif

}

void WinUtil::asyncEvent(wxEvtHandler* evtHandler, int message, int command, void* data)
{
	wxCommandEvent cmd(message, command);
	cmd.SetClientData(data);
	wxPostEvent(evtHandler, cmd);
}

void WinUtil::notifyUser(const tstring& title, const tstring& text, int icon)
{
	wxNotificationMessage notify(title, text);
	notify.SetFlags(icon);
#ifdef __WXMSW__
	// ensure that current taskbar icon is used
	// FIXME: notify.UseTaskBarIcon(mainWindow->getTaskBarIcon());
#endif
	notify.Show();
}
