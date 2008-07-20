#include "stdafx.h"
#include "MediaPlayersLib.h"
#include "Util.h"

#include "Winamp.h"
#include "WMPlayerRemoteApi.h"
#include "iTunesCOMInterface.h"
#include <control.h>
#include <strmif.h>

std::string MediaPlayersLib::getWinampSpam(const std::string& format) {
	HWND hwndWinamp = FindWindow(_T("Winamp v1.x"), NULL);
	if(hwndWinamp) {
		StringMap params;
		int waVersion = (int)SendMessage(hwndWinamp,WM_USER, 0, IPC_GETVERSION),
			majorVersion, minorVersion;
		majorVersion = waVersion >> 12;
		if (((waVersion & 0x00F0) >> 4) == 0) {
			minorVersion = ((waVersion & 0x0f00) >> 8) * 10 + (waVersion & 0x000f);
		} else {
			minorVersion = ((waVersion & 0x00f0) >> 4) * 10 + (waVersion & 0x000f);
		}
		params["version"] = Util::toString(majorVersion + minorVersion / 100.0);
		int state = (int)SendMessage(hwndWinamp,WM_USER, 0, IPC_ISPLAYING);
		switch (state) {
			case 0: params["state"] = "stopped";
				break;
			case 1: params["state"] = "playing";
				break;
			case 3: params["state"] = "paused";
		}
		TCHAR titleBuffer[2048];
		int buffLength = sizeof(titleBuffer);
		GetWindowText(hwndWinamp, titleBuffer, buffLength);
		string title = Util::convertFromWide(titleBuffer);
		params["rawtitle"] = title;

		int starpos = (int)title.find("***");
		if (starpos >= 1) {
			string firstpart = title.substr(0, starpos - 1);
			if (firstpart == title.substr(title.size() - firstpart.size(), title.size())) {
				// fix title
				title = title.substr(starpos, title.size());
			}
		}
		// fix the title if scrolling is on, so need to put the stairs to the end of it
		string titletmp = title.substr(title.find("***") + 2, title.size());
		title = titletmp + title.substr(0, title.size() - titletmp.size());
		title = title.substr(title.find(_T('.')) + 2, title.size());
		if (title.rfind('-') != string::npos) {
			params["title"] = title.substr(0, title.rfind('-') - 1);
		}
		int curPos = (int)SendMessage(hwndWinamp,WM_USER, 0, IPC_GETOUTPUTTIME);
		int length = (int)SendMessage(hwndWinamp,WM_USER, 1, IPC_GETOUTPUTTIME);
		if (curPos == -1) {
			curPos = 0;
		} else {
			curPos /= 1000;
		}
		int intPercent;
		if (length > 0 ) {
			intPercent = curPos * 100 / length;
		} else {
			length = 0;
			intPercent = 0;
		}
		params["percent"] = Util::toString(intPercent) + "%";
		params["elapsed"] = Util::formatSeconds(curPos);
		params["length"] = Util::formatSeconds(length);
		int numFront = (int)min(max(intPercent / 10, 0), 10), numBack = (int)min(max(10 - 1 - numFront, 0), 10);
		string inFront = string(numFront, '-'), inBack = string(numBack, '-');

		params["bar"] = "[" + inFront + (state ? "|" : "-") + inBack + "]";
		int waSampleRate = (int)SendMessage(hwndWinamp,WM_USER, 0, IPC_GETINFO),
			waBitRate = (int)SendMessage(hwndWinamp,WM_USER, 1, IPC_GETINFO),
			waChannels = (int)SendMessage(hwndWinamp,WM_USER, 2, IPC_GETINFO);
		params["bitrate"] = Util::toString(waBitRate) + "kbps";
		params["sample"] = Util::toString(waSampleRate) + "kHz";
		// later it should get some improvement:
		string waChannelName;
		switch(waChannels) {
			case 2:
				params["channels"] = "stereo";
				break;
			case 6:
				params["channels"] = "5.1 surround";
				break;
			default:
				params["channels"] = "mono";
		}
		return Util::formatParams(format, params);
	} else {
		return "";
	}
}

std::string MediaPlayersLib::getWindowsMediaPlayerSpam(const std::string& format, HWND parent) {
	HWND playerWnd = FindWindow(_T("WMPlayerApp"), NULL);
	// If it's not running don't even bother...
	if(playerWnd != NULL) {
		// Load COM
		CoInitialize(NULL);

		// Pointers 
		CComPtr<IWMPPlayer>					Player;
		CComPtr<IAxWinHostWindow>			Host;
		CComPtr<IObjectWithSite>			HostObj;
		CComObject<WMPlayerRemoteApi>		*WMPlayerRemoteApiCtrl = NULL;

		// Other
		HRESULT								hresult;
		CAxWindow *DummyWnd;
		StringMap params;

		// Create hidden window to host the control (if there just was other way to do this... as CoCreateInstance has no access to the current running instance)
		AtlAxWinInit();
		DummyWnd = new CAxWindow();
		hresult = DummyWnd? S_OK : E_OUTOFMEMORY;

		if(SUCCEEDED(hresult)) {
			DummyWnd->Create(parent, NULL, NULL, WS_CHILD | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_SYSMENU);
			hresult = ::IsWindow(DummyWnd->m_hWnd)? S_OK : E_FAIL;
		}
		
		// Set WMPlayerRemoteApi
		if(SUCCEEDED(hresult)) {
			hresult = DummyWnd->QueryHost(IID_IObjectWithSite, (void **)&HostObj);
			hresult = HostObj.p? hresult : E_FAIL;

			if(SUCCEEDED(hresult)) {
				hresult = CComObject<WMPlayerRemoteApi>::CreateInstance(&WMPlayerRemoteApiCtrl);
				if(WMPlayerRemoteApiCtrl) {
					WMPlayerRemoteApiCtrl->AddRef();
				} else {
					hresult = E_POINTER;
				}
			}

			if(SUCCEEDED(hresult)) {
				hresult = HostObj->SetSite((IWMPRemoteMediaServices *)WMPlayerRemoteApiCtrl);
			}
		}
		
		if(SUCCEEDED(hresult)) {
			hresult = DummyWnd->QueryHost(&Host);
			hresult = Host.p? hresult : E_FAIL;
		}

		// Create WMP Control 
		if(SUCCEEDED(hresult)) {
			hresult = Host->CreateControl(CComBSTR(L"{6BF52A52-394A-11d3-B153-00C04F79FAA6}"), DummyWnd->m_hWnd, 0);
		}
	
		// Now we can finally start to interact with WMP, after we successfully get the "Player"
		if(SUCCEEDED(hresult)) {
			hresult = DummyWnd->QueryControl(&Player);
			hresult = Player.p? hresult : E_FAIL;
		}

		// We've got this far now the grande finale, get the metadata :)
		if(SUCCEEDED(hresult)) {
			CComPtr<IWMPMedia>		Media;
			CComPtr<IWMPControls>	Controls;

			if(SUCCEEDED(Player->get_currentMedia(&Media)) && Media.p != NULL) {
				Player->get_controls(&Controls);

				// Windows Media Player version
				CComBSTR bstrWMPVer;
				Player->get_versionInfo(&bstrWMPVer);
				if(bstrWMPVer != NULL) {
					::COLE2T WMPVer(bstrWMPVer);
					params["fullversion"] = Util::convertFromWide(WMPVer.m_szBuffer);
					params["version"] = params["fullversion"].substr(0, params["fullversion"].find("."));
				}

				// Pre-formatted status message from Windows Media Player
				CComBSTR bstrWMPStatus;
				Player->get_status(&bstrWMPStatus);
				if(bstrWMPStatus != NULL) {
					::COLE2T WMPStatus(bstrWMPStatus);
					params["status"] = Util::convertFromWide(WMPStatus.m_szBuffer);
				}

				// Name of the currently playing media
				CComBSTR bstrMediaName;
				Media->get_name(&bstrMediaName);
				if(bstrMediaName != NULL) {
					::COLE2T MediaName(bstrMediaName);
					params["title"] = Util::convertFromWide(MediaName.m_szBuffer);
				}

				// How much the user has already played 
				// I know this is later duplicated with get_currentPosition() for percent and bar, but for some reason it's overall faster this way 
				CComBSTR bstrMediaPosition;
				Controls->get_currentPositionString(&bstrMediaPosition);
				if(bstrMediaPosition != NULL) {
					::COLE2T MediaPosition(bstrMediaPosition);
					params["elapsed"] = Util::convertFromWide(MediaPosition.m_szBuffer);
				}

				// Full duratiuon of the media
				// I know this is later duplicated with get_duration() for percent and bar, but for some reason it's overall faster this way 
				CComBSTR bstrMediaDuration;
				Media->get_durationString(&bstrMediaDuration);
				if(bstrMediaDuration != NULL) {
					::COLE2T MediaDuration(bstrMediaDuration);
					params["length"] = Util::convertFromWide(MediaDuration.m_szBuffer);
				}

				// Name of the artist (use Author as secondary choice)
				CComBSTR bstrArtistName;
				Media->getItemInfo(CComBSTR(_T("Author")), &bstrArtistName);
				if(bstrArtistName != NULL) {
					::COLE2T ArtistName(bstrArtistName);
					params["artist"] = Util::convertFromWide(ArtistName.m_szBuffer);
				} else {
					Media->getItemInfo(CComBSTR(_T("WM/AlbumArtist")), &bstrArtistName);
					if(bstrArtistName != NULL) {
						::COLE2T ArtistName(bstrArtistName);
						params["artist"] = Util::convertFromWide(ArtistName.m_szBuffer);
					}
				}

				// Name of the album
				CComBSTR bstrAlbumTitle;
				Media->getItemInfo(CComBSTR(_T("WM/AlbumTitle")), &bstrAlbumTitle);
				if(bstrAlbumTitle != NULL) {
					::COLE2T AlbumName(bstrAlbumTitle);
					params["album"] = Util::convertFromWide(AlbumName.m_szBuffer);
				}

				// Genre of the media
				CComBSTR bstrMediaGen;
				Media->getItemInfo(CComBSTR(_T("WM/Genre")), &bstrMediaGen);
				if(bstrMediaGen != NULL) {
					::COLE2T MediaGen(bstrMediaGen);
					params["genre"] = Util::convertFromWide(MediaGen.m_szBuffer);
				}

				// Year of publiciation
				CComBSTR bstrMediaYear;
				Media->getItemInfo(CComBSTR(_T("WM/Year")), &bstrMediaYear);
				if(bstrMediaYear != NULL) {
					::COLE2T MediaYear(bstrMediaYear);
					params["year"] = Util::convertFromWide(MediaYear.m_szBuffer);
				} else {
					Media->getItemInfo(CComBSTR(_T("ReleaseDateYear")), &bstrMediaYear);
					if(bstrMediaYear != NULL) {
						::COLE2T MediaYear(bstrMediaYear);
						params["year"] = Util::convertFromWide(MediaYear.m_szBuffer);
					}
				}

				// Bitrate, displayed as Windows Media Player displays it
				CComBSTR bstrMediaBitrate;
				Media->getItemInfo(CComBSTR(_T("Bitrate")), &bstrMediaBitrate);
				if(bstrMediaBitrate != NULL) {
					::COLE2T MediaBitrate(bstrMediaBitrate);
					double BitrateAsKbps = (Util::toDouble(Util::convertFromWide(MediaBitrate.m_szBuffer))/1000);
					params["bitrate"] = Util::toString(int(BitrateAsKbps)) + "kbps";
				}

				// Size of the file
				CComBSTR bstrMediaSize;
				Media->getItemInfo(CComBSTR(_T("Size")), &bstrMediaSize);
				if(bstrMediaSize != NULL) {
					::COLE2T MediaSize(bstrMediaSize);
					params["size"] = Util::formatBytes(_atoi64(Util::convertFromWide(MediaSize.m_szBuffer).c_str()));
				}

				// Users rating for this media
				CComBSTR bstrUserRating;
				Media->getItemInfo(CComBSTR(_T("UserRating")), &bstrUserRating);
				if(bstrUserRating != NULL) {
					if(bstrUserRating == "0") {
						params["rating"] = "unrated";
					} else if(bstrUserRating == "1") {
						params["rating"] = "*";
					} else if(bstrUserRating == "25") {
						params["rating"] = "* *";
					} else if(bstrUserRating == "50") {
						params["rating"] = "* * *";
					} else if(bstrUserRating == "75") {
						params["rating"] = "* * * *";
					} else if(bstrUserRating == "99") {
						params["rating"] = "* * * * *";
					} else {
						params["rating"] = "";
					}
				}

				// Bar & percent
				double elapsed;
				double length;
				Controls->get_currentPosition(&elapsed);
				Media->get_duration(&length);
				if(elapsed > 0) {
					int intPercent;
					if (length > 0 ) {
						intPercent = int(elapsed) * 100 / int(length);
					} else {
						length = 0;
						intPercent = 0;
					}
					params["percent"] = Util::toString(intPercent) + "%";
					int numFront = (int)min(max(intPercent / 10, 0), 10),
						numBack = (int)min(max(10 - 1 - numFront, 0), 10);
					string inFront = string(numFront, '-'),
						inBack = string(numBack, '-');
					params["bar"] = "[" + inFront + (elapsed > 0 ? "|" : "-") + inBack + "]";
				} else {
					params["percent"] = "0%";
					params["bar"] = "[|---------]";
				}
			}
		}

		// Release WMPlayerRemoteApi, if it's there
		if(WMPlayerRemoteApiCtrl) {
			WMPlayerRemoteApiCtrl->Release();
		}
			
		// Destroy the hoster window, and unload COM
		DummyWnd->DestroyWindow();
		delete DummyWnd;
		CoUninitialize();
			
		// If there is something in title, we have at least partly succeeded 
		if(params["title"] != "") {
			return Util::formatParams(format, params);
		} else {
			return "no_media";
		}
	} else {
		return "";
	}
	return string();
}

std::string MediaPlayersLib::get_iTunesSpam(const std::string& format) {
	HWND playerWnd = FindWindow(_T("iTunes"), _T("iTunes"));
	if(playerWnd != NULL) {
		// Pointer
		IiTunes *iITunes;
	
		// Load COM library
		CoInitialize(NULL);

		// Others
		StringMap params;

		// note - CLSID_iTunesApp and IID_IiTunes are defined in iTunesCOMInterface_i.c
		//Create an instance of the top-level object.  iITunes is an interface pointer to IiTunes.  (weird capitalization, but that's how Apple did it)
		if (SUCCEEDED(::CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&iITunes))) {
			long length(0), elapsed;

			//pTrack is a pointer to the track.  This gets passed to other functions to get track data.  wasTrack lets you check if the track was grabbed.
			IITTrack *pTrack;
			//Sanity check -- should never fail if CoCreateInstance succeeded.  You may want to use this for debug output if it does ever fail.
			if (SUCCEEDED(iITunes->get_CurrentTrack(&pTrack)) && pTrack != NULL) {
				//Get album, then call ::COLE2T() to convert the text to array
				BSTR album;
				pTrack->get_Album(&album);
				if (album != NULL) {
					::COLE2T Album(album);
					params["album"] = Util::convertFromWide(Album.m_szBuffer);
				}

				//Same for artist
				BSTR artist;
				pTrack->get_Artist(&artist);
				if(artist != NULL) {
					::COLE2T Artist(artist);
					params["artist"] = Util::convertFromWide(Artist.m_szBuffer);
				}

				//Track name (get_Name is inherited from IITObject, of which IITTrack is derived)
				BSTR name;
				pTrack->get_Name(&name);
				if(name != NULL) {
					::COLE2T Name(name);
					params["title"] = Util::convertFromWide(Name.m_szBuffer);
				}

				// Genre
				BSTR genre;
				pTrack->get_Genre(&genre);
				if(genre != NULL) {
					::COLE2T Genre(name);
					params["genre"] = Util::convertFromWide(Genre.m_szBuffer);
				}

				//Total song time
				pTrack->get_Duration(&length);
				if (length > 0) { 
					params["length"] = Util::formatSeconds(length); // <--- once more with feeling
				}

				//Bitrate
				long bitrate;
				pTrack->get_BitRate(&bitrate);
				if (bitrate > 0) {
					params["bitrate"] = Util::toString(bitrate) + "kbps"; 
				} //<--- I'm not gonna play those games.  Mind games.  Board games.  I'm like, come on fhqugads...

				//Frequency
				long frequency;
				pTrack->get_SampleRate(&frequency);
				if (frequency > 0) { 
					params["frequency"] = Util::toString(frequency/1000) + "kHz";
				}

				//Year
				long year;
				pTrack->get_Year(&year);
				if (year > 0) {
					params["year"] = Util::toString(year);
				}
			
				//Size
				long size;
				pTrack->get_Size(&size);
				if (size > 0) {
					params["size"] = Util::formatBytes(size);
				}

				//Release (decrement reference count to 0) track object so it can unload and free itself; otherwise, it's locked in memory.
				pTrack->Release();
			}

			//Player status (stopped, playing, FF, rewind)
			int state(0);
			ITPlayerState pStatus;
			iITunes->get_PlayerState(&pStatus);
			if (pStatus == ITPlayerStateStopped) {
				params["state"] = "stopped";
				state = 1;
			} else if (pStatus == ITPlayerStatePlaying) {
				params["state"] = "playing";
			}

			//Player position (in seconds, you'll want to convert for your output)
			iITunes->get_PlayerPosition(&elapsed);
			if(elapsed > 0) {
				params["elapsed"] = Util::formatSeconds(elapsed);
				int intPercent;
				if (length > 0 ) {
					intPercent = elapsed * 100 / length;
				} else {
					length = 0;
					intPercent = 0;
				}
				params["percent"] = Util::toString(intPercent) + "%";
				int numFront = (int)min(max(intPercent / 10, 0), 10),
					numBack = (int)min(max(10 - 1 - numFront, 0), 10);
				string inFront = string(numFront, '-'),
				   inBack = string(numBack, '-');
				params["bar"] = "[" + inFront + (elapsed > 0 ? "|" : "-") + inBack + "]";
			}

			//iTunes version
			BSTR version;
			iITunes->get_Version(&version);
			if(version != NULL) {
				::COLE2T iVersion(version);
				params["version"] = Util::convertFromWide(iVersion.m_szBuffer);
			}

			//Release (decrement reference counter to 0) IiTunes object so it can unload and free itself; otherwise, it's locked in memory
			iITunes->Release();
		}

		//unload COM library -- this is also essential to prevent leaks and to keep it working the next time.
		CoUninitialize();

		// If there is something in title, we have at least partly succeeded 
		if(params["title"] != "") {
			return Util::formatParams(format, params);
		} else {
			return "no_media";
		}
	} else {
		return "";
	}
}

std::string MediaPlayersLib::getMediaPlayerClassicSpam(const std::string& format) {
	StringMap params;
	bool success = false;
	CComPtr<IRunningObjectTable> pROT;
	CComPtr<IEnumMoniker> pEM;
	CComQIPtr<IFilterGraph> pFG;
	if(GetRunningObjectTable(0, &pROT) == S_OK && pROT->EnumRunning(&pEM) == S_OK) {
		CComPtr<IBindCtx> pBindCtx;
		CreateBindCtx(0, &pBindCtx);
		for(CComPtr<IMoniker> pMoniker; pEM->Next(1, &pMoniker, NULL) == S_OK; pMoniker = NULL) {
			LPOLESTR pDispName = NULL;
			if(pMoniker->GetDisplayName(pBindCtx, NULL, &pDispName) != S_OK)
				continue;
			wstring strw(pDispName);
			CComPtr<IMalloc> pMalloc;
			if(CoGetMalloc(1, &pMalloc) != S_OK)
				continue;
			pMalloc->Free(pDispName);
			// Prefix string literals with the L character to indicate a UNCODE string.
			if(strw.find(L"(MPC)") == wstring::npos)
				continue;
			CComPtr<IUnknown> pUnk;
			if(pROT->GetObject(pMoniker, &pUnk) != S_OK)
				continue;
			pFG = pUnk;
			if(!pFG)
				continue;
			success = true;
			break;
		}

		if (success) {
			// file routine (contains size routine)
			CComPtr<IEnumFilters> pEF;
			if(pFG->EnumFilters(&pEF) == S_OK) {
				// from the file routine
				ULONG cFetched = 0;
				for(CComPtr<IBaseFilter> pBF; pEF->Next(1, &pBF, &cFetched) == S_OK; pBF = NULL) {
					if(CComQIPtr<IFileSourceFilter> pFSF = pBF) {
						LPOLESTR pFileName = NULL;
						AM_MEDIA_TYPE mt;
						if(pFSF->GetCurFile(&pFileName, &mt) == S_OK) {
							//wondering if this is good...
							string filename(Util::convertFromWide(pFileName));
							if(!filename.empty()) {
								params["filename"] = Util::getFileName(filename); //otherwise fully qualified
								params["title"] = params["filename"].substr(0, params["filename"].size() - 4);
								params["size"] = Util::formatBytes(Util::getFileSize(Util::convertToWide(filename)));
							}

							CoTaskMemFree(pFileName);
							// alternative to FreeMediaType(mt)
							// provided by MSDN DirectX 9 help page for FreeMediaType
							if (mt.cbFormat != 0)
							{
								CoTaskMemFree((PVOID)mt.pbFormat);
								mt.cbFormat = 0;
								mt.pbFormat = NULL;
							}
							if (mt.pUnk != NULL)
							{
								// Unecessary because pUnk should not be used, but safest.
								mt.pUnk->Release();
								mt.pUnk = NULL;
							}
							// end provided by MSDN
							break;
						}
					}
				}
			}

			// paused / stopped / running?
			CComQIPtr<IMediaControl> pMC;
			OAFilterState fs;
			int state = 0;
			if((pMC = pFG) && (pMC->GetState(0, &fs) == S_OK)) {
				switch(fs) {
					case State_Running:
						params["state"] = "playing";
						state = 1;
						break;
					case State_Paused:
						params["state"] = "paused";
						state = 3;
						break;
					case State_Stopped:
						params["state"] = "stopped";
						state = 0;
				};
			}

			// position routine
			CComQIPtr<IMediaSeeking> pMS = pFG;
			REFERENCE_TIME pos, dur;
			if((pMS->GetCurrentPosition(&pos) == S_OK) && (pMS->GetDuration(&dur) == S_OK)) {
				params["elapsed"] =  Util::formatSeconds(pos/10000000);
				params["length"] =  Util::formatSeconds(dur/10000000);
				int intPercent = 0;
				if (dur != 0)
					intPercent = (int) (pos * 100 / dur);
				params["percent"] = Util::toString(intPercent) + "%";
				int numFront = (int)min(max(intPercent / 10, 0), 10),
					numBack = (int)min(max(10 - 1 - numFront, 0), 10);
				string inFront = string(numFront, '-'),
					   inBack = string(numBack, '-');
				params["bar"] = "[" + inFront + (state ? "|" : "-") + inBack + "]";
			}
		}
	}

	if(success) {
		return Util::formatParams(format, params);
	} else {
		return "";
	}
}