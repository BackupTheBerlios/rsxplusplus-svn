/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
 *
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

#ifndef DCPLUSPLUS_DCPP_FAVORITE_MANAGER_H
#define DCPLUSPLUS_DCPP_FAVORITE_MANAGER_H

#include "SettingsManager.h"

#include "CriticalSection.h"
#include "HttpConnection.h"
#include "User.h"
#include "UserCommand.h"
#include "FavoriteUser.h"
#include "Singleton.h"
#include "ClientManagerListener.h"
#include "FavoriteManagerListener.h"
#include "ClientManager.h"
#include "StringTokenizer.h" //RSX++

namespace dcpp {
	
class HubEntry {
public:
	typedef vector<HubEntry> List;
	
	HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) throw() : 
	name(aName), server(aServer), description(aDescription), country(Util::emptyString), 
	rating(Util::emptyString), reliability(0.0), shared(0), minShare(0), users(Util::toInt(aUsers)), minSlots(0), maxHubs(0), maxUsers(0) { }

	HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers, const string& aCountry,
		const string& aShared, const string& aMinShare, const string& aMinSlots, const string& aMaxHubs, const string& aMaxUsers,
		const string& aReliability, const string& aRating) : name(aName), server(aServer), description(aDescription), country(aCountry), 
		rating(aRating), reliability((float)(Util::toFloat(aReliability) / 100.0)), shared(Util::toInt64(aShared)), minShare(Util::toInt64(aMinShare)),
		users(Util::toInt(aUsers)), minSlots(Util::toInt(aMinSlots)), maxHubs(Util::toInt(aMaxHubs)), maxUsers(Util::toInt(aMaxUsers)) 
	{

	}

	HubEntry() throw() { }
	HubEntry(const HubEntry& rhs) throw() : name(rhs.name), server(rhs.server), description(rhs.description), country(rhs.country), 
		rating(rhs.rating), reliability(rhs.reliability), shared(rhs.shared), minShare(rhs.minShare), users(rhs.users), minSlots(rhs.minSlots),
		maxHubs(rhs.maxHubs), maxUsers(rhs.maxUsers) { }

	~HubEntry() throw() { }

	GETSET(string, name, Name);
	GETSET(string, server, Server);
	GETSET(string, description, Description);
	GETSET(string, country, Country);
	GETSET(string, rating, Rating);
	GETSET(float, reliability, Reliability);
	GETSET(int64_t, shared, Shared);
	GETSET(int64_t, minShare, MinShare);
	GETSET(int, users, Users);
	GETSET(int, minSlots, MinSlots);
	GETSET(int, maxHubs, MaxHubs)
	GETSET(int, maxUsers, MaxUsers);
};

class FavoriteHubEntry {
public:
	typedef FavoriteHubEntry* Ptr;
	typedef vector<Ptr> List;
	typedef List::const_iterator Iter;

	FavoriteHubEntry() throw() : connect(false), encoding(Text::systemCharset), chatusersplit(0), stealth(false), userliststate(true), mode(0), ip(Util::emptyString)
		//RSX++
		, favEmail(Util::emptyString), awayMsg(Util::emptyString), userProtected(Util::emptyString), checkOnConnect(false),
		checkClients(false), checkFilelists(false), checkMyInfo(false), hideShare(false),
		useFilter(RSXBOOLSETTING(USE_FILTER_FAV)), autosearch(false), useHL(RSXBOOLSETTING(USE_HL_FAV)), usersLimit(0), groupId(0)
		//END
		{ }
	FavoriteHubEntry(const HubEntry& rhs) throw() : name(rhs.getName()), server(rhs.getServer()), encoding(Text::systemCharset),
		description(rhs.getDescription()), connect(false), chatusersplit(0), stealth(false), userliststate(true), mode(0), ip(Util::emptyString),
		//RSX++
		favEmail(Util::emptyString), awayMsg(Util::emptyString), userProtected(Util::emptyString), checkOnConnect(false), 
		checkClients(false), checkFilelists(false), checkMyInfo(false), hideShare(false),
		useFilter(RSXBOOLSETTING(USE_FILTER_FAV)), autosearch(false), useHL(RSXBOOLSETTING(USE_HL_FAV)), usersLimit(0), groupId(0)
		//END
		{ }
	FavoriteHubEntry(const FavoriteHubEntry& rhs) throw() : userdescription(rhs.userdescription), name(rhs.getName()), 
		server(rhs.getServer()), description(rhs.getDescription()), password(rhs.getPassword()), connect(rhs.getConnect()), 
		nick(rhs.nick), chatusersplit(rhs.chatusersplit), stealth(rhs.stealth),
		userliststate(rhs.userliststate), mode(rhs.mode), ip(rhs.ip), encoding(rhs.getEncoding()),
		//RSX++
		favEmail(rhs.favEmail), awayMsg(rhs.awayMsg), userProtected(rhs.userProtected) , checkOnConnect(rhs.checkOnConnect), 
		checkClients(rhs.checkClients),checkFilelists(rhs.checkFilelists), checkMyInfo(rhs.checkMyInfo), hideShare(rhs.hideShare), 
		useFilter(rhs.useFilter), autosearch(rhs.autosearch), useHL(rhs.useHL), usersLimit(rhs.usersLimit),
		groupId(rhs.groupId)
		//END
		{ }
	~FavoriteHubEntry() throw() { 
		//RSX++
		for(FavoriteHubEntry::Action::List::const_iterator j = action.begin(); j != action.end(); ++j) {
			delete j->second;
		}
		//END
	}
	
	const string& getNick(bool useDefault = true) const { 
		return (!nick.empty() || !useDefault) ? nick : SETTING(NICK);
	}

	void setNick(const string& aNick) { nick = aNick; }

	GETSET(string, userdescription, UserDescription);
	GETSET(string, name, Name);
	GETSET(string, server, Server);
	GETSET(string, description, Description);
	GETSET(string, password, Password);
	GETSET(string, headerOrder, HeaderOrder);
	GETSET(string, headerWidths, HeaderWidths);
	GETSET(string, headerVisible, HeaderVisible);
	GETSET(bool, connect, Connect);
	GETSET(string, encoding, Encoding);
	GETSET(int, chatusersplit, ChatUserSplit);
	GETSET(bool, stealth, Stealth);
	GETSET(bool, userliststate, UserListState);	
	GETSET(int, mode, Mode); // 0 = default, 1 = active, 2 = passive
	GETSET(string, ip, IP);
	//RSX++
	GETSET(string, favEmail, FavEmail);
	GETSET(string, awayMsg, AwayMsg);
	GETSET(string, userProtected, UserProtected);
	GETSET(bool, checkOnConnect, CheckOnConnect);
	GETSET(bool, checkClients, CheckClients);
	GETSET(bool, checkFilelists, CheckFilelists);
	GETSET(bool, checkMyInfo, CheckMyInfo);
	GETSET(bool, hideShare, HideShare);
	GETSET(bool, useFilter, UseFilter);
	GETSET(bool, autosearch, Autosearch);
	GETSET(bool, useHL, UseHL);
	GETSET(int, usersLimit, UsersLimit);
	GETSET(int, groupId, GroupId);
	//END

	//RSX++ //Raw Manager
	struct Action {
		typedef Action* Ptr;
		typedef unordered_map<int, Ptr> List;

		Action(bool aActif, string aRaw = Util::emptyString) throw() : actif(aActif) {
			if(!aRaw.empty()) {
				StringTokenizer<string> tok(aRaw, ',');
				StringList l = tok.getTokens();
				for(StringIter j = l.begin(); j != l.end(); ++j){
					if(Util::toInt(*j) != 0)
						raw.push_back(Raw(Util::toInt(*j)));
				}
			}
		};
		struct Raw {
			Raw(int aRawId) throw() : rawId(aRawId) { };
			GETSET(int, rawId, RawId);
		};
		typedef vector<Raw> RawsList;
		RawsList raw;

		GETSET(bool, actif, Actif);
	};

	Action::List action;
	//END
private:
	string nick;
};

class RecentHubEntry {
public:
	typedef RecentHubEntry* Ptr;
	typedef vector<Ptr> List;
	typedef List::const_iterator Iter;

	~RecentHubEntry() throw() { }	
	
	GETSET(string, name, Name);
	GETSET(string, server, Server);
	GETSET(string, description, Description);
	GETSET(string, users, Users);
	GETSET(string, shared, Shared);	
};

class DirectoriesEx {
public:
	typedef DirectoriesEx* Ptr;
	typedef vector<Ptr> List;
	typedef List::const_iterator Iter;

	DirectoriesEx() throw() {}
	DirectoriesEx(string n, string p, string e) : name(n), path(p), extension(e) {};
	~DirectoriesEx() throw() { }	

	GETSET(string, name, Name);
	GETSET(string, path, Path);
	GETSET(string, extension, Extension);
};
//RSX++
class PreviewApplication {
public:
	typedef PreviewApplication* Ptr;
	typedef vector<Ptr> List;
	typedef List::const_iterator Iter;

	PreviewApplication() throw() {}
	PreviewApplication(string n, string a, string r, string e) : name(n), application(a), arguments(r), extension(e) {};
	~PreviewApplication() throw() { }	

	GETSET(string, name, Name);
	GETSET(string, application, Application);
	GETSET(string, arguments, Arguments);
	GETSET(string, extension, Extension);
};
//RSX++ //Filters
class Filters {
public:
	typedef Filters* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
 
	Filters() { };
	Filters(const string& aFstring, bool aUseColor, bool hasFontC, bool hasBgC, int aFontColor, int aBackColor, bool aBoldFont, bool aItalicFont, bool aUnderline, bool aStrikeout) 
		throw() : fstring(aFstring), useColor(aUseColor), hasFontColor(hasFontC), hasBgColor(hasBgC), fontColor(aFontColor), backColor(aBackColor), boldFont(aBoldFont), italicFont(aItalicFont),
					underlineFont(aUnderline), strikeoutFont(aStrikeout) { };

	GETSET(string, fstring, Fstring);
	GETSET(bool, useColor, UseColor);
	GETSET(bool, hasFontColor, HasFontColor);
	GETSET(bool, hasBgColor, HasBgColor);
	GETSET(int, fontColor, FontColor);
	GETSET(int, backColor, BackColor);
	GETSET(bool, boldFont, BoldFont);
	GETSET(bool, italicFont, ItalicFont);
	GETSET(bool, underlineFont, UnderlineFont);
	GETSET(bool, strikeoutFont, StrikeoutFont);
};
//RSX++ //HighLight
class HighLight {
public:
	typedef HighLight* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
 
	HighLight() { };
	HighLight(const string& aString, bool hasFontC, bool hasBgC, int aFontColor, int aBackColor, bool aBoldFont, bool aItalicFont, bool uf, bool sf,
		bool aDisplayPopup, bool aFlashWindow, bool aPlaySound, const string& soundPath)
		throw() : hstring(aString), hasFontColor(hasFontC), hasBgColor(hasBgC), fontColor(aFontColor), backColor(aBackColor), boldFont(aBoldFont), italicFont(aItalicFont), underlineFont(uf), strikeoutFont(sf),
		displayPopup(aDisplayPopup), flashWindow(aFlashWindow), playSound(aPlaySound), soundFilePath(soundPath) { };

	GETSET(string, hstring, Hstring);
	GETSET(bool, hasFontColor, HasFontColor);
	GETSET(bool, hasBgColor, HasBgColor);
	GETSET(int, fontColor, FontColor);
	GETSET(int, backColor, BackColor);
	GETSET(bool, boldFont, BoldFont);
	GETSET(bool, italicFont, ItalicFont);
	GETSET(bool, underlineFont, UnderlineFont);
	GETSET(bool, strikeoutFont, StrikeoutFont);
	GETSET(bool, displayPopup, DisplayPopup);
	GETSET(bool, flashWindow, FlashWindow);
	GETSET(bool, playSound, PlaySound);
	GETSET(string, soundFilePath, SoundFilePath)
};
//END

class SimpleXML;

/**
 * Public hub list, favorites (hub&user). Assumed to be called only by UI thread.
 */
class FavoriteManager : public Speaker<FavoriteManagerListener>, private HttpConnectionListener, public Singleton<FavoriteManager>,
	private SettingsManagerListener, private ClientManagerListener
{
public:
// Public Hubs
	enum HubTypes {
		TYPE_NORMAL,
		TYPE_BZIP2
	};
	StringList getHubLists();
	void setHubList(int aHubList);
	int getSelectedHubList() { return lastServer; }
	void refresh(bool forceDownload = false);
	HubTypes getHubListType() { return listType; }
	HubEntryList getPublicHubs() {
		Lock l(cs);
		return publicListMatrix[publicListServer];
	}
	bool isDownloading() { return (useHttp && running); }

// Favorite Users
	typedef unordered_map<CID, FavoriteUser> FavoriteMap;
	FavoriteMap getFavoriteUsers() { Lock l(cs); return users; }
	PreviewApplication::List& getPreviewApps() { return previewApplications; }

	void addFavoriteUser(const UserPtr& aUser);
	bool isFavoriteUser(const UserPtr& aUser) const { Lock l(cs); return users.find(aUser->getCID()) != users.end(); }
	void removeFavoriteUser(const UserPtr& aUser);

	bool hasSlot(const UserPtr& aUser) const;
	void setUserDescription(const UserPtr& aUser, const string& description);
	void setAutoGrant(const UserPtr& aUser, bool grant);
	void userUpdated(const OnlineUser& info);
	time_t getLastSeen(const UserPtr& aUser) const;
	std::string getUserURL(const UserPtr& aUser) const;
	
// Favorite Hubs
	FavoriteHubEntryList& getFavoriteHubs() { return favoriteHubs; }

	void addFavorite(const FavoriteHubEntry& aEntry);
	void removeFavorite(const FavoriteHubEntry* entry);
	bool isFavoriteHub(const std::string& aUrl);
	FavoriteHubEntry* getFavoriteHubEntry(const string& aServer) const;

// Favorite Directories
	bool addFavoriteDir(const string& aDirectory, const string& aName);
	bool removeFavoriteDir(const string& aName);
	bool renameFavoriteDir(const string& aName, const string& anotherName);
	StringPairList getFavoriteDirs() { return favoriteDirs; }

// Recent Hubs
	RecentHubEntry::List& getRecentHubs() { return recentHubs; };

	void addRecent(const RecentHubEntry& aEntry);
	void removeRecent(const RecentHubEntry* entry);
	void updateRecent(const RecentHubEntry* entry);

	RecentHubEntry* getRecentHubEntry(const string& aServer) {
		for(RecentHubEntry::Iter i = recentHubs.begin(); i != recentHubs.end(); ++i) {
			RecentHubEntry* r = *i;
			if(Util::stricmp(r->getServer(), aServer) == 0) {
				return r;
			}
		}
		return NULL;
	}

	//RSX++ //dependent dirs
	DirectoriesEx::List& getDirectoriesEx() { return dirsEx; }
	DirectoriesEx* addDirEx(string name, string path, string extension){
		DirectoriesEx* dx = new DirectoriesEx(name, path, extension);
		dirsEx.push_back(dx);
		return dx;
	}

	void removeDirEx(unsigned int index){
		if(dirsEx.size() > index)
			dirsEx.erase(dirsEx.begin() + index);	
	}

	void getDirEx(unsigned int index, DirectoriesEx &dx){
		if(dirsEx.size() > index)
			dx = *dirsEx[index];	
	}
	
	void updateDirEx(int index, DirectoriesEx &dx){
		*dirsEx[index] = dx;
	}

	//RSX++ //HighLight
	HighLight* addHighLight(const string& stm, bool hFC, bool hBGC, int fc, int bc, bool bf, bool iF, bool uf, bool sf,
		bool dp, bool fw, bool ps, const string& sfp) {
		HighLight* hl = new HighLight(stm, hFC, hBGC, fc, bc, bf, iF, uf, sf, dp, fw, ps, sfp);
		hls.push_back(hl);
		return hl;
	}
	void getHighLight(unsigned int index, HighLight &hl) {
		if(hls.size() > index)
			hl = *hls[index];
	}
	void updateHighLight(int index, HighLight &hl) {
		*hls[index] = hl;
	}
	void removeHighLight(unsigned int index) {
		if(hls.size() > index)
			hls.erase(hls.begin() + index);
	}
	//RSX++ //Filters
	Filters* addFilter(const string& fstring, bool useColor, bool hFC, bool hBGC, int fc, int bc, bool bf, bool iF, bool uF, bool sF) {
		Filters* fs = new Filters(fstring, useColor, hFC, hBGC, fc, bc, bf, iF, uF, sF);
		filters.push_back(fs);
		return fs;
	}
	void getFilter(unsigned int index, Filters &fs) {
		if(filters.size() > index)
			fs = *filters[index];
	}
	void updateFilter(int index, Filters &fs) {
		*filters[index] = fs;
	}
	void removeFilter(unsigned int index) {
		if(filters.size() > index)
			filters.erase(filters.begin() + index);
	}
	//RSX++ //away msg string
	string getAwayMessage(const string& aServer);
	//RSX++ //FavHubGroups
	StringList& getFavGroups() { return favGroups; }
	bool addFavGroup(const string& gName) {
		for(StringIter i = favGroups.begin(); i != favGroups.end(); ++i) {
			if(gName.compare((*i)) == 0) return false;
		}
		favGroups.push_back(gName);
		return true;
	}
	void removeFavGroup(uint8_t pos) {
		Lock l(cs);
		StringIter i = favGroups.begin() + pos;
		if(i != favGroups.end())
			favGroups.erase(i);
	}
	bool editFavGroup(uint8_t pos, const string& gName) {
		for(StringIter i = favGroups.begin(); i != favGroups.end(); ++i)
			if(gName.compare((*i)) == 0) return false;
		favGroups[pos] = gName;
		return true;
	}
	//END
	PreviewApplication* addPreviewApp(string name, string application, string arguments, string extension){
		PreviewApplication* pa = new PreviewApplication(name, application, arguments, extension);
		previewApplications.push_back(pa);
		return pa;
	}

	PreviewApplication* removePreviewApp(unsigned int index){
		if(previewApplications.size() > index)
			previewApplications.erase(previewApplications.begin() + index);	
		return NULL;
	}

	PreviewApplication* getPreviewApp(unsigned int index, PreviewApplication &pa){
		if(previewApplications.size() > index)
			pa = *previewApplications[index];	
		return NULL;
	}
	
	PreviewApplication* updatePreviewApp(int index, PreviewApplication &pa){
		*previewApplications[index] = pa;
		return NULL;
	}

	void removeallRecent() {
		recentHubs.clear();
		recentsave();
	}

	//RSX++ //Raw Manager
	bool getActifAction(FavoriteHubEntry* entry, int actionId);
	void setActifAction(FavoriteHubEntry* entry, int actionId, bool actif);
	bool getActifRaw(FavoriteHubEntry* entry, int actionId, int rawId);
	void setActifRaw(FavoriteHubEntry* entry, int actionId, int rawId, bool actif);
	//END

// User Commands
	UserCommand addUserCommand(int type, int ctx, Flags::MaskType flags, const string& name, const string& command, const string& hub);
	bool getUserCommand(int cid, UserCommand& uc);
	int findUserCommand(const string& aName, const string& aUrl);
	bool moveUserCommand(int cid, int pos);
	void updateUserCommand(const UserCommand& uc);
	void removeUserCommand(int cid);
	void removeUserCommand(const string& srv);
	void removeHubUserCommands(int ctx, const string& hub);

	UserCommand::List getUserCommands() { Lock l(cs); return userCommands; }
	UserCommand::List getUserCommands(int ctx, const StringList& hub, bool& op);
	//RSX++
	Filters::List& getFilters() { Lock l(cs); return filters; }
	HighLight::List& getHLs() { Lock l(cs); return hls; }

	void loadFilters();
	void filtersSave();
	void saveHL();
	//END
	void load();
	void save();
	void recentsave();
	
private:
	FavoriteHubEntryList favoriteHubs;
	StringPairList favoriteDirs;
	RecentHubEntry::List recentHubs;
	PreviewApplication::List previewApplications;
	UserCommand::List userCommands;
	int lastId;

	FavoriteMap users;
	//RSX++
	Filters::List filters;
	DirectoriesEx::List dirsEx;
	HighLight::List hls;
	StringList favGroups;
	//END

	mutable CriticalSection cs;

	// Public Hubs
	typedef unordered_map<string, HubEntryList> PubListMap;
	PubListMap publicListMatrix;
	string publicListServer;
	bool useHttp, running;
	HttpConnection* c;
	int lastServer;
	HubTypes listType;
	string downloadBuf;
	
	/** Used during loading to prevent saving. */
	bool dontSave;

	friend class Singleton<FavoriteManager>;
	
	FavoriteManager();
	~FavoriteManager() throw();
	
	FavoriteHubEntryList::const_iterator getFavoriteHub(const string& aServer);
	void loadXmlList(const string& xml);

	RecentHubEntry::Iter getRecentHub(const string& aServer) const {
		for(RecentHubEntry::Iter i = recentHubs.begin(); i != recentHubs.end(); ++i) {
			if(Util::stricmp((*i)->getServer(), aServer) == 0) {
				return i;
			}
		}
		return recentHubs.end();
	}

	// ClientManagerListener
	void on(UserUpdated, const OnlineUser& user) throw();
	void on(UserConnected, const UserPtr& user) throw();
	void on(UserDisconnected, const UserPtr& user) throw();

	// HttpConnectionListener
	void on(Data, HttpConnection*, const uint8_t*, size_t) throw();
	void on(Failed, HttpConnection*, const string&) throw();
	void on(Complete, HttpConnection*, const string&) throw();
	void on(Redirected, HttpConnection*, const string&) throw();
	void on(TypeNormal, HttpConnection*) throw();
	void on(TypeBZ2, HttpConnection*) throw();

	void onHttpFinished(bool fromHttp) throw();

	// SettingsManagerListener
	void on(SettingsManagerListener::Load, SimpleXML& xml) throw() {
		load(xml);
		recentload(xml);
		previewload(xml);
		dirsExLoad(xml); //RSX++
	}

	void on(SettingsManagerListener::Save, SimpleXML& xml) throw() {
		previewsave(xml);
		dirsExSave(xml); //RSX++
	}

	void load(SimpleXML& aXml);
	void recentload(SimpleXML& aXml);
	void previewload(SimpleXML& aXml);
	void previewsave(SimpleXML& aXml);
	//RSX++
	void loadFilters(SimpleXML& aXml);
	void loadHighLight(SimpleXML& aXml);
	void dirsExLoad(SimpleXML& aXml);
	void dirsExSave(SimpleXML& aXml);
	//END
	string getConfigFile() { return Util::getConfigPath() + "Favorites.xml"; }
};

} // namespace dcpp

#endif // !defined(FAVORITE_MANAGER_H)

/**
 * @file
 * $Id: FavoriteManager.h 382 2008-03-09 10:40:22Z BigMuscle $
 */
