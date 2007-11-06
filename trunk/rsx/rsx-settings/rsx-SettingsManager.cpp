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
#include "../../client/DCPlusPlus.h"

#include "rsx-SettingsManager.h"

#include "../../client/SimpleXML.h"
#include "../../client/Util.h"
#include "../../client/File.h"
#include "../../client/version.h"

const string RSXSettingsManager::settingTags[] =
{
	// Strings
	"ProtectedUsers", 
	"UpdateProfiles", "UpdateISP", "UpdateMyInfoProfilesUrl",
	
	"ClipboardNameOne", "ClipboardNameTwo", "ClipboardNameThree", "ClipboardNameFour", "ClipboardNameFive", "ClipboardNameSix", 
	"ClipboardTextOne", "ClipboardTextTwo", "ClipboardTextThree", "ClipboardTextFour", "ClipboardTextFive", "ClipboardTextSix",

	"HighPrioFiles", "LowPrioFiles", "ShareSkiplist", "DownloadSkiplist", "ProfileVerUrl",
	"SENTRY", 
	// Ints
	"CheckDelay", "SleepTime", "MaxFilelists", "MaxTestSURs", "UseSendDelayedRaw", 
	"FavoriteUserIsProtectedUser", "CheckAllClientsBeforeFilelists", "UncheckClientProtectedUser",
	"UncheckListProtectedUser", "MaxTimeouts", "MaxDisconnects", "DisplayCheatsInMainChat",
	"ShowShareCheckedUsers", "DeleteCheckedFilelists", "CheckConflicts", "ISPCheck", "UsePointsAdlSearch", "UseIpWatch",
	"IgnoreUserRegExpOrWC", "AutoUpdateIpOnStartup", "SlowDownloadSpeed", "SlowDownloadTime", "RawSenderSleepTime",
	"IgnorePMSpammers", "DefaultProcessPriority",

	"TimeoutRaw", "DisconnectRaw", 
	"FakeShareRaw", "ListLenMismatch", 
	"FilelistTooSmallOrBig", "FilelistUnavailable",
	"NoTTHF", "VersionMisMatch", 
	"MyInfoSpamKick", "CTMSpamKick", 
	"BadIsp", "ADLLowPointsRaw", 
	"ADLMediumPointsRaw", "ADLHighPointsRaw",
	"HashErrorRaw", "rmDCRaw", 
	"DCPlusPlusEmulationRaw", "FilelistVersionMismatch", 
	"SlowDownloadRaw", "PMSpamKick", 
	"FakeSlotCount",

	"ShowTimeoutRaw", "ShowDisconnectRaw", 
	"ShowFakeShareRaw", "ShowLisLenMismatch", 
	"ShowFilelistTooSmallOrBig", "ShowFilelistUnavailable", 
	"ShowNoTTHF", "ShowVersionMisMatch", 
	"ShowMyInfoSpamKick", "ShowCTMSpamKick", 
	"ShowBadIsp", "ShowADLLowPointsRaw", 
	"ShowADLMediumPointsRaw", "ShowADLHighPointsRaw",
	"ShowHashErrorRaw", "ShowrmDCRaw", 
	"ShowDCPlusPlusEmulationRaw", "ShowFilelistVersionMismatch", 
	"ShowSlowDownloadRaw", "ShowPMSpamKick", 
	"ShowFakeSlotCount",

	"MyInfoCounter", "MyInfoTime", "CTMCounter", "CTMTime", "PMCounter", "PMTime",

	"PercentHashed", "PercentFakeShareTolerated", "MaxFilelistSize", "MinFilelistSize", 

	"UseChatFilter", "UsePartialFileSharing", "AutosearchEnable", "AutosearchTime", "AutosearchEvery", 
	"AutosearchRecheckTime", "UseHighLight", "GetUpdateProfile", "UseFiltersOnNewHubs", "UseHighlightsOnNewHubs", "ProtectPMUser",
	"UseSlowDownloadKick",
	"SENTRY",
	// Int64
	"TotalDetects", "TotalFailedDetects", "TotalRawCommandsSent",
	"SENTRY"
};

RSXSettingsManager::RSXSettingsManager()
{
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

	setDefault(UPDATE_URL,		"http://rsxplusplus.sf.net/xml/");
	setDefault(UPDATE_MYINFOS,	"http://rsxplusplus.sf.net/xml/");
	setDefault(UPDATE_ISP_URL,	"http://rsxplusplus.sf.net/xml/");
	setDefault(PROFILE_VER_URL,	"http://rsxplusplus.sf.net/xml/");

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
	setDefault(RAW_SENDER_SLEEP_TIME,			120);
	setDefault(PERCENT_HASHED,					10);
	setDefault(PERCENT_FAKE_SHARE_TOLERATED,	20);
	setDefault(MINIMUM_FILELIST_SIZE,			10);
	setDefault(MAXIMUM_FILELIST_SIZE,			10*1024*1024);

	setDefault(USE_SEND_DELAYED_RAW,				true);
	setDefault(FAV_USER_IS_PROTECTED_USER,			true);
	setDefault(CHECK_ALL_CLIENTS_BEFORE_FILELISTS,	true);
	setDefault(UNCHECK_CLIENT_PROTECTED_USER,		true);
	setDefault(UNCHECK_LIST_PROTECTED_USER,			true);
	setDefault(CHECK_CONFLICTS,						false);
	setDefault(ISP_CHECKING,						true);
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
	setDefault(BAD_ISP_RAW,					0);
	setDefault(ADL_RAW_LOW_POINTS,			0);
	setDefault(ADL_RAW_MEDIUM_POINTS,		0);
	setDefault(ADL_RAW_HIGH_POINTS,			0);
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
	setDefault(SHOW_BAD_ISP_RAW,				true);
	setDefault(SHOW_ADL_RAW_LOW_POINTS,			true);
	setDefault(SHOW_ADL_RAW_MEDIUM_POINTS,		true);
	setDefault(SHOW_ADL_RAW_HIGH_POINTS,		true);
	setDefault(SHOW_HASH_ERROR_RAW,				true);
	setDefault(SHOW_RMDC_RAW,					true);
	setDefault(SHOW_DCPP_EMULATION_RAW,			true);
	setDefault(SHOW_FILELIST_VERSION_MISMATCH,	true);
	setDefault(SHOW_SDL_RAW,					true);
	setDefault(SHOW_PM_SPAM_KICK,				true);
	setDefault(SHOW_FAKE_SLOT_COUNT,			true);

	//strings
	setDefault(PROTECTED_USERS,				Util::emptyString);
	setDefault(CLIPBOARD_NAME_ONE,			"Tag");
	setDefault(CLIPBOARD_NAME_TWO,			"Lock");
	setDefault(CLIPBOARD_NAME_THREE,		"Pk");
	setDefault(CLIPBOARD_NAME_FOUR,			"Supports");
	setDefault(CLIPBOARD_NAME_FIVE,			"TestSUR");
	setDefault(CLIPBOARD_NAME_SIX,			"CD");
	setDefault(CLIPBOARD_TEXT_ONE,			"^<\\+\\+ V:%[version],M:([AP5]){1},H:([0-9]){1,3}/([0-9]){1,3}/([0-9]){1,3}(,S:([0-9]){1,3}|,S:([0-9]){1,3},O:([0-9]){1,10})>$");
	setDefault(CLIPBOARD_TEXT_TWO,			"^EXTENDEDPROTOCOLABCABCABCABCABCABC$");
	setDefault(CLIPBOARD_TEXT_THREE,		"^DCPLUSPLUS%[version]ABCABC$");
	setDefault(CLIPBOARD_TEXT_FOUR,			"BZList MiniSlots XmlBZList GetTestZBlock GetZBlock ADCGet TTHL ZLIG CHUNK ");
	setDefault(CLIPBOARD_TEXT_FIVE,			"File Not Available");
	setDefault(CLIPBOARD_TEXT_SIX,			"Your client is not allowed in this hub.");

	setDefault(HIGH_PRIO_FILES,				"*.sfv;*.nfo;*sample*;*cover*;*.pls;*.m3u");
	setDefault(LOW_PRIO_FILES,				"");
	setDefault(SKIPLIST_SHARE,				"*.ioFTPD;*.checked;*.raidenftpd.acl;*.SimSfvChk.log;*.message;Descript.ion;*.upChk.log;thumbs.db;*.crc;*.temp;Desktop.ini");
	setDefault(SKIPLIST_DOWNLOAD,			"*.ioFTPD;*.checked;*.raidenftpd.acl;*.SimSfvChk.log;*.message;Descript.ion;*.upChk.log;thumbs.db;*.crc;*.temp;Desktop.ini");

	//others
	setDefault(USE_CHAT_FILTER,				false);
	setDefault(USE_HIGHLIGHT,				false);
	setDefault(PARTIAL_FILE_SHARING,		true);
	setDefault(IPUPDATE,					false);
	setDefault(GET_UPDATE_PROFILE,			true);
	setDefault(USE_FILTER_FAV,				false);
	setDefault(USE_HL_FAV,					false);
	setDefault(DEFAULT_PRIO,				3); //normal prio
}

void RSXSettingsManager::load(string const& aFileName) {
	try {
		SimpleXML xml;
		xml.fromXML(File(aFileName, File::READ, File::OPEN).read());
		xml.resetCurrentChild();
		xml.stepIn();

		if(xml.findChild("RSXSettings")) {
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

		fire(RSXSettingsManagerListener::Load(), xml);
		xml.stepOut();

	} catch(...) {
		//...
	}
}

void RSXSettingsManager::save(string const& aFileName) {

	SimpleXML xml;
	xml.addTag("RSX");
	xml.stepIn();
	xml.addTag("RSXSettings");
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
	
	fire(RSXSettingsManagerListener::Save(), xml);

	try {
		File out(aFileName + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&out);
		f.write(SimpleXML::utf8Header);
		xml.toXML(&f);
		f.flush();
		out.close();
		File::deleteFile(aFileName);
		File::renameFile(aFileName + ".tmp", aFileName);
	} catch(...) {
		// ...
	}
}

bool RSXSettingsManager::getType(const char* name, int& n, int& type) const {
	for(n = 0; n < INT64_LAST; n++) {
		if(strcmp(settingTags[n].c_str(), name) == 0) {
			if(n < STR_LAST) {
				type = TYPE_STRING;
				return true;
			} else if(n < INT_LAST) {
				type = TYPE_INT;
				return true;
			} else {
				type = TYPE_INT64;
				return true;
			}
		}
	}
	return false;
}