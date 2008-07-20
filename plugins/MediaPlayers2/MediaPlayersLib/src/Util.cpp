#include "stdafx.h"
#include <time.h>

#include "Util.h"

string Util::formatBytes(__int64 aBytes) {
	char buf[64];
	if(aBytes < 1024) {
		_snprintf(buf, sizeof(buf), "%d %s", (int)(aBytes&0xffffffff), "B");
	} else if(aBytes < 1048576) {
		_snprintf(buf, sizeof(buf), "%.02f %s", (double)aBytes/(1024.0), "KB");
	} else if(aBytes < 1073741824) {
		_snprintf(buf, sizeof(buf), "%.02f %s", (double)aBytes/(1048576.0), "MB");
	} else if(aBytes < (__int64)1099511627776) {
		_snprintf(buf, sizeof(buf), "%.02f %s", (double)aBytes/(1073741824.0), "GB");
	} else if(aBytes < (__int64)1125899906842624) {
		_snprintf(buf, sizeof(buf), "%.02f %s", (double)aBytes/(1099511627776.0), "TB");
	} else if(aBytes < (__int64)1152921504606846976)  {
		_snprintf(buf, sizeof(buf), "%.02f %s", (double)aBytes/(1125899906842624.0), "PB");
	} else {
		_snprintf(buf, sizeof(buf), "%.02f %s", (double)aBytes/(1152921504606846976.0), "EB");
	}

	return buf;
}

__int64 Util::getFileSize(const wstring& aFileName) throw() {
	WIN32_FIND_DATA fd;
	HANDLE hFind;

	hFind = FindFirstFile(aFileName.c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE) {
		return -1;
	} else {
		FindClose(hFind);
		return ((__int64)fd.nFileSizeHigh << 32 | (__int64)fd.nFileSizeLow);
	}
}

double Util::toDouble(const string& aString) {
	// Work-around for atof and locales...
	lconv* lv = localeconv();
	string::size_type i = aString.find_last_of(".,");
	if(i != string::npos && aString[i] != lv->decimal_point[0]) {
		string tmp(aString);
		tmp[i] = lv->decimal_point[0];
		return atof(tmp.c_str());
	}
	return atof(aString.c_str());
}

static bool canConvert( const char * input, unsigned int cp) {
	if(cp == CodePage::utf8) {
		unsigned int skipNext = 0;
		for(const char * idx = input; *idx != 0; ++idx ) {
			if(skipNext != 0) {
				if(!(*idx & 0x80))
					return false;
				--skipNext;
				continue;
			}
			switch(*idx & 0xFC) {
				case 0xFC : // 111111xx
					++skipNext;
				case 0xF8 : // 111110xx
					++skipNext;
				case 0xF0 : // 111100xx
				case 0xF4 : // 111101xx
					++skipNext;
				case 0xE0 : // 111000xx
				case 0xE4 : // 111001xx
				case 0xE8 : // 111010xx
				case 0xEC : // 111011xx
					++skipNext;
				case 0xC0 : // 110000xx
				case 0xC4 : // 110001xx
				case 0xC8 : // 110010xx
				case 0xCC : // 110011xx
				case 0xD0 : // 110100xx
				case 0xD8 : // 110110xx
				case 0xDC : // 110111xx
					++skipNext;
			}
		}

		// Now if skipNext is not 0 this means we've seen a leading character but no trailing one...
		// This is a BUG and CAN'T be converted since it implies we've seen only "half" a charcter...
		if(skipNext != 0)
			return false;
		return true;
	}
	return true;
}

wstring Util::convertToWide(const string& str) {
	unsigned int codepage = CodePage::utf8;
	const char* input = str.c_str();
	if(0 == input || '\0' == input[0])
		return wstring();

	if(!canConvert(input, codepage)) {
		return wstring();
	}

	int size = ::MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, input, - 1, 0, 0);
	if(size == 0) {
		return wstring();
	}

	wchar_t* buf = new wchar_t[size];
	size = ::MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, input, - 1, buf, size);
	wstring ret(buf);
	delete[] buf;
	return ret;
}

string Util::convertFromWide(const wstring& str) {
	unsigned int codepage = CodePage::utf8;
	const wchar_t* input = str.c_str();
	if(0 == input || L'\0' == input[0])
		return string();

	int size = ::WideCharToMultiByte(codepage, 0, input, - 1, 0, 0, 0, 0);
	char* buf = new char[size];
	::WideCharToMultiByte(codepage, 0, input, - 1, buf, size, 0, 0);
	string ret(buf);
	delete[] buf;
	return ret;
}

string fixedftime(const string& format, struct tm* t) {
	string ret = format;
	const char codes[] = "aAbBcdHIjmMpSUwWxXyYzZ%";

	char tmp[4];
	tmp[0] = '%';
	tmp[1] = tmp[2] = tmp[3] = 0;

	StringMap sm;
	static const size_t BUF_SIZE = 1024;
	//boost::scoped_array<char> buf(new char[BUF_SIZE]);
	string buf;
	buf.resize(BUF_SIZE);
	for(size_t i = 0; i < strlen(codes); ++i) {
		tmp[1] = codes[i];
		tmp[2] = 0;
		strftime(&buf[0], BUF_SIZE-1, tmp, t);
		sm[tmp] = &buf[0];

		tmp[1] = '#';
		tmp[2] = codes[i];
		strftime(&buf[0], BUF_SIZE-1, tmp, t);
		sm[tmp] = &buf[0]; 
	}

	for(StringMapIter i = sm.begin(); i != sm.end(); ++i) {
		for(string::size_type j = ret.find(i->first); j != string::npos; j = ret.find(i->first, j)) {
			ret.replace(j, i->first.length(), i->second);
			j += i->second.length() - i->first.length();
		}
	}

	return ret;
}

string formatTime(const string &msg, const time_t t) {
	if (!msg.empty()) {
		size_t bufsize = msg.size() + 256;
		struct tm* loc = localtime(&t);

		if(!loc) {
			return "";
		}
		wstring buf(bufsize, 0);
		buf.resize(_tcsftime(&buf[0], buf.size()-1, Util::convertToWide(msg).c_str(), loc));

		if(buf.empty()) {
			return fixedftime(msg, loc);
		}

		return Util::convertFromWide(buf);
	}
	return "";
}

string Util::formatParams(const string& format, StringMap& params) {
	string result = format;
	string::size_type i, j, k;
	i = 0;
	while (( j = result.find("%[", i)) != string::npos) {
		if( (result.size() < j + 2) || ((k = result.find(']', j + 2)) == string::npos) ) {
			break;
		}
		string name = result.substr(j + 2, k - j - 2);
		StringMapIter smi = params.find(name);
		if(smi == params.end()) {
			result.erase(j, k-j + 1);
			i = j;
		} else {
			if(smi->second.find_first_of("%\\./") != string::npos) {
				string tmp = smi->second;	// replace all % in params with %% for strftime
				string::size_type m = 0;
				while(( m = tmp.find('%', m)) != string::npos) {
					tmp.replace(m, 1, "%%");
					m+=2;
				}

				result.replace(j, k-j + 1, tmp);
				i = j + tmp.size();
			} else {
				result.replace(j, k-j + 1, smi->second);
				i = j + smi->second.size();
			}
		}
	}

	result = formatTime(result, time(NULL));
	return result;
}