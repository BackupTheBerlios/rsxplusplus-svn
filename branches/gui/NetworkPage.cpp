/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
 * Copyright (C) 2010 Oyashiro-sama, oyashirosama dot hnnkni at gmail dot com
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
#include "../client/SettingsManager.h"
#include "../client/Socket.h"
#include "WinUtil.h"

#include <IPHlpApi.h>
#pragma comment(lib, "iphlpapi.lib")

#include "NetworkPage.h"

SettingsPage::TextItem NetworkPage::texts[] = {
	{ IDC_DIRECT,						ResourceManager::SETTINGS_DIRECT			},
	{ IDC_FIREWALL_UPNP,				ResourceManager::SETTINGS_FIREWALL_UPNP		},
	{ IDC_FIREWALL_NAT,					ResourceManager::SETTINGS_FIREWALL_NAT		},
	{ IDC_FIREWALL_PASSIVE,				ResourceManager::SETTINGS_FIREWALL_PASSIVE	},
	{ IDC_OVERRIDE,						ResourceManager::SETTINGS_OVERRIDE			},
	{ IDC_UPDATE_IP,					ResourceManager::SETTINGS_UPDATE_IP			},
	{ IDC_SETTINGS_IP,					ResourceManager::SETTINGS_EXTERNAL_IP		},
	{ IDC_SETTINGS_PORT_TCP,			ResourceManager::SETTINGS_TCP_PORT			},
	{ IDC_SETTINGS_PORT_UDP,			ResourceManager::SETTINGS_UDP_PORT			},
	{ IDC_SETTINGS_PORT_TLS,			ResourceManager::SETTINGS_TLS_PORT			},
	{ IDC_SETTINGS_PORT_DHT,			ResourceManager::SETTINGS_DHT_PORT			},
	{ IDC_SETTINGS_INCOMING,			ResourceManager::SETTINGS_INCOMING			},
	{ IDC_SETTINGS_BIND_ADDRESS,		ResourceManager::SETTINGS_BIND_ADDRESS		},
	{ IDC_SETTINGS_BIND_ADDRESS_HELP,	ResourceManager::SETTINGS_BIND_ADDRESS_HELP },
	{ IDC_NATT,							ResourceManager::ALLOW_NAT_TRAVERSAL		},
	{ 0,								ResourceManager::SETTINGS_AUTO_AWAY			}
};

SettingsPage::Item NetworkPage::items[] = {
	{ IDC_CONNECTION_DETECTION,	SettingsManager::AUTO_DETECT_CONNECTION,	SettingsPage::T_BOOL	},
	{ IDC_EXTERNAL_IP,			SettingsManager::EXTERNAL_IP,				SettingsPage::T_STR		},
	{ IDC_PORT_TCP,				SettingsManager::TCP_PORT,					SettingsPage::T_INT		},
	{ IDC_PORT_UDP,				SettingsManager::UDP_PORT,					SettingsPage::T_INT		},
	{ IDC_PORT_TLS,				SettingsManager::TLS_PORT,					SettingsPage::T_INT		},
	{ IDC_PORT_DHT,				SettingsManager::DHT_PORT,					SettingsPage::T_INT		},
	{ IDC_OVERRIDE,				SettingsManager::NO_IP_OVERRIDE,			SettingsPage::T_BOOL	},
	{ IDC_UPDATE_IP,			SettingsManager::UPDATE_IP,					SettingsPage::T_BOOL	},
	{ IDC_BIND_ADDRESS,			SettingsManager::BIND_ADDRESS,				SettingsPage::T_STR		},
	{ IDC_NATT,					SettingsManager::ALLOW_NAT_TRAVERSAL,		SettingsPage::T_BOOL	},
	{ 0,						0,											SettingsPage::T_END		}
};

BEGIN_EVENT_TABLE(NetworkPage, SettingsPage)
	EVT_CHECKBOX(IDC_CONNECTION_DETECTION, NetworkPage::onInRadio)
	EVT_RADIOBUTTON(IDC_DIRECT, NetworkPage::onInRadio)
	EVT_RADIOBUTTON(IDC_FIREWALL_UPNP, NetworkPage::onInRadio)
	EVT_RADIOBUTTON(IDC_FIREWALL_NAT, NetworkPage::onInRadio)
	EVT_RADIOBUTTON(IDC_FIREWALL_PASSIVE, NetworkPage::onInRadio)
END_EVENT_TABLE()

NetworkPage::NetworkPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_BIND_ADDRESS, wxEmptyString), wxVERTICAL);

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(2, 2, 0, 0);
	fgSizer1->AddGrowableCol(0);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	bindAddrLabel = new wxStaticText(this, IDC_SETTINGS_BIND_ADDRESS_HELP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bindAddrLabel->Wrap(-1);
	fgSizer1->Add(bindAddrLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	bindAddrCombo = new wxComboBox(this, IDC_BIND_ADDRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	fgSizer1->Add(bindAddrCombo, 0, wxRIGHT, 5);

	sbSizer1->Add(fgSizer1, 1, wxEXPAND, 5);

	bSizer1->Add(sbSizer1, 0, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_INCOMING, wxEmptyString), wxVERTICAL);

	autoDetect = new wxCheckBox(this, IDC_CONNECTION_DETECTION, _("Enable automatic incoming connection type detection"));
	sbSizer2->Add(autoDetect, 0, wxALL, 5);

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer2->AddGrowableCol(0);
	fgSizer2->AddGrowableRow(0);
	fgSizer2->SetFlexibleDirection(wxBOTH);
	fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxVERTICAL);

	inDirectRadio = new wxRadioButton(this, IDC_DIRECT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(inDirectRadio, 0, wxALL, 5);

	inFwUpnpRadio = new wxRadioButton(this, IDC_FIREWALL_UPNP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(inFwUpnpRadio, 0, wxALL, 5);

	inFwManRadio = new wxRadioButton(this, IDC_FIREWALL_NAT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(inFwManRadio, 0, wxALL, 5);

	inFwPassRadio = new wxRadioButton(this, IDC_FIREWALL_PASSIVE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(inFwPassRadio, 0, wxALL, 5);

	fgSizer2->Add(bSizer2, 1, wxEXPAND, 5);

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer3->SetFlexibleDirection(wxBOTH);
	fgSizer3->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	tcpPortLabel = new wxStaticText(this, IDC_SETTINGS_PORT_TCP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	tcpPortLabel->Wrap(-1);
	fgSizer3->Add(tcpPortLabel, 0, wxALIGN_RIGHT | wxALL, 5);

	tcpPortSpin = new wxSpinCtrl(this, IDC_PORT_TCP, wxEmptyString, wxDefaultPosition, wxSize(60,-1),
		wxSP_ARROW_KEYS, 0, 65535);
	fgSizer3->Add(tcpPortSpin, 0, wxBOTTOM | wxRIGHT, 5);

	udpPortLabel = new wxStaticText(this, IDC_SETTINGS_PORT_UDP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	udpPortLabel->Wrap(-1);
	fgSizer3->Add(udpPortLabel, 0, wxALIGN_RIGHT | wxALL, 5);

	udpPortSpin = new wxSpinCtrl(this, IDC_PORT_UDP, wxEmptyString, wxDefaultPosition, wxSize(60,-1),
		wxSP_ARROW_KEYS, 0, 65535);
	fgSizer3->Add(udpPortSpin, 0, wxBOTTOM | wxRIGHT, 5);

	tlsPortLabel = new wxStaticText(this, IDC_SETTINGS_PORT_TLS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	tlsPortLabel->Wrap(-1);
	fgSizer3->Add(tlsPortLabel, 0, wxALIGN_RIGHT | wxALL, 5);

	tlsPortSpin = new wxSpinCtrl(this, IDC_PORT_TLS, wxEmptyString, wxDefaultPosition, wxSize(60,-1),
		wxSP_ARROW_KEYS, 0, 65535);
	fgSizer3->Add(tlsPortSpin, 0, wxBOTTOM | wxRIGHT, 5);

	dhtPortLabel = new wxStaticText(this, IDC_SETTINGS_PORT_DHT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	dhtPortLabel->Wrap(-1);
	fgSizer3->Add(dhtPortLabel, 0, wxALIGN_RIGHT | wxALL, 5);

	dhtPortSpin = new wxSpinCtrl(this, IDC_PORT_DHT, wxEmptyString, wxDefaultPosition, wxSize(60,-1),
		wxSP_ARROW_KEYS, 0, 65535);
	fgSizer3->Add(dhtPortSpin, 0, wxRIGHT, 5);

	fgSizer2->Add(fgSizer3, 1, wxEXPAND, 5);

	sbSizer2->Add(fgSizer2, 1, wxEXPAND, 5);

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxVERTICAL);

	wanLabel = new wxStaticText(this, IDC_SETTINGS_IP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	wanLabel->Wrap(-1);
	bSizer4->Add(wanLabel, 0, wxBOTTOM | wxLEFT, 5);

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxHORIZONTAL);

	wanIpEdit = new wxTextCtrl(this, IDC_EXTERNAL_IP, wxEmptyString, wxDefaultPosition, wxSize(150,-1), 0);
	bSizer5->Add(wanIpEdit, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	bSizer4->Add(bSizer5, 1, wxEXPAND, 5);

	noOverrideCheck = new wxCheckBox(this, IDC_OVERRIDE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(noOverrideCheck, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	updIpCheck = new wxCheckBox(this, IDC_UPDATE_IP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(updIpCheck, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	allowNatCheck = new wxCheckBox(this, IDC_NATT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(allowNatCheck, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);

	bSizer3->Add(bSizer4, 1, wxEXPAND, 5);

	sbSizer2->Add(bSizer3, 0, wxEXPAND, 5);

	bSizer1->Add(sbSizer2, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);

	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	/*if(!(WinUtil::getOsMajor() >= 5 && WinUtil::getOsMinor() >= 1  //WinXP & WinSvr2003
		|| WinUtil::getOsMajor() >= 6 )) //Vista
	{
		inFwUpnpRadio->Enable(false);
	}*/

	SettingsPage::translate(texts);
	SettingsPage::read(items);

	switch(SETTING(INCOMING_CONNECTIONS))
	{
		case SettingsManager::INCOMING_FIREWALL_UPNP:
			inFwUpnpRadio->SetValue(true);
			break;
		case SettingsManager::INCOMING_FIREWALL_NAT:
			inFwManRadio->SetValue(true);
			break;
		case SettingsManager::INCOMING_FIREWALL_PASSIVE:
			inFwPassRadio->SetValue(true);
			break;
		default:
			inDirectRadio->SetValue(true);
			break;
	}

	doInRadio();

	getAddresses();
	bindAddrCombo->SetValue(SETTING(BIND_ADDRESS));
}

NetworkPage::~NetworkPage()
{
}

void NetworkPage::onInRadio(wxCommandEvent &WXUNUSED(event))
{
	doInRadio();
}

void NetworkPage::doInRadio()
{
	bool auto_detect = autoDetect->GetValue();
	bool direct = inDirectRadio->GetValue();
	bool upnp = inFwUpnpRadio->GetValue();
	bool nat = inFwManRadio->GetValue();
	bool nat_traversal = allowNatCheck->GetValue();

	inDirectRadio->Enable(!auto_detect);
	inFwUpnpRadio->Enable(!auto_detect);
	inFwManRadio->Enable(!auto_detect);
	inFwPassRadio->Enable(!auto_detect);
	updIpCheck->Enable(!auto_detect);
	wanLabel->Enable(!auto_detect);

	wanIpEdit->Enable(!auto_detect && (direct || upnp || nat || nat_traversal));
	noOverrideCheck->Enable(!auto_detect && (direct || upnp || nat || nat_traversal));

	tcpPortSpin->Enable(!auto_detect && (upnp || nat));
	udpPortSpin->Enable(!auto_detect && (upnp || nat));
	tlsPortSpin->Enable(!auto_detect && (upnp || nat));
	allowNatCheck->Enable(!auto_detect && !direct && !upnp && !nat); // for passive settings only
}

void NetworkPage::getAddresses()
{
	IP_ADAPTER_INFO* AdapterInfo = NULL;
	DWORD dwBufLen = NULL;

	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if(dwStatus == ERROR_BUFFER_OVERFLOW)
	{
		AdapterInfo = (IP_ADAPTER_INFO*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBufLen);
		dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	}

	if(dwStatus == ERROR_SUCCESS)
	{
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
		while (pAdapterInfo)
		{
			IP_ADDR_STRING* pIpList = &(pAdapterInfo->IpAddressList);
			while (pIpList)
			{
				bindAddrCombo->AppendString(Text::toT(pIpList->IpAddress.String).c_str());
				pIpList = pIpList->Next;
			}
			pAdapterInfo = pAdapterInfo->Next;
		}
	}

	if(AdapterInfo)
		HeapFree(GetProcessHeap(), 0, AdapterInfo);
}

void NetworkPage::write()
{
	SettingsPage::write(items);

	int ct = SettingsManager::INCOMING_DIRECT;

	if (inFwUpnpRadio->GetValue())
	{
		ct = SettingsManager::INCOMING_FIREWALL_UPNP;
	}
	else if (inFwManRadio->GetValue())
	{
		ct = SettingsManager::INCOMING_FIREWALL_NAT;
	}
	else if (inFwPassRadio->GetValue())
	{
		ct = SettingsManager::INCOMING_FIREWALL_PASSIVE;
	}

	if(SETTING(INCOMING_CONNECTIONS) != ct)
	{
		settings->set(SettingsManager::INCOMING_CONNECTIONS, ct);
	}
}

