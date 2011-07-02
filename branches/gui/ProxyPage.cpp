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

#include "ProxyPage.h"

SettingsPage::TextItem ProxyPage::texts[] = {
	{ IDC_DIRECT_OUT,					ResourceManager::SETTINGS_DIRECT			},
	{ IDC_SOCKS5,						ResourceManager::SETTINGS_SOCKS5			},
	{ IDC_SETTINGS_SOCKS5_IP,			ResourceManager::SETTINGS_SOCKS5_IP			},
	{ IDC_SETTINGS_SOCKS5_PORT,			ResourceManager::SETTINGS_SOCKS5_PORT		},
	{ IDC_SETTINGS_SOCKS5_USERNAME,		ResourceManager::SETTINGS_SOCKS5_USERNAME	},
	{ IDC_SETTINGS_SOCKS5_PASSWORD,		ResourceManager::PASSWORD					},
	{ IDC_SOCKS_RESOLVE,				ResourceManager::SETTINGS_SOCKS5_RESOLVE	},
	{ IDC_SETTINGS_OUTGOING,			ResourceManager::SETTINGS_OUTGOING			},
	{ 0,								ResourceManager::SETTINGS_AUTO_AWAY			}
};

SettingsPage::Item ProxyPage::items[] = {
	{ IDC_SOCKS_SERVER,			SettingsManager::SOCKS_SERVER,				SettingsPage::T_STR		},
	{ IDC_SOCKS_PORT,			SettingsManager::SOCKS_PORT,				SettingsPage::T_INT		},
	{ IDC_SOCKS_USER,			SettingsManager::SOCKS_USER,				SettingsPage::T_STR		},
	{ IDC_SOCKS_PASSWORD,		SettingsManager::SOCKS_PASSWORD,			SettingsPage::T_STR		},
	{ IDC_SOCKS_RESOLVE,		SettingsManager::SOCKS_RESOLVE,				SettingsPage::T_BOOL	},
	{ 0,						0,											SettingsPage::T_END		}
};

BEGIN_EVENT_TABLE(ProxyPage, SettingsPage)
	EVT_RADIOBUTTON(IDC_DIRECT_OUT, ProxyPage::onOutRadio)
	EVT_RADIOBUTTON(IDC_SOCKS5, ProxyPage::onOutRadio)
END_EVENT_TABLE()

ProxyPage::ProxyPage(wxWindow* parent, SettingsManager *s) : SettingsPage(parent, s)
{
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_SETTINGS_OUTGOING, wxEmptyString), wxVERTICAL);

	outDirectRadio = new wxRadioButton(this, IDC_DIRECT_OUT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	outDirectRadio->SetValue(true);
	sbSizer3->Add(outDirectRadio, 0, wxALL, 5);

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxHORIZONTAL);

	outSocksRadio = new wxRadioButton(this, IDC_SOCKS5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer6->Add(outSocksRadio, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);

	resolveSocksCheck = new wxCheckBox(this, IDC_SOCKS_RESOLVE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);

	bSizer6->Add(resolveSocksCheck, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);

	sbSizer3->Add(bSizer6, 0, wxEXPAND, 5);

	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer(2, 4, 0, 0);
	fgSizer4->AddGrowableCol(0);
	fgSizer4->AddGrowableCol(2);
	fgSizer4->SetFlexibleDirection(wxBOTH);
	fgSizer4->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	outSocksIpLabel = new wxStaticText(this, IDC_SETTINGS_SOCKS5_IP, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	outSocksIpLabel->Wrap(-1);
	fgSizer4->Add(outSocksIpLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);

	outSocksIpEdit = new wxTextCtrl(this, IDC_SOCKS_SERVER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	outSocksIpEdit->SetMaxLength(250);
	fgSizer4->Add(outSocksIpEdit, 0, wxBOTTOM | wxRIGHT, 5);

	outSocksPortLabel = new wxStaticText(this, IDC_SETTINGS_SOCKS5_PORT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	outSocksPortLabel->Wrap(-1);
	fgSizer4->Add(outSocksPortLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);

	outSocksPortSpin = new wxSpinCtrl(this, IDC_SOCKS_PORT, wxEmptyString, wxDefaultPosition, wxSize(60,-1),
		wxSP_ARROW_KEYS, 0, 65535);
	fgSizer4->Add(outSocksPortSpin, 0, wxBOTTOM | wxRIGHT, 5);

	outLoginLabel = new wxStaticText(this, IDC_SETTINGS_SOCKS5_USERNAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	outLoginLabel->Wrap(-1);
	fgSizer4->Add(outLoginLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);

	outSocksLoginEdit = new wxTextCtrl(this, IDC_SOCKS_USER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	outSocksLoginEdit->SetMaxLength(250);
	fgSizer4->Add(outSocksLoginEdit, 0, wxBOTTOM | wxRIGHT, 5);

	outSocksPassLabel = new wxStaticText(this, IDC_SETTINGS_SOCKS5_PASSWORD, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	outSocksPassLabel->Wrap(-1);
	fgSizer4->Add(outSocksPassLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);

	outSocksPassEdit = new wxTextCtrl(this, IDC_SOCKS_PASSWORD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	outSocksPassEdit->SetMaxLength(250);
	fgSizer4->Add(outSocksPassEdit, 0, wxBOTTOM | wxRIGHT, 5);

	sbSizer3->Add(fgSizer4, 0, wxEXPAND, 5);

	this->SetSizerAndFit(sbSizer3);

	SettingsPage::translate(texts);
	SettingsPage::read(items);

	switch(SETTING(OUTGOING_CONNECTIONS))
	{
		case SettingsManager::OUTGOING_SOCKS5:
			outSocksRadio->SetValue(true);
			break;
		default:
			outDirectRadio->SetValue(true);
			break;
	}

	doOutRadio();
}

ProxyPage::~ProxyPage()
{
}

void ProxyPage::onOutRadio(wxCommandEvent &WXUNUSED(event))
{
	doOutRadio();
}

void ProxyPage::doOutRadio()
{
	const bool isSOCKS = outSocksRadio->GetValue();
	resolveSocksCheck->Enable(isSOCKS);
	outSocksIpEdit->Enable(isSOCKS);
	outSocksPortSpin->Enable(isSOCKS);
	outSocksLoginEdit->Enable(isSOCKS);
	outSocksPassEdit->Enable(isSOCKS);
}

void ProxyPage::write()
{
	wxString str = outSocksIpEdit->GetValue();
	str.Replace(_T(" "), wxEmptyString);
	outSocksIpEdit->SetValue(str);

	SettingsPage::write(items);

	int ct = SettingsManager::OUTGOING_DIRECT;

	if(outSocksRadio->GetValue())
		ct = SettingsManager::OUTGOING_SOCKS5;

	if(SETTING(OUTGOING_CONNECTIONS) != ct)
	{
		settings->set(SettingsManager::OUTGOING_CONNECTIONS, ct);
		Socket::socksUpdated();
	}
}

