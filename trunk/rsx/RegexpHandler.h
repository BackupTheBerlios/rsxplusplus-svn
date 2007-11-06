// RegexpHandler.h Added by Sulan 2005-05-20
// Pothead had a little play with it

#if !defined(AFX_REGEXP_HANDLER_H__E9B493AC_97A7_4A18_AF7C_06BFE1926A52__INCLUDED_)
#define AFX_REGEXP_HANDLER_H__E9B493AC_97A7_4A18_AF7C_06BFE1926A52__INCLUDED_

#include "../client/ResourceManager.h"
//#include "DebugManager.h"
#include "../client/pme.h"
#include "RsxUtil.h"

class RegexpHandler 
{
public:
/*	static bool isValid(const string& aString, const bool& caseSensative = true ) {
		PME reg(aString, caseSensative ? "" : "i");
		return Util::toBool(reg.IsValid());
	}
*/
	static bool matchProfile(const string& aString, const string& aProfile, const bool& caseSensative = true) {
		//DETECTION_DEBUG("\t\tMatching String: " + aString + " to Profile: " + aProfile);
		PME reg(aProfile, caseSensative ? "" : "i");
		return reg.IsValid() ? RsxUtil::toBool(reg.match(aString)) : false;
	}
	//Return the tags version number for %[version]
	
	static string getVersion(const string& aExp, const string& aTag) {
		string::size_type i = aExp.find("%[version]");
		if (i == string::npos) { 
			i = aExp.find("%[version2]"); 
			return splitVersion(aExp.substr(i + 11), splitVersion(aExp.substr(0, i), aTag, 1), 0);
		}
		return splitVersion(aExp.substr(i + 10), splitVersion(aExp.substr(0, i), aTag, 1), 0);
	}
	//Check if regexp is valid and return if it is a match or no match
	static string matchExp(const string& aExp, const string& aString, const bool& caseSensative = true) {
		PME reg(aExp, caseSensative ? "" : "i");
		if(!reg.IsValid()) { 
			return CSTRING(S_INVALID); 
		}
		return reg.match(aString) ? CSTRING(S_MATCH) : CSTRING(S_MISSMATCH);
	}

	//Format the params so we can view the regexp string
	static string formatRegExp(const string& msg, StringMap& params) {
		string result = msg;
		string::size_type i, j, k;
		i = 0;
		while (( j = result.find("%[", i)) != string::npos) {
			if( (result.size() < j + 2) || ((k = result.find(']', j + 2)) == string::npos) ) {
				break;
			}
			string name = result.substr(j + 2, k - j - 2);
			StringMapIter smi = params.find(name);
			if(smi != params.end()) {
				result.replace(j, k-j + 1, smi->second);
				i = j + smi->second.size();
			} else {
				i = k + 1;
			}
		}
		return result;
	}
	// Check if string is an IP
	static bool isIp(const string& aString) {
		return matchProfile(aString, "\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b") ? true : false;
	}

private:
	static string splitVersion(const string& aExp, const string& aTag, const int part) {
		PME reg(aExp);
		if(!reg.IsValid()) { return ""; }
		reg.split(aTag, 2);
		return reg[part];
	}
};
#endif // !defined(AFX_REGEXP_HANDLER_H__E9B493AC_97A7_4A18_AF7C_06BFE1926A52__INCLUDED_)
/**
 * @file
 * $Id: RegexpHandler.h 1.0 2005-08-03 22:19:45 Virus27 $
 */