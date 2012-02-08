/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_USER_CONNECTION_H
#define DCPLUSPLUS_DCPP_USER_CONNECTION_H

#include "forward.h"
#include "TimerManager.h"
#include "UserConnectionListener.h"
#include "BufferedSocketListener.h"
#include "BufferedSocket.h"
#include "File.h"
#include "User.h"
#include "AdcCommand.h"
#include "MerkleTree.h"
#include "DebugManager.h"
#include "ClientManager.h"

#include "sdk/interfaces/UserConnection.hpp"

namespace dcpp {

class UserConnection : public Speaker<UserConnectionListener>, 
	private BufferedSocketListener, public Flags, private CommandHandler<UserConnection>,
	private boost::noncopyable, public interfaces::UserConnection
{
public:
	friend class ConnectionManager;
	
	static const string FEATURE_MINISLOTS;
	static const string FEATURE_XML_BZLIST;
	static const string FEATURE_ADCGET;
	static const string FEATURE_ZLIB_GET;
	static const string FEATURE_TTHL;
	static const string FEATURE_TTHF;
	static const string FEATURE_ADC_BAS0;
	static const string FEATURE_ADC_BASE;
	static const string FEATURE_ADC_BZIP;
	static const string FEATURE_ADC_TIGR;

	static const string FILE_NOT_AVAILABLE;
	
	enum Flags {
		FLAG_NMDC					= 0x01,
		FLAG_OP						= 0x02,
		FLAG_UPLOAD					= 0x04,
		FLAG_DOWNLOAD				= 0x08,
		FLAG_INCOMING				= 0x10,
		FLAG_ASSOCIATED				= 0x20,
		FLAG_SUPPORTS_MINISLOTS		= 0x40,
		FLAG_SUPPORTS_XML_BZLIST	= 0x80,
		FLAG_SUPPORTS_ADCGET		= 0x100,
		FLAG_SUPPORTS_ZLIB_GET		= 0x200,
		FLAG_SUPPORTS_TTHL			= 0x400,
		FLAG_SUPPORTS_TTHF			= 0x800,
		FLAG_STEALTH				= 0x1000,
		FLAG_SECURE					= 0x2000
	};
	
	enum States {
		// ConnectionManager
		STATE_UNCONNECTED,
		STATE_CONNECT,

		// Handshake
		STATE_SUPNICK,		// ADC: SUP, Nmdc: $Nick
		STATE_INF,
		STATE_LOCK,
		STATE_DIRECTION,
		STATE_KEY,

		// UploadManager
		STATE_GET,			// Waiting for GET
		STATE_SEND,			// Waiting for $Send

		// DownloadManager
		STATE_SND,	// Waiting for SND
		STATE_IDLE, // No more downloads for the moment

		// Up & down
		STATE_RUNNING,		// Transmitting data

	};
	
	enum SlotTypes {	
		NOSLOT		= 0,
		STDSLOT		= 1,
		EXTRASLOT	= 2,
		PARTIALSLOT	= 3
	};	

	short getNumber() const { return (short)((((size_t)this)>>2) & 0x7fff); }

	// NMDC stuff
	void myNick(const string& aNick) { send("$MyNick " + Text::fromUtf8(aNick, *encoding) + '|'); }
	void lock(const string& aLock, const string& aPk) { send ("$Lock " + aLock + " Pk=" + aPk + '|'); }
	void key(const string& aKey) { send("$Key " + aKey + '|'); }
	void direction(const string& aDirection, int aNumber) { send("$Direction " + aDirection + " " + Util::toString(aNumber) + '|'); }
	void fileLength(const string& aLength) { send("$FileLength " + aLength + '|'); }
	void error(const string& aError) { send("$Error " + aError + '|'); }
	void listLen(const string& aLength) { send("$ListLen " + aLength + '|'); }
	
	void maxedOut(size_t qPos = 0) {
		bool sendPos = qPos > 0;

		if(isSet(FLAG_NMDC)) {
			send("$MaxedOut" + (sendPos ? (" " + Util::toString(qPos)) : Util::emptyString) + "|");
		} else {
			AdcCommand cmd(AdcCommand::SEV_RECOVERABLE, AdcCommand::ERROR_SLOTS_FULL, "Slots full");
			if(sendPos) {
				cmd.addParam("QP", Util::toString(qPos));
			}
			send(cmd);
		}
	}
	
	
	void fileNotAvail(const std::string& msg = FILE_NOT_AVAILABLE) { isSet(FLAG_NMDC) ? send("$Error " + msg + "|") : send(AdcCommand(AdcCommand::SEV_RECOVERABLE, AdcCommand::ERROR_FILE_NOT_AVAILABLE, msg)); }
	void supports(const StringList& feat);
	void getListLen() { send("$GetListLen|"); }

	// ADC Stuff
	void sup(const StringList& features);
	void inf(bool withToken);
	void get(const string& aType, const string& aName, const int64_t aStart, const int64_t aBytes) {  send(AdcCommand(AdcCommand::CMD_GET).addParam(aType).addParam(aName).addParam(Util::toString(aStart)).addParam(Util::toString(aBytes))); }
	void snd(const string& aType, const string& aName, const int64_t aStart, const int64_t aBytes) {  send(AdcCommand(AdcCommand::CMD_SND).addParam(aType).addParam(aName).addParam(Util::toString(aStart)).addParam(Util::toString(aBytes))); }
	void send(const AdcCommand& c) { send(c.toString(0, isSet(FLAG_NMDC))); }

	void setDataMode(int64_t aBytes = -1) { dcassert(socket); socket->setDataMode(aBytes); }
	void setLineMode(size_t rollback) { dcassert(socket); socket->setLineMode(rollback); }

	void connect(const string& aServer, uint16_t aPort, uint16_t localPort, const BufferedSocket::NatRoles natRole) throw(SocketException, ThreadException);
	void accept(const Socket& aServer) throw(SocketException, ThreadException);

	void updated() { if(socket) socket->updated(); }

	void disconnect(bool graceless = false) { if(socket) socket->disconnect(graceless); }
	void transmitFile(InputStream* f) { socket->transmitFile(f); }

	const string& getDirectionString() const {
		dcassert(isSet(FLAG_UPLOAD) ^ isSet(FLAG_DOWNLOAD));
		return isSet(FLAG_UPLOAD) ? UPLOAD : DOWNLOAD;
	}

	const UserPtr& getUser() const { return user; }
	UserPtr& getUser() { return user; }
	const HintedUser getHintedUser() const { return HintedUser(user, hubUrl); }

	bool isSecure() const { return socket && socket->isSecure(); }
	bool isTrusted() const { return socket && socket->isTrusted(); }
	std::string getCipherName() const { return socket ? socket->getCipherName() : Util::emptyString; }

	const string& getRemoteIp() const { if(socket) return socket->getIp(); else return Util::emptyString; }
	Download* getDownload() { dcassert(isSet(FLAG_DOWNLOAD)); return download; }
	uint16_t getPort() const { if(socket) return socket->getPort(); else return 0; }
	void setDownload(Download* d) { dcassert(isSet(FLAG_DOWNLOAD)); download = d; }
	Upload* getUpload() { dcassert(isSet(FLAG_UPLOAD)); return upload; }
	void setUpload(Upload* u) { dcassert(isSet(FLAG_UPLOAD)); upload = u; }
	
	void handle(AdcCommand::SUP t, const AdcCommand& c) { fire(t, this, c); }
	void handle(AdcCommand::INF t, const AdcCommand& c) { fire(t, this, c); }
	void handle(AdcCommand::GET t, const AdcCommand& c) { fire(t, this, c); }
	void handle(AdcCommand::SND t, const AdcCommand& c) { fire(t, this, c);	}
	void handle(AdcCommand::STA t, const AdcCommand& c) { fire(t, this, c);	}
	void handle(AdcCommand::RES t, const AdcCommand& c) { fire(t, this, c); }
	void handle(AdcCommand::GFI t, const AdcCommand& c) { fire(t, this, c);	}

	// Ignore any other ADC commands for now
	//template<typename T> void handle(T , const AdcCommand& ) { }
	//RSX++ or pass it to plugins :)
	template<typename T> void handle(T , const AdcCommand& cmd) {
		plugins.handleAdcCommand(this, cmd);
	}
	//END

	int64_t getChunkSize() const { return chunkSize; }
	void updateChunkSize(int64_t leafSize, int64_t lastChunk, uint64_t ticks);
	
	GETSET(string, hubUrl, HubUrl);
	GETSET(string, token, Token);
	GETSET(int64_t, speed, Speed);
	GETSET(uint64_t, lastActivity, LastActivity);
	GETSET(States, state, State);

	GETSET(string*, encoding, Encoding);
	GETSET(uint8_t, slotType, SlotType);
	
	BufferedSocket const* getSocket() { return socket; } 

private:
	int64_t chunkSize;
	BufferedSocket* socket;
	UserPtr user;

	static const string UPLOAD, DOWNLOAD;
	
	union {
		Download* download;
		Upload* upload;
	};
	/* RSX++ - moved to .cpp
	// We only want ConnectionManager to create this...
	UserConnection(bool secure_) throw() : encoding(const_cast<string*>(&Text::systemCharset)), state(STATE_UNCONNECTED),
		lastActivity(0), speed(0), chunkSize(0), socket(0), download(NULL), slotType(NOSLOT) {
		if(secure_) {
			setFlag(FLAG_SECURE);
		}
	}

	~UserConnection() throw() {
		BufferedSocket::putSocket(socket);
		dcassert(!download);
	}
	*/
	UserConnection(bool secure_) throw();
	~UserConnection() throw();
	//END

	friend struct DeleteFunction;
	//RSX++
	class ProxyListener : public UserConnectionListener {
	public:
		ProxyListener(CriticalSection& _cs) : cs(_cs) { }

		void addListener(interfaces::UserConnectionListener* listener) {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				if(*i == listener)
					return;
			}
			ls.push_back(listener);
		}

		void remListener(interfaces::UserConnectionListener* listener) {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				if(*i == listener) {
					ls.erase(i);
					break;
				}
			}
		}

	private:
		friend class UserConnection;
		typedef std::deque<interfaces::UserConnectionListener*> Listeners;

		template<bool incoming>
		bool handleLine(UserConnection* uc, const char* line) {
			bool handled = false;
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				if(incoming) {
					(*i)->onUserConnection_IncomingLine(uc, line, handled);
				} else {
					(*i)->onUserConnection_OutgoingLine(uc, line, handled);
				}
			}
			return handled;
		}

		Listeners ls;
		CriticalSection& cs;

		void on(UserConnectionListener::BytesSent, UserConnection* uc, size_t n1, size_t n2) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_BytesSent(uc, n1, n2);
			}
		}
		void on(UserConnectionListener::Connected, UserConnection* uc) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_Connected(uc);
			}
		}
		void on(UserConnectionListener::Data, UserConnection* uc, const uint8_t* data, size_t len) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_Data(uc, data, len);
			}
		}
		void on(UserConnectionListener::Failed, UserConnection* uc, const string& r) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_Failed(uc, r.c_str());
			}
		}
		void on(UserConnectionListener::CLock, UserConnection* uc, const string& r, const string& s) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_CLock(uc, r.c_str(), s.c_str());
			}
		}
		void on(UserConnectionListener::Key, UserConnection* uc, const string& key) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_Key(uc, key.c_str());
			}
		}
		void on(UserConnectionListener::Direction, UserConnection* uc, const string& r, const string& s) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_Direction(uc, r.c_str(), s.c_str());
			}
		}
		void on(UserConnectionListener::Get, UserConnection* uc, const string& s, int64_t n) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_Get(uc, s.c_str(), n);
			}
		}
		void on(UserConnectionListener::Send, UserConnection* uc) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_Send(uc);
			}
		}
		/*void on(UserConnectionListener::GetListLength, UserConnection* uc) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
			
			}
		}*/
		void on(UserConnectionListener::MaxedOut, UserConnection* uc, string param = Util::emptyString) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_MaxedOut(uc, param.empty() ? 0 : param.c_str());
			}
		}
		void on(UserConnectionListener::ModeChange, UserConnection* uc) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_ModeChange(uc);
			}
		}
		void on(UserConnectionListener::MyNick, UserConnection* uc, const string& nick) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_MyNick(uc, nick.c_str());
			}
		}
		void on(UserConnectionListener::TransmitDone, UserConnection* uc) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_TransmitDone(uc);
			}
		}
		void on(UserConnectionListener::Supports, UserConnection* uc, const StringList& list) throw() {
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_Supports(uc, 0); //todo
			}
		}
		void on(UserConnectionListener::FileNotAvailable, UserConnection* uc) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_FileNotAvailable(uc);
			}
		}
		void on(UserConnectionListener::Updated, UserConnection* uc) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {
				(*i)->onUserConnection_Updated(uc);
			}
		}
		/*void on(UserConnectionListener::ListLength, UserConnection* uc, const string&) throw() { 
			Lock l(cs);
			for(Listeners::iterator i = ls.begin(); i != ls.end(); ++i) {

			}
		}*/

		void on(AdcCommand::SUP, UserConnection* uc, const AdcCommand& cmd) throw() { 
			handleAdcCommand(uc, cmd);
		}
		void on(AdcCommand::INF, UserConnection* uc, const AdcCommand& cmd) throw() { 
			handleAdcCommand(uc, cmd);
		}
		void on(AdcCommand::GET, UserConnection* uc, const AdcCommand& cmd) throw() { 
			handleAdcCommand(uc, cmd);
		}
		void on(AdcCommand::SND, UserConnection* uc, const AdcCommand& cmd) throw() { 
			handleAdcCommand(uc, cmd);
		}
		void on(AdcCommand::STA, UserConnection* uc, const AdcCommand& cmd) throw() { 
			handleAdcCommand(uc, cmd);
		}
		void on(AdcCommand::RES, UserConnection* uc, const AdcCommand& cmd) throw() { 
			handleAdcCommand(uc, cmd);
		}
		void on(AdcCommand::GFI, UserConnection* uc, const AdcCommand& cmd) throw() { 
			handleAdcCommand(uc, cmd);
		}

		void handleAdcCommand(UserConnection* uc, const AdcCommand& cmd) throw();
	}plugins;

	void sendData(const void* data, size_t len) { send(std::string((const char*)data, len), false); }
	void sendLine(const char* line) { send(line, false); }

	void addEventListener(interfaces::UserConnectionListener* listener) {
		plugins.addListener(listener);
	}
	void remEventListener(interfaces::UserConnectionListener* listener) {
		plugins.remListener(listener);
	}

	void setFlags(int flags) { setFlag(flags); }
	bool isFlagSet(int flag) { return isSet(flag); }
	bool isAnyFlagSet(int flags) { return isAnySet(flags); }
	const char* getIp() const { return getRemoteIp().c_str(); }
	const char* getHubURL() const { return hubUrl.c_str(); }
	void parseLine(const char* line) { onLine(line, true); }
	//END

	void setUser(const UserPtr& aUser) {
		user = aUser;
	}

	void onLine(const string& aLine, bool dropPlugins = false) throw();

	void send(const string& aString, bool ext = true) {
		lastActivity = GET_TICK();
		COMMAND_DEBUG(aString, DebugManager::CLIENT_OUT, getRemoteIp());
		//RSX++
		if(ext && plugins.handleLine<false>(this, aString.c_str())) 
			return;
		//END
		socket->write(aString);
	}

	void on(Connected) throw();
	void on(Line, const string&) throw();
	void on(Data, uint8_t* data, size_t len) throw();
	void on(BytesSent, size_t bytes, size_t actual) throw() ;
	void on(ModeChange) throw();
	void on(TransmitDone) throw();
	void on(Failed, const string&) throw();
	void on(Updated) throw();
};

} // namespace dcpp

#endif // !defined(USER_CONNECTION_H)

/**
 * @file
 * $Id: UserConnection.h 482 2010-02-13 10:49:30Z bigmuscle $
 */
