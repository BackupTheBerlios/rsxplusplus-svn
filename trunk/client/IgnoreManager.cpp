///////////////////////////////////////////////////////////////////////////////
//
//	Handles saving and loading of ignorelists
//
///////////////////////////////////////////////////////////////////////////////

#include "stdinc.h"
#include "IgnoreManager.h"

#include "Util.h"
#include "../rsx/RegexUtil.h"
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

bool IgnoreManager::isIgnored(const tstring& aNick) {
	bool ret = false;

	if(ignoredUsers.count(aNick))
		ret = true;

	if(RSXPP_BOOLSETTING(IGNORE_USE_REGEXP_OR_WC) && !ret) {
		Lock l(cs);
		for(TStringHashIterC i = ignoredUsers.begin(); i != ignoredUsers.end(); ++i) {
			if(strnicmp(*i, _T("$Re:"), 4) == 0) {
				if((*i).length() > 4) {
					try {
						boost::wregex reg((*i).substr(4), boost::regex_constants::icase);
						ret = boost::regex_search(aNick.begin(), aNick.end(), reg);
					} catch(...) { }
					if(ret) break;
				}
			} else {
				ret = Wildcard::patternMatch(Text::toLower(aNick), Text::toLower(*i), false);
				if(ret)
					break;
			}
		}
	}

	return ret;
}

// SettingsManagerListener
void IgnoreManager::on(SettingsManagerListener::Load, SimpleXML& aXml) {
	load(aXml);
}

void IgnoreManager::on(SettingsManagerListener::Save, SimpleXML& aXml) {
	save(aXml);
}
}; // namespace dcpp