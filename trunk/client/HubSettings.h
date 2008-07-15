#ifndef RSXPLUSPLUS_HUB_SETTINGS
#define RSXPLUSPLUS_HUB_SETTINGS

#include "CriticalSection.h"
#include "Util.h"

namespace dcpp {
class HubSettings {
public:
	HubSettings() throw() { }
	~HubSettings() throw();

	typedef unordered_map<uint32_t, string> SettingsMap;

	void set(const char* name, const string& s);
	string get(const char* name) const;
	bool isSet(const char* name) const;

	void updateSettings(const SettingsMap& s);
	const SettingsMap& getSettings() const;

	void clearSettings();
private:
	SettingsMap settings;
	static FastCriticalSection cs;
};
// some macros to define function for specified types
#define GS_BOOL(n, x) bool get##n() const { return isSet(x); } void set##n(bool v) { set(x, v ? "1" : Util::emptyString); }
#define GS_STR(n, x) string get##n() const { return get(x); } void set##n(const string& v) { set(x, v); }
#define GS_INT(n, x) int get##n() const { return Util::toInt(get(x)); } void set##n(int v) { set(x, Util::toString(v)); }
#define GS_UINT32(n, x) uint32_t get##n() const { return Util::toUInt32(get(x)); } void set##n(uint32_t v) { set(x, Util::toString(v)); }
#define GS_DOUBLE(n, x) double get##n() const { return Util::toDouble(get(x)); } void set##n(double v) { set(x, Util::toString(v)); }
#define GS_FLOAT(n, x) float get##n() const { return Util::toFloat(get(x)); } void set##n(float v) { set(x, Util::toString(v)); }

} // namespace dcpp
#endif
