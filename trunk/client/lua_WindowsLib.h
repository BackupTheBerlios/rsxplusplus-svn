#ifndef RSXPLUSPLUS_LUA_WINDOWS_LIB
#define RSXPLUSPLUS_LUA_WINDOWS_LIB

namespace dcpp {
namespace lua_winlib {
	const char* libname = "windows";

	// help functions
	static UINT str2wmmsg(const string& str) {
		//what a nice short func...
			   if(str == "WM_COMMAND") { return WM_COMMAND;
		} else if(str == "WM_NOTIFY") { return WM_NOTIFY;
		} else if(str == "WM_INPUTLANGCHANGEREQUEST") { return WM_INPUTLANGCHANGEREQUEST;
		} else if(str == "WM_INPUTLANGCHANGE") { return WM_INPUTLANGCHANGE;
		} else if(str == "WM_TCARD") { return WM_TCARD;
		} else if(str == "WM_HELP") { return WM_HELP;
		} else if(str == "WM_USERCHANGED") { return WM_USERCHANGED;
		} else if(str == "WM_NOTIFYFORMAT") { return WM_NOTIFYFORMAT;
		} else if(str == "WM_CONTEXTMENU") { return WM_CONTEXTMENU;
		} else if(str == "WM_STYLECHANGING") { return WM_STYLECHANGING;
		} else if(str == "WM_STYLECHANGED") { return WM_STYLECHANGED;
		} else if(str == "WM_DISPLAYCHANGE") { return WM_DISPLAYCHANGE;
		} else if(str == "WM_GETICON") { return WM_GETICON;
		} else if(str == "WM_SETICON") { return WM_SETICON;
		} else if(str == "WM_NCCREATE") { return WM_NCCREATE;
		} else if(str == "WM_NCDESTROY") { return WM_NCDESTROY;
		} else if(str == "WM_NCCALCSIZE") { return WM_NCCALCSIZE;
		} else if(str == "WM_NCHITTEST") { return WM_NCHITTEST;
		} else if(str == "WM_NCPAINT") { return WM_NCPAINT;
		} else if(str == "WM_NCACTIVATE") { return WM_NCACTIVATE;
		} else if(str == "WM_GETDLGCODE") { return WM_GETDLGCODE;
		} else if(str == "WM_NCMOUSEMOVE") { return WM_NCMOUSEMOVE;
		} else if(str == "WM_NCLBUTTONDOWN") { return WM_NCLBUTTONDOWN;
		} else if(str == "WM_NCLBUTTONUP") { return WM_NCLBUTTONUP;
		} else if(str == "WM_NCLBUTTONDBLCLK") { return WM_NCLBUTTONDBLCLK;
		} else if(str == "WM_NCRBUTTONDOWN") { return WM_NCRBUTTONDOWN;
		} else if(str == "WM_NCRBUTTONUP") { return WM_NCRBUTTONUP;
		} else if(str == "WM_NCRBUTTONDBLCLK") { return WM_NCRBUTTONDBLCLK;
		} else if(str == "WM_NCMBUTTONDOWN") { return WM_NCMBUTTONDOWN;
		} else if(str == "WM_NCMBUTTONUP") { return WM_NCMBUTTONUP;
		} else if(str == "WM_NCMBUTTONDBLCLK") { return WM_NCMBUTTONDBLCLK;
		} else if(str == "WM_NCXBUTTONDOWN") { return WM_NCXBUTTONDOWN;
		} else if(str == "WM_NCXBUTTONUP") { return WM_NCXBUTTONUP;
		} else if(str == "WM_NCXBUTTONDBLCLK") { return WM_NCXBUTTONDBLCLK;
		} else if(str == "WM_KEYFIRST") { return WM_KEYFIRST;
		} else if(str == "WM_KEYDOWN") { return WM_KEYDOWN;
		} else if(str == "WM_KEYUP") { return WM_KEYUP;
		} else if(str == "WM_CHAR") { return WM_CHAR;
		} else if(str == "WM_DEADCHAR") { return WM_DEADCHAR;
		} else if(str == "WM_SYSKEYDOWN") { return WM_SYSKEYDOWN;
		} else if(str == "WM_SYSKEYUP") { return WM_SYSKEYUP;
		} else if(str == "WM_SYSCHAR") { return WM_SYSCHAR;
		} else if(str == "WM_SYSDEADCHAR") { return WM_SYSDEADCHAR;
		} else if(str == "WM_KEYLAST") { return WM_KEYLAST;
		} else if(str == "WM_IME_STARTCOMPOSITION") { return WM_IME_STARTCOMPOSITION;
		} else if(str == "WM_IME_ENDCOMPOSITION") { return WM_IME_ENDCOMPOSITION;
		} else if(str == "WM_IME_COMPOSITION") { return WM_IME_COMPOSITION;
		} else if(str == "WM_IME_KEYLAST") { return WM_IME_KEYLAST;
		} else if(str == "WM_INITDIALOG") { return WM_INITDIALOG;
		} else if(str == "WM_SYSCOMMAND") { return WM_SYSCOMMAND;
		} else if(str == "WM_TIMER") { return WM_TIMER;
		} else if(str == "WM_HSCROLL") { return WM_HSCROLL;
		} else if(str == "WM_VSCROLL") { return WM_VSCROLL;
		} else if(str == "WM_INITMENU") { return WM_INITMENU;
		} else if(str == "WM_INITMENUPOPUP") { return WM_INITMENUPOPUP;
		} else if(str == "WM_MENUSELECT") { return WM_MENUSELECT;
		} else if(str == "WM_MENUCHAR") { return WM_MENUCHAR;
		} else if(str == "WM_ENTERIDLE") { return WM_ENTERIDLE;
		} else if(str == "WM_MENURBUTTONUP") { return WM_MENURBUTTONUP;
		} else if(str == "WM_MENUDRAG") { return WM_MENUDRAG;
		} else if(str == "WM_MENUGETOBJECT") { return WM_MENUGETOBJECT;
		} else if(str == "WM_UNINITMENUPOPUP") { return WM_UNINITMENUPOPUP;
		} else if(str == "WM_MENUCOMMAND") { return WM_MENUCOMMAND;
		} else if(str == "WM_CHANGEUISTATE") { return WM_CHANGEUISTATE;
		} else if(str == "WM_UPDATEUISTATE") { return WM_UPDATEUISTATE;
		} else if(str == "WM_QUERYUISTATE") {return WM_QUERYUISTATE;
		} else {
			return 0;
		}
	}
	static int messageBox(lua_State *L) {
		if(lua_gettop(L) == 2 && lua_isstring(L, -1) && lua_isstring(L, -2)) {
			const tstring& msg = Text::toT(lua_tostring(L, -1));
			const tstring& title = Text::toT(lua_tostring(L, -2));
			::MessageBox(0, msg.c_str(), title.c_str(), 0);
			return 1;
		}
		return 0;
	}
	static int findWindow(lua_State* L) {
		if(lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)) {
			const tstring& s1 = Text::toT(lua_tostring(L, -2));
			const tstring& s2 = Text::toT(lua_tostring(L, -1));
			lua_pushlightuserdata(L, ::FindWindow(s1.empty() ? NULL : s1.c_str(), s2.empty() ? NULL : s2.c_str()));
			return 1;
		}
		return 0;
	}
	template<typename T>
	static T convToType(lua_State* L, int pos) {
		if(lua_isnumber(L, pos)) {
			return static_cast<T>(lua_tonumber(L, pos));
		} else if(lua_isstring(L, pos)) {
			return reinterpret_cast<T>(lua_tostring(L, pos));
		} else if(lua_islightuserdata(L, pos)) {
			return reinterpret_cast<T>(lua_touserdata(L, pos));
		} else if(lua_isuserdata(L, pos)) {
			return reinterpret_cast<T>(lua_touserdata(L, pos));
		} else {
			return 0;
		}
	}
	static int wndMessage(lua_State* L, bool send) {
		if (lua_gettop(L) == 4 && lua_islightuserdata(L, -4)) {
			UINT aCmd = NULL;
			WPARAM wParam = NULL;
			LPARAM lParam = NULL;
			if(lua_isnumber(L, -3)) {
				aCmd = static_cast<UINT>(lua_tonumber(L, -3));
			} else if(lua_isstring(L, -3)) {
				aCmd = str2wmmsg(lua_tostring(L, -3));
			}

			wParam = convToType<WPARAM>(L, -2);
			lParam = convToType<LPARAM>(L, -1);

			if(send)
				lua_pushlightuserdata(L, (void*)::SendMessage(reinterpret_cast<HWND>(lua_touserdata(L, -4)), aCmd, wParam, lParam));
			else
				lua_pushlightuserdata(L, (void*)::PostMessage(reinterpret_cast<HWND>(lua_touserdata(L, -4)), aCmd, wParam, lParam));
			return 1;
		}
		return 0;
	}
	static int sendMessage(lua_State* L) {
		return wndMessage(L, true);
	}
	static int postMessage(lua_State* L) {
		return wndMessage(L, false);
	}
	static int isWindow(lua_State* L) {
		if(lua_gettop(L) == 1) {
			if(lua_islightuserdata(L, -1))
				return ::IsWindow(reinterpret_cast<HWND>(lua_touserdata(L, -1)));
		}
		return 0;
	}
	static int getWmMessage(lua_State* L) {
		if(lua_gettop(L) == 1 && lua_isstring(L, -1)) {
			return str2wmmsg(lua_tostring(L, -1));
		}
		return 0;
	}
	static const luaL_Reg lib[] = {
		{"MessageBox", messageBox},
		{"FindWindow", findWindow},
		{"SendMessage", sendMessage},
		{"PostMessage", postMessage},
		{"IsWindow", isWindow},
		{"GetWMMessageNum", getWmMessage},
		{NULL, NULL}
	};
} // namespace lua_winlib
} // namespace dcpp
#endif