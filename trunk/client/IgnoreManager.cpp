///////////////////////////////////////////////////////////////////////////////
//
//	Handles saving and loading of ignorelists
//
///////////////////////////////////////////////////////////////////////////////

#include "stdinc.h"
#include "IgnoreManager.h"

#include "Util.h"
#include "pme.h"
#include "../rsx/Wildcards.h"
#include "ClientManager.h"

namespace dcpp {

void IgnoreManager::load(SimpleXML& aXml) {
	if(aXml.findChild("IgnoreList")) {
		aXml.stepIn();
		while(aXml.findChild("User")) {	
			ignoredUsers.insert(Text::toT(aXml.getChildAttrib("Nick")));
		}
		aXml.stepOut();
	}
}

void IgnoreManager::save(SimpleXML& aXml) {
	aXml.addTag("IgnoreList");
	aXml.stepIn();

	for(TStringHashIterC i = ignoredUsers.begin(); i != ignoredUsers.end(); ++i) {
		aXml.addTag("User");
		aXml.addChildAttrib("Nick", Text::fromT(*i));
	}
	aXml.stepOut();
}

void IgnoreManager::storeIgnore(const UserPtr& user) {
	ignoredUsers.insert(Text::toT(ClientManager::getInstance()->getNicks(user->getCID())[0]));
}

void IgnoreManager::removeIgnore(const UserPtr& user) {
	ignoredUsers.erase(Text::toT(ClientManager::getInstance()->getNicks(user->getCID())[0]));
}

bool IgnoreManager::isIgnored(const string& aNick) {
	bool ret = false;

	if(ignoredUsers.count(Text::toT(aNick)))
		ret = true;

	if(RSXBOOLSETTING(IGNORE_USE_REGEXP_OR_WC) && !ret) {
		Lock l(cs);
		for(TStringHashIterC i = ignoredUsers.begin(); i != ignoredUsers.end(); ++i) {
			const string tmp = Text::fromT(*i);
			if(Util::strnicmp(tmp, "$Re:", 4) == 0) {
				if(tmp.length() > 4) {
					PME regexp(tmp.substr(4), "gims");

					if(regexp.match(aNick)) {
						ret = true;
						break;
					}
				}
			} else {
				ret = Wildcard::patternMatch(Text::toLower(aNick), Text::toLower(tmp), false);
				if(ret)
					break;
			}
		}
	}

	return ret;
}

// SettingsManagerListener
void IgnoreManager::on(RSXSettingsManagerListener::Load, SimpleXML& aXml) {
	load(aXml);
}

void IgnoreManager::on(RSXSettingsManagerListener::Save, SimpleXML& aXml) {
	save(aXml);
}
}; // namespace dcpp