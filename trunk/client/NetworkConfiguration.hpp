/*
 * Copyright (C) 2007-2009 adrian_007, adrian-007 on o2 point pl
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

#ifndef NETWORK_CONFIGURATION_HPP
#define NETWORK_CONFIGURATION_HPP

#include "Singleton.h"
#include "Speaker.h"
#include "TimerManager.h"

namespace dcpp {
class Socket;
class SocketException;

class NetworkConfiguration : public Singleton<NetworkConfiguration>, public TimerManagerListener {
public:
	NetworkConfiguration();
	~NetworkConfiguration();

	void initPorts() { setPorts(); }
	bool openPorts();
	void closePorts();
	bool restart();
	std::string getIp() const { return ip; }
	bool isOk() const { return ok; }
private:
	void checkState();
	void createSocket(Socket& s);
	bool requestPorts(Socket& s, bool close);

	void fillRequest(char buf[12], uint16_t port, bool tcp, uint32_t lifetime);
	std::string toIP(uint32_t aIp) const;

	uint8_t count;
	uint16_t tcpPort;
	uint16_t udpPort;
	uint16_t tlsPort;
	std::string ip;

	friend class Singleton<NetworkConfiguration>;

	void on(TimerManagerListener::Minute, uint64_t tick);
	void setPorts(bool flush = false);

	bool ok;
};
}

#endif
/**
 * @file
 * $Id$
 */
