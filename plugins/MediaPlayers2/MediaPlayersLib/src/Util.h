#ifndef UTIL_H
#define UTIL_H

namespace CodePage {
	const unsigned int acp = 0;
	const unsigned int oem = 1;
	const unsigned int macintosh = 2;
	const unsigned int thread = 3;
	const unsigned int symbol = 42;
	const unsigned int utf7 = 65000;
	const unsigned int utf8 = 65001;
} // namespace CodePage


#include <map>
typedef std::map<std::string, std::string> StringMap;
typedef StringMap::const_iterator StringMapIter;
#if defined(_MSC_VER)
#define _LL(x) x##ll
#define _ULL(x) x##ull
#define I64_FMT "%I64d"
#define U64_FMT "%I64d"
#endif

class Util {
public:
	static string toString(short val) {
		char buf[8];
		_snprintf(buf, sizeof(buf), "%d", (int)val);
		return buf;
	}
	static string toString(unsigned short val) {
		char buf[8];
		_snprintf(buf, sizeof(buf), "%u", (unsigned int)val);
		return buf;
	}
	static string toString(int val) {
		char buf[16];
		_snprintf(buf, sizeof(buf), "%d", val);
		return buf;
	}
	static string toString(unsigned int val) {
		char buf[16];
		_snprintf(buf, sizeof(buf), "%u", val);
		return buf;
	}
	static string toString(long val) {
		char buf[32];
		_snprintf(buf, sizeof(buf), "%ld", val);
		return buf;
	}
	static string toString(unsigned long val) {
		char buf[32];
		_snprintf(buf, sizeof(buf), "%lu", val);
		return buf;
	}
	static string toString(long long val) {
		char buf[32];
		_snprintf(buf, sizeof(buf), I64_FMT, val);
		return buf;
	}
	static string toString(unsigned long long val) {
		char buf[32];
		_snprintf(buf, sizeof(buf), U64_FMT, val);
		return buf;
	}
	static string toString(double val) {
		char buf[16];
		_snprintf(buf, sizeof(buf), "%0.2f", val);
		return buf;
	}

	static string formatSeconds(__int64 aSec, bool supressHours = false) {
		char buf[64];
		if (!supressHours)
			_snprintf(buf, sizeof(buf), "%01lu:%02d:%02d", (unsigned long)(aSec / (60*60)), (int)((aSec / 60) % 60), (int)(aSec % 60));
		else
			_snprintf(buf, sizeof(buf), "%02d:%02d", (int)(aSec / 60), (int)(aSec % 60));	
		return buf;
	}

	static string formatBytes(__int64 aBytes);

	static string getFileName(const string& path) {
		string::size_type i = path.rfind("\\");
		return (i != string::npos) ? path.substr(i + 1) : path;
	}

	static __int64 getFileSize(const wstring& aFileName) throw();
	static double toDouble(const string& aString);
	static std::wstring convertToWide(const std::string& str);
	static std::string convertFromWide(const std::wstring& str);
	static std::string formatParams(const std::string& format, StringMap& params);
};

#endif //UTIL_H