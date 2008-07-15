#include "stdinc.h"
#include "DCPlusPlus.h"
#include "Util.h"
#include "HubSettings.h"

namespace dcpp {
FastCriticalSection HubSettings::cs;
HubSettings::~HubSettings() throw() {
	clearSettings();
}

void HubSettings::set(const char* name, const string& val) {
	dcdebug("\t\tHubSettings::set (%s, %s)\n", name, val.c_str());
	FastLock l(cs);
	if(val.empty())
		settings.erase(*(uint32_t*)name);
	else
		settings[*(uint32_t*)name] = val;
}

string HubSettings::get(const char* name) const {
	FastLock l(cs);
	SettingsMap::const_iterator i = settings.find(*(uint32_t*)name);
	return i == settings.end() ? Util::emptyString : i->second;
}

bool HubSettings::isSet(const char* name) const {
	FastLock l(cs);
	SettingsMap::const_iterator i = settings.find(*(uint32_t*)name);
	return i != settings.end();
}

void HubSettings::updateSettings(const SettingsMap& s) {
	FastLock l(cs);
	settings = s;
}

const HubSettings::SettingsMap& HubSettings::getSettings() const {
	FastLock l(cs);
	return settings;
}

void HubSettings::clearSettings() {
	FastLock l(cs);
	settings.clear();
}
}