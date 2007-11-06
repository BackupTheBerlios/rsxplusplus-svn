--[[ 
	Name: 		Players
	Author: 		adrian_007(c)
	Date: 		02/09/2007
	Description: 	Lua script for controlling media players (Winamp and Windows Media Player) by commands..
	
	Written for RSX++
	Note: This is only an example of using Win functions FindWindow and SendMessage!
]]--

dcpp:setListener("ownChatOut", "players", function(hub, text)
	--// 273 = WM_COMMAND
	local cmd = string.lower(text)
	local hwndWinamp = 	DC():FindWindowHandle("Winamp v1.x", 0)
	local hwndWmp = 	DC():FindWindowHandle("WMPlayerApp", 0)

	if cmd == "/mp p" then
		if DC():IsWindow(hwndWinamp) then
			DC():SendWindowMessageEx(hwndWinamp, 273, 40046, 0)
			hub:addLine("play/pause")
		elseif DC():IsWindow(hwndWmp) then
			DC():SendWindowMessageEx(hwndWmp, 273, 18808, 0)
			hub:addLine("play/pause")
		end
		return 1
	elseif cmd == "/mp s" or cmd == "/mp stop" then
		if DC():IsWindow(hwndWinamp) then
			DC():SendWindowMessageEx(hwndWinamp, 273, 40047, 0)
			hub:addLine("stopped")
		elseif DC():IsWindow(hwndWmp) then
			DC():SendWindowMessageEx(hwndWmp, 273, 18809, 0)
			hub:addLine("stopped")
		end
		return 1
	elseif cmd == "/mp b" or cmd == "/mp back" then
		if DC():IsWindow(hwndWinamp) then
			DC():SendWindowMessageEx(hwndWinamp, 273, 40044, 0)
			hub:addLine("back")
		elseif DC():IsWindow(hwndWmp) then
			DC():SendWindowMessageEx(hwndWmp, 273, 18810, 0)
			hub:addLine("back")
		end
		return 1
	elseif cmd == "/mp n" or cmd== "/mp next" then
		if DC():IsWindow(hwndWinamp) then
			DC():SendWindowMessageEx(hwndWinamp, 273, 40048, 0)
			hub:addLine("next")
		elseif DC():IsWindow(hwndWmp) then
			DC():SendWindowMessageEx(hwndWmp, 273, 18811, 0)
			hub:addLine("next")
		end
		return 1
	elseif cmd == "/mp pl" or cmd == "/mp play" then
		if DC():IsWindow(hwndWinamp) then
			DC():SendWindowMessageEx(hwndWinamp, 273, 40045, 0)
			hub:addLine("play")
		end
		return 1
	elseif cmd == "/mp up" then
		if DC():IsWindow(hwndWinamp) then
			DC():SendWindowMessageEx(hwndWinamp, 273, 40058, 0)
			hub:addLine("volume up")
		elseif DC():IsWindow(hwndWmp) then
			DC():SendWindowMessageEx(hwndWmp, 273, 18815, 0)
			hub:addLine("volume up")
		end
		return 1
	elseif cmd == "/mp down" then
		if DC():IsWindow(hwndWinamp) then
			DC():SendWindowMessageEx(hwndWinamp, 273, 40059, 0)
			hub:addLine("volume down")
		elseif DC():IsWindow(hwndWmp) then
			DC():SendWindowMessageEx(hwndWmp, 273, 18816, 0)
			hub:addLine("volume down")
		end
		return 1
	elseif cmd == "/help" then
		local tmp = "players.lua\n" ..
					"\tby this script you can controll Winamp and Windows Media Player\n"..
					"\tcommands:\n"..
					"\t/mp p				play/pause\n"..
					"\t/mp pl(play)			play\n"..
					"\t/mp s(stop)			stop\n"..
					"\t/mp b(back)			back\n"..
					"\t/mp n(next)			next\n"..
					"\t/mp up				volume up\n"..
					"\t/mp down			volume down\n"
		hub:addLine(tmp)
		return 1
	end
end)

DC():PrintDebug( "  ** Loaded players.lua **" )