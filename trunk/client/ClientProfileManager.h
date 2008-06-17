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

#if !defined(CLIENT_PROFILE_MANAGER_H)
#define CLIENT_PROFILE_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "../rsx/RsxUtil.h"

#include "CriticalSection.h"
#include "Singleton.h"

namespace dcpp {

class ClientProfile {
public:
	typedef vector<ClientProfile> List;

	ClientProfile() : id(0), priority(0), rawToSend(0), keyType(0) { };

	ClientProfile(int aId, int aProfileId, const string& aName, const string& aVersion, const string& aTag, 
		const string& aExtendedTag, const string& aLock, const string& aPk, const string& aSupports, const string& aTestSUR, 
		const string& aUserConCom, const string& aStatus, const string& aCheatingDescription, int aRawToSend, int aUseExtraVersion, 
		int aCheckMismatch,const string& aConnection, const string& aComment, int aRecheck, int aKeyType) 
		throw() : id(aId), profileId(aProfileId), name(aName), version(aVersion), tag(aTag), extendedTag(aExtendedTag), lock(aLock), 
		pk(aPk), supports(aSupports), testSUR(aTestSUR), userConCom(aUserConCom), status(aStatus), cheatingDescription(aCheatingDescription), 
		rawToSend(aRawToSend), useExtraVersion(aUseExtraVersion), checkMismatch(aCheckMismatch), connection(aConnection), comment(aComment),
		recheck(aRecheck), keyType(aKeyType)
	{ };

	ClientProfile(const ClientProfile& rhs) : id(rhs.id), profileId(rhs.profileId), name(rhs.name), version(rhs.version), tag(rhs.tag), 
		extendedTag(rhs.extendedTag), lock(rhs.lock), pk(rhs.pk), supports(rhs.supports), testSUR(rhs.testSUR), userConCom(rhs.userConCom), 
		status(rhs.status), cheatingDescription(rhs.cheatingDescription), rawToSend(rhs.rawToSend), useExtraVersion(rhs.useExtraVersion), 
		checkMismatch(rhs.checkMismatch), connection(rhs.connection), comment(rhs.comment), recheck(rhs.recheck), keyType(rhs.keyType)
	{ }

	ClientProfile& operator=(const ClientProfile& rhs) {id = rhs.id; profileId = rhs.profileId; name = rhs.name; version = rhs.version;
		tag = rhs.tag; extendedTag = rhs.extendedTag; lock = rhs.lock; pk = rhs.pk; supports = rhs.supports; testSUR = rhs.testSUR;
		userConCom = rhs.userConCom; status = rhs.status; rawToSend = rhs.rawToSend; cheatingDescription = rhs.cheatingDescription; 
		useExtraVersion = rhs.useExtraVersion; checkMismatch = rhs.checkMismatch; connection = rhs.connection; comment = rhs.comment;
		recheck = rhs.recheck; keyType = rhs.keyType;
		return *this;
	}

	GETSET(int, id, Id);
	GETSET(int, profileId, ProfileId);
	GETSET(string, name, Name);
	GETSET(string, version, Version);
	GETSET(string, tag, Tag);
	GETSET(string, extendedTag, ExtendedTag);
	GETSET(string, lock, Lock);
	GETSET(string, pk, Pk);
	GETSET(string, supports, Supports);
	GETSET(string, testSUR, TestSUR);
	GETSET(string, userConCom, UserConCom);
	GETSET(string, status, Status);
	GETSET(string, cheatingDescription, CheatingDescription);
	GETSET(string, connection, Connection);
	GETSET(string, comment, Comment);
	GETSET(int, priority, Priority);
	GETSET(int, rawToSend, RawToSend);
	GETSET(int, useExtraVersion, UseExtraVersion);
	GETSET(int, checkMismatch, CheckMismatch);
	GETSET(int, recheck, Recheck);
	GETSET(int, keyType, KeyType);
};

class MyinfoProfile {
public:
	typedef vector<MyinfoProfile> List;

	MyinfoProfile() : id(0), priority(0), rawToSend(0) { };

	MyinfoProfile(int aId, const string& aName, const string& aVersion, const string& aTag, const string& aExtendedTag, 
		const string& aNick, const string& aShared, const string& aEmail, const string& aStatus, const string& aCheatingDescription, 
		int aRawToSend, bool aUseExtraVersion, const string& aConnection, const string& aComment, int aProfileId) 
		throw() : id(aId), name(aName), version(aVersion), tag(aTag), extendedTag(aExtendedTag), 
		nick(aNick), shared(aShared), email(aEmail), status(aStatus), cheatingDescription(aCheatingDescription), 
		rawToSend(aRawToSend), useExtraVersion(aUseExtraVersion), connection(aConnection), comment(aComment), 
		profileId(aProfileId)
		{ };

	MyinfoProfile(const MyinfoProfile& rhs) : id(rhs.id), name(rhs.name), version(rhs.version), tag(rhs.tag),
		extendedTag(rhs.extendedTag), nick(rhs.nick),  shared(rhs.shared), email(rhs.email), status(rhs.status), 
		cheatingDescription(rhs.cheatingDescription), rawToSend(rhs.rawToSend), useExtraVersion(rhs.useExtraVersion), 
		connection(rhs.connection), comment(rhs.comment), profileId(rhs.profileId)
	{ }

	MyinfoProfile& operator=(const MyinfoProfile& rhs) {id = rhs.id; name = rhs.name; version = rhs.version; tag = rhs.tag;
		extendedTag = rhs.extendedTag; nick = rhs.nick; shared = rhs.shared; email = rhs.email; status = rhs.status;
		rawToSend = rhs.rawToSend; cheatingDescription = rhs.cheatingDescription; useExtraVersion = rhs.useExtraVersion; 
		connection = rhs.connection; comment = rhs.comment; profileId = rhs.profileId;
		return *this;
	}

	GETSET(int, id, Id);
	GETSET(string, name, Name);
	GETSET(string, version, Version);
	GETSET(string, tag, Tag);
	GETSET(string, extendedTag, ExtendedTag);
	GETSET(string, nick, Nick);
	GETSET(string, shared, Shared);
	GETSET(string, email, Email);
	GETSET(string, status, Status);
	GETSET(string, cheatingDescription, CheatingDescription);
	GETSET(string, connection, Connection);
	GETSET(string, comment, Comment);
	GETSET(int, priority, Priority);
	GETSET(int, rawToSend, RawToSend);
	GETSET(bool, useExtraVersion, UseExtraVersion);
	GETSET(int, profileId, ProfileId);
};

class FileListDetectorProfile {
public:
	typedef vector<FileListDetectorProfile> List;

	FileListDetectorProfile() : id(0), rawToSend(0) { };

	FileListDetectorProfile(int aId, const string& aName,  const string& aDetect, const string& aCheatingDescription, 
		int aRawToSend, bool aBadClient) throw() : id(aId), name(aName), detect(aDetect),
		cheatingDescription(aCheatingDescription), rawToSend(aRawToSend), badClient(aBadClient) { };

	FileListDetectorProfile(const FileListDetectorProfile& rhs) : id(rhs.id), name(rhs.name),
		detect(rhs.detect), cheatingDescription(rhs.cheatingDescription), rawToSend(rhs.rawToSend),
		badClient(rhs.badClient) { }

	FileListDetectorProfile& operator=(const FileListDetectorProfile& rhs) { id = rhs.id; name = rhs.name;
		detect = rhs.detect; rawToSend = rhs.rawToSend; cheatingDescription = rhs.cheatingDescription;
		badClient = rhs.badClient;
		return *this;
	}

	GETSET(int, id, Id);
	GETSET(string, name, Name);
	GETSET(string, detect, Detect);
	GETSET(string, cheatingDescription, CheatingDescription);
	GETSET(int, rawToSend, RawToSend);
	GETSET(bool, badClient, BadClient);
};

class SimpleXML;
class ClientProfileManager : public Singleton<ClientProfileManager> {
public:
	StringMap&						getParams()								{ Lock l(ccs);		return params; }	
	ClientProfile::List&			getClientProfiles()						{ Lock l(ccs);		return clientProfiles; }
	ClientProfile::List&			getClientProfiles(StringMap &paramList) { Lock l(ccs);		paramList = params; return clientProfiles; }
	FileListDetectorProfile::List&	getFileListDetectors()					{ Lock l(fgcs);		return fileListDetector; }
	MyinfoProfile::List&			getMyinfoProfiles()						{ Lock l(mipcs);	return myinfoProfiles; }
	MyinfoProfile::List&			getMyinfoProfiles(StringMap &paramList) { Lock l(mipcs);	paramList = params; return myinfoProfiles; }

	ClientProfile addClientProfile(const string& name, const string& version, const string& tag, const string& extendedTag, const string& lock, const string& pk, const string& supports, const string& testSUR, const string& userConCom, const string& status,const string& cheatingdescription, int rawToSend, int useExtraVersion, int checkMismatch, const string& connection, const string& comment, int recheck, int keyType, int profileId, bool isNew = false);
	MyinfoProfile addMyinfoProfile(const string& name, const string& version, const string& tag, const string& extendedTag, const string& nick, const string& shared, const string& email, const string& status, const string& cheatingdescription, int rawToSend, bool useExtraVersion, const string& connection, const string& comment, int profileId, bool isNew = false);
	FileListDetectorProfile addFileListDetector(const string& name, const string& detect, const string& cheatingdescription, int rawToSend, bool badClient,bool isNew = false);

	void addClientProfile(const StringList& sl);
	void addMyinfoProfile(const StringList& sl);
	void addFileListDetector(const StringList& sl);

	bool getClientProfile(int id, ClientProfile& cp);
	bool getMyinfoProfile(int id, MyinfoProfile& ip);
	bool getFileListDetector(int id, FileListDetectorProfile& fd);

	void removeClientProfile(int id);
	void removeMyinfoProfile(int id);
	void removeFileListDetector(int id);

	void updateClientProfile(const ClientProfile& cp);
	void updateMyinfoProfile(const MyinfoProfile& ip);
	void updateFileListDetector(const FileListDetectorProfile& fd);

	bool moveClientProfile(int id, int pos);
	bool moveMyinfoProfile(int id, int pos);

	ClientProfile::List& reloadClientProfiles();
	MyinfoProfile::List& reloadMyinfoProfiles();

	void reloadClientProfilesFromHttp();
	void reloadMyinfoProfilesFromHttp();

	void load() { loadClientProfiles(); loadMyinfoProfiles(); };

	void loadClientProfiles();
	void loadMyinfoProfiles();

	void saveClientProfiles();
	void saveMyinfoProfiles();

	GETSET(string, profileVersion, ProfileVersion);
	GETSET(string, profileMessage, ProfileMessage);
	GETSET(string, profileUrl, ProfileUrl);

	GETSET(string, myinfoProfileVersion, MyinfoProfileVersion);
	GETSET(string, myinfoProfileMessage, MyinfoProfileMessage);
	GETSET(string, myinfoProfileUrl, MyinfoProfileUrl);

private:
	friend class Singleton<ClientProfileManager>;

	ClientProfileManager() : lastProfile(0), lastFDProfile(0), lastMyinfoProfile(0) {}
	~ClientProfileManager() { }

	ClientProfile::List clientProfiles;
	MyinfoProfile::List myinfoProfiles;
	FileListDetectorProfile::List fileListDetector;

	ClientProfile::List newCpList;
	MyinfoProfile::List newMyinfoProfiles;
	FileListDetectorProfile::List newFileListDetector;

	StringMap params;
	StringMap newParams;

	void loadClientProfiles(SimpleXML& aXml, bool isNew = false);
	void loadMyinfoProfiles(SimpleXML& aXml, bool isNew = false);
	void loadFakeShares(SimpleXML& aXml);

	int lastProfile;
	int lastMyinfoProfile;
	int lastFDProfile;

	CriticalSection ccs;
	CriticalSection mipcs;
	CriticalSection fgcs;
};
}; // namespace dcpp
#endif //CLIENT_PROFILE_MANAGER_H
