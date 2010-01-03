/*
 * Copyright (C) 2007-2010 adrian_007, adrian-007 on o2 point pl
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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Socket.h"
#include "SettingsManager.h"
#include "NetworkConfiguration.hpp"

namespace dcpp {
const int natpmp_version = 0;
const int natpmp_lifetime = 3600;
const int natpmp_deadtime = 0;

NetworkConfiguration::NetworkConfiguration() : count(60), ok(false), ip(Util::emptyString), randDone(false) {
	TimerManager::getInstance()->addListener(this);
}

NetworkConfiguration::~NetworkConfiguration() {
	TimerManager::getInstance()->removeListener(this);
}

void NetworkConfiguration::fillRequest(char buf[12], uint16_t port, bool tcp, uint32_t lifetime /*= 3720*/) {
	memzero(buf, 12);
	buf[0] = natpmp_version;
	buf[1] = tcp ? 2 : 1;
	buf[4] = buf[6] = (char)(port >> 8);
	buf[5] = buf[7] = (char)port;
	buf[8] = (char)(lifetime >> 24);
	buf[9] = (char)(lifetime >> 16);
	buf[10] = (char)(lifetime >> 8);
	buf[11] = (char)lifetime;
}

void NetworkConfiguration::checkState() {
	if(!ok) {
		closePorts();
	}
}

void NetworkConfiguration::createSocket(Socket& s) {
	s.create(Socket::TYPE_UDP);
	s.bind(5351, SETTING(BIND_ADDRESS));
	s.connect(SETTING(NAT_PMP_GATEWAY), 5351);
}

bool NetworkConfiguration::openPorts() {
	if(SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_NAT_PMP) {
		try {
			Socket s;
			createSocket(s);
			ok = requestPorts(s, false);
		} catch(const SocketException& e) {
			ok = false;
			dcdebug("NAT-PMP::SocketException %s\n", e.getError().c_str());
		}
		checkState();
	}
	return ok;
}

void NetworkConfiguration::closePorts() {
	try {
		Socket s;
		createSocket(s);
		requestPorts(s, true);
	} catch(const SocketException& e) {
		dcdebug("NAT-PMP::SocketException %s\n", e.getError().c_str());
	}
	ok = false;
}

bool NetworkConfiguration::restart() {
	count = 60;
	if(SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_NAT_PMP) {
		setPorts(true);
		try {
			Socket s;
			createSocket(s);
			requestPorts(s, true);
			ok = requestPorts(s, false);
		} catch(const SocketException& e) {
			ok = false;
			dcdebug("NAT-PMP::SocketException %s\n", e.getError().c_str());
		}
		checkState();
	} else {
		closePorts();
	}
	return ok;
}

bool NetworkConfiguration::requestPorts(Socket& s, bool close) {
	char buf[12];
	char rcv[16];

	bool tcp = false;
	bool udp = false;
	bool tls = false;
	bool dht = false;
	{
		fillRequest(buf, tcpPort, true, close ? natpmp_deadtime : natpmp_lifetime);
		s.writeAll(buf, 12, 250);

		memzero(rcv, 16);
		int len = s.readAll(rcv, 16, 250);
		if(len == 16) {
			uint16_t resCode = (uint16_t)(rcv[2] << 8) + (uint16_t)rcv[3];
			if(rcv[1] == 130 && resCode == 0) {
				uint16_t externalPort = (uint16_t)(rcv[10] << 8) + (uint16_t)rcv[11];
				tcp = externalPort == tcpPort;
#ifdef _DEBUG
				uint32_t elapsed = (uint32_t)(rcv[12] << 24) + (uint32_t)(rcv[13] << 16) + (uint32_t)(rcv[14] << 8) + (uint32_t)rcv[15];
				if(externalPort > 0 && elapsed > 0)
					dcdebug("NAT-PMP::IN Mapped TCP port: %i for %i seconds\n", externalPort, elapsed);
				else
					dcdebug("NAT-PMP::IN Closed TCP port: %i\n", tcpPort);
#endif
			}
		}
	}
	{
		fillRequest(buf, udpPort, false, close ? natpmp_deadtime : natpmp_lifetime);
		s.writeAll(buf, 12, 250);

		memzero(rcv, 16);
		int len = s.readAll(rcv, 16, 250);
		if(len == 16) {
			uint16_t resCode = (uint16_t)(rcv[2] << 8) + (uint16_t)rcv[3];
			if(rcv[1] == 129 && resCode == 0) {
				uint16_t externalPort = (uint16_t)(rcv[10] << 8) + (uint16_t)rcv[11];
				udp = externalPort == udpPort;
#ifdef _DEBUG
				uint32_t elapsed = (uint32_t)(rcv[12] << 24) + (uint32_t)(rcv[13] << 16) + (uint32_t)(rcv[14] << 8) + (uint32_t)rcv[15];
				if(externalPort > 0 && elapsed > 0)
					dcdebug("NAT-PMP::IN Mapped UDP port: %i for %i seconds\n", externalPort, elapsed);
				else
					dcdebug("NAT-PMP::IN Closed UDP port: %i\n", udpPort);
#endif
			}
		}
	}
	{
		fillRequest(buf, tlsPort, true, close ? natpmp_deadtime : natpmp_lifetime);
		s.writeAll(buf, 12, 250);

		memzero(rcv, 16);
		int len = s.readAll(rcv, 16, 250);
		if(len == 16) {
			uint16_t resCode = (uint16_t)(rcv[2] << 8) + (uint16_t)rcv[3];
			if(rcv[1] == 130 && resCode == 0) {
				uint16_t externalPort = (uint16_t)(rcv[10] << 8) + (uint16_t)rcv[11];
				tls = externalPort == tlsPort;
#ifdef _DEBUG
				uint32_t elapsed = (uint32_t)(rcv[12] << 24) + (uint32_t)(rcv[13] << 16) + (uint32_t)(rcv[14] << 8) + (uint32_t)rcv[15];
				if(externalPort > 0 && elapsed > 0)
					dcdebug("NAT-PMP::IN Mapped TLS port: %i for %i seconds\n", externalPort, elapsed);
				else
					dcdebug("NAT-PMP::IN Closed TLS port: %i\n", tlsPort);
#endif
			}
		}
	}
	{
		fillRequest(buf, dhtPort, false, close ? natpmp_deadtime : natpmp_lifetime);
		s.writeAll(buf, 12, 250);

		memzero(rcv, 16);
		int len = s.readAll(rcv, 16, 250);
		if(len == 16) {
			uint16_t resCode = (uint16_t)(rcv[2] << 8) + (uint16_t)rcv[3];
			if(rcv[1] == 129 && resCode == 0) {
				uint16_t externalPort = (uint16_t)(rcv[10] << 8) + (uint16_t)rcv[11];
				dht = externalPort == udpPort;
#ifdef _DEBUG
				uint32_t elapsed = (uint32_t)(rcv[12] << 24) + (uint32_t)(rcv[13] << 16) + (uint32_t)(rcv[14] << 8) + (uint32_t)rcv[15];
				if(externalPort > 0 && elapsed > 0)
					dcdebug("NAT-PMP::IN Mapped DHT port: %i for %i seconds\n", externalPort, elapsed);
				else
					dcdebug("NAT-PMP::IN Closed DHT port: %i\n", udpPort);
#endif
			}
		}
	}
	{
		memzero(buf, 12);
		s.writeAll(buf, 2, 250);

		memzero(rcv, 16);
		int len = s.readAll(rcv, 12, 250);
		uint16_t resCode = (uint16_t)(rcv[2] << 8) + (uint16_t)rcv[3];
		if(len == 12 && resCode == 0) {
			if(rcv[1] == 128) {
				uint32_t ipnum = (uint32_t)(rcv[8] << 24) + (uint32_t)(rcv[9] << 16) + (uint32_t)(rcv[10] << 8) + (uint32_t)rcv[11];
				string aIp = toIP(ipnum);
				dcdebug("NAT-PMP::IP %s\n", aIp.c_str());
				if(!Util::isPrivateIp(aIp)) {
					ip = aIp;
				} else {
					ip = Util::emptyString;
				}
			}
		}
	}
	return tcp && udp && tls && dht;
}

void NetworkConfiguration::setPorts(bool flush /*= false*/) {
	if(SETTING(INCOMING_CONNECTIONS) != SettingsManager::INCOMING_FIREWALL_NAT_PMP)
		return;
	if(SETTING(FIREWALL_RAND_PORTS)) {
		if(!randDone || flush) { 
			uint16_t tmp = static_cast<uint16_t>(Util::rand(1000, 65535));
			tcpPort = tmp;

			tmp = static_cast<uint16_t>(Util::rand(1000, 65535));
			while(tmp == tcpPort)
				tmp = static_cast<uint16_t>(Util::rand(1000, 65535));
			udpPort = tmp;

			tmp = static_cast<uint16_t>(Util::rand(1000, 65535));
			while(tmp == tcpPort || tmp == udpPort)
				tmp = static_cast<uint16_t>(Util::rand(1000, 65535));
			tlsPort = tmp;

			tmp = static_cast<uint16_t>(Util::rand(1000, 65535));
			while(tmp == tcpPort || tmp == udpPort || tmp == tlsPort)
				tmp = static_cast<uint16_t>(Util::rand(1000, 65535));
			dhtPort = tmp;
		}
		SettingsManager::getInstance()->set(SettingsManager::TCP_PORT, tcpPort);
		SettingsManager::getInstance()->set(SettingsManager::UDP_PORT, udpPort);
		SettingsManager::getInstance()->set(SettingsManager::TLS_PORT, tlsPort);
		SettingsManager::getInstance()->set(SettingsManager::DHT_PORT, dhtPort);
		randDone = true;
	} else {
		tcpPort = static_cast<uint16_t>(SETTING(TCP_PORT));
		udpPort = static_cast<uint16_t>(SETTING(UDP_PORT));
		tlsPort = static_cast<uint16_t>(SETTING(TLS_PORT));
		dhtPort = static_cast<uint16_t>(SETTING(DHT_PORT));
	}
}

std::string NetworkConfiguration::toIP(uint32_t ipnum) const {
	std::string ret = Util::toString((ipnum / 16777216) % 256) + '.' + Util::toString((ipnum / 65536) % 256) + '.' + Util::toString((ipnum / 256) % 256) + '.' + Util::toString(ipnum % 256);
	return ret;
}

void NetworkConfiguration::on(TimerManagerListener::Minute, uint64_t /*tick*/) {
	// renew ports every hour
	if(--count == 0) {
		count = 60;
		dcdebug("NAT-PMP::ACT Renewing port's mappings\n");
		restart();
	}
}
}

/**
 * @file
 * $Id$
 */
