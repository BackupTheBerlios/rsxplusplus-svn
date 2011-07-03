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

#include "stdafx.h"
#include "main.h"

#include "../client/DCPlusPlus.h"
#include "../client/File.h"
#include "../client/MerkleTree.h"
#include "../client/SettingsManager.h"
#include "../client/UPnPManager.h"
#include "../client/version.h"

#include "ExtendedTrace.h"
#include "MainWindow.h"
#include "SplashWindow.h"
#include "UPnP_COM.h"
#include "UPnP_MiniUPnPc.h"
#include "ImageManager.h"

#include <delayimp.h>

#include <wx/debugrpt.h>
#include <wx/sound.h>

CriticalSection cs;
enum { DEBUG_BUFSIZE = 8192 };
static char guard[DEBUG_BUFSIZE];
static int recursion = 0;
static char tth[192*8/(5*8)+2];
static bool firstException = true;

static char buf[DEBUG_BUFSIZE];

IMPLEMENT_APP(RSXPlusPlus)

RSXPlusPlus::RSXPlusPlus()
{
#ifdef _WIN32
	// this is needed for UPnP to work correctly
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
}

RSXPlusPlus::~RSXPlusPlus()
{
#ifdef _WIN32
	CoUninitialize();
#endif
}

void RSXPlusPlus::OnFatalException()
{
	Lock l(cs);

	if(recursion++ > 30)
		exit(-1);

	if(firstException) 
	{
		File::deleteFile(Util::getPath(Util::PATH_RESOURCES) + "exceptioninfo.txt");
		firstException = false;
	}


	File f(Util::getPath(Util::PATH_RESOURCES) + "exceptioninfo.txt", File::WRITE, File::OPEN | File::CREATE);
	f.setEndPos(0);
	
	// TODO: exception code from wxCrashContext
	sprintf(buf, "Version: %s\r\n", VERSIONSTRING);
	f.write(buf, strlen(buf));

#if defined(SVNVERSION)
	sprintf(buf, "SVN: %s\r\n", SVNVERSION);	
	f.write(buf, strlen(buf));
#endif	
	
	f.write("OS: " + string(wxGetOsDescription().mb_str()) + "\r\n");

	time_t now;
	time(&now);
	strftime(buf, DEBUG_BUFSIZE, "Time: %Y-%m-%d %H:%M:%S\r\n", localtime(&now));

	f.write(buf, strlen(buf));

	f.write(LIT("TTH: "));
	f.write(tth, strlen(tth));
	f.write(LIT("\r\n"));

    f.write(LIT("\r\n"));

	string stack;
#ifndef _WIN64
	TextStackWalker tsw(&stack);
	tsw.WalkFromException();
#else
#pragma message(__FILE__ ": TODO: write stack walker for x64")
#endif

	f.write(stack);
	f.write(LIT("\r\n"));
	f.close();


	if ((!SETTING(SOUND_EXC).empty()) && (!BOOLSETTING(SOUNDS_DISABLED)))
		wxSound::Play(Text::toT(SETTING(SOUND_EXC)), wxSOUND_ASYNC);

	WinUtil::notifyUser(_("RSX++ has crashed"), _("exceptioninfo.txt was generated"), wxICON_ERROR);

	if(wxMessageBox(_("RSX++ just encountered a fatal bug and should have written an exceptioninfo.txt the same directory as the executable. You can report this at ") + _(RSXPP_TRAC) + _(" to help us find out what happened. Go there now?"), _("RSX++ has crashed"), wxYES_NO | wxICON_ERROR) == wxYES) 
	{
		WinUtil::openLink(_T(RSXPP_TRAC));
	}
}

void callBack(void* x, const tstring& a) 
{
	SplashWindow* splash = (SplashWindow*)x;
	splash->setText(a);
	splash->Refresh();
	splash->Update();
}

bool RSXPlusPlus::OnInit()
{
	dcapp = new wxSingleInstanceChecker(wxT("{RSXPLUSPLUS-AEE8350A-B49A-4753-AB4B-E55479A48351}"));
	if(dcapp->IsAnotherRunning())
	{
		// Allow for more than one instance...
		bool multiple = false;

		if(argc == 1) 
		{
			if (wxMessageBox(_("There is already an instance of RSX++ running.\nDo you want to launch another instance anyway?"), 
				wxT(APPNAME) wxT(" ") wxT(VERSIONSTRING), wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT) == wxYES) 
			{
				multiple = true;
			}
		}

		if(multiple == false) 
		{
			/* TODO: migrate
			HWND hOther = NULL;
			EnumWindows(searchOtherInstance, (LPARAM)&hOther);

			if( hOther != NULL ) 
			{
				// pop up
				::SetForegroundWindow(hOther);

				if( IsIconic(hOther)) 
				{
					// restore
					::ShowWindow(hOther, SW_RESTORE);
				}
				sendCmdLine(hOther, argc, argv);
			}
			*/
			delete dcapp; // OnExit() won't be called if we return false
            dcapp = NULL;

            return false;
		}
	}
	
	wxImage::AddHandler(new wxPNGHandler);
	ImageManager::newInstance();

#ifndef _DEBUG
	// if we install our own exception handler, the debugger won't break on exception
	// and it will be very hard to debug crashes, so enabled it for release build only
	wxHandleFatalExceptions();
#endif

	try 
	{	
		File f(Text::fromT(tstring(argv[0])), File::READ, File::OPEN);
		TigerTree tth(TigerTree::calcBlockSize(f.getSize(), 1));
		size_t n = 0;
		size_t n2 = DEBUG_BUFSIZE;
		while( (n = f.read(buf, n2)) > 0) 
		{
			tth.update(buf, n);
			n2 = DEBUG_BUFSIZE;
		}
		tth.finalize();
		strcpy(::tth, tth.getRoot().toBase32().c_str());
		WinUtil::tth = Text::toT(::tth);
	} 
	catch(const FileException&) 
	{
		dcdebug("Failed reading exe\n");
	}	

	SplashWindow* splash = new SplashWindow();
	startup(callBack, (void*)splash);
	splash->Destroy();

	/* FIXME: if(ResourceManager::getInstance()->isRTL()) 
	{
		SetProcessDefaultLayout(LAYOUT_RTL);
	}*/

	UPnPManager::getInstance()->addImplementation(new UPnP_MiniUPnPc());
	UPnPManager::getInstance()->addImplementation(new UPnP_COM());

	wxSize size(890, 700);
	wxPoint pos = wxDefaultPosition;

	if( (SETTING(MAIN_WINDOW_POS_X) != -1) &&
		(SETTING(MAIN_WINDOW_POS_Y) != -1) &&
		(SETTING(MAIN_WINDOW_SIZE_X) != -1) &&
		(SETTING(MAIN_WINDOW_SIZE_Y) != -1) ) 
	{
		pos.x = SETTING(MAIN_WINDOW_POS_X);
		pos.y = SETTING(MAIN_WINDOW_POS_Y);
		size.SetWidth(SETTING(MAIN_WINDOW_SIZE_X));
		size.SetHeight(SETTING(MAIN_WINDOW_SIZE_Y));
		
		// Now, let's ensure we have sane values here...
		if((pos.x < 0 ) || (pos.y < 0)) { pos = wxDefaultPosition; }
		if((size.GetWidth() < 10) || (size.GetHeight() < 10)) { size = wxDefaultSize; }
	}

	MainWindow* wndMain = new MainWindow(pos, size);

	if(ResourceManager::getInstance()->isRTL())
		wndMain->SetLayoutDirection(wxLayout_RightToLeft);

	if(BOOLSETTING(MINIMIZE_ON_STARTUP)) 
	{
		wndMain->Iconize(true);
	} 
	else 
	{
		wndMain->Show();
		
		// TODO: add support for user-defined state (like it was with nCmdShow)
		if(SETTING(MAIN_WINDOW_STATE) == 1)
			wndMain->Maximize();
	}
 
	return true;
}

int RSXPlusPlus::OnExit()
{
	shutdown();

	delete dcapp;
	ImageManager::deleteInstance();

	return 0;
}

/**
 * @file
 * $Id: main.cpp 561 2011-01-05 16:29:49Z bigmuscle $
 */
