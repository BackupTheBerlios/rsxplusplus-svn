--// quiet_login.lua -- stop spamming logfiles with MOTDs

if not dcpp._chat_ready then
	dcpp._chat_ready = {}
end	

dcpp:setListener( "userMyInfo", "quiet_login",
	function( hub, user, message )
		if hub:getOwnNick() == user:getNick() then
			dcpp._chat_ready[hub]._ready = 1
		end
	end
)

dcpp:setListener( "chat", "quiet_login",
	function( hub, user, text )
		if not dcpp._chat_ready[hub]._ready then
			dcpp._chat_ready[hub]._text = dcpp._chat_ready[hub]._text ..
			                              text .. "\n"
			return 1
		end
	end							
)

dcpp:setListener( "connected", "quiet_login",
	function( hub )
		dcpp._chat_ready[hub] = { _ready = nil, _text = "" }
	end
)

-- However, if login failed, best know why.
dcpp:setListener( "disconnected", "quiet_login",
	function( hub )
		-- If disconnected prematurely, show archived messages.
		-- They probably include error messages.
		if not dcpp._chat_ready[hub]._ready then
			DC():PrintDebug(dcpp._chat_ready[hub]._text)
		end

		dcpp._chat_ready[hub] = nil
	end
)

DC():PrintDebug( "  ** Loaded quiet_login.lua **" )
