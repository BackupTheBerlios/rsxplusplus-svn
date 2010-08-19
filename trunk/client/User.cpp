/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "User.h"

#include "AdcHub.h"
#include "Client.h"
#include "StringTokenizer.h"
#include "FavoriteUser.h"

#include "ClientManager.h"
#include "DetectionManager.h"
#include "UserCommand.h"
#include "ResourceManager.h"
#include "FavoriteManager.h"
//RSX++
#include "../rsx/RegexUtil.h"
#include "../rsx/IpManager.h"
#include "../rsx/Wildcards.h"
#include "../rsx/RsxUtil.h"
#include "PluginsManager.h"
//END

namespace dcpp {

FastCriticalSection Identity::cs;

OnlineUser::OnlineUser(const UserPtr& ptr, ClientBase& client_, uint32_t sid_) : identity(ptr, sid_), client(client_), isInList(false) { 
	if(getUser().get() != 0 && !getUser()->isSet(User::DHT))
		identity.isProtectedUser(getClient(), true); //RSX++ // run init check
}

bool Identity::isTcpActive(const Client* c) const {
	if(c != NULL && user == ClientManager::getInstance()->getMe()) {
		return c->isActive(); // userlist should display our real mode
	} else {
		return (!user->isSet(User::NMDC)) ?
			!getIp().empty() && supports(AdcHub::TCP4_FEATURE) :
			!user->isSet(User::PASSIVE);	
	}
}

bool Identity::isUdpActive() const {
	if(getIp().empty() || getUdpPort().empty())
		return false;
	return (!user->isSet(User::NMDC)) ? supports(AdcHub::UDP4_FEATURE) : !user->isSet(User::PASSIVE);
}

void Identity::getParams(StringMap& sm, const string& prefix, bool compatibility, bool dht) const {
	{
		FastLock l(cs);
		for(InfIter i = info.begin(); i != info.end(); ++i) {
			sm[prefix + string((char*)(&i->first), 2)] = i->second;
		}
	}
	if(!dht && user) {
		sm[prefix + "NI"] = getNick();
		sm[prefix + "SID"] = getSIDString();
		sm[prefix + "CID"] = user->getCID().toBase32();
		sm[prefix + "TAG"] = getTag();
		sm[prefix + "CO"] = getConnection();
		sm[prefix + "SSshort"] = Util::formatBytes(get("SS"));
		//RSX++
		sm[prefix + "RSshort"] = Util::formatBytes(get("RS"));
		sm[prefix + "LSshort"] = Util::formatBytes(get("LS"));
		sm[prefix + "TCTime"] = Util::formatTime("%d-%m %H:%M:%S", Util::toInt64(get("TC")));
		sm[prefix + "FCTime"] = Util::formatTime("%d-%m %H:%M:%S", Util::toInt64(get("FC")));
		//END
		if(compatibility) {
			if(prefix == "my") {
				sm["mynick"] = getNick();
				sm["mycid"] = user->getCID().toBase32();
			} else {
				sm["nick"] = getNick();
				sm["cid"] = user->getCID().toBase32();
				sm["ip"] = get("I4");
				sm["tag"] = getTag();
				sm["description"] = get("DE");
				sm["email"] = get("EM");
				sm["share"] = get("SS");
				sm["shareshort"] = Util::formatBytes(get("SS"));
				sm["realshareformat"] = Util::formatBytes(get("RS"));
				//RSX++
				sm["listsize"] = Util::formatBytes(get("LS"));
				sm["host"] =					get("HT");
				sm["isp"] =						getISP();
				//some simple names instead of Ax ;)
				sm["adlFile"] =					get("A1");
				sm["adlComment"] =				get("A2");
				sm["adlFileSize"] =				get("A3");
				sm["adlTTH"] =					get("A4");
				sm["adlForbiddenSize"] =		get("A5");
				sm["adlTotalPoints"] =			get("A6");
				sm["adlFilesCount"] =			get("A7");
				sm["adlFileSizeShort"] =		Util::formatBytes(get("A3"));
				sm["adlForbiddenSizeShort"] =	Util::formatBytes(get("A5"));
				//END
			}
		}
	}
}

bool Identity::isClientType(ClientType ct) const {
	int type = Util::toInt(get("CT"));
	return (type & ct) == ct;
}

string Identity::getTag() const {
	if(!get("TA").empty())
		return get("TA");
	if(get("VE").empty() || get("HN").empty() || get("HR").empty() || get("HO").empty() || get("SL").empty())
		return Util::emptyString;
	return "<" + get("VE") + ",M:" + string(isTcpActive() ? "A" : "P") + ",H:" + get("HN") + "/" +
		get("HR") + "/" + get("HO") + ",S:" + get("SL") + ">";
}

string Identity::get(const char* name) const {
	FastLock l(cs);
	InfIter i = info.find(*(short*)name);
	return i == info.end() ? Util::emptyString : i->second;
}

bool Identity::isSet(const char* name) const {
	FastLock l(cs);
	InfIter i = info.find(*(short*)name);
	return i != info.end();
}


void Identity::set(const char* name, const string& val) {
	FastLock l(cs);
	if(val.empty())
		info.erase(*(short*)name);
	else
		info[*(short*)name] = val;
}

bool Identity::supports(const string& name) const {
	string su = get("SU");
	StringTokenizer<string> st(su, ',');
	for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
		if(*i == name)
			return true;
	}
	return false;
}

void FavoriteUser::update(const OnlineUser& info) { 
	setNick(info.getIdentity().getNick()); 
	setUrl(info.getClient().getHubUrl()); 
}
//RSX++
string Identity::setCheat(const Client& c, const string& aCheatDescription, bool aBadClient, bool aBadFilelist /*=false*/, bool aDisplayCheat /*=true*/) {
	if(!c.isOp() || isOp()) {
		return Util::emptyString;
	}

	StringMap ucParams;
	getParams(ucParams, "user", true);
	string cheat = Util::formatParams(aCheatDescription, ucParams, false);

	string newCheat = Util::emptyString;
	bool newOne = false;

	string currentCS = get("CS");
	StringTokenizer<string> st(currentCS, ';');
	for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
		if((*i).find(cheat) == string::npos) {
			newCheat += (*i) + ";";
			newOne = true;
		}
	}

	newCheat += cheat + ";";

	if(newOne) {
		if((!SETTING(FAKERFILE).empty()) && (!BOOLSETTING(SOUNDS_DISABLED))) {
			PlaySound(Text::toT(SETTING(FAKERFILE)).c_str(), NULL, SND_FILENAME | SND_ASYNC);
		}
	}

	if(!cheat.empty())
		set("CS", newCheat);
	if(aBadClient)
		set("BC", "1");
	if(aBadFilelist)
 		set("BF", "1");

	if(RSXPP_BOOLSETTING(DISPLAY_CHEATS_IN_MAIN_CHAT) && aDisplayCheat) {
		string report = "*** " + STRING(USER) + " " + getNick() + " - " + cheat;
		return report;
	}
	return Util::emptyString;
}
//END
map<string, string> Identity::getReport() const {
	map<string, string> reportSet;

	string sid = getSIDString();

	{
		FastLock l(cs);
		for(InfIter i = info.begin(); i != info.end(); ++i) {
			string name = string((char*)(&i->first), 2);
			string value = i->second;

#define TAG(x,y) (x + (y << 8))
			
			// TODO: translate known tags and format values to something more readable
			switch(i->first) {
				case TAG('A','W'): name = "Away mode"; break;
				case TAG('B','O'): name = "Bot"; break;
				case TAG('C','L'): name = "Client name"; break;
				case TAG('C','M'): name = "Comment"; break;
				case TAG('C','O'): name = "Connection"; break;
				case TAG('C','S'): name = "Cheat description"; break;
				case TAG('C','T'): name = "Client type"; break;
				case TAG('D','E'): name = "Description"; break;
				case TAG('D','S'): name = "Download speed"; value = Util::formatBytes(value) + "/s"; break;
				case TAG('E','M'): name = "E-mail"; break;
				case TAG('F','C'): name = "Fake Check status"; break;
				case TAG('F','D'): name = "Filelist disconnects"; break;
				case TAG('G','E'): name = "Filelist generator"; break;
				case TAG('H','N'): name = "Hubs Normal"; break;
				case TAG('H','O'): name = "Hubs OP"; break;
				case TAG('H','R'): name = "Hubs Registered"; break;
				case TAG('I','4'): name = "IPv4 Address"; value += " (" + Socket::getRemoteHost(value) + ")"; break;
				case TAG('I','6'): name = "IPv6 Address"; value += " (" + Socket::getRemoteHost(value) + ")"; break;
				case TAG('I','D'): name = "Client ID"; break;
				case TAG('K','P'): name = "KeyPrint"; break;
				case TAG('L','O'): name = "NMDC Lock"; break;
				case TAG('N','I'): name = "Nick"; break;
				case TAG('O','P'): name = "Operator"; break;
				case TAG('P','K'): name = "NMDC Pk"; break;
				case TAG('R','S'): name = "Shared bytes - real"; value = Text::fromT(Util::formatExactSize(Util::toInt64(value))); break;
				case TAG('S','F'): name = "Shared files"; break;
				case TAG('S','I'): name = "Session ID"; value = sid; break;
				case TAG('S','L'): name = "Slots"; break;
				case TAG('S','S'): name = "Shared bytes - reported"; value = Text::fromT(Util::formatExactSize(Util::toInt64(value))); break;
				case TAG('S','T'): name = "NMDC Status"; value = Util::formatStatus(Util::toInt(value)); break;
				case TAG('S','U'): name = "Supports"; break;
				case TAG('T','A'): name = "Tag"; break;
				case TAG('T','O'): name = "Timeouts"; break;
				case TAG('U','4'): name = "IPv4 UDP port"; break;
				case TAG('U','6'): name = "IPv6 UDP port"; break;
				case TAG('U','S'): name = "Upload speed"; value = Util::formatBytes(value) + "/s"; break;
				case TAG('V','E'): name = "Client version"; break;
				case TAG('L','T'): name = "Login time"; break;
				case TAG('M','T'): name = "UserInfo"; break;
				case TAG('M','C'): name = "UserInfo count"; break;
				case TAG('T','S'): name = "TestSUR"; break;
				case TAG('A','H'): name = "All hubs count"; break;
				case TAG('I', 'C'): name = ""; break;
				case TAG('W','O'): name = ""; break;	// for GUI purposes
				default: name += " (unknown)";

			}

			if(!name.empty())
				reportSet.insert(make_pair(name, value));
		}
	}

	return reportSet;
}

string Identity::updateClientType(OnlineUser& ou) {
	if(getUser()->isSet(User::DCPLUSPLUS)) {
		const float versionf = Util::toFloat(getVersion());
		if((get("LL") == "11") && (getBytesShared() > 0)) {
			setClientType("DC++ Stealth");
			const string& report = ou.setCheat("Fake FileList - ListLen = 11B", true, true, RSXPP_BOOLSETTING(SHOW_LISTLEN_MISMATCH));
			ClientManager::getInstance()->sendAction(ou, RSXPP_SETTING(LISTLEN_MISMATCH));
			logDetect(true);
			return report;
		} else if(strncmp(getTag().c_str(), "<++ V:", 6) == 0 && versionf < 1.001f && versionf >= 0.69f) {
			//suppose to be dc++  >= 0.69
			if(get("LL") != "42" && !get("LL").empty()) {
				setClientType("Faked DC++");
				set("CM", "Supports corrupted files...");
				const string& report = ou.setCheat("ListLen mis-match (V:0.69+)", true, false, RSXPP_BOOLSETTING(SHOW_LISTLEN_MISMATCH));
				ClientManager::getInstance()->sendAction(ou, RSXPP_SETTING(LISTLEN_MISMATCH));
				logDetect(true);
				return report;
			} else if(versionf > (float)0.699 && !get("TS").empty() && get("TS") != "GetListLength not supported") {
				const string& report = ou.setCheat("DC++ emulation", true, false, RSXPP_BOOLSETTING(SHOW_DCPP_EMULATION_RAW));
				ClientManager::getInstance()->sendAction(ou, RSXPP_SETTING(DCPP_EMULATION_RAW));
				logDetect(true);
				return report;
			}
		}
	}

	uint64_t tick = GET_TICK();

	StringMap params;
	getDetectionParams(params); // get identity fields and escape them, then get the rest and leave as-is
	const DetectionManager::DetectionItems& profiles = DetectionManager::getInstance()->getProfiles(params);
   
	for(DetectionManager::DetectionItems::const_iterator i = profiles.begin(); i != profiles.end(); ++i) {
		const DetectionEntry& entry = *i;
		if(!entry.isEnabled)
			continue;
		DetectionEntry::INFMap INFList;
		if(!entry.defaultMap.empty()) {
			// fields to check for both, adc and nmdc
			INFList = entry.defaultMap;
		} 

		if(getUser()->isSet(User::NMDC) && !entry.nmdcMap.empty()) {
			INFList.insert(INFList.end(), entry.nmdcMap.begin(), entry.nmdcMap.end());
		} else if(!entry.adcMap.empty()) {
			INFList.insert(INFList.end(), entry.adcMap.begin(), entry.adcMap.end());
		}

		if(INFList.empty())
			continue;

		bool _continue = false;

		DETECTION_DEBUG("\tChecking profile: " + entry.name);

		for(DetectionEntry::INFMap::const_iterator j = INFList.begin(); j != INFList.end(); ++j) {
			string aPattern = Util::formatRegExp(j->second, params);
			string aField = getDetectionField(j->first);
			DETECTION_DEBUG("\t\tPattern: " + aPattern + " Field: " + aField);
			if(!RegexUtil::match(aField, aPattern)) {
				_continue = true;
				break;
			}
		}
		if(_continue)
			continue;

		DETECTION_DEBUG("**** Client found: " + entry.name + " time taken: " + Util::toString(GET_TICK()-tick) + " ms ****\r\n");

		setClientType(entry.name);
		set("CM", entry.comment);
		//set("BC", entry.cheat.empty() ? Util::emptyString : "1");
		logDetect(true);

		if(entry.checkMismatch && getUser()->isSet(User::NMDC) &&  (params["VE"] != params["PKVE"])) { 
			setClientType(entry.name + " Version mis-match");
			return ou.setCheat(entry.cheat + " Version mis-match", true, false, ou.getClient().isActionActive(RSXPP_SETTING(VERSION_MISMATCH)));
		}

		string report = Util::emptyString;
		if(!entry.cheat.empty()) {
			report = ou.setCheat(entry.cheat, true, false, ou.getClient().isActionActive(entry.rawToSend));
		}

		ClientManager::getInstance()->sendAction(ou, entry.rawToSend);
		return report;
	}

	logDetect(false);
	setClientType("Unknown");
	return Util::emptyString;
}

string Identity::getDetectionField(const string& aName) const {
	if(aName.length() == 2) {
		if(aName == "TA")
			return getTag();
		else if(aName == "CO")
			return getConnection();
		else
			return get(aName.c_str());
	} else {
		if(aName == "PKVE") {
			return getPkVersion();
		}
		return Util::emptyString;
	}
}

void Identity::getDetectionParams(StringMap& p) {
	getParams(p, Util::emptyString, false);
	p["PKVE"] = getPkVersion();
	//p["VEformat"] = getVersion();
   
	if(!user->isSet(User::NMDC)) {
		string version = get("VE");
		string::size_type i = version.find(' ');
		if(i != string::npos)
			p["VEformat"] = version.substr(i+1);
		else
			p["VEformat"] = version;
	} else {
		p["VEformat"] = get("VE");
	}

	// convert all special chars to make regex happy
	for(StringMap::iterator i = p.begin(); i != p.end(); ++i) {
		// looks really bad... but do the job
		Util::replace("\\", "\\\\", i->second); // this one must be first
		Util::replace("[", "\\[", i->second);
		Util::replace("]", "\\]", i->second);
		Util::replace("^", "\\^", i->second);
		Util::replace("$", "\\$", i->second);
		Util::replace(".", "\\.", i->second);
		Util::replace("|", "\\|", i->second);
		Util::replace("?", "\\?", i->second);
		Util::replace("*", "\\*", i->second);
		Util::replace("+", "\\+", i->second);
		Util::replace("(", "\\(", i->second);
		Util::replace(")", "\\)", i->second);
		Util::replace("{", "\\{", i->second);
		Util::replace("}", "\\}", i->second);
	}
}

string Identity::getPkVersion() const {
	string pk = get("PK");

	string::const_iterator begin = pk.begin();
	string::const_iterator end = pk.end();
	boost::match_results<string::const_iterator> result;
	boost::regex reg("[0-9]+\\.[0-9]+", boost::regex_constants::icase);
	if(boost::regex_search(begin, end, result, reg, boost::match_default))
		return result.str();
	return Util::emptyString;
}

string Identity::myInfoDetect(OnlineUser& ou) {
	checkTagState(ou);

	StringMap params;
	getDetectionParams(params); // get identity fields and escape them, then get the rest and leave as-is
	const DetectionManager::DetectionItems& profiles = DetectionManager::getInstance()->getProfiles(params, true);

	for(DetectionManager::DetectionItems::const_iterator i = profiles.begin(); i != profiles.end(); ++i) {
		const DetectionEntry& entry = *i;
		if(!entry.isEnabled)
			continue;
		DetectionEntry::INFMap INFList;
		if(!entry.defaultMap.empty()) {
			// fields to check for both, adc and nmdc
			INFList = entry.defaultMap;
		} 

		if(getUser()->isSet(User::NMDC) && !entry.nmdcMap.empty()) {
			INFList.insert(INFList.end(), entry.nmdcMap.begin(), entry.nmdcMap.end());
		} else if(!entry.adcMap.empty()) {
			INFList.insert(INFList.end(), entry.adcMap.begin(), entry.adcMap.end());
		}

		if(INFList.empty())
			continue;

		bool _continue = false;
		DETECTION_DEBUG("\tChecking User Info Profile: " + entry.name);

		for(DetectionEntry::INFMap::const_iterator j = INFList.begin(); j != INFList.end(); ++j) {
			string aPattern = Util::formatRegExp(j->second, params);
			string aField = getDetectionField(j->first);
			DETECTION_DEBUG("\t\tPattern: " + aPattern + " Field: " + aField);
			if(!RegexUtil::match(aField, aPattern)) {
				_continue = true;
				break;
			}
		}
		if(_continue)
			continue;

		DETECTION_DEBUG("**** Client found: " + entry.name + " ****\r\n");

		setMyInfoType(entry.name);
		set("CM", entry.comment);

		string report = Util::emptyString;
		if(!entry.cheat.empty()) {
			report = ou.setCheat(entry.cheat, true, false, ou.getClient().isActionActive(entry.rawToSend));
		}

		ClientManager::getInstance()->sendAction(ou, entry.rawToSend);
		return report;
	}
	return Util::emptyString;
}
//RSX++ //Protected users
bool Identity::isProtectedUser(const Client& c, bool OpBotHubCheck) const {
	if(isSet("PR") || getUser()->isSet(User::PROTECTED))
		return true;

	string RegProtect = RSXPP_SETTING(PROTECTED_USERS);
	if(!c.getProtectedUsers().empty()) {
		RegProtect = c.getProtectedUsers();
	}

	bool ret = false;
	if(OpBotHubCheck && (isOp() || isBot() || isHub())) {
		ret = true;
	} else if(RSXPP_SETTING(FAV_USER_IS_PROTECTED_USER) && FavoriteManager::getInstance()->isFavoriteUser(getUser())) {
		ret = true;
	} else if(!RegProtect.empty()) {
		if(RSXPP_BOOLSETTING(USE_WILDCARDS_TO_PROTECT)) {
			if(Wildcard::patternMatch(getNick(), RegProtect, '|')) {
				ret = true;
			}
		} else {
			if(RegexUtil::match(getNick(), RegProtect, true)) {
				ret = true;
			}
		}
	}
	if(ret == true) {
		const_cast<Identity&>(*this).set("PR", "1");
	}
	return ret;
}
//RSX++ //checking stuff
bool OnlineUser::getChecked(bool filelist/* = false*/, bool checkComplete/* = true*/) {
	if(!identity.isTcpActive() && !getClient().isActive()) {
		identity.setClientType("[Passive]");
		setTestSURComplete();
		setFileListComplete();
		return true;
	} else if(getUser()->isSet(User::OLD_CLIENT)) {
		setTestSURComplete();
		setFileListComplete();
		return true;
	} else if(isProtectedUser()) {
		if((RSXPP_SETTING(UNCHECK_CLIENT_PROTECTED_USER) && !filelist) || (RSXPP_SETTING(UNCHECK_LIST_PROTECTED_USER) && filelist)) {
			identity.setClientType("[Protected]");
			setTestSURComplete();
			setFileListComplete();
			return true;
		}
	}
	if(checkComplete) //prevent double checking (shouldCheckClient/Filelist)
		return filelist ? identity.isFileListChecked() : identity.isClientChecked();
	return false;
}
//RSX++ //$MyINFO Flood
bool Identity::isMyInfoSpamming() {
	set("MC", Util::toString(Util::toInt(get("MC")) + 1));

	uint64_t now = GET_TICK();
	uint64_t i = now - lastMyInfo;
	if(i < RSXPP_SETTING(MYINFO_TIME)) {
		myinfoFloodCounter++;
	} else {
		myinfoFloodCounter = 0;
	}

	lastMyInfo = now;
	if(myinfoFloodCounter > RSXPP_SETTING(MYINFO_COUNTER)) {
		myinfoFloodCounter = 0;
		return true;
	}
	return false;
}
//RSX++ //$ConnectToMe Flood
bool Identity::isCtmSpamming() const {
	uint64_t now = GET_TICK();
	uint64_t i = now - lastCte;
	if(i < RSXPP_SETTING(CTM_TIME)) {
		cteFloodCounter++;
	} else {
		cteFloodCounter = 0;
	}
	lastCte = now;
	if(cteFloodCounter > RSXPP_SETTING(CTM_COUNTER)) {
		cteFloodCounter = 0;
		return true;
	}
	return false;
}
//RSX++ //PM flood
bool Identity::isPmSpamming() const {
	uint64_t now = GET_TICK();
	uint64_t i = now - lastPm;
	if(i < RSXPP_SETTING(PM_TIME)) {
		pmFloodCounter++;
	} else {
		pmFloodCounter = 0;
	}
	lastPm = now;
	if(pmFloodCounter > RSXPP_SETTING(PM_COUNTER)) {
		pmFloodCounter = 0;
		return true;
	}
	return false;
}
//RSX++ //IP Check
void Identity::checkIP(OnlineUser& ou) {
	if(RSXPP_BOOLSETTING(USE_IPWATCH)) {
		set("IC", "1"); //ip checked, to avoid cheat spam caused by search
		const IPWatch::List& il = IpManager::getInstance()->getWatch();
		bool matched = false;
		for(IPWatch::List::const_iterator j = il.begin(); j != il.end(); ++j) {
			string strToMatch = Util::emptyString;
			switch((*j)->getMode()) {
				//case 0: strToMatch = getIp(); break;
				case 1: strToMatch = get("HT"); break;
				//case 2: strToMatch = getIp(); break;
				default: strToMatch = getIp();
			}
			switch((*j)->getMatchType()) {
				case 0: {
					if(Wildcard::patternMatch(strToMatch, (*j)->getPattern(), '|'))
						matched = true;
					break;
				}
				case 1: {
					if(RegexUtil::match(strToMatch, (*j)->getPattern()))
						matched = true;
					break;
				}
				case 2: {
					if(RsxUtil::isIpInRange(getIp(), (*j)->getPattern()))
						matched = true;
					break;
				}
				default: break;
			}

			if(matched) {
				if(!(*j)->getIsp().empty()) {
					setISP((*j)->getIsp());
				}
				switch((*j)->getTask()) {
					case 0:  {
						set("PR", "1");
						setClientType("[Protected IP]");
						break;
					}
					case 1: { 
						string report = ou.setCheat((*j)->getCheat(), false, false, (*j)->getDisplayCheat());
						ClientManager::getInstance()->sendAction(ou, (*j)->getAction());
						if(!report.empty())
							ou.getClient().cheatMessage(report);
						ou.getClient().updated(&ou);
						break;
					}
					case 2: {
						StringMap ucParams;
						getParams(ucParams, "user", true);
						if(ou.getClient().isOp())
							ou.getClient().cheatMessage("*** [IP Watch] " + getNick() + " - " + Util::formatParams((*j)->getCheat(), ucParams, false)); 
						break;
					}
				}
				break; //break at first match
			}
		}
	}
}
//RSX++ //Filelist Detector
string Identity::checkFilelistGenerator(OnlineUser& ou) {
	if((get("FG") == "DC++ 0.403")) {
		if(RegexUtil::match(getTag(), "^<StrgDC\\+\\+ V:1.00 RC([89]){1}")) {
			string report = ou.setCheat("rmDC++ in StrongDC++ %[userVE] emulation mode" , true, false, RSXPP_BOOLSETTING(SHOW_RMDC_RAW));
			setClientType("rmDC++");
			logDetect(true);
			ou.getClient().updated(&ou);
			ClientManager::getInstance()->sendAction(ou, RSXPP_SETTING(RMDC_RAW));
			return report;
		}
	}

	if(!get("VE").empty() && strncmp(getTag().c_str(), "<++ V:", 6) == 0) {
		if((Util::toFloat(get("VE")) > 0.668)) {
			if(get("FI").empty() || get("FB").empty()) {
				string report = ou.setCheat("DC++ emulation", true, false, RSXPP_BOOLSETTING(SHOW_DCPP_EMULATION_RAW));
				logDetect(true);
				ou.getClient().updated(&ou);
				ClientManager::getInstance()->sendAction(ou, RSXPP_SETTING(DCPP_EMULATION_RAW));
				return report;
			}
		} else {
			if(!get("FI").empty() || !get("FB").empty()) {
				string report = ou.setCheat("DC++ emulation", true, false, RSXPP_BOOLSETTING(SHOW_DCPP_EMULATION_RAW));
				logDetect(true);
				ou.getClient().updated(&ou);
				ClientManager::getInstance()->sendAction(ou, RSXPP_SETTING(DCPP_EMULATION_RAW));
				return report;
			}
		}
	}


	if(RegexUtil::match(get("FG"), "^DC\\+\\+.*")) {
		if(!get("VE").empty() && (get("VE") != getFilelistGeneratorVer())) {
			string report = ou.setCheat("Filelist Version mis-match", false, true, RSXPP_BOOLSETTING(SHOW_FILELIST_VERSION_MISMATCH));
			logDetect(true);
			ou.getClient().updated(&ou);
			ClientManager::getInstance()->sendAction(ou, RSXPP_SETTING(FILELIST_VERSION_MISMATCH));
			return report;
		}
	}

/*	const FileListDetectorProfile::List& lst = ClientProfileManager::getInstance()->getFileListDetectors();
	for(FileListDetectorProfile::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const FileListDetectorProfile& fd = *i;	
		if(!RegexUtil::match(get("FG"), fd.getDetect())) { continue; }

		string report = Util::emptyString;
		if(fd.getBadClient()) {
			if(!fd.getName().empty()) setClientType(fd.getName());
			report = ou.setCheat(fd.getCheatingDescription(), false, true, fd.getCheatingDescription().empty()?false:true);
		}
		logDetect(true);
		ClientManager::getInstance()->sendAction(ou, fd.getRawToSend());
		return report;
	}
	logDetect(false);*/
	return Util::emptyString;
}

string Identity::getVersion() const {
	if(user->isSet(User::NMDC))
		return get("VE");
	string version = get("VE");
	string::size_type i = version.find(' ');
	if(i != string::npos)
		return version.substr(i+1);
	return version;
}

string Identity::getFilelistGeneratorVer() const {
	if(!get("FG").empty()) {
		string genVer = get("FG");
		string::size_type i = genVer.find(' ');
		if(i != string::npos ) {
			genVer = genVer.substr(i+1);
		}
		return genVer;
	} else {
		return Util::emptyString;
	}
}

string Identity::checkrmDC(OnlineUser& ou) {
	string report = Util::emptyString;
	if(RegexUtil::match(getVersion(), "^0.40([0123]){1}$")) {
		report = ou.setCheat("rmDC++ in DC++ %[userVE] emulation mode" , true, false, RSXPP_BOOLSETTING(SHOW_RMDC_RAW));
		setClientType("rmDC++");
		ClientManager::getInstance()->sendAction(ou, RSXPP_SETTING(RMDC_RAW));
	}
	return report;
}

void Identity::logDetect(bool successful) {
	if(successful) {
		RSXPP_SET(TOTAL_DETECTS, RSXPP_SETTING(TOTAL_DETECTS) + 1);
	} else {
		RSXPP_SET(TOTAL_FAILED_DETECTS, RSXPP_SETTING(TOTAL_FAILED_DETECTS) + 1);
	}
}

void Identity::checkTagState(OnlineUser& ou) {
	string usrTag = getTag();
	if(usrTag.empty()) return;
	bool isActive = isTcpActive(&ou.getClient());

	if(isActive && (usrTag.find(",M:P,") != string::npos)) {
		ou.getClient().cheatMessage("*** " + getNick() + " - Tag states passive mode, but user is using active mode"); 
	} else if(!isActive && (usrTag.find(",M:A,") != string::npos)) {
		ou.getClient().cheatMessage("*** " + getNick() + " - Tag states active mode, but user is using passive mode"); 
	}
}

void Identity::cleanUser() {
	resetCounters();
	setMyInfoType(Util::emptyString); //UserInfo client type
	//set("I4", Util::emptyString); //IP
	setClientType(Util::emptyString); //Client Type
	set("BC", Util::emptyString); //Bad Client
	set("BF", Util::emptyString); //Bad FileList
	set("TC", Util::emptyString); //Client checked
	set("TQ", Util::emptyString); //TestSUR queued
	set("FC", Util::emptyString); //Filelist checked
	set("FQ", Util::emptyString); //filelist check queued
	set("LL", Util::emptyString); //ListLen
	set("PK", Util::emptyString); //PK string
	set("SU", Util::emptyString); //Supports
	set("LO", Util::emptyString); //Lock
	set("FG", Util::emptyString); //XML Generator
	set("TS", Util::emptyString); //TestSUR string
	set("FD", Util::emptyString); //Disconnects
	set("TO", Util::emptyString); //Timeouts
	set("LS", Util::emptyString); //FileList size
	set("FI", Util::emptyString); //FileList CID
	set("FB", Util::emptyString); //FileList Base
	set("SF", Util::emptyString); //Shared Files count
	set("CS", Util::emptyString); //Cheating description
	set("CM", Util::emptyString); //Comment
	set("RS", Util::emptyString); //Real Share
	set("IC", Util::emptyString); //IP checked
	set("IS", Util::emptyString); //ISP
	set("HT", Util::emptyString); //host
	set("A1", Util::emptyString); //adls filename
	set("A2", Util::emptyString); //adls comment
	set("A3", Util::emptyString); //adls file size
	set("A4", Util::emptyString); //adls tth
	set("A5", Util::emptyString); //adls forbidden size
	set("A6", Util::emptyString); //adls total points
	set("A7", Util::emptyString); //adls files count
	set("SC", Util::emptyString); //real slots count
	set("IC", Util::emptyString); //ip checked
	//set("R1", Util::emptyString); //recheck
}

string Identity::checkSlotsCount(OnlineUser& ou, int realSlots) {
	set("SC", Util::toString(realSlots));
	string report = Util::emptyString;
	if(Util::toInt(get("SL")) > realSlots) { //more slots showed than real opened? cheater ;]
		report = ou.setCheat("fake slots count - tag states  %[userSL] slots, but real user have opened %[userSC] slots", true, false, RSXPP_BOOLSETTING(SHOW_FAKE_SLOT_COUNT));
		ClientManager::getInstance()->sendAction(ou, RSXPP_SETTING(FAKE_SLOT_COUNT));
	}
	return report;
}

int OnlineUser::compareItems(const OnlineUser* a, const OnlineUser* b, uint8_t col)  {
	if(col == COLUMN_NICK) {
		bool a_isOp = a->getIdentity().isOp(),
			b_isOp = b->getIdentity().isOp();
		if(a_isOp && !b_isOp)
			return -1;
		if(!a_isOp && b_isOp)
			return 1;
		if(BOOLSETTING(SORT_FAVUSERS_FIRST)) {
			bool a_isFav = FavoriteManager::getInstance()->isFavoriteUser(a->getIdentity().getUser()),
				b_isFav = FavoriteManager::getInstance()->isFavoriteUser(b->getIdentity().getUser());
			if(a_isFav && !b_isFav)
				return -1;
			if(!a_isFav && b_isFav)
				return 1;
		}
		// workaround for faster hub loading
		// lstrcmpiA(a->identity.getNick().c_str(), b->identity.getNick().c_str());
	}
	switch(col) {
		case COLUMN_SHARED:
		case COLUMN_EXACT_SHARED: return compare(a->identity.getBytesShared(), b->identity.getBytesShared());
		case COLUMN_SLOTS: return compare(Util::toInt(a->identity.get("SL")), Util::toInt(b->identity.get("SL")));
		case COLUMN_HUBS: return compare(Util::toInt(a->identity.get("AH")), Util::toInt(b->identity.get("AH")));
	}
	return lstrcmpi(a->getText(col).c_str(), b->getText(col).c_str());
}

tstring OnlineUser::getText(uint8_t col) const {
	switch(col) {
		case COLUMN_NICK: return Text::toT(identity.getNick());
		case COLUMN_SHARED: return Util::formatBytesW(identity.getBytesShared());
		case COLUMN_EXACT_SHARED: return Util::formatExactSize(identity.getBytesShared());
		case COLUMN_DESCRIPTION: return Text::toT(identity.getDescription());
		case COLUMN_TAG: return Text::toT(identity.getTag());
		case COLUMN_CONNECTION: return identity.get("US").empty() ? Text::toT(identity.getConnection()) : (Text::toT(Util::formatBytes(identity.get("US"))) + _T("/s"));
		case COLUMN_IP: {
			string ip(identity.getIp());
			if(!ip.empty()) {
				ip = Util::getIpCountry(ip) + " (" + ip + ")";
			}
			return Text::toT(ip);
		}
		case COLUMN_EMAIL: return Text::toT(identity.getEmail());
		case COLUMN_VERSION: return Text::toT(identity.getVersion());
		case COLUMN_MODE: return identity.isTcpActive(&getClient()) ? _T("A") : _T("P");
		case COLUMN_HUBS: {
			const tstring hn = Text::toT(identity.get("HN"));
			const tstring hr = Text::toT(identity.get("HR"));
			const tstring ho = Text::toT(identity.get("HO"));
			const tstring cn = Util::toStringW(Util::toInt(identity.get("HN")) + Util::toInt(identity.get("HR")) + Util::toInt(identity.get("HO")));
			const_cast<Identity&>(identity).set("AH", Text::fromT(cn));
			return (hn.empty() || hr.empty() || ho.empty()) ? Util::emptyStringT : (cn + _T("(") + hn + _T("/") + hr + _T("/") + ho + _T(")"));
		}
		case COLUMN_SLOTS: return Text::toT(identity.get("SL"));
		case COLUMN_CLIENTS : {
			if(identity.isBot() || identity.isHub())
				return _T("BOT");
			else if(identity.isOp())
				return _T("OP");
			else
				return Text::toT(identity.getClientType());
		}
		case COLUMN_MYINFOS: {
			if(identity.isBot() || identity.isHub())
				return _T("BOT");
			else if(identity.isOp())
				return _T("OP");
			else
				return Text::toT(identity.getMyInfoType());
		}
		case COLUMN_CHEATING_DESCRIPTION: return Text::toT(identity.get("CS"));
		case COLUMN_HOST: return Text::toT(identity.get("HT"));
		case COLUMN_ISP: return Text::toT(identity.getISP());
		case COLUMN_SUPPORT: return Text::toT(identity.get("SU"));
		case COLUMN_STATUS: { 
			if(getUser()->isSet(User::NMDC)) {
				return Text::toT(Util::formatStatus(identity.getStatus()));
			} else {
				switch(Util::toInt(identity.get("AW"))) {
					case 0: return _T("Normal (0)");
					case 1: return _T("Away (1)");
					case 2: return _T("Extended Away (2)");
					default: return Text::toT("Unknown (" + identity.get("AW") + ")");
				}
			}
		}
		case COLUMN_COMMENT: return Text::toT(identity.get("CM"));
		default: return Util::emptyStringT;
	}
}

bool OnlineUser::update(int sortCol, const tstring& oldText) {
	bool needsSort = ((identity.get("WO").empty() ? false : true) != identity.isOp());
	
	if(sortCol == -1) {
		isInList = true;
	} else {
		needsSort = needsSort || (oldText != getText(static_cast<uint8_t>(sortCol)));
	}

	return needsSort;
}
//RSX++
dcpp_param OnlineUser::userCallFunc(const char* type, dcpp_param p1, dcpp_param p2, dcpp_param p3, int* handled) {
	*handled = DCPP_TRUE;
	if(strncmp(type, "User/", 5) == 0) {
		const char* cmd = type+5;
		OnlineUser* ou = reinterpret_cast<OnlineUser*>(p1);
		if(ou) {
			if(strncmp(cmd, "Identity/", 9) == 0) {
				Identity& i = ou->getIdentity();

				if(strncmp(cmd+9, "Get/", 4) == 0) {
					dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p3);
					if(!buf) return DCPP_FALSE;
					string value = i.get(reinterpret_cast<const char*>(p2));
					return PluginsManager::dcppBuffer_strcpy(value, buf);
				} else if(strncmp(cmd+9, "Set/", 4) == 0) {
					i.set(reinterpret_cast<const char*>(p2), reinterpret_cast<const char*>(p3));
					return DCPP_TRUE;
				}
				return DCPP_FALSE;
			} else if(strncmp(cmd, "GetHubObject", 12) == 0) {
				if(p2) {
					*reinterpret_cast<dcpp_param*>(p2) = reinterpret_cast<dcpp_param>(&ou->getClient());
					return DCPP_TRUE;
				}
				return DCPP_FALSE;
			} else if(strncmp(cmd, "GetCID", 6) == 0) {
				dcppBuffer* buf = reinterpret_cast<dcppBuffer*>(p2);
				if(!buf) return DCPP_FALSE;
				if(p3) {
					if(buf->size >= CID::SIZE) {
						memcpy(buf->buf, ou->getUser()->getCID().data(), CID::SIZE);
						return DCPP_TRUE;
					}
					return DCPP_FALSE;
				} else {
					std::string cid = ou->getUser()->getCID().toBase32();
					return PluginsManager::dcppBuffer_strcpy(cid, buf);
				}
			} else if(strncmp(cmd, "GetParams", 9) == 0) {
				//TODO
			}
		}
	}
	*handled = DCPP_FALSE;
	return DCPP_FALSE;
}

} // namespace dcpp

/**
 * @file
 * $Id: User.cpp 492 2010-03-26 14:31:56Z bigmuscle $
 */
