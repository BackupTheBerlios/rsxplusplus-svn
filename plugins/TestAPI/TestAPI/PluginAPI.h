/* 
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

#ifndef PLUGIN_API_H
#define PLUGIN_API_H

#ifndef _STLP_MAP
# include <map>
#endif

class Client;

class iClient {
public:
	virtual const std::wstring getHubName(Client* /*c*/) const { return _T(""); }
	virtual const std::wstring getHubUrl(Client* /*c*/) const { return _T(""); }

	virtual void sendHubMessage(Client* /*client*/, const std::wstring& /*aMsg*/) { }
	virtual void addHubLine(Client* /*client*/, const std::wstring& /*aMsg*/, int /*type*/ = 0) { }
};

class iPluginAPI : public iClient {
public:
	virtual void logMessage(const std::wstring& /*aMsg*/) { }
	virtual int getIntSetting(const std::wstring& /*sName*/) { return 0; }
	virtual int64_t getInt64Setting(const std::wstring& /*sName*/) { return 0; }
	virtual std::wstring getStringSetting(const std::wstring& /*sName*/) { return _T(""); }
	virtual HWND getMainWnd() { return NULL; }
	virtual int getSVNRevision() { return 0; }
	virtual const std::wstring getClientProfileVersion() const { return _T(""); }
	virtual const std::wstring getMyInfoProfileVersion() const { return _T(""); }
	virtual bool RegExpMatch(const std::wstring& /*strToMatch*/, const std::wstring& /*regexp*/, const std::wstring& /*opt*/ = _T("")) { return false; }
	virtual bool WildcardMatch(const std::wstring& /*strToMatch*/, const std::wstring& /*pattern*/, std::wstring& /*delim*/) { return false; }
	virtual void setSetting(const std::wstring& /*pName*/, const std::wstring& /*sName*/, const std::wstring& /*sValue*/) { }
	virtual const std::wstring getSetting(const std::wstring& /*pName*/, const std::wstring& /*sName*/) const { return _T(""); }
	virtual map<std::wstring, std::wstring> getSettings(const std::wstring& /*pName*/) = 0;
	virtual const std::wstring getDataPath() const { return _T(""); }
};

class PluginAPI {
public:
	PluginAPI() : iApi(NULL) { };
	virtual ~PluginAPI() { };

	virtual const std::wstring getPluginName() const = 0;
	virtual const std::wstring getPluginVersion() const = 0;
	virtual const std::wstring getPluginDescription() const = 0;
	virtual const std::wstring getPluginAuthor() const = 0;
	virtual const int getPluginID() const = 0;
	virtual HBITMAP getPluginIcon() = 0;

	virtual void onLoad() = 0;
	virtual void onUnload() = 0;
	virtual void setDefaults() = 0;

	virtual void onToolbarClick() = 0;
	virtual bool onHubEnter(Client* client, const wstring& aMessage) = 0;
	virtual bool onHubMessage(Client* client, const wstring& aMessage) = 0;

	void setInterface(iPluginAPI* i) {
		if(i == NULL && iApi != NULL) {
			delete iApi;
		}
		iApi = i; 
	}

	iPluginAPI* getAPI() { return iApi; }

	void setSetting(const std::wstring& settingName, const std::wstring& settingVal) {
		getAPI()->setSetting(getPluginName(), settingName, settingVal);
	}

	std::wstring getSetting(const std::wstring& settingName) {
		return getAPI()->getSetting(getPluginName(), settingName);
	}

	map<wstring, wstring> getSettings() {
		return getAPI()->getSettings(getPluginName());
	}
private:
	iPluginAPI* iApi;
};
#endif

/**
 * @file
 * $Id: PluginAPI.h 42 2007-10-31 18:27:40Z adrian_007 $
 */