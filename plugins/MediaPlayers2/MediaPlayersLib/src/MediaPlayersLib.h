#ifndef RSXPLUSPLUS_MEDIA_PLAYERS_LIB
#define RSXPLUSPLUS_MEDIA_PLAYERS_LIB

class MediaPlayersLib {
public:
	static std::string __cdecl getWinampSpam(const std::string& format);
	static std::string __cdecl getWindowsMediaPlayerSpam(const std::string& format, HWND parent);
	static std::string __cdecl get_iTunesSpam(const std::string& format);
	static std::string __cdecl getMediaPlayerClassicSpam(const std::string& format);
};
#endif