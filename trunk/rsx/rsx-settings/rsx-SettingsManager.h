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

#ifndef RSX_SETTINGS_MANAGER_H
#define RSX_SETTINGS_MANAGER_H

#include "../../client/Util.h"
#include "../../client/Speaker.h"
#include "../../client/Singleton.h"

class SimpleXML;

class RSXSettingsManagerListener {
public:
	virtual ~RSXSettingsManagerListener() { }
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Load;
	typedef X<1> Save;

	virtual void on(Load, SimpleXML&) throw() { }
	virtual void on(Save, SimpleXML&) throw() { }
};

class RSXSettingsManager : public Singleton<RSXSettingsManager>, public Speaker<RSXSettingsManagerListener>
{
public:
	enum Types {
		TYPE_STRING,
		TYPE_INT,
		TYPE_INT64
	};

	enum StrSetting { STR_FIRST,
		PROTECTED_USERS = STR_FIRST, 
		UPDATE_URL, UPDATE_IPWATCH_URL, UPDATE_MYINFOS,
		
		CLIPBOARD_NAME_ONE, CLIPBOARD_NAME_TWO, CLIPBOARD_NAME_THREE, CLIPBOARD_NAME_FOUR, CLIPBOARD_NAME_FIVE, CLIPBOARD_NAME_SIX, 
		CLIPBOARD_TEXT_ONE, CLIPBOARD_TEXT_TWO, CLIPBOARD_TEXT_THREE, CLIPBOARD_TEXT_FOUR, CLIPBOARD_TEXT_FIVE, CLIPBOARD_TEXT_SIX,

		HIGH_PRIO_FILES, LOW_PRIO_FILES, SKIPLIST_SHARE, SKIPLIST_DOWNLOAD, PROFILE_VER_URL,
		STR_LAST };

	enum IntSetting { INT_FIRST = STR_LAST + 1,
		CHECK_DELAY = INT_FIRST, SLEEP_TIME, MAX_FILELISTS, MAX_TESTSURS, USE_SEND_DELAYED_RAW,
		FAV_USER_IS_PROTECTED_USER, CHECK_ALL_CLIENTS_BEFORE_FILELISTS, UNCHECK_CLIENT_PROTECTED_USER,
		UNCHECK_LIST_PROTECTED_USER, MAX_TIMEOUTS, MAX_DISCONNECTS, DISPLAY_CHEATS_IN_MAIN_CHAT,
		SHOW_SHARE_CHECKED_USERS, DELETE_CHECKED_FILELISTS, USE_POINTS_IN_ADLS, USE_IPWATCH,
		IGNORE_USE_REGEXP_OR_WC, IPUPDATE, SDL_SPEED, SDL_TIME, RAW_SENDER_SLEEP_TIME,
		IGNORE_PM_SPAMMERS, DEFAULT_PRIO, FLASH_WINDOW_ON_PM, FLASH_WINDOW_ON_NEW_PM, AUTO_START,

		//raws
		TIMEOUT_RAW,					DISCONNECT_RAW, 
		FAKESHARE_RAW,					LISTLEN_MISMATCH, 
		FILELIST_TOO_SMALL_BIG,			FILELIST_NA, 
		NO_TTHF,						VERSION_MISMATCH, 
		MYINFO_SPAM_KICK,				CTM_SPAM_KICK,
		ADL_RAW_LOW_POINTS,				ADL_RAW_MEDIUM_POINTS,
		ADL_RAW_HIGH_POINTS,			HASH_ERROR_RAW,
		RMDC_RAW,						DCPP_EMULATION_RAW,
		FILELIST_VERSION_MISMATCH,		SDL_RAW,
		PM_SPAM_KICK,					FAKE_SLOT_COUNT,

		//displaying cheats
		SHOW_TIMEOUT_RAW,				SHOW_DISCONNECT_RAW, 
		SHOW_FAKESHARE_RAW,				SHOW_LISTLEN_MISMATCH, 
		SHOW_FILELIST_TOO_SMALL_BIG,	SHOW_FILELIST_NA,
		SHOW_NO_TTHF,					SHOW_VERSION_MISMATCH, 
		SHOW_MYINFO_SPAM_KICK,			SHOW_CTM_SPAM_KICK, 
		SHOW_ADL_RAW_LOW_POINTS,		SHOW_ADL_RAW_MEDIUM_POINTS,
		SHOW_ADL_RAW_HIGH_POINTS,		SHOW_HASH_ERROR_RAW,
		SHOW_RMDC_RAW,					SHOW_DCPP_EMULATION_RAW,
		SHOW_FILELIST_VERSION_MISMATCH, SHOW_SDL_RAW,
		SHOW_PM_SPAM_KICK,				SHOW_FAKE_SLOT_COUNT,

		//flood stuff
		MYINFO_COUNTER, MYINFO_TIME, CTM_COUNTER, CTM_TIME, PM_COUNTER, PM_TIME,

		//fake share
		PERCENT_HASHED, PERCENT_FAKE_SHARE_TOLERATED, MINIMUM_FILELIST_SIZE, MAXIMUM_FILELIST_SIZE,
		
		USE_CHAT_FILTER, AUTOSEARCH_ENABLED, AUTOSEARCH_ENABLED_TIME, AUTOSEARCH_EVERY,
		AUTOSEARCH_RECHECK_TIME, USE_HIGHLIGHT, GET_UPDATE_PROFILE, USE_FILTER_FAV, USE_HL_FAV, PROTECT_PM_USERS,
		USE_SDL_KICK, SORT_PROTECTED_AFTER_FAV,
		INT_LAST };

	enum Int64Setting { INT64_FIRST = INT_LAST + 1,
		TOTAL_DETECTS = INT64_FIRST, TOTAL_FAILED_DETECTS, TOTAL_RAW_COMMANDS_SENT,
		INT64_LAST, SETTINGS_LAST = INT64_LAST 
	};

	bool getType(const char* name, int& n, int& type) const;

	const string& get(StrSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? strSettings[key - STR_FIRST] : strDefaults[key - STR_FIRST];
	}

	int get(IntSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? intSettings[key - INT_FIRST] : intDefaults[key - INT_FIRST];
	}
	int64_t get(Int64Setting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? int64Settings[key - INT64_FIRST] : int64Defaults[key - INT64_FIRST];
	}

	bool getBool(IntSetting key, bool useDefault = true) const {
		return (get(key, useDefault) != 0);
	}

	void set(StrSetting key, string const& value) {
		strSettings[key - STR_FIRST] = value;
		isSet[key] = !value.empty();
	}

	void set(IntSetting key, int value) {
		intSettings[key - INT_FIRST] = value;
		isSet[key] = true;
	}

	void set(IntSetting key, const string& value) {
		if(value.empty()) {
			intSettings[key - INT_FIRST] = 0;
			isSet[key] = false;
		} else {
			intSettings[key - INT_FIRST] = Util::toInt(value);
			isSet[key] = true;
		}
	}

	void set(Int64Setting key, int64_t value) {
		int64Settings[key - INT64_FIRST] = value;
		isSet[key] = true;
	}

	void set(Int64Setting key, const string& value) {
		if(value.empty()) {
			int64Settings[key - INT64_FIRST] = 0;
			isSet[key] = false;
		} else {
			int64Settings[key - INT64_FIRST] = Util::toInt64(value);
			isSet[key] = true;
		}
	}

	void set(IntSetting key, bool value) { set(key, (int)value); }

	void setDefault(StrSetting key, string const& value) {
		strDefaults[key - STR_FIRST] = value;
	}

	void setDefault(IntSetting key, int value) {
		intDefaults[key - INT_FIRST] = value;
	}
	void setDefault(Int64Setting key, int64_t value) {
		int64Defaults[key - INT64_FIRST] = value;
	}

	bool isDefault(int aSet) { return !isSet[aSet]; }

	void load() {
		load(Util::getConfigPath() + "RSXSettings.xml");
	}
	void save() {
		save(Util::getConfigPath() + "RSXSettings.xml");
	}

	void load(const string& aFileName);
	void save(const string& aFileName);

private:
	friend class Singleton<RSXSettingsManager>;
	RSXSettingsManager();
	~RSXSettingsManager() throw() { }

	static const string settingTags[SETTINGS_LAST+1];

	string strSettings[STR_LAST - STR_FIRST];
	int    intSettings[INT_LAST - INT_FIRST];
	int64_t int64Settings[INT64_LAST - INT64_FIRST];
	string strDefaults[STR_LAST - STR_FIRST];
	int    intDefaults[INT_LAST - INT_FIRST];
	int64_t int64Defaults[INT64_LAST - INT64_FIRST];
	bool isSet[SETTINGS_LAST];
};

// Shorthand accessor macros
#define RSXSETTING(k) (RSXSettingsManager::getInstance()->get(RSXSettingsManager::k, true))
#define RSXBOOLSETTING(k) (RSXSettingsManager::getInstance()->getBool(RSXSettingsManager::k, true))
#define RSXS_SET(x,y) (RSXSettingsManager::getInstance()->set(RSXSettingsManager::x, y));

#endif // !defined(RSX_SETTINGS_MANAGER_H)
