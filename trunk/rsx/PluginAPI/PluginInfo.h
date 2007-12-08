#ifndef PLUGIN_INFO_H
#define PLUGIN_INFO_H

class PluginInfo {
public:
	typedef int (__cdecl  * PLUGIN_ID)();
	typedef iPlugin* (__cdecl * PLUGIN_LOAD)();
	typedef void (__cdecl * PLUGIN_UNLOAD)();

	PluginInfo(HINSTANCE _h, PLUGIN_LOAD loader, PLUGIN_UNLOAD unloader, int _id);
	~PluginInfo();

	void loadPlugin();
	void unloadPlugin();
	void setSetting(const string& aName, const string& aVal);
	const string& getSetting(const string& aName);
	StringMap& getSettings() { return settings; }

	GETSET(HMODULE, handle, Handle);
	GETSET(iPlugin*, plugin, Plugin);

	GETSET(int, id, Id);
	GETSET(string, name, Name);
	GETSET(string, version, Version);
	GETSET(int, icon, Icon);
private:
	CriticalSection cs;

	StringMap settings;
	PLUGIN_LOAD load;
	PLUGIN_UNLOAD unload;
};
#endif

/**
 * @file
 * $Id: MainFrm.cpp,v 1.20 2004/07/21 13:15:15 bigmuscle Exp
 */