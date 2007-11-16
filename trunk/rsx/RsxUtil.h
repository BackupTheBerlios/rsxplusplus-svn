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

#ifndef RSXUTIL_H
#define RSXUTIL_H

class RsxUtil {
public:
	enum ProcessPrio {
		IDLE = 0,
		BELOW_NORMAL = 1,
		NORMAL = 2,
		ABOVE_NORMAL = 3,
		HIGH = 4,
		REALTIME = 5
	};
	static void				init();
	static void				uinit();
	static bool				checkVersion(const string& tag, bool adcT = false);
	static wstring			formatSeconds(int64_t aSec);
	static string			toIP(const uint32_t ipnum);
	static uint32_t			toIpNumber(const string& aIp);
	static uint32_t			getUpperRange(const string& aRange);
	static uint32_t			getLowerRange(const string& aRange);
	static void				changeProcessPriority(ProcessPrio level);
	static int				getActualProcessPrio();
	static void				generateTestSURString();
	static const string&	getTestSURString();
	static const string		getOsVersion();
	static string			getUpdateFileNames(const int number);
	static bool				toBool(const string& aString);
	static bool				toBool(const int aNumber);
	static void				trim(string& source, const string& delims = " \t\r\n");
	static tstring			replace(const tstring& aString, const tstring& fStr, const tstring& rStr);
	static string			replace(const string& aString, const string& fStr, const string& rStr);
	static bool				compareLower(const string& firstStr, const string& secondStr);
	static bool				compareLower(const tstring& firstStr, const tstring& secondStr);
	static int				CalcContrastColor(int crBg);
	static bool				isIpInRange(const string& aIp, const string& aRange);

private:
	static string tmpTestSur;
	static StringList tags;
	static StringList adcTags;
};
#endif //RSXUTIL_H