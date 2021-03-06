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

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "SimpleXML.h"

#include "rsxppSettingsManager.h"
#include "PluginsManager.h"

namespace dcpp {
const string rsxppSettingsManager::settingTags[] =
{
	// Strings
	"ProtectedUsers", 
	"UpdateProfiles", "UpdateIpWatch", "UpdateMyInfoProfilesUrl",
	"HighPrioFiles", "LowPrioFiles", "ShareSkiplist", "DownloadSkiplist", "ProfileVerUrl", "Dictionary", 
	"SENTRY", 
	// Ints
	"CheckDelay", "SleepTime", "MaxFilelists", "MaxTestSURs", "UseSendDelayedRaw", 
	"FavoriteUserIsProtectedUser", "CheckAllClientsBeforeFilelists", "UncheckClientProtectedUser",
	"UncheckListProtectedUser", "MaxTimeouts", "MaxDisconnects", "DisplayCheatsInMainChat",
	"ShowShareCheckedUsers", "DeleteCheckedFilelists", "UsePointsAdlSearch", "UseIpWatch",
	"IgnoreUserRegExpOrWC", "AutoUpdateIpOnStartup", "SlowDownloadSpeed", "SlowDownloadTime",
	"IgnorePMSpammers", "DefaultProcessPriority", "FlashWindowOnPM", "FlashWindowOnNewPM", "AutoStart",

	"TimeoutRaw",
	"DisconnectRaw", 
	"FakeShareRaw",
	"ListLenMismatch", 
	"FilelistTooSmallOrBig",
	"FilelistUnavailable",
	"NoTTHF",
	"VersionMisMatch", 
	"MyInfoSpamKick",
	"CTMSpamKick", 
	"ADLSearchDefaultAction",
	"HashErrorRaw", 
	"rmDCRaw",
	"DCPlusPlusEmulationRaw", 
	"FilelistVersionMismatch",
	"SlowDownloadRaw", 
	"PMSpamKick",
	"FakeSlotCount",

	"ShowTimeoutRaw",
	"ShowDisconnectRaw", 
	"ShowFakeShareRaw",
	"ShowLisLenMismatch", 
	"ShowFilelistTooSmallOrBig",
	"ShowFilelistUnavailable", 
	"ShowNoTTHF",
	"ShowVersionMisMatch", 
	"ShowMyInfoSpamKick",
	"ShowCTMSpamKick", 
	"ShowADLSearchDefaultAction",
	"ShowHashErrorRaw", 
	"ShowrmDCRaw",
	"ShowDCPlusPlusEmulationRaw", 
	"ShowFilelistVersionMismatch",
	"ShowSlowDownloadRaw", 
	"ShowPMSpamKick",
	"ShowFakeSlotCount",

	"MinPointsToDisplayCheat",
	"MyInfoCounter", "MyInfoTime", "CTMCounter", "CTMTime", "PMCounter", "PMTime",
	"PercentHashed", "PercentFakeShareTolerated", "MaxFilelistSize", "MinFilelistSize", 

	"UseChatFilter", "AutosearchEnable", "AutosearchTime", "AutosearchEvery", 
	"AutosearchRecheckTime", "UseHighLight", "GetUpdateProfile", "UseFiltersOnNewHubs", "UseHighlightsOnNewHubs", "ProtectPMUser",
	"UseSlowDownloadKick", "IpInChat", "CountryInChat",
	"ShowClientNewVer", "ShowMyInfoNewVer", "ShowIpWatchNewVer", "UseWildcardsToProtect",
	"UpdateProfileCheats", "UpdateProfileComments", "ShowLuaErrorMessage", "MaxChatBufsize",
	"SENTRY",
	// Int64
	"TotalDetects", "TotalFailedDetects", "TotalRawCommandsSent",
	"SENTRY"
};

rsxppSettingsManager::rsxppSettingsManager() {
	SettingsManager::getInstance()->addListener(this);
	for(int i=0; i<SETTINGS_LAST; i++)
		isSet[i] = false;

	for(int j=0; j<INT_LAST-INT_FIRST; j++) {
		intDefaults[j] = 0;
		intSettings[j] = 0;
	}
	for(int k=0; k<INT64_LAST-INT64_FIRST; k++) {
		int64Defaults[k] = 0;
		int64Settings[k] = 0;
	}

	setDefault(UPDATE_URL,			"http://dcaml.svn.sourceforge.net/svnroot/dcaml/");
	setDefault(UPDATE_MYINFOS,		"http://dcaml.svn.sourceforge.net/svnroot/dcaml/");
	setDefault(UPDATE_IPWATCH_URL,	"http://dcaml.svn.sourceforge.net/svnroot/dcaml/");
	setDefault(PROFILE_VER_URL,		"http://dcaml.svn.sourceforge.net/svnroot/dcaml/");

	setDefault(MAX_FILELISTS,					10);
	setDefault(MAX_TESTSURS,					20);
	setDefault(CHECK_DELAY,						1200);
	setDefault(SLEEP_TIME,						800);
	setDefault(MAX_TIMEOUTS,					9);
	setDefault(MAX_DISCONNECTS,					9);
	setDefault(MYINFO_COUNTER,					5);
	setDefault(MYINFO_TIME,						1500);
	setDefault(CTM_COUNTER,						5);
	setDefault(CTM_TIME,						1500);
	setDefault(PM_COUNTER,						5);
	setDefault(PM_TIME,							1500);
	setDefault(AUTOSEARCH_EVERY,				5);
	setDefault(AUTOSEARCH_RECHECK_TIME,			30);
	setDefault(SDL_SPEED,						100);
	setDefault(SDL_TIME,						60);
	setDefault(PERCENT_HASHED,					10);
	setDefault(PERCENT_FAKE_SHARE_TOLERATED,	20);
	setDefault(MINIMUM_FILELIST_SIZE,			10);
	setDefault(MAXIMUM_FILELIST_SIZE,			10*1024*1024);
	setDefault(MIN_POINTS_TO_DISPLAY_CHEAT,		0);

	setDefault(USE_SEND_DELAYED_RAW,				true);
	setDefault(FAV_USER_IS_PROTECTED_USER,			true);
	setDefault(CHECK_ALL_CLIENTS_BEFORE_FILELISTS,	true);
	setDefault(UNCHECK_CLIENT_PROTECTED_USER,		true);
	setDefault(UNCHECK_LIST_PROTECTED_USER,			true);
	setDefault(USE_IPWATCH,							true);
	setDefault(AUTOSEARCH_ENABLED,					true);
	setDefault(AUTOSEARCH_ENABLED_TIME,				true);
	setDefault(USE_SDL_KICK,						true);
	setDefault(IGNORE_USE_REGEXP_OR_WC,				true);
	setDefault(IGNORE_PM_SPAMMERS,					true);
	setDefault(PROTECT_PM_USERS,					true);
	setDefault(SHOW_SHARE_CHECKED_USERS,			true);
	setDefault(DISPLAY_CHEATS_IN_MAIN_CHAT,			true);
	setDefault(DELETE_CHECKED_FILELISTS,			true);
	setDefault(FLASH_WINDOW_ON_PM,					false);
	setDefault(FLASH_WINDOW_ON_NEW_PM,				true);
	setDefault(AUTO_START,							false);
	setDefault(SHOW_CLIENT_NEW_VER,					true);
	setDefault(SHOW_MYINFO_NEW_VER,					true);
	setDefault(SHOW_IPWATCH_NEW_VER,				true);

	//raws
	setDefault(TIMEOUT_RAW,					0);
	setDefault(DISCONNECT_RAW,				0);
	setDefault(FAKESHARE_RAW,				0);
	setDefault(LISTLEN_MISMATCH,			0);
	setDefault(FILELIST_TOO_SMALL_BIG,		0);
	setDefault(FILELIST_NA,					0);
	setDefault(VERSION_MISMATCH,			0);
	setDefault(MYINFO_SPAM_KICK,			0);
	setDefault(CTM_SPAM_KICK,				0);
	setDefault(ADLSEARCH_DEFAULT_ACTION,	0);
	setDefault(HASH_ERROR_RAW,				0);
	setDefault(RMDC_RAW,					0);
	setDefault(DCPP_EMULATION_RAW,			0);
	setDefault(FILELIST_VERSION_MISMATCH,	0);
	setDefault(SDL_RAW,						0);
	setDefault(PM_SPAM_KICK,				0);
	setDefault(FAKE_SLOT_COUNT,				0);

	//display cheats
	setDefault(SHOW_TIMEOUT_RAW,				true);
	setDefault(SHOW_DISCONNECT_RAW,				true);
	setDefault(SHOW_FAKESHARE_RAW,				true);
	setDefault(SHOW_LISTLEN_MISMATCH,			true);
	setDefault(SHOW_FILELIST_TOO_SMALL_BIG,		true);
	setDefault(SHOW_FILELIST_NA,				true);
	setDefault(SHOW_VERSION_MISMATCH,			true);
	setDefault(SHOW_MYINFO_SPAM_KICK,			true);
	setDefault(SHOW_CTM_SPAM_KICK,				true);
	setDefault(SHOW_ADLSEARCH_DEFAULT_ACTION,	true);
	setDefault(SHOW_HASH_ERROR_RAW,				true);
	setDefault(SHOW_RMDC_RAW,					true);
	setDefault(SHOW_DCPP_EMULATION_RAW,			true);
	setDefault(SHOW_FILELIST_VERSION_MISMATCH,	true);
	setDefault(SHOW_SDL_RAW,					true);
	setDefault(SHOW_PM_SPAM_KICK,				true);
	setDefault(SHOW_FAKE_SLOT_COUNT,			true);

	//strings
	setDefault(PROTECTED_USERS,				Util::emptyString);
	setDefault(HIGH_PRIO_FILES,				"*.sfv;*.nfo;*sample*;*cover*;*.pls;*.m3u");
	setDefault(LOW_PRIO_FILES,				"");
	setDefault(SKIPLIST_SHARE,				"*.ioFTPD;*.checked;*.raidenftpd.acl;*.SimSfvChk.log;*.message;Descript.ion;*.upChk.log;thumbs.db;*.crc;*.temp;Desktop.ini");
	setDefault(SKIPLIST_DOWNLOAD,			"*.ioFTPD;*.checked;*.raidenftpd.acl;*.SimSfvChk.log;*.message;Descript.ion;*.upChk.log;thumbs.db;*.crc;*.temp;Desktop.ini");
	setDefault(DICTIONARY,					Util::emptyString);

	//others
	setDefault(USE_CHAT_FILTER,				false);
	setDefault(USE_HIGHLIGHT,				false);
	setDefault(IPUPDATE,					false);
	setDefault(GET_UPDATE_PROFILE,			true);
	setDefault(USE_FILTER_FAV,				false);
	setDefault(USE_HL_FAV,					false);
	setDefault(IP_IN_CHAT,					false);
	setDefault(COUNTRY_IN_CHAT,				false);
	setDefault(USE_WILDCARDS_TO_PROTECT,	false);
	setDefault(DEFAULT_PRIO,				3); //normal prio
	setDefault(UPDATE_PROFILE_CHEATS,		false);
	setDefault(UPDATE_PROFILE_COMMENTS,		false);
	setDefault(SHOW_LUA_ERROR_MESSAGE,		true);
	setDefault(MAX_CHAT_BUFSIZE,			25000);
}

rsxppSettingsManager::~rsxppSettingsManager() {
	SettingsManager::getInstance()->removeListener(this);
	for(ExtActions::const_iterator i = ext_actions.begin(); i != ext_actions.end(); ++i)
		delete *i;
	ext_actions.clear();
}

void rsxppSettingsManager::addAction(int id, int actionId, bool display, std::string name, std::string cheat) {
	Lock l(cs);
	for(ExtActions::const_iterator i = ext_actions.begin(); i != ext_actions.end(); ++i)
		if((*i)->id == id || stricmp((*i)->name, name) == 0) return;
	
	ExtAction* a = new ExtAction;
	a->id = id;
	a->display = display;
	a->actionId = actionId;
	a->name = name;
	a->cheat = cheat;
	ext_actions.push_back(a);
}

int rsxppSettingsManager::getActionId(int id) {
	Lock l(cs);
	for(ExtActions::const_iterator i = ext_actions.begin(); i != ext_actions.end(); ++i)
		if((*i)->id == id) return (*i)->actionId;
	return 0;
}

void rsxppSettingsManager::on(Load, SimpleXML& xml) throw() { 
	try {
		if(xml.findChild("RSXPlusPlusSettings")) {
			xml.stepIn();

			int i;
			
			for(i=STR_FIRST; i<STR_LAST; i++) {
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);
				
				if(xml.findChild(attr))
					set(StrSetting(i), xml.getChildData());
				xml.resetCurrentChild();
			}
			for(i=INT_FIRST; i<INT_LAST; i++) {
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);
				
				if(xml.findChild(attr))
					set(IntSetting(i), Util::toInt(xml.getChildData()));
				xml.resetCurrentChild();
			}
			for(i=INT64_FIRST; i<INT64_LAST; i++) {
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);
				
				if(xml.findChild(attr))
					set(Int64Setting(i), Util::toInt64(xml.getChildData()));
				xml.resetCurrentChild();
			}
			xml.stepOut();
		}
		if(xml.findChild("ExternalActionsRaws")) {
			xml.stepIn();
			while(xml.findChild("Action"))
				addAction(xml.getIntChildAttrib("ID"), xml.getIntChildAttrib("ActionID"), xml.getBoolChildAttrib("Display"), xml.getChildAttrib("Name"), xml.getChildAttrib("CheatingDescription"));
			xml.stepOut();
		}
		if(xml.findChild("ExtensionsConfig")) {
			xml.stepIn();
			while(xml.findChild("Item"))
				setExtSetting(xml.getChildAttrib("Name"), xml.getChildAttrib("Value"));
			xml.stepOut();
		}
	} catch(...) {
		// ...
	}
}

void rsxppSettingsManager::on(Save, SimpleXML& xml) throw() { 
	xml.addTag("RSXPlusPlusSettings");
	xml.stepIn();

	int i;
	string type("type"), curType("string");
	
	for(i=STR_FIRST; i<STR_LAST; i++) {
		if(isSet[i]) {
			xml.addTag(settingTags[i], get(StrSetting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}

	curType = "int";
	for(i=INT_FIRST; i<INT_LAST; i++) {
		if(isSet[i]) {
			xml.addTag(settingTags[i], get(IntSetting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}
	curType = "int64";
	for(i=INT64_FIRST; i<INT64_LAST; i++) {
		if(isSet[i])
		{
			xml.addTag(settingTags[i], get(Int64Setting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}
	xml.stepOut();
	xml.addTag("ExternalActionsRaws");
	xml.stepIn();
	for(ExtActions::const_iterator i = ext_actions.begin(); i != ext_actions.end(); ++i) {
		xml.addTag("Action");
		xml.addChildAttrib("ID", (*i)->id);
		xml.addChildAttrib("ActionID", (*i)->actionId);
		xml.addChildAttrib("Display", (*i)->display);
		xml.addChildAttrib("Name", (*i)->name);
		xml.addChildAttrib("CheatingDescription", (*i)->cheat);
	}
	xml.stepOut();
	xml.addTag("ExtensionsConfig");
	xml.stepIn();
	for(StringMap::const_iterator j = extSettings.begin(); j != extSettings.end(); ++j) {
		xml.addTag("Item");
		xml.addChildAttrib("Name", j->first);
		xml.addChildAttrib("Value", j->second);
	}
	xml.stepOut();
}

int rsxppSettingsManager::getInt(const string& sname) {
	for(int n = INT_FIRST; n < INT_LAST; n++)
		if(strcmp(settingTags[n].c_str(), sname.c_str()) == 0)
			return get((IntSetting)n);
	return 0;
}

int64_t rsxppSettingsManager::getInt64(const string& sname) {
	for(int n = INT64_FIRST; n < INT64_LAST; n++)
		if(strcmp(settingTags[n].c_str(), sname.c_str()) == 0)
			return get((Int64Setting)n);
	return 0;
}

string rsxppSettingsManager::getString(const string& sname) const {
	for(int n = STR_FIRST; n < STR_LAST; n++)
		if(strcmp(settingTags[n].c_str(), sname.c_str()) == 0)
			return get((StrSetting)n);
	return Util::emptyString;
}
#ifndef SVNBUILD
void rsxppSettingsManager::set(IntSetting key, int value) {
	if((key == SLEEP_TIME) && (value < 200)) {
		value = 200;
	} else if((key == MAX_FILELISTS) && (value > 50)) {
		value = 50;
	} else if((key == MAX_TESTSURS) && (value > 50)) {
		value = 50;
	}
	intSettings[key - INT_FIRST] = value;
	isSet[key] = true;
}
#endif
const std::string& rsxppSettingsManager::getExtSetting(const std::string& name, const std::string& defaultValue) {
	StringMap::iterator i = extSettings.find(name);
	if(i == extSettings.end()) {
		std::pair<StringMap::iterator, bool> p = extSettings.insert(std::make_pair(name, defaultValue));
		if(p.second)
			return p.first->second;
		else
			return Util::emptyString;
	}
	return i->second;
}

void rsxppSettingsManager::setExtSetting(const std::string& name, const std::string& value) {
	extSettings[name] = value;
}

void rsxppSettingsManager::lock() { cs.lock(); }
void rsxppSettingsManager::unlock() { cs.unlock(); }

} // namespace dcpp