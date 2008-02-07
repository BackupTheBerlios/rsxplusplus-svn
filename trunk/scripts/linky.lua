--[[
	Name: 		Linky
	Author: 		adrian_007(c)
	Date: 		09/09/2007
	Description: 	Lua script for searchng for given param on defined websites.
	
	Written for RSX++
]]--

dcpp:setListener( "ownChatOut", "linky", function(hub, msg)
	if string.sub(msg, 1, 3) == "/g " then
		local link = "http://www.google.com/search?q=" .. DC():DecodeURI(string.sub(msg, 4))
		DC():OpenLink(DC():ToUtf8(link))
		return 1
	elseif string.sub(msg, 1, 6) == "/imdb " then
		local link = "http://www.imdb.com/find?q=" .. DC():DecodeURI(string.sub(msg, 7))
		DC():OpenLink(DC():ToUtf8(link))
		return 1
	elseif string.sub(msg, 1, 3) == "/y " then
		local link = "http://youtube.com/results?search_query=" .. DC():DecodeURI(string.sub(msg, 4))
		DC():OpenLink(DC():ToUtf8(link))
		return 1
	elseif string.sub(msg, 1, 6) == "/wiki " then
		local link = "http://en.wikipedia.org/wiki/" .. DC():DecodeURI(string.sub(msg, 7))
		DC():OpenLink(DC():ToUtf8(link))
		return 1
	elseif string.sub(msg, 1, 3) == "/u " then
		local link = DC():DecodeURI(string.sub(msg, 4))
		DC():OpenLink(DC():ToUtf8(link))
		return 1
	elseif msg == "/help" then
		local helpMsg = "linky.lua\n" ..
						"\t/g\t\t\t\tsearch in google.com\n"..
						"\t/imdb\t\t\t\tsearch film from IMDB database\n"..
						"\t/y\t\t\t\tsearch in youtube.com\n"..
						"\t/wiki\t\t\t\tsearch in wikipedia (en)\n"..
						"\t/u\t\t\t\tgo to given URL (using default web browser)"
		hub:addLine(helpMsg)
		return 1
	end
end)

DC():PrintDebug( "  ** Loaded linky.lua **" )
