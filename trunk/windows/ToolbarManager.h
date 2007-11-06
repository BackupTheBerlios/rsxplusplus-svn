////////////////////////////////////////////////
//	ToolbarManager.h
//	
//	This is based on portions of:
//	http://www.codeproject.com/wtl/regst.asp
//	(Copyright (c) 2001 Magomed Abdurakhmanov)
//
//	Changed save to xml file instead of registry
//
//	No warranties given. Provided as is.

#ifndef TOOLBARMANAGER_H
#define TOOLBARMANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/DCPlusPlus.h"

#include "../client/Singleton.h"
#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "../client/SimpleXML.h"

class ToolbarEntry {
public:
	typedef ToolbarEntry* Ptr;
	typedef vector<Ptr> List;
	typedef List::const_iterator Iter;

	ToolbarEntry() throw() { }
	~ToolbarEntry() throw() { }

	GETSET(string, name, Name);
	GETSET(string, id, ID);
	GETSET(string, cx, CX);
	GETSET(string, breakline, BreakLine);
	GETSET(int, bandcount, BandCount);
};

class ToolbarManager: public Singleton<ToolbarManager>, private RSXSettingsManagerListener
{
public:
	ToolbarManager();
	~ToolbarManager() throw();

	// Get & Set toolbar positions
	void getFrom(CReBarCtrl& ReBar, const string& aName);
	void applyTo(CReBarCtrl& ReBar, const string& aName) const;

private:
	// Get data by name
	ToolbarEntry* getToolbarEntry(const string& aName) const;

	// Remove old entry, when adding new
	void removeToolbarEntry(const ToolbarEntry* entry);

	// Save & load
	void load(SimpleXML& aXml);
	void save(SimpleXML& aXml);

	// SettingsManagerListener
	virtual void on(RSXSettingsManagerListener::Load, SimpleXML& xml) throw();
	virtual void on(RSXSettingsManagerListener::Save, SimpleXML& xml) throw();

	// Store Toolbar infos here
	ToolbarEntry::List toolbarEntries;

};

#endif // TOOLBARMANAGER_H
