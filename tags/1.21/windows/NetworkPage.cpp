/* 
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "../client/Socket.h"

#include "../client/SimpleXML.h"
#include "../client/version.h"
#include "../client/rsxppSettingsManager.h" //RSX++

#include "Resource.h"
#include "NetworkPage.h"
#include "WinUtil.h"

#include <IPHlpApi.h>
#pragma comment(lib, "iphlpapi.lib")

PropPage::TextItem NetworkPage::texts[] = {
	{ IDC_DIRECT,						ResourceManager::SETTINGS_DIRECT			},
	{ IDC_DIRECT_OUT,					ResourceManager::SETTINGS_DIRECT			},
	{ IDC_FIREWALL_UPNP,				ResourceManager::SETTINGS_FIREWALL_UPNP		},
	{ IDC_FIREWALL_NAT,					ResourceManager::SETTINGS_FIREWALL_NAT		},
	{ IDC_FIREWALL_PASSIVE,				ResourceManager::SETTINGS_FIREWALL_PASSIVE	},
	{ IDC_OVERRIDE,						ResourceManager::SETTINGS_OVERRIDE			},
	{ IDC_IPUPDATE,						ResourceManager::SETTINGS_UPDATE_IP			},
	{ IDC_SOCKS5,						ResourceManager::SETTINGS_SOCKS5			}, 
	{ IDC_SETTINGS_PORTS,				ResourceManager::SETTINGS_PORTS				},
	{ IDC_SETTINGS_IP,					ResourceManager::SETTINGS_EXTERNAL_IP		},
	{ IDC_SETTINGS_PORT_TCP,			ResourceManager::SETTINGS_TCP_PORT			},
	{ IDC_SETTINGS_PORT_UDP,			ResourceManager::SETTINGS_UDP_PORT			},
	{ IDC_SETTINGS_PORT_TLS,			ResourceManager::SETTINGS_TLS_PORT			},
	{ IDC_SETTINGS_PORT_DHT,			ResourceManager::SETTINGS_DHT_PORT			},
	{ IDC_SETTINGS_SOCKS5_IP,			ResourceManager::SETTINGS_SOCKS5_IP			},
	{ IDC_SETTINGS_SOCKS5_PORT,			ResourceManager::SETTINGS_SOCKS5_PORT		},
	{ IDC_SETTINGS_SOCKS5_USERNAME,		ResourceManager::SETTINGS_SOCKS5_USERNAME	},
	{ IDC_SETTINGS_SOCKS5_PASSWORD,		ResourceManager::PASSWORD					},
	{ IDC_SOCKS_RESOLVE,				ResourceManager::SETTINGS_SOCKS5_RESOLVE	},
	{ IDC_SETTINGS_INCOMING,			ResourceManager::SETTINGS_INCOMING			},
	{ IDC_SETTINGS_OUTGOING,			ResourceManager::SETTINGS_OUTGOING			},
	{ IDC_SETTINGS_BIND_ADDRESS,		ResourceManager::SETTINGS_BIND_ADDRESS		},
	{ IDC_SETTINGS_BIND_ADDRESS_HELP,	ResourceManager::SETTINGS_BIND_ADDRESS_HELP },
	{ IDC_NATT,							ResourceManager::ALLOW_NAT_TRAVERSAL		},
	//RSX++
	{ IDC_CON_CHECK, ResourceManager::CHECK_SETTINGS },
	{ IDC_GETIP, ResourceManager::GET_IP },
	//END
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item NetworkPage::items[] = {
	{ IDC_EXTERNAL_IP,		SettingsManager::EXTERNAL_IP,			PropPage::T_STR		}, 
	{ IDC_PORT_TCP,			SettingsManager::TCP_PORT,				PropPage::T_INT		}, 
	{ IDC_PORT_UDP,			SettingsManager::UDP_PORT,				PropPage::T_INT		}, 
	{ IDC_PORT_TLS,			SettingsManager::TLS_PORT,				PropPage::T_INT		},
	{ IDC_PORT_DHT,			SettingsManager::DHT_PORT,				PropPage::T_INT		},	
	{ IDC_OVERRIDE,			SettingsManager::NO_IP_OVERRIDE,		PropPage::T_BOOL	},
	{ IDC_IPUPDATE,			rsxppSettingsManager::IPUPDATE,			PropPage::T_BOOL_RSX },
	{ IDC_SOCKS_SERVER,		SettingsManager::SOCKS_SERVER,			PropPage::T_STR		},
	{ IDC_SOCKS_PORT,		SettingsManager::SOCKS_PORT,			PropPage::T_INT		},
	{ IDC_SOCKS_USER,		SettingsManager::SOCKS_USER,			PropPage::T_STR		},
	{ IDC_SOCKS_PASSWORD,	SettingsManager::SOCKS_PASSWORD,		PropPage::T_STR		},
	{ IDC_SOCKS_RESOLVE,	SettingsManager::SOCKS_RESOLVE,			PropPage::T_BOOL	},
	{ IDC_BIND_ADDRESS,		SettingsManager::BIND_ADDRESS,			PropPage::T_STR		},
	{ IDC_NATT,				SettingsManager::ALLOW_NAT_TRAVERSAL,	PropPage::T_BOOL	},
	//RSX++
	{ IDC_NAT_PMP_GATEWAY, SettingsManager::NAT_PMP_GATEWAY, PropPage::T_STR },
	{ IDC_NAT_PMP_RANDOM_PORTS, SettingsManager::FIREWALL_RAND_PORTS, PropPage::T_BOOL },
	{ IDC_IPUPDATE,		rsxppSettingsManager::IPUPDATE,	PropPage::T_BOOL_RSX },
	//END
	{ 0, 0, PropPage::T_END }
};

void NetworkPage::write()
{
	TCHAR tmp[1024];
	GetDlgItemText(IDC_SOCKS_SERVER, tmp, 1024);
	tstring x = tmp;
	tstring::size_type i;

	while((i = x.find(' ')) != string::npos)
		x.erase(i, 1);
	SetDlgItemText(IDC_SOCKS_SERVER, x.c_str());

	GetDlgItemText(IDC_SERVER, tmp, 1024);
	x = tmp;

	while((i = x.find(' ')) != string::npos)
		x.erase(i, 1);

	SetDlgItemText(IDC_SERVER, x.c_str());
	
	PropPage::write((HWND)(*this), items);

	// Set connection active/passive
	int ct = SettingsManager::INCOMING_DIRECT;

	if(IsDlgButtonChecked(IDC_FIREWALL_UPNP))
		ct = SettingsManager::INCOMING_FIREWALL_UPNP;
	else if(IsDlgButtonChecked(IDC_FIREWALL_NAT))
		ct = SettingsManager::INCOMING_FIREWALL_NAT;
	else if(IsDlgButtonChecked(IDC_FIREWALL_PASSIVE))
		ct = SettingsManager::INCOMING_FIREWALL_PASSIVE;
	//RSX++
	else if(IsDlgButtonChecked(IDC_FIREWALL_NAT_PMP))
		ct = SettingsManager::INCOMING_FIREWALL_NAT_PMP;
	//END
	if(SETTING(INCOMING_CONNECTIONS) != ct) {
		settings->set(SettingsManager::INCOMING_CONNECTIONS, ct);
	}

	ct = SettingsManager::OUTGOING_DIRECT;
	
	if(IsDlgButtonChecked(IDC_SOCKS5))
		ct = SettingsManager::OUTGOING_SOCKS5;

	if(SETTING(OUTGOING_CONNECTIONS) != ct) {
		settings->set(SettingsManager::OUTGOING_CONNECTIONS, ct);
		Socket::socksUpdated();
	}
}

LRESULT NetworkPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	
	if(!(WinUtil::getOsMajor() >= 5 && WinUtil::getOsMinor() >= 1  //WinXP & WinSvr2003
		|| WinUtil::getOsMajor() >= 6 )) //Vista
	{
		::EnableWindow(GetDlgItem(IDC_FIREWALL_UPNP), FALSE);
	}
	switch(SETTING(INCOMING_CONNECTIONS)) {
		case SettingsManager::INCOMING_DIRECT: CheckDlgButton(IDC_DIRECT, BST_CHECKED); break;
		case SettingsManager::INCOMING_FIREWALL_UPNP: CheckDlgButton(IDC_FIREWALL_UPNP, BST_CHECKED); break;
		case SettingsManager::INCOMING_FIREWALL_NAT_PMP: CheckDlgButton(IDC_FIREWALL_NAT_PMP, BST_CHECKED); break; //RSX++
		case SettingsManager::INCOMING_FIREWALL_NAT: CheckDlgButton(IDC_FIREWALL_NAT, BST_CHECKED); break;
		case SettingsManager::INCOMING_FIREWALL_PASSIVE: CheckDlgButton(IDC_FIREWALL_PASSIVE, BST_CHECKED); break;
		default: CheckDlgButton(IDC_DIRECT, BST_CHECKED); break;
	}

	switch(SETTING(OUTGOING_CONNECTIONS)) {
		case SettingsManager::OUTGOING_DIRECT: CheckDlgButton(IDC_DIRECT_OUT, BST_CHECKED); break;
		case SettingsManager::OUTGOING_SOCKS5: CheckDlgButton(IDC_SOCKS5, BST_CHECKED); break;
		default: CheckDlgButton(IDC_DIRECT_OUT, BST_CHECKED); break;
	}

	PropPage::read((HWND)(*this), items);

	fixControls();

	desc.Attach(GetDlgItem(IDC_SOCKS_SERVER));
	desc.LimitText(250);
	desc.Detach();
	desc.Attach(GetDlgItem(IDC_SOCKS_PORT));
	desc.LimitText(5);
	desc.Detach();
	desc.Attach(GetDlgItem(IDC_SOCKS_USER));
	desc.LimitText(250);
	desc.Detach();
	desc.Attach(GetDlgItem(IDC_SOCKS_PASSWORD));
	desc.LimitText(250);
	desc.Detach();

	//RSX++
	ConnCheckUrl.SubclassWindow(GetDlgItem(IDC_CON_CHECK));
	ConnCheckUrl.SetHyperLinkExtendedStyle(HLINK_COMMANDBUTTON|HLINK_UNDERLINEHOVER);
	ConnCheckUrl.m_tip.AddTool(ConnCheckUrl, Text::toT(STRING(CHECK_SETTINGS)).c_str(), &ConnCheckUrl.m_rcLink, 1);
	//END

	BindCombo.Attach(GetDlgItem(IDC_BIND_ADDRESS));
	//BindCombo.AddString(_T("0.0.0.0"));
	getAddresses();
	BindCombo.SetCurSel(BindCombo.FindString(0, Text::toT(SETTING(BIND_ADDRESS)).c_str()));
	
	if(BindCombo.GetCurSel() == -1) {
		BindCombo.AddString(Text::toT(SETTING(BIND_ADDRESS)).c_str());
		BindCombo.SetCurSel(BindCombo.FindString(0, Text::toT(SETTING(BIND_ADDRESS)).c_str()));
	}

	return TRUE;
}

void NetworkPage::fixControls() {
	BOOL direct = IsDlgButtonChecked(IDC_DIRECT) == BST_CHECKED;
	BOOL upnp = IsDlgButtonChecked(IDC_FIREWALL_UPNP) == BST_CHECKED;
	BOOL nat = IsDlgButtonChecked(IDC_FIREWALL_NAT) == BST_CHECKED;
	//RSX++
	BOOL natpmp = IsDlgButtonChecked(IDC_FIREWALL_NAT_PMP) == BST_CHECKED;
	BOOL randPorts = IsDlgButtonChecked(IDC_NAT_PMP_RANDOM_PORTS) == BST_CHECKED;
	BOOL nat_traversal = IsDlgButtonChecked(IDC_NATT) == BST_CHECKED;
	//END

	::EnableWindow(GetDlgItem(IDC_EXTERNAL_IP), direct || upnp || natpmp || nat);
	::EnableWindow(GetDlgItem(IDC_OVERRIDE), direct || upnp || natpmp || nat);
	::EnableWindow(GetDlgItem(IDC_NATT), !direct && !upnp && !nat); // for pasive settings only

	//RSX++
	::EnableWindow(GetDlgItem(IDC_NAT_PMP_RANDOM_PORTS), natpmp);
	::EnableWindow(GetDlgItem(IDC_NAT_PMP_GATEWAY), natpmp);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_NAT_PMP_GATEWAY), natpmp);
	::EnableWindow(GetDlgItem(IDC_IPUPDATE), direct || upnp || natpmp || nat);
	::EnableWindow(GetDlgItem(IDC_GETIP), direct || upnp || natpmp || nat);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_IP), direct || upnp || natpmp || nat);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_PORTS), upnp || (natpmp && !randPorts) || nat);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_PORT_TCP), upnp || (natpmp && !randPorts) || nat);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_PORT_UDP), upnp || (natpmp && !randPorts) || nat);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_PORT_TLS), upnp || (natpmp && !randPorts) || nat);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_PORT_DHT), upnp || (natpmp && !randPorts) || nat);
	::EnableWindow(GetDlgItem(IDC_CON_CHECK), direct || upnp || natpmp || nat);
	//END

	::EnableWindow(GetDlgItem(IDC_PORT_TCP), upnp || (natpmp && !randPorts) || nat);
	::EnableWindow(GetDlgItem(IDC_PORT_UDP), upnp || (natpmp && !randPorts) || nat);
	::EnableWindow(GetDlgItem(IDC_PORT_TLS), upnp || (natpmp && !randPorts) || nat);
	::EnableWindow(GetDlgItem(IDC_PORT_DHT), upnp || (natpmp && !randPorts) || nat);	

	BOOL socks = IsDlgButtonChecked(IDC_SOCKS5);
	::EnableWindow(GetDlgItem(IDC_SOCKS_SERVER), socks);
	::EnableWindow(GetDlgItem(IDC_SOCKS_PORT), socks);
	::EnableWindow(GetDlgItem(IDC_SOCKS_USER), socks);
	::EnableWindow(GetDlgItem(IDC_SOCKS_PASSWORD), socks);
	::EnableWindow(GetDlgItem(IDC_SOCKS_RESOLVE), socks);
	//RSX++
	::EnableWindow(GetDlgItem(IDC_SETTINGS_SOCKS5_IP), socks);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_SOCKS5_PORT), socks);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_SOCKS5_USERNAME), socks);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_SOCKS5_PASSWORD), socks);
	//END
}

void NetworkPage::getAddresses() {
	IP_ADAPTER_INFO* AdapterInfo = NULL;
	DWORD dwBufLen = NULL;

	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if(dwStatus == ERROR_BUFFER_OVERFLOW) {
		AdapterInfo = (IP_ADAPTER_INFO*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBufLen);
		dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);		
	}

	if(dwStatus == ERROR_SUCCESS) {
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
		while (pAdapterInfo) {
			IP_ADDR_STRING* pIpList = &(pAdapterInfo->IpAddressList);
			while (pIpList) {
				BindCombo.AddString(Text::toT(pIpList->IpAddress.String).c_str());
				pIpList = pIpList->Next;
			}
			pAdapterInfo = pAdapterInfo->Next;
		}
	}
	
	if(AdapterInfo)
		HeapFree(GetProcessHeap(), 0, AdapterInfo);	
}

LRESULT NetworkPage::onClickedActive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}
//RSX++
LRESULT NetworkPage::onGetIP(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	::EnableWindow(GetDlgItem(IDC_GETIP), false);
	HTTPDownloadManager::getInstance()->addRequest(boost::bind(&NetworkPage::onHttpRequest, this, _1, _2), "http://checkip.dyndns.org/", false);
	return 0;
}

LRESULT NetworkPage::onCheckConn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	write();
	WinUtil::openLink(Text::toT("http://connect.majestyc.net/?t=" + Util::toString(SETTING(TCP_PORT)) + "&u=" + Util::toString(SETTING(UDP_PORT))));
	return 0;
}

void NetworkPage::onHttpRequest(string buf, bool isFailed) {
	if(!isFailed) {
		if(!buf.empty()) {
			try {
				SimpleXML xml;
				xml.fromXML(buf);
				if(xml.findChild("html")) {
					xml.stepIn();
					if(xml.findChild("body")) {
						string x = xml.getChildData().substr(20);
						if(Util::isPrivateIp(x)) {
								CheckRadioButton(IDC_DIRECT, IDC_FIREWALL_PASSIVE, IDC_FIREWALL_PASSIVE);
								fixControls();
						}
						SetDlgItemText(IDC_SERVER, Text::toT(x).c_str());
					} else {
						if(Util::isPrivateIp(Util::getLocalIp())) {
								CheckRadioButton(IDC_DIRECT, IDC_FIREWALL_PASSIVE, IDC_FIREWALL_PASSIVE);
								fixControls();
						}
						SetDlgItemText(IDC_SERVER, Text::toT(Util::getLocalIp()).c_str());
					}
				}
			} catch(const SimpleXMLException&) {
				//...
			}
		}
	} else {
		if(Util::isPrivateIp(Util::getLocalIp())) {
			CheckRadioButton(IDC_DIRECT, IDC_FIREWALL_PASSIVE, IDC_FIREWALL_PASSIVE);
			fixControls();
		}
		SetDlgItemText(IDC_SERVER, Text::toT(Util::getLocalIp()).c_str());
	}
	::EnableWindow(GetDlgItem(IDC_GETIP), true);
}
//END
