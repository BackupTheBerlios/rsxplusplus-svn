/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
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

#include "stdafx.h"
#include "ExtendedTrace.h"

#include "../client/Text.h"
#include "../client/Util.h"

#define LIBCOUNT		47

TCHAR* crashLibs[LIBCOUNT][2] = {
	{ L"Vlsp", L"V-One Smartpass" },
	{ L"mclsp", L"McAfee AV" },
	{ L"Niphk", L"Norman AV" },
	{ L"aslsp", L"Aventail Corporation VPN" },
	{ L"AXShlEx", L"Alcohol 120%" },
	{ L"gdlsphlr", L"McAfee" },
	{ L"mlang", L"IE" },
	{ L"cslsp", L"McAfee" },
	{ L"winsflt", L"PureSight Internet Content Filter" },
	{ L"imslsp", L"ZoneLabs IM Secure" },
	{ L"apitrap", L"Norton Cleansweep [?]" },
	{ L"sockspy", L"BitDefender Antivirus" },
	{ L"imon", L"Eset NOD32" },
	{ L"KvWspXp(_1)", L"Kingsoft Antivirus" },
	{ L"nl_lsp", L"NetLimiter" },
	{ L"OSMIM", L"Marketscore Internet Accelerator" },
	{ L"opls", L"Opinion Square [malware]" },
	{ L"PavTrc", L"Panda Anti-Virus" },
	{ L"pavlsp", L"Panda Anti-Virus" },
	{ L"AppToPort", L"Wyvern Works  Firewall" },
	{ L"SpyDll", L"Nice Spy [malware]" },
	{ L"WBlind", L"Window Blinds" },
	{ L"UPS10", L"Uniscribe Unicode Script Processor Library" },
	{ L"SOCKS32", L"Sockscap [?]" },
	{ L"___j", L"Worm: W32.Maslan.C@mm" },
	{ L"nvappfilter", L"NVidia nForce Network Access Manager" },
	{ L"mshp32", L"Worm: W32.Worm.Feebs" },
	{ L"ProxyFilter", L"Hide My IP 2007" },
	{ L"msui32", L"Malware MSUI32" },
	{ L"fsma32", L"F-Secure Management Agent" },
	{ L"FSLSP", L"F-Secure Antivirus/Internet Security" },
	{ L"msxq32", L"Trojan.Win32.Agent.bi" },
	{ L"CurXP0", L"Stardock CursorXP" },
	{ L"msnq32", L"Trojan" },
	{ L"proxy32", L"FreeCap" },
	{ L"iFW_Xfilter", L"System Mechanic Professional 7's Firewall" },
	{ L"spi", L"Ashampoo Firewall" },
	{ L"haspnt32", L"AdWare.Win32.BHO.cw" },
	{ L"TCompLsp", L"Traffic Compressor" },
	{ L"MSCTF", L"Microsoft Text Service Module" },
	{ L"radhslib", L"Naomi web filter" },
	{ L"msftp", L"Troj/Agent-GNA" },
	{ L"ftp34", L"Troj/Agent-GZF" },
	{ L"imonlsp", L"Internet Monitor Layered Service provider" },
	{ L"McVSSkt", L"McAfee VirusScan Winsock Helper" },
	{ L"adguard", L"Sir AdGuard" },
	{ L"msjetoledb40", L"Microsoft Jet 4.0" }
};

static void checkBuggyLibrary(const wxString& library) 
{
	for(int i = 0; i < LIBCOUNT; i++) 
	{
		if(library.CmpNoCase(crashLibs[i][0]) == 0)
		{
			tstring app = crashLibs[i][1];
			size_t BUF_SIZE = TSTRING(LIB_CRASH).size() + app.size() + 16;
			
			tstring buf;
			buf.resize(BUF_SIZE);

			snwprintf(&buf[0], buf.size(), CTSTRING(LIB_CRASH), app.c_str());
		
			wxMessageBox(&buf[0], _T("Unhandled exception"), wxOK);
			exit(1);
		}
	}
}

#ifndef _WIN64
void TextStackWalker::OnStackFrame(const wxStackFrame& frame)
{
	if(frame.HasSourceLocation())
	{
		*stackString += Text::fromT(frame.GetFileName()) + "(" + Util::toString(frame.GetLine()) + ")";
	}
	else
	{
		checkBuggyLibrary(frame.GetModule());

		char buf[11];
		sprintf(buf, "0x%08X", (size_t)frame.GetAddress());
		*stackString += Text::fromT(frame.GetModule()) + "!" + buf;
	}
	
	*stackString += ": " + Text::fromT(frame.GetName());
	*stackString += "\r\n";
}
#else

#endif

/**
* @file
* $Id: ExtendedTrace.cpp 540 2010-07-29 21:31:57Z bigmuscle $
*/
