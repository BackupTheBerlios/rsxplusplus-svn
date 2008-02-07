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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ClientProfileManager.h"
#include "SimpleXML.h"
#include "RawManager.h"

#define MYINFO_PROFILES_FILE "MyinfoProfiles.xml"
#define FAKES_FILE "Fakes.xml"
#define ISP_FILE "ISPs.xml"

void ClientProfileManager::saveClientProfiles() {
	try {
		SimpleXML xml;
		xml.addTag("Profiles");
		xml.stepIn();

		xml.addTag("ClientProfilesV2");
		xml.stepIn();
		for(ClientProfile::List::const_iterator l = clientProfiles.begin(); l != clientProfiles.end(); ++l) {
			xml.addTag("ClientProfile");
			xml.stepIn();
			xml.addTag("Name", l->getName());
			xml.addTag("Version", l->getVersion());
			xml.addTag("Tag", l->getTag());
			xml.addTag("ExtendedTag", l->getExtendedTag());
			xml.addTag("Lock", l->getLock());
			xml.addTag("Pk", l->getPk());
			xml.addTag("Supports", l->getSupports());
			xml.addTag("TestSUR", l->getTestSUR());
			xml.addTag("UserConCom", l->getUserConCom());
			xml.addTag("Status", l->getStatus());
			xml.addTag("CheatingDescription", l->getCheatingDescription());
			xml.addTag("RawToSend", Util::toString(RawManager::getInstance()->getValidAction(l->getRawToSend())));
			xml.addTag("UseExtraVersion", Util::toString(l->getUseExtraVersion()));
			xml.addTag("CheckMismatch", Util::toString(l->getCheckMismatch()));
			xml.addTag("Connection", l->getConnection());
			xml.addTag("Comment", l->getComment());
			xml.addTag("Recheck", l->getRecheck());
			xml.addTag("KeyType", l->getKeyType());
			xml.addTag("ProfileID", l->getProfileId());
			xml.stepOut();
		}
		xml.stepOut();

		xml.addTag("Params");
		xml.stepIn();
		for(StringMap::iterator m = params.begin(); m != params.end(); ++m) {
			xml.addTag("Param");
			xml.addChildAttrib("Name", m->first);
			xml.addChildAttrib("RegExp", m->second);
		}
		xml.stepOut();

		xml.addTag("FileListDetector");
		xml.stepIn();
		for(FileListDetectorProfile::List::const_iterator l = fileListDetector.begin(); l != fileListDetector.end(); ++l) {
			xml.addTag("FileListDetectorProfile");
			xml.stepIn();
			xml.addTag("Name", l->getName());
			xml.addTag("Detect", l->getDetect());
			xml.addTag("CheatingDescription", l->getCheatingDescription());
			xml.addTag("RawToSend", Util::toString(RawManager::getInstance()->getValidAction(l->getRawToSend())));
			xml.addTag("BadClient", Util::toString(l->getBadClient()));
			xml.stepOut();
		}
		xml.stepOut();

		xml.addTag("ProfileInfo");
		xml.stepIn();
			xml.addTag("ProfileVersion", ClientProfileManager::getInstance()->getProfileVersion());
			xml.addTag("ProfileMessage", ClientProfileManager::getInstance()->getProfileMessage());
			xml.addTag("ProfileUrl", ClientProfileManager::getInstance()->getProfileUrl());
		xml.stepOut();

		xml.stepOut();

		string fname = Util::getConfigPath() + "Profiles.xml";

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);

	} catch(const Exception& e) {
		dcdebug("FavoriteManager::saveClientProfiles: %s\n", e.getError().c_str());
	}
}

void ClientProfileManager::loadClientProfiles(SimpleXML& aXml, bool isNew/* = false*/) {
	StringList sl, fl;
	aXml.resetCurrentChild();
	if(aXml.findChild("ClientProfilesV2")) {
		aXml.stepIn();
		while(aXml.findChild("ClientProfile")) {
			aXml.stepIn();
			if(aXml.findChild("Name"))					{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Version"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Tag"))					{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("ExtendedTag"))			{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Lock"))					{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Pk"))					{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Supports"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("TestSUR"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("UserConCom"))			{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Status"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("CheatingDescription"))	{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("RawToSend"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("UseExtraVersion"))		{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("CheckMismatch"))			{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Connection"))			{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Comment"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Recheck"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("KeyType"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("ProfileID"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }

			addClientProfile(sl[0], sl[1], sl[2], sl[3], sl[4], sl[5], sl[6], sl[7], sl[8], sl[9], sl[10], RawManager::getInstance()->getValidAction(Util::toInt(sl[11])),
				Util::toInt(sl[12]), Util::toInt(sl[13]), sl[14], sl[15], Util::toInt(sl[16]), Util::toInt(sl[17]), Util::toInt(sl[18]), isNew);
			sl.clear();
			aXml.stepOut();
		}
		aXml.stepOut();
	}
	aXml.resetCurrentChild();
	if(aXml.findChild("Params")) {
		aXml.stepIn();
		while(aXml.findChild("Param")) {
			if(isNew) {
				newParams[aXml.getChildAttrib("Name")] = aXml.getChildAttrib("RegExp");
			} else {
				params[aXml.getChildAttrib("Name")] = aXml.getChildAttrib("RegExp");
			}
		}
		aXml.stepOut();
	}

	aXml.resetCurrentChild();
	if(aXml.findChild("FileListDetector")) {
		aXml.stepIn();
		while(aXml.findChild("FileListDetectorProfile")) {
			aXml.stepIn();
			if(aXml.findChild("Name"))					{ fl.push_back(aXml.getChildData()); }	else { fl.push_back(Util::emptyString); }	
			if(aXml.findChild("Detect"))				{ fl.push_back(aXml.getChildData()); }	else { fl.push_back(Util::emptyString); }	
			if(aXml.findChild("CheatingDescription"))	{ fl.push_back(aXml.getChildData()); }	else { fl.push_back(Util::emptyString); }
			if(aXml.findChild("RawToSend"))				{ fl.push_back(aXml.getChildData()); }	else { fl.push_back(Util::emptyString); }
			if(aXml.findChild("BadClient"))				{ fl.push_back(aXml.getChildData()); }	else { fl.push_back(Util::emptyString); }
			
			addFileListDetector(fl[0], fl[1], fl[2], RawManager::getInstance()->getValidAction(Util::toInt(fl[3])), RsxUtil::toBool(fl[4]), isNew);
			fl.clear();
			aXml.stepOut();
		}
		aXml.stepOut();
	}

	aXml.resetCurrentChild();
	if(aXml.findChild("ProfileInfo")) {
		aXml.stepIn();
		if(aXml.findChild("ProfileVersion"))	{ ClientProfileManager::getInstance()->setProfileVersion(aXml.getChildData()); }
		if(aXml.findChild("ProfileMessage"))	{ ClientProfileManager::getInstance()->setProfileMessage(aXml.getChildData()); }
		if(aXml.findChild("ProfileUrl"))		{ ClientProfileManager::getInstance()->setProfileUrl(aXml.getChildData()); }
		aXml.stepOut();
	}
}

void ClientProfileManager::loadClientProfiles() {
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + "Profiles.xml", File::READ, File::OPEN).read());
		
		if(xml.findChild("Profiles")) {
			xml.stepIn();
			loadClientProfiles(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::loadClientProfiles: %s\n", e.getError().c_str());
	}
}
//@todo rewrite it, because this is one big ugly piece of code
//same with myinfo
void ClientProfileManager::reloadClientProfilesFromHttp() {
	Lock l(ccs);
	lastProfile = 0;

	//load new list
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + "Profiles.xml.new", File::READ, File::OPEN).read());
		
		if(xml.findChild("Profiles")) {
			xml.stepIn();
			loadClientProfiles(xml, true);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::loadClientProfiles: %s\n", e.getError().c_str());
	}

	//compare new and old list, and if id match, copy raw setting
	for(ClientProfile::List::iterator newCp = newCpList.begin(); newCp != newCpList.end(); ++newCp) {
		for(ClientProfile::List::const_iterator oldCp = clientProfiles.begin(); oldCp != clientProfiles.end(); ++oldCp) {
			if((*newCp).getProfileId() > 0 && ((*newCp).getProfileId() == (*oldCp).getProfileId())) {
				(*newCp).setRawToSend((*oldCp).getRawToSend());
			}
		}
	}
	//update old lists
	clientProfiles.clear();
	clientProfiles = newCpList;

	params.clear();
	params = newParams;

	fileListDetector.clear();
	fileListDetector = newFileListDetector;

	//and clear new lists
	newCpList.clear();
	newParams.clear();
	newFileListDetector.clear();

	//save results
	saveClientProfiles();

	//last - delete new file
	try {
		File::deleteFile(Util::getConfigPath() + "Profiles.xml.new");
	} catch(...) {
		//probably file don't exist :)
	}
}

//RSX++ // MyInfo
void ClientProfileManager::saveMyinfoProfiles() {
	try {
		SimpleXML xml;
		xml.addTag("Myinfo");
		xml.stepIn();

		xml.addTag("MyinfoProfiles");
		xml.stepIn();
		for(MyinfoProfile::List::const_iterator l = myinfoProfiles.begin(); l != myinfoProfiles.end(); ++l) {
			xml.addTag("MyinfoProfile");
			xml.stepIn();
			xml.addTag("Name", l->getName());
			xml.addTag("Version", l->getVersion());
			xml.addTag("Tag", l->getTag());
			xml.addTag("ExtendedTag", l->getExtendedTag());
			xml.addTag("Nick", l->getNick());
			xml.addTag("Shared", l->getShared());
			xml.addTag("Email", l->getEmail());
			xml.addTag("Status", l->getStatus());
			xml.addTag("CheatingDescription", l->getCheatingDescription());
			xml.addTag("RawToSend", Util::toString(l->getRawToSend()));
			xml.addTag("UseExtraVersion", Util::toString(l->getUseExtraVersion()));
			xml.addTag("Connection", l->getConnection());
			xml.addTag("Comment", l->getComment());
			xml.addTag("ProfileID", l->getProfileId());
			xml.stepOut();
		}
		xml.stepOut();

		xml.addTag("ProfileInfo");
		xml.stepIn();
			xml.addTag("ProfileVersion", ClientProfileManager::getInstance()->getMyinfoProfileVersion());
			xml.addTag("ProfileMessage", ClientProfileManager::getInstance()->getMyinfoProfileMessage());
			xml.addTag("ProfileUrl", ClientProfileManager::getInstance()->getMyinfoProfileUrl());
		xml.stepOut();

		xml.stepOut();

		string fname = Util::getConfigPath() + MYINFO_PROFILES_FILE;

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);

	} catch(const Exception& e) {
		dcdebug("HubManager::saveMyinfoProfiles: %s\n", e.getError().c_str());
	}
}

void ClientProfileManager::loadMyinfoProfiles() {
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + MYINFO_PROFILES_FILE, File::READ, File::OPEN).read());
		if(xml.findChild("Myinfo")) {
			xml.stepIn();
			loadMyinfoProfiles(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("HubManager::loadMyinfoProfiles: %s\n", e.getError().c_str());
	}
}

void ClientProfileManager::loadMyinfoProfiles(SimpleXML& aXml, bool isNew/* = false*/) {
	StringList sl;
	aXml.resetCurrentChild();
	if(aXml.findChild("MyinfoProfiles")) {
		aXml.stepIn();
		while(aXml.findChild("MyinfoProfile")) {
			aXml.stepIn();
			if(aXml.findChild("Name"))					{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Version"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Tag"))					{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("ExtendedTag"))			{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Nick"))					{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Shared"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Email"))					{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Status"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("CheatingDescription"))	{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("RawToSend"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("UseExtraVersion"))		{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Connection"))			{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }
			if(aXml.findChild("Comment"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }		
			if(aXml.findChild("ProfileID"))				{ sl.push_back(aXml.getChildData()); }	else { sl.push_back(Util::emptyString); }		

			addMyinfoProfile(sl[0], sl[1], sl[2], sl[3], sl[4], sl[5], sl[6], sl[7], sl[8], Util::toInt(sl[9]), RsxUtil::toBool(sl[10]), sl[11], sl[12],
				Util::toInt(sl[13]), isNew);
			sl.clear();
			aXml.stepOut();
		}
		aXml.stepOut();
	} 
	aXml.resetCurrentChild();
	if(aXml.findChild("ProfileInfo")) {
		aXml.stepIn();
		if(aXml.findChild("ProfileVersion"))	{ ClientProfileManager::getInstance()->setMyinfoProfileVersion(aXml.getChildData()); }
		if(aXml.findChild("ProfileMessage"))	{ ClientProfileManager::getInstance()->setMyinfoProfileMessage(aXml.getChildData()); }
		if(aXml.findChild("ProfileUrl"))		{ ClientProfileManager::getInstance()->setMyinfoProfileUrl(aXml.getChildData()); }
		aXml.stepOut();
	}
}

void ClientProfileManager::reloadMyinfoProfilesFromHttp() {
	Lock l(ccs);
	lastMyinfoProfile = 0;

	//load new list
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + "MyinfoProfiles.xml.new", File::READ, File::OPEN).read());
		
		if(xml.findChild("Myinfo")) {
			xml.stepIn();
			loadMyinfoProfiles(xml, true);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::loadMyInfoProfiles: %s\n", e.getError().c_str());
	}

	//compare new and old list, and if id match, copy raw setting
	for(MyinfoProfile::List::iterator newCp = newMyinfoProfiles.begin(); newCp != newMyinfoProfiles.end(); ++newCp) {
		for(MyinfoProfile::List::const_iterator oldCp = myinfoProfiles.begin(); oldCp != myinfoProfiles.end(); ++oldCp) {
			if((*newCp).getProfileId() > 0 && ((*newCp).getProfileId() == (*oldCp).getProfileId())) {
				(*newCp).setRawToSend((*oldCp).getRawToSend());
			}
		}
	}
	//update old lists
	myinfoProfiles.clear();
	myinfoProfiles = newMyinfoProfiles;

	//and clear new lists
	newMyinfoProfiles.clear();

	//save results
	saveMyinfoProfiles();

	//delete new file
	try {
		File::deleteFile(Util::getConfigPath() + "MyinfoProfiles.xml.new");
	} catch(...) {
		//probably file don't exist :)
	}
}

ClientProfile ClientProfileManager::addClientProfile(const string& name, const string& version, const string& tag, const string& extendedTag, const string& lock, 
													 const string& pk, const string& supports, const string& testSUR, const string& userConCom, const string& status,
													 const string& cheatingdescription, int rawToSend, int useExtraVersion, int checkMismatch, const string& connection, 
													 const string& comment, int recheck, int keyType, int profileId, bool isNew/* = false*/) {
	Lock l(ccs);
	if(isNew) {
		newCpList.push_back(ClientProfile(lastProfile++, profileId, name, version, tag, extendedTag, 
			lock, pk, supports, testSUR, userConCom, status, cheatingdescription, rawToSend, useExtraVersion, 
			checkMismatch, connection, comment, recheck, keyType));
		return newCpList.back();
	} else {
		clientProfiles.push_back(ClientProfile(lastProfile++, profileId, name, version, tag, extendedTag, 
			lock, pk, supports, testSUR, userConCom, status, cheatingdescription, rawToSend, useExtraVersion, 
			checkMismatch, connection, comment, recheck, keyType));
		return clientProfiles.back();
	}
}

void ClientProfileManager::addClientProfile(const StringList& sl) {
	Lock l(ccs);
	clientProfiles.push_back(ClientProfile(lastProfile++, Util::toInt(sl[0]), sl[1], sl[2], sl[3], sl[4], 
		sl[5], sl[6], sl[7], sl[8], sl[9], sl[10], sl[11], Util::toInt(sl[12]), Util::toInt(sl[13]), 
		Util::toInt(sl[14]), sl[15],  sl[16], Util::toInt(sl[17]), Util::toInt(sl[18])));
}

bool ClientProfileManager::getClientProfile(int id, ClientProfile& cp) {
	Lock l(ccs);
	for(ClientProfile::List::const_iterator i = clientProfiles.begin(); i != clientProfiles.end(); ++i) {
		if(i->getId() == id) {
			cp = *i;
			return true;
		}
	}
	return false;
}

void ClientProfileManager::removeClientProfile(int id) {
	Lock l(ccs);
	for(ClientProfile::List::iterator i = clientProfiles.begin(); i != clientProfiles.end(); ++i) {
		if(i->getId() == id) {
			clientProfiles.erase(i);
			break;
		}
	}
}

void ClientProfileManager::updateClientProfile(const ClientProfile& cp) {
	Lock l(ccs);
	for(ClientProfile::List::iterator i = clientProfiles.begin(); i != clientProfiles.end(); ++i) {
		if(i->getId() == cp.getId()) {
			*i = cp;
			break;
		}
	}
}

bool ClientProfileManager::moveClientProfile(int id, int pos) {
	dcassert(pos == -1 || pos == 1);
	Lock l(ccs);
	for(ClientProfile::List::iterator i = clientProfiles.begin(); i != clientProfiles.end(); ++i) {
		if(i->getId() == id) {
			swap(*i, *(i + pos));
			return true;
		}
	}
	return false;
}

ClientProfile::List& ClientProfileManager::reloadClientProfiles() { 
	Lock l(ccs);
	clientProfiles.clear();
	params.clear();
	fileListDetector.clear();
	loadClientProfiles();
	return clientProfiles;
}

MyinfoProfile ClientProfileManager::addMyinfoProfile(const string& name, const string& version, const string& tag, const string& extendedTag, const string& nick, 
													 const string& shared, const string& email, const string& status, const string& cheatingdescription, int rawToSend, 
													 bool useExtraVersion, const string& connection, const string& comment, int profileId, bool isNew/* = false*/) {
	Lock l(mipcs);
	if(isNew) {
		newMyinfoProfiles.push_back(MyinfoProfile(lastMyinfoProfile++, name, version, tag, extendedTag, nick, shared, 
			email,status, cheatingdescription,rawToSend, useExtraVersion, connection, comment, profileId));
		return newMyinfoProfiles.back();
	} else {
		myinfoProfiles.push_back(MyinfoProfile(lastMyinfoProfile++, name, version, tag, extendedTag, nick, shared, 
			email,status, cheatingdescription,rawToSend, useExtraVersion, connection, comment, profileId));
		return myinfoProfiles.back();
	}
}

void ClientProfileManager::addMyinfoProfile(const StringList& sl) {
	Lock l(mipcs);
	myinfoProfiles.push_back(MyinfoProfile(lastMyinfoProfile++, sl[0], sl[1], sl[2], sl[3], sl[4], sl[5], sl[6], 
		sl[7], sl[8], Util::toInt(sl[9]), RsxUtil::toBool(sl[10]), sl[11], sl[12], Util::toInt(sl[13])));
	saveMyinfoProfiles();
}

bool ClientProfileManager::getMyinfoProfile(int id, MyinfoProfile& ip) {
	Lock l(mipcs);
	for(MyinfoProfile::List::const_iterator i = myinfoProfiles.begin(); i != myinfoProfiles.end(); ++i) {
		if(i->getId() == id) {
			ip = *i;
			return true;
		}
	}
	return false;
}

void ClientProfileManager::removeMyinfoProfile(int id) {
	Lock l(mipcs);
	for(MyinfoProfile::List::iterator i = myinfoProfiles.begin(); i != myinfoProfiles.end(); ++i) {
		if(i->getId() == id) {
			myinfoProfiles.erase(i);
			break;
		}
	}
}

void ClientProfileManager::updateMyinfoProfile(const MyinfoProfile& ip) {
	Lock l(mipcs);
	for(MyinfoProfile::List::iterator i = myinfoProfiles.begin(); i != myinfoProfiles.end(); ++i) {
		if(i->getId() == ip.getId()) {
			*i = ip;
			break;
		}
	}
}

bool ClientProfileManager::moveMyinfoProfile(int id, int pos) {
	dcassert(pos == -1 || pos == 1);
	Lock l(mipcs);
	for(MyinfoProfile::List::iterator i = myinfoProfiles.begin(); i != myinfoProfiles.end(); ++i) {
		if(i->getId() == id) {
			swap(*i, *(i + pos));
			return true;
		}
	}
	return false;
}

MyinfoProfile::List& ClientProfileManager::reloadMyinfoProfiles() { 
	Lock l(mipcs);
	myinfoProfiles.clear();
	loadMyinfoProfiles();
	return myinfoProfiles;
}

FileListDetectorProfile ClientProfileManager::addFileListDetector(const string& name, const string& detect, const string& cheatingdescription, 
																  int rawToSend, bool badClient, bool isNew/* = false*/) {
	Lock l(fgcs);
	if(isNew) {
		newFileListDetector.push_back(FileListDetectorProfile(lastFDProfile++, name, detect, cheatingdescription,rawToSend,badClient));
		return newFileListDetector.back();
	} else {
		fileListDetector.push_back(FileListDetectorProfile(lastFDProfile++, name, detect, cheatingdescription,rawToSend,badClient));
		return fileListDetector.back();
	}
}

void ClientProfileManager::addFileListDetector(const StringList& sl) {
	Lock l(fgcs);
	fileListDetector.push_back(FileListDetectorProfile(lastFDProfile++, sl[0], sl[1], sl[2], 0, RsxUtil::toBool(sl[3])));
}
	
bool ClientProfileManager::getFileListDetector(int id, FileListDetectorProfile& fd) {
	Lock l(fgcs);
	for(FileListDetectorProfile::List::const_iterator i = fileListDetector.begin(); i != fileListDetector.end(); ++i) {
		if(i->getId() == id) {
			fd = *i;
			return true;
		}
	}
	return false;
}

void ClientProfileManager::removeFileListDetector(int id) {
	Lock l(fgcs);
	for(FileListDetectorProfile::List::iterator i = fileListDetector.begin(); i != fileListDetector.end(); ++i) {
		if(i->getId() == id) {
			fileListDetector.erase(i);
			break;
		}
	}
}

void ClientProfileManager::updateFileListDetector(const FileListDetectorProfile& fd) {
	Lock l(fgcs);
	for(FileListDetectorProfile::List::iterator i = fileListDetector.begin(); i != fileListDetector.end(); ++i) {
		if(i->getId() == fd.getId()) {
			*i = fd;
			break;
		}
	}
}
//END