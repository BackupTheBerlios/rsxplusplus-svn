///////////////////////////////////////////////////////////////////////////////
//
//	Handles saving and loading of ignorelists
//
///////////////////////////////////////////////////////////////////////////////

#ifndef IGNOREMANAGER_H
#define IGNOREMANAGER_H

#include "DCPlusPlus.h"

#include "Singleton.h"
#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "SimpleXML.h"
#include "forward.h"

namespace dcpp {

class IgnoreManager: public Singleton<IgnoreManager>, private RSXSettingsManagerListener
{
public:
	IgnoreManager() { RSXSettingsManager::getInstance()->addListener(this); }
	~IgnoreManager() { RSXSettingsManager::getInstance()->removeListener(this); }

	// store & remove ignores through/from hubframe
	void storeIgnore(const UserPtr& user);
	void removeIgnore(const UserPtr& user);

	// check if user is ignored
	bool isIgnored(const string& aNick);

	// get and put ignorelist (for MiscPage)
	unordered_set<tstring> getIgnoredUsers() { Lock l(cs); return ignoredUsers; }
	void putIgnoredUsers(unordered_set<tstring> ignoreList) { Lock l(cs); ignoredUsers = ignoreList; }

private:
	typedef unordered_set<tstring> TStringHash;
	typedef TStringHash::const_iterator TStringHashIterC;
	CriticalSection cs;

	// save & load
	void load(SimpleXML& aXml);
	void save(SimpleXML& aXml);

	// SettingsManagerListener
	void on(RSXSettingsManagerListener::Load, SimpleXML& xml) throw();
	void on(RSXSettingsManagerListener::Save, SimpleXML& xml) throw();

	// contains the ignored nicks and patterns 
	TStringHash ignoredUsers;
};
}; // namespace dcpp
#endif // IGNOREMANAGER_H