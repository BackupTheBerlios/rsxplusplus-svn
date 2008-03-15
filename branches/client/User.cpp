/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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
#include "Client.h"
#include "StringTokenizer.h"
#include "FavoriteUser.h"

#include "ClientManager.h"
#include "ClientProfileManager.h"
#include "pme.h"
#include "UserCommand.h"
#include "ResourceManager.h"
#include "FavoriteManager.h"
//RSX++
#include "../rsx/RegexpHandler.h"
#include "../rsx/IpManager.h"
#include "../rsx/Wildcards.h"
#include "DetectionManager.h"
//END
FastCriticalSection Identity::cs;
//RSX++
const string& User::getNick(bool first /*= true*/) const {
	if(isSet(User::NMDC)) {
		return getFirstNick();
	} else {
		if(first) {
			return ClientManager::getInstance()->getNicks(getCID())[0];
		} else {
			//make compiler happy, prevent warning
			const string& aNicks = Util::toString(ClientManager::getInstance()->getNicks(getCID()));
			return aNicks;
		}
	}
}
//END

OnlineUser::OnlineUser(const UserPtr& ptr, Client& client_, uint32_t sid_) : identity(ptr, sid_), client(client_), isInList(false) { 
	inc();
}

void Identity::getParams(StringMap& sm, const string& prefix, bool compatibility) const {
	{
		FastLock l(cs);
		for(InfMap::const_iterator i = info.begin(); i != info.end(); ++i) {
			sm[prefix + string((char*)(&i->first), 2)] = i->second;
		}
	}
	if(user) {
		sm[prefix + "NI"] = getNick();
		sm[prefix + "SID"] = getSIDString();
		sm[prefix + "CID"] = user->getCID().toBase32();
		sm[prefix + "TAG"] = getTag();
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
	InfMap::const_iterator i = info.find(*(short*)name);
	return i == info.end() ? Util::emptyString : i->second;
}

bool Identity::isSet(const char* name) const {
	FastLock l(cs);
	InfMap::const_iterator i = info.find(*(short*)name);
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

string Identity::getConnection() const {
	string connection = get("US");
	if(connection.find_first_not_of("0123456789.,") == string::npos) {
		double us = Util::toDouble(connection);
		if(us > 0) {
			char buf[16];
			snprintf(buf, sizeof(buf), "%.3g", us / 1024 / 1024);

			char *cp;
			if( (cp=strchr(buf, ',')) != NULL) *cp='.';

			return buf;
		} else {
			return connection;
		}
	} else {
		return connection;
	}
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
	const string& cheat = Text::toUtf8(Util::formatParams(aCheatDescription, ucParams, false));

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

	if(!cheat.empty())	set("CS", newCheat);
	if(aBadClient)		set("BC", "1");
	if(aBadFilelist)	set("BF", "1");

	if(RSXBOOLSETTING(DISPLAY_CHEATS_IN_MAIN_CHAT) && aDisplayCheat) {
		string report = "*** " + STRING(USER) + " " + getNick() + " - " + cheat;
		return report;
	}
	return Util::emptyString;
}
//END
const string Identity::getReport() const {
	string report = "\r\n *** User Info:";
	report += "\r\n-]> Description:	" +				getDescription();
	report += "\r\n-]> Email:		" +				getEmail();
	report += "\r\n-]> Connection:	" +				getConnection();
	report += "\r\n-]> User CID:	" +				isEmpty(getUser()->getCID().toBase32());
	if(!getUser()->isSet(User::NMDC))
		report += "\r\n-]> User SID:	" +			isEmpty(getSIDString()) + " (" + get("SI") + ")";
	report += "\r\n-]> IP:		" +					getIp() + (getIp().empty() ? Util::emptyString : (" (" + Util::getIpCountry(getIp()) + ")"));
	report += "\r\n-]> Host:		" +				isEmpty(get("HT"));
	report += "\r\n-]> ISP:		" +					getISP();
	if(!get("SL").empty())
		report += "\r\n-]> Slots:		" +			(get("SL")) + (get("SC").empty() ? Util::emptyString : " (" + get("SC") + ")");
	else
		report += "\r\n-]> Slots:		N/A";
	report += "\r\n-]> Logged in:	" +				get("LT") + " (" + Text::fromT(Util::formatSeconds((GET_TICK() - loggedIn) / 1000)) + ')';

	report += "\r\n\r\n *** Client Info:";
	report += "\r\n-]> Client:		" +				(getClientType().empty() ? static_cast<const string>("N/A") : getClientType() + "\t[" + Util::formatTime("%d-%m %H:%M:%S", Util::toInt64(get("TC"))) + ']');
	report += "\r\n-]> Client $MyINFO:	" +			isEmpty(get("MT"));
	report += "\r\n-]> Lock:		" +				isEmpty(get("LO"));
	report += "\r\n-]> Pk:		" +					isEmpty(get("PK"));
	report += "\r\n-]> Tag:		" +					isEmpty(getTag());
	report += "\r\n-]> Supports:	" +				isEmpty(get("SU"));
	report += "\r\n-]> Status:	" +					Util::formatStatus(Util::toInt(getStatus()));
	report += "\r\n-]> TestSUR:	" +					isEmpty(get("TS"));
	report += "\r\n-]> Disconnects:	" +				isEmpty(get("FD"));
	report += "\r\n-]> Timeouts:	" +				isEmpty(get("TO"));
	report += "\r\n-]> Commands:	" +				isEmpty(get("UC"));
	report += "\r\n-]> Cheat status:	" +			isEmpty(get("CS"));
	report += "\r\n-]> Comment:	" +					isEmpty(get("CM"));

	report += "\r\n\r\n *** FileList\\Share Info";
	report += "\r\n-]> XML Generator:	" +			isEmpty(get("FG"));
	report += "\r\n-]> Last downspd:	" +			Util::formatBytes(getUser()->getLastDownloadSpeed()) + "/s";
	report += "\r\n-]> FileList CID:	" +			isEmpty(get("FI"));
	report += "\r\n-]> FileList Base:	" +			isEmpty(get("FB"));

	int64_t listSize = Util::toInt64(get("LS")), listLen = Util::toInt64(get("LL"));
	report += "\r\n-]> Filelist size:	" +			((listSize != -1) ? (string)(Util::formatBytes(listSize) + "  (" + Text::fromT(Util::formatExactSize(listSize)) + " )") : "N/A");
	
	report += "\r\n-]> ListLen:	" +					(listLen != -1 ? (string)(Util::formatBytes(listLen) + "  (" + Text::fromT(Util::formatExactSize(listLen)) + " )") : "N/A");
	report += "\r\n-]> Stated Share:	" +			Util::formatBytes(getBytesShared()) + "  (" + Text::fromT(Util::formatExactSize(getBytesShared())) + " )";

	int64_t realBytes =  (!get("RS").empty()) ?		Util::toInt64(get("RS")) : -1;
	report += "\r\n-]> Real Share:	" +				((realBytes > -1) ? (string)(Util::formatBytes(realBytes) + "  (" + Text::fromT(Util::formatExactSize(realBytes)) + " )\t[" + Util::formatTime("%d-%m %H:%M", Util::toInt64(getFileListChecked())) + ']') : "N/A");
	report += "\r\n-]> Shared files:	" +			isEmpty(get("SF"));

	return report;
}

const string Identity::updateClientType(OnlineUser& ou) {
	if(getUser()->isSet(User::DCPLUSPLUS)) {
		const float versionf = Util::toFloat(getVersion());
		if((get("LL") == "11") && (getBytesShared() > 0)) {
			setClientType("DC++ Stealth");
			const string& report = ou.setCheat("Fake FileList - ListLen = 11B", true, true, RSXBOOLSETTING(SHOW_LISTLEN_MISMATCH));
			ClientManager::getInstance()->sendAction(ou, RSXSETTING(LISTLEN_MISMATCH));
			logDetect(true);
			return report;
		} else if(strncmp(getTag().c_str(), "<++ V:", 6) == 0 && versionf < (float)1.001 && versionf >= (float)0.69) {
			//suppose to be dc++  >= 0.69
			if(get("LL") != "42" && !get("LL").empty()) {
				setClientType("Faked DC++");
				set("CM", "Supports corrupted files...");
				const string& report = ou.setCheat("ListLen mis-match (V:0.69+)", true, false, RSXBOOLSETTING(SHOW_LISTLEN_MISMATCH));
				ClientManager::getInstance()->sendAction(ou, RSXSETTING(LISTLEN_MISMATCH));
				logDetect(true);
				return report;
			} else if(versionf > (float)0.699 && !get("TS").empty() && get("TS") != "GetListLength not supported") {
				const string& report = ou.setCheat("DC++ emulation", true, false, RSXBOOLSETTING(SHOW_DCPP_EMULATION_RAW));
				ClientManager::getInstance()->sendAction(ou, RSXSETTING(DCPP_EMULATION_RAW));
				logDetect(true);
				return report;
			}
		}
	}

	int64_t tick = GET_TICK();

	StringMap params;
	const DetectionManager::DetectionItems& profiles = DetectionManager::getInstance()->getProfiles(params);
	getDetectionParams(params);

	for(DetectionManager::DetectionItems::const_iterator i = profiles.begin(); i != profiles.end(); ++i) {
		const DetectionEntry& entry = *i;
		if(!entry.isEnabled)
			continue;
		StringMap INFList = entry.infMap;
		bool con = false;

		DETECTION_DEBUG("\tChecking profile: " + entry.name);

		for(StringMap::const_iterator j = INFList.begin(); j != INFList.end(); ++j) {
			const string& aPattern = Util::formatParams(j->second, params, false);
			if(!RegexpHandler::matchProfile(getDetectionField(j->first), aPattern)) {
				con = true;
				break;
			}
		}
		if(con)
			continue;

		DETECTION_DEBUG("Client found: " + entry.name + " time taken: " + Util::toString(GET_TICK()-tick) + " milliseconds");

		setClientType(entry.name/* + " " + getVersion()*/);
		set("CM", entry.comment);
		set("BC", entry.cheat.empty() ? Util::emptyString : "1");
		logDetect(true);

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
		if(aName == "NI")
			return getNick();
		else if(aName == "TA")
			return getTag();
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
	getParams(p, "", false);
	p["PKVE"] = getPkVersion();
}

string Identity::getPkVersion() const {
	string pk = get("PK");
	if(pk.find("DCPLUSPLUS") != string::npos && pk.find("ABCABC") != string::npos) {
		return pk.substr(10, pk.length() - 16);
	}
	return Util::emptyString;
}

const string Identity::myInfoDetect(OnlineUser& ou) {
	StringMap params;
	const MyinfoProfile::List& lst = ClientProfileManager::getInstance()->getMyinfoProfiles(params);
	
	//empty status = adc user, here status is empty till user change it
	const string& fixed_status = getStatus().empty() ? "0" : getStatus(); 
	checkTagState(ou);

	for(MyinfoProfile::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const MyinfoProfile& cq = *i;	

		string version, extraVersion, formattedTagExp, verTagExp;
		string tagExp = cq.getTag();

		verTagExp = Util::formatRegExp(cq.getTag(), params);

		formattedTagExp = verTagExp;
		string::size_type j = formattedTagExp.find("%[version]");
		if(j != string::npos) {
			formattedTagExp.replace(j, 10, ".*");
		}
		
		string extTagExp = cq.getExtendedTag();
		string formattedExtTagExp = extTagExp;
		j = extTagExp.find("%[version2]");
		if(j != string::npos) {
			formattedExtTagExp.replace(j, 11, ".*");
		}

		if(!RegexpHandler::matchProfile(getTag(), formattedTagExp))									{ continue; }
		if(!RegexpHandler::matchProfile(getDescription(), formattedExtTagExp))						{ continue; }
		if(!RegexpHandler::matchProfile(getConnection(), cq.getConnection()))						{ continue; }
		if(!RegexpHandler::matchProfile(fixed_status, cq.getStatus()))								{ continue; }
		if(!RegexpHandler::matchProfile(get("SS"), cq.getShared()))									{ continue; } 
		if(!RegexpHandler::matchProfile(getNick(), cq.getNick()))									{ continue; } 
		if(!RegexpHandler::matchProfile(getEmail(), cq.getEmail()))									{ continue; } 

		if(verTagExp.find("%[version]") != string::npos) {	version = RegexpHandler::getVersion(verTagExp, getTag()); }
		if(extTagExp.find("%[version2]") != string::npos) {	extraVersion = RegexpHandler::getVersion(extTagExp, getDescription()); }
		if(!(cq.getVersion().empty()) && !RegexpHandler::matchProfile(version, cq.getVersion()))	{ continue; }

		if(cq.getUseExtraVersion())
			setMyInfoType((cq.getName() + " " + extraVersion)); 
		else 
			setMyInfoType((cq.getName() + " " + version));

		set("CM", cq.getComment());

		string report = Util::emptyString;
		if(!cq.getCheatingDescription().empty()) {
			report = ou.setCheat(cq.getCheatingDescription(), true, false, ou.getClient().isActionActive(cq.getRawToSend()));
		}
		ClientManager::getInstance()->sendAction(ou, cq.getRawToSend());
		return report;
	}
	return Util::emptyString;
}
//RSX++ //Protected users
bool Identity::isProtectedUser(const Client& c, bool OpBotHubCheck) const {
	if(isSet("PR") || getUser()->isSet(User::PROTECTED))
		return true;

	string RegProtect = RSXSETTING(PROTECTED_USERS);
	if(!c.getUserProtected().empty()) {
		RegProtect = c.getUserProtected();
	}

	bool ret = false;
	if(OpBotHubCheck && (isOp() || isBot() || isHub())) {
		ret = true;
	} else if(RSXSETTING(FAV_USER_IS_PROTECTED_USER) && FavoriteManager::getInstance()->isFavoriteUser(getUser())) {
		ret = true;
	} else {
		if(!RegProtect.empty()) {
			if(RSXBOOLSETTING(USE_WILDCARDS_TO_PROTECT)) {
				if(Wildcard::patternMatch(getNick(), RegProtect, '|')) {
					ret = true;
				}
			} else {
				if(RegexpHandler::matchProfile(getNick(), RegProtect)) {
					ret = true;
				}
			}
		}
	}
	if(ret == true) {
		const_cast<Identity&>(*this).set("PR", "1");
	}
	return ret;
}

void OnlineUser::updateUser() { getClient().updated(*this); }
//RSX++ //checking stuff
bool OnlineUser::isCheckable(bool delay/* = true*/) const {
	return (!identity.isHub() && !identity.isBot() && !identity.isOp() && !identity.isHidden()) && (!delay || (GET_TICK() - identity.getLoggedIn()) > RSXSETTING(CHECK_DELAY));
}

bool OnlineUser::shouldCheckFileList(bool onlyFilelist /*= false*/) const {
	if(identity.isFileListQueued() || identity.isFileListChecked() || (!onlyFilelist && identity.isTestSURQueued()))
		return false;
	return ((GET_TIME() - Util::toInt64(identity.getTestSURChecked()) > 10) || onlyFilelist);
}

bool OnlineUser::getChecked(bool filelist/* = false*/) {
	if(isProtectedUser()) {
		if((RSXSETTING(UNCHECK_CLIENT_PROTECTED_USER) && !filelist) || (RSXSETTING(UNCHECK_LIST_PROTECTED_USER) && filelist)) {
			identity.setClientType("[Protected]");
			setTestSURComplete();
			setFileListComplete();
			return true;
		}
	} else if(!identity.isTcpActive() && !getClient().isActive()) {
		identity.setClientType("[Passive]");
		setTestSURComplete();
		setFileListComplete();
		return true;
	} else if(getUser()->isSet(User::OLD_CLIENT)) {
		setTestSURComplete();
		setFileListComplete();
		return true;
	}
	return filelist ? identity.isFileListChecked() : identity.isClientChecked();
}
//RSX++ //$MyINFO Flood
bool Identity::isMyInfoSpamming() {
	set("MC", Util::toString(Util::toInt(get("MC")) + 1));

	uint64_t now = GET_TICK();
	uint64_t i = now - lastMyInfo;
	if(i < RSXSETTING(MYINFO_TIME)) {
		myinfoFloodCounter++;
	} else {
		myinfoFloodCounter = 0;
	}

	lastMyInfo = now;
	if(myinfoFloodCounter > RSXSETTING(MYINFO_COUNTER)) {
		myinfoFloodCounter = 0;
		return true;
	}
	return false;
}
//RSX++ //$ConnectToMe Flood
bool Identity::isCtmSpamming() {
	uint64_t now = GET_TICK();
	uint64_t i = now - lastCte;
	if(i < RSXSETTING(CTM_TIME)) {
		cteFloodCounter++;
	} else {
		cteFloodCounter = 0;
	}
	lastCte = now;
	if(cteFloodCounter > RSXSETTING(CTM_COUNTER)) {
		cteFloodCounter = 0;
		return true;
	}
	return false;
}
//RSX++ //PM flood
bool Identity::isPmSpamming() {
	uint64_t now = GET_TICK();
	uint64_t i = now - lastPm;
	if(i < RSXSETTING(PM_TIME)) {
		pmFloodCounter++;
	} else {
		pmFloodCounter = 0;
	}
	lastPm = now;
	if(pmFloodCounter > RSXSETTING(PM_COUNTER)) {
		pmFloodCounter = 0;
		return true;
	}
	return false;
}
//RSX++ //IP Check
void Identity::checkIP(OnlineUser& ou) {
	if(RSXBOOLSETTING(USE_IPWATCH)) {
		set("IC", "1"); //ip checked, to avoid cheat spam caused by search
		IPWatch::List& il = IpManager::getInstance()->getWatch();
		bool matched = false;
		for(IPWatch::Iter j = il.begin(); j != il.end(); j++) {
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
					PME reg((*j)->getPattern(), "gi");
					if(reg.IsValid() && reg.match(strToMatch))
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
						ou.updateUser();
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
const string Identity::checkFilelistGenerator(OnlineUser& ou) {
	{
		PME reg("^<StrgDC\\+\\+ V:1.00 RC([89]){1}");
		if((get("FG") == "DC++ 0.403")) {
			if(reg.match(getTag())) {
				string report = ou.setCheat("rmDC++ in StrongDC++ %[userVE] emulation mode" , true, false, RSXBOOLSETTING(SHOW_RMDC_RAW));
				setClientType("rmDC++");
				logDetect(true);
				ou.updateUser();
				ClientManager::getInstance()->sendAction(ou, RSXSETTING(RMDC_RAW));
				return report;
			}
		}
	}

	if(!get("VE").empty() && strncmp(getTag().c_str(), "<++ V:", 6) == 0) {
		if((Util::toFloat(get("VE")) > 0.668)) {
			if(get("FI").empty() || get("FB").empty()) {
				string report = ou.setCheat("DC++ emulation", true, false, RSXBOOLSETTING(SHOW_DCPP_EMULATION_RAW));
				logDetect(true);
				ou.updateUser();
				ClientManager::getInstance()->sendAction(ou, RSXSETTING(DCPP_EMULATION_RAW));
				return report;
			}
		} else {
			if(!get("FI").empty() || !get("FB").empty()) {
				string report = ou.setCheat("DC++ emulation", true, false, RSXBOOLSETTING(SHOW_DCPP_EMULATION_RAW));
				logDetect(true);
				ou.updateUser();
				ClientManager::getInstance()->sendAction(ou, RSXSETTING(DCPP_EMULATION_RAW));
				return report;
			}
		}
	}

	{
		PME reg("^DC\\+\\+.*");
		if(reg.match(get("FG"))) {
			if(!get("VE").empty() && (get("VE") != getFilelistGeneratorVer())) {
				string report = ou.setCheat("Filelist Version mis-match", false, true, RSXBOOLSETTING(SHOW_FILELIST_VERSION_MISMATCH));
				logDetect(true);
				ou.updateUser();
				ClientManager::getInstance()->sendAction(ou, RSXSETTING(FILELIST_VERSION_MISMATCH));
				return report;
			}
		}
	}

	const FileListDetectorProfile::List& lst = ClientProfileManager::getInstance()->getFileListDetectors();
	for(FileListDetectorProfile::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const FileListDetectorProfile& fd = *i;	
		if(!RegexpHandler::matchProfile(get("FG"), fd.getDetect())) { continue; }

		string report = Util::emptyString;
		if(fd.getBadClient()) {
			if(!fd.getName().empty()) setClientType(fd.getName());
			report = ou.setCheat(fd.getCheatingDescription(), false, true, fd.getCheatingDescription().empty()?false:true);
		}
		logDetect(true);
		ClientManager::getInstance()->sendAction(ou, fd.getRawToSend());
		return report;
	}
	logDetect(false);
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

const string Identity::checkrmDC(OnlineUser& ou) {
	string report = Util::emptyString;
	PME reg("^0.40([0123]){1}$");
	if(reg.match(getVersion())) {
		report = ou.setCheat("rmDC++ in DC++ %[userVE] emulation mode" , true, false, RSXBOOLSETTING(SHOW_RMDC_RAW));
		setClientType("rmDC++");
		ClientManager::getInstance()->sendAction(ou, RSXSETTING(RMDC_RAW));
	}
	return report;
}

void Identity::logDetect(bool successful) {
	if(successful) {
		RSXS_SET(TOTAL_DETECTS, RSXSETTING(TOTAL_DETECTS) + 1);
	} else {
		RSXS_SET(TOTAL_FAILED_DETECTS, RSXSETTING(TOTAL_FAILED_DETECTS) + 1);
	}
}

void Identity::checkTagState(OnlineUser& ou) {
	if(isTcpActive() && (getTag().find(",M:P,") != string::npos)) {
		ou.getClient().cheatMessage("*** " + getNick() + " - Tag states passive mode, but he's using active commands"); 
	} else if(!isTcpActive() && (getTag().find(",M:A,") != string::npos)) {
		ou.getClient().cheatMessage("*** " + getNick() + " - Tag states active mode, but he's using passive commands"); 
	}
}

void Identity::cleanUser() {
	resetCounters();
	set("MT", Util::emptyString); //MyINFO client type
	set("I4", Util::emptyString); //IP
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
	set("R1", Util::emptyString); //recheck
}

const string Identity::checkSlotsCount(OnlineUser& ou, int realSlots) {
	set("SC", Util::toString(realSlots));
	string report = Util::emptyString;
	if(Util::toInt(get("SL")) > realSlots) { //more slots showed than real opened? cheater ;]
		report = ou.setCheat("fake slots count - tag states  %[userSL] slots, but real user have opened %[userSC] slots", true, false, RSXBOOLSETTING(SHOW_FAKE_SLOT_COUNT));
		ClientManager::getInstance()->sendAction(ou, RSXSETTING(FAKE_SLOT_COUNT));
	}
	return report;
}
//iUser methods
void OnlineUser::sendPM(const char* aMsg) {
	ClientManager::getInstance()->privateMessage(identity.getUser(), string(aMsg));
}

iClient* OnlineUser::getUserClient() { 
	return &getClient(); 
}
//END
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
		case COLUMN_UPLOAD_SPEED: return compare(a->identity.getUser()->getLastDownloadSpeed(), b->identity.getUser()->getLastDownloadSpeed());
	}
	return lstrcmpi(a->getText(col).c_str(), b->getText(col).c_str());
}

const tstring OnlineUser::getText(uint8_t col) const {
	switch(col) {
		case COLUMN_NICK: return Text::toT(identity.getNick());
		case COLUMN_SHARED: return Util::formatBytesW(identity.getBytesShared());
		case COLUMN_EXACT_SHARED: return Util::formatExactSize(identity.getBytesShared());
		case COLUMN_DESCRIPTION: return Text::toT(identity.getDescription());
		case COLUMN_TAG: return Text::toT(identity.getTag());
		case COLUMN_CONNECTION: return Text::toT(identity.getConnection());
		case COLUMN_EMAIL: return Text::toT(identity.getEmail());
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
		case COLUMN_VERSION: return Text::toT(identity.getVersion());
		case COLUMN_MODE: return identity.isTcpActive() ? _T("A") : _T("P");
		case COLUMN_HUBS: {
			const tstring hn = Text::toT(identity.get("HN"));
			const tstring hr = Text::toT(identity.get("HR"));
			const tstring ho = Text::toT(identity.get("HO"));
			const tstring cn = Util::toStringW(Util::toInt(identity.get("HN")) + Util::toInt(identity.get("HR")) + Util::toInt(identity.get("HO")));
			const_cast<Identity&>(identity).set("AH", Text::fromT(cn));
			return (hn.empty() || hr.empty() || ho.empty()) ? Util::emptyStringT : (cn + _T("(") + hn + _T("/") + hr + _T("/") + ho + _T(")"));
		}
		case COLUMN_SLOTS: return Text::toT(identity.get("SL"));
		case COLUMN_UPLOAD_SPEED: {
			if(identity.getUser()->getLastDownloadSpeed() > 0)
				return Util::toStringW(identity.getUser()->getLastDownloadSpeed()) + _T(" kB/s");
			else if(identity.getUser()->isSet(User::FIREBALL))
				return _T(">= 100 kB/s");
			else
				return _T("N/A");
		}
		case COLUMN_IP: {
			string ip = identity.getIp();
			string country = ip.empty() ? Util::emptyString : Util::getIpCountry(ip);
			if (!country.empty())
				ip = country + " (" + ip + ")";
			return Text::toT(ip);
		}
		case COLUMN_HOST: return Text::toT(identity.get("HT"));
		case COLUMN_ISP: return Text::toT(identity.getISP());
		case COLUMN_PK: return Text::toT(identity.get("PK"));
		case COLUMN_LOCK: return Text::toT(identity.get("LO"));
		case COLUMN_SUPPORT: return Text::toT(identity.get("SU"));
		case COLUMN_STATUS: { 
			if(getUser()->isSet(User::NMDC)) {
				return Text::toT(Util::formatStatus(Util::toInt(identity.getStatus())));
			} else {
				switch(Util::toInt(identity.getStatus())) {
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

tstring old = Util::emptyStringT;
bool OnlineUser::update(int sortCol, const tstring& oldText) {
	bool needsSort = ((identity.get("WO").empty() ? false : true) != identity.isOp());
	
	if(sortCol == -1) {
		isInList = true;
	} else {
		needsSort = needsSort || (oldText != getText(static_cast<uint8_t>(sortCol)));
	}

	return needsSort;
}

/**
 * @file
 * $Id: User.cpp 347 2007-12-27 20:58:14Z bigmuscle $
 */