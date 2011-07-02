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
#include "../client/DCPlusPlus.h"
#include "WinUtil.h"

#include "FavHubProperties.h"

#include "../client/FavoriteManager.h"
#include "../client/ResourceManager.h"

BEGIN_EVENT_TABLE(FavHubProperties, wxDialog)
	EVT_BUTTON(wxID_OK, FavHubProperties::onOk)
	EVT_TEXT(IDC_HUBNICK, FavHubProperties::onTextUpdated)
	EVT_TEXT(IDC_HUBPASS, FavHubProperties::onTextUpdated)
	EVT_TEXT(IDC_HUBUSERDESCR, FavHubProperties::onTextUpdated)
END_EVENT_TABLE()

FavHubProperties::FavHubProperties(wxWindow* parent, FavoriteHubEntry *entry, const wxString& title,
	const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, wxID_ANY, title, pos, size, style)
{
	_entry = entry;

	wxTextValidator textValidator(wxFILTER_EXCLUDE_CHAR_LIST);
	wxArrayString _excludeList;
	_excludeList.Add(_T("$"));
	_excludeList.Add(_T("|"));
	_excludeList.Add(_T(" "));
	_excludeList.Shrink();
	textValidator.SetExcludes(_excludeList);

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_FH_HUB, CTSTRING(HUB)), wxVERTICAL);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(4, 2, 0, 0);
	fgSizer1->AddGrowableCol(1);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	hubNameLabel = new wxStaticText(this, IDC_FH_NAME, CTSTRING(HUB_NAME), wxDefaultPosition, wxDefaultSize, 0);
	hubNameLabel->Wrap(-1);
	fgSizer1->Add(hubNameLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 5);
	
	hubNameEdit = new wxTextCtrl(this, IDC_HUBNAME, wxEmptyString, wxDefaultPosition, wxSize(250,-1), 0);
	fgSizer1->Add(hubNameEdit, 0, wxALL | wxEXPAND, 5);
	
	hubAddrLabel = new wxStaticText(this, IDC_FH_ADDRESS, CTSTRING(HUB_ADDRESS), wxDefaultPosition, wxDefaultSize, 0);
	hubAddrLabel->Wrap(-1);
	fgSizer1->Add(hubAddrLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	hubAddrEdit = new wxTextCtrl(this, IDC_HUBADDR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(hubAddrEdit, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);
	
	hubDescLabel = new wxStaticText(this, IDC_FH_HUB_DESC, CTSTRING(DESCRIPTION), wxDefaultPosition, wxDefaultSize, 0);
	hubDescLabel->Wrap(-1);
	fgSizer1->Add(hubDescLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	hubDescEdit = new wxTextCtrl(this, IDC_HUBDESCR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(hubDescEdit, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);
	
	wxStaticText* encodingLabel = new wxStaticText(this, wxID_ANY, _("Encoding"), wxDefaultPosition, wxDefaultSize, 0);
	encodingLabel->Wrap(-1);
	fgSizer1->Add(encodingLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	encodingCombo = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	fgSizer1->Add(encodingCombo, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);

	sbSizer2->Add(fgSizer1, 1, wxEXPAND, 5);
	
	bSizer4->Add(sbSizer2, 0, wxALL | wxEXPAND, 5);
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_FH_IDENT, CTSTRING(FAVORITE_HUB_IDENTITY)), wxVERTICAL);
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer(3, 2, 0, 0);
	fgSizer4->AddGrowableCol(1);
	fgSizer4->SetFlexibleDirection(wxBOTH);
	fgSizer4->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	idNickLabel = new wxStaticText(this, IDC_FH_NICK, CTSTRING(NICK), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	idNickLabel->Wrap(-1);
	fgSizer4->Add(idNickLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);
	
	idNickEdit = new wxTextCtrl(this, IDC_HUBNICK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, textValidator);
	fgSizer4->Add(idNickEdit, 0, wxALL | wxEXPAND, 5);
	
	idPassLabel = new wxStaticText(this, IDC_FH_PASSWORD, CTSTRING(PASSWORD), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	idPassLabel->Wrap(-1);
	fgSizer4->Add(idPassLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	idPassEdit = new wxTextCtrl(this, IDC_HUBPASS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, textValidator);
	fgSizer4->Add(idPassEdit, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	idDescLabel = new wxStaticText(this, IDC_FH_USER_DESC, CTSTRING(DESCRIPTION), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	idDescLabel->Wrap(-1);
	fgSizer4->Add(idDescLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	idDescEdit = new wxTextCtrl(this, IDC_HUBUSERDESCR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, textValidator);
	fgSizer4->Add(idDescEdit, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	sbSizer3->Add(fgSizer4, 0, wxEXPAND, 5);
	
	idEmulDcCheck = new wxCheckBox(this, IDC_STEALTH, CTSTRING(STEALTH_MODE), wxDefaultPosition, wxDefaultSize, 0);
	
	sbSizer3->Add(idEmulDcCheck, 0, wxALL, 5);
	
	bSizer4->Add(sbSizer3, 1, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);
	
	bSizer2->Add(bSizer4, 1, wxEXPAND, 5);
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxVERTICAL);
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Raw commands")), wxVERTICAL);
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer(5, 2, 0, 0);
	fgSizer2->AddGrowableCol(0);
	fgSizer2->SetFlexibleDirection(wxBOTH);
	fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	raw1Edit = new wxTextCtrl(this, IDC_RAW_ONE, wxEmptyString, wxDefaultPosition, wxSize(250,21), wxTE_MULTILINE);
	fgSizer2->Add(raw1Edit, 0, wxALL|wxEXPAND, 5);
	
	raw1Label = new wxStaticText(this, wxID_ANY, _("Raw 1"), wxDefaultPosition, wxDefaultSize, 0);
	raw1Label->Wrap(-1);
	fgSizer2->Add(raw1Label, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	
	raw2Edit = new wxTextCtrl(this, IDC_RAW_TWO, wxEmptyString, wxDefaultPosition, wxSize(-1,21), wxTE_MULTILINE);
	fgSizer2->Add(raw2Edit, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	raw2Label = new wxStaticText(this, wxID_ANY, _("Raw 2"), wxDefaultPosition, wxDefaultSize, 0);
	raw2Label->Wrap(-1);
	fgSizer2->Add(raw2Label, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	raw3Edit = new wxTextCtrl(this, IDC_RAW_THREE, wxEmptyString, wxDefaultPosition, wxSize(-1,21), wxTE_MULTILINE);
	fgSizer2->Add(raw3Edit, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	raw3Label = new wxStaticText(this, wxID_ANY, _("Raw 3"), wxDefaultPosition, wxDefaultSize, 0);
	raw3Label->Wrap(-1);
	fgSizer2->Add(raw3Label, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	raw4Edit = new wxTextCtrl(this, IDC_RAW_FOUR, wxEmptyString, wxDefaultPosition, wxSize(-1,21), wxTE_MULTILINE);
	fgSizer2->Add(raw4Edit, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	raw4Label = new wxStaticText(this, wxID_ANY, _("Raw 4"), wxDefaultPosition, wxDefaultSize, 0);
	raw4Label->Wrap(-1);
	fgSizer2->Add(raw4Label, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	raw5Edit = new wxTextCtrl(this, IDC_RAW_FIVE, wxEmptyString, wxDefaultPosition, wxSize(-1,21), wxTE_MULTILINE);
	fgSizer2->Add(raw5Edit, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	raw5Label = new wxStaticText(this, wxID_ANY, _("Raw 5"), wxDefaultPosition, wxDefaultSize, 0);
	raw5Label->Wrap(-1);
	fgSizer2->Add(raw5Label, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
	
	sbSizer4->Add(fgSizer2, 1, wxEXPAND, 5);
	
	bSizer5->Add(sbSizer4, 0, wxALL | wxEXPAND, 5);
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_CONN_BORDER, CTSTRING(CONNECTION)), wxVERTICAL);
	
	connDefRadio = new wxRadioButton(this, IDC_DEFAULT, CTSTRING(DEFAULT), wxDefaultPosition, wxDefaultSize, 0);
	sbSizer5->Add(connDefRadio, 0, wxALL, 5);
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxHORIZONTAL);
	
	connDirRadio = new wxRadioButton(this, IDC_ACTIVE, CTSTRING(SETTINGS_DIRECT), wxDefaultPosition, wxDefaultSize, 0);
	bSizer6->Add(connDirRadio, 0, wxALL, 5);
	
	
	bSizer6->Add(0, 0, 1, wxEXPAND, 5);
	
	connIpLabel = new wxStaticText(this, IDC_SETTINGS_IP, _("IP"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	connIpLabel->Wrap(-1);
	bSizer6->Add(connIpLabel, 0, wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	
	connIpEdit = new wxTextCtrl(this, IDC_SERVER, wxEmptyString, wxDefaultPosition, wxSize(-1,-1), 0);
	bSizer6->Add(connIpEdit, 0, wxLEFT | wxRIGHT, 5);
	
	
	bSizer6->Add(0, 0, 1, wxEXPAND, 5);
	
	sbSizer5->Add(bSizer6, 1, wxEXPAND, 5);
	
	connFwRadio = new wxRadioButton(this, IDC_PASSIVE, CTSTRING(SETTINGS_FIREWALL_PASSIVE), wxDefaultPosition, wxDefaultSize, 0);
	sbSizer5->Add(connFwRadio, 0, wxALL | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	
	bSizer5->Add(sbSizer5, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);
	
	bSizer2->Add(bSizer5, 1, wxEXPAND, 5);
	
	bSizer1->Add(bSizer2, 1, wxEXPAND, 5);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, IDC_OTHER_BORDER, wxEmptyString), wxHORIZONTAL);
	
	minSearchLabel = new wxStaticText(this, IDC_FAV_SEARCH_INTERVAL, CTSTRING(MINIMUM_SEARCH_INTERVAL), wxDefaultPosition, wxDefaultSize,
		wxALIGN_RIGHT);
	minSearchLabel->Wrap(-1);
	sbSizer1->Add(minSearchLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	
	minSearchSpin = new wxSpinCtrl(this, IDC_FAV_SEARCH_INTERVAL_SPIN, wxEmptyString, wxDefaultPosition, wxSize(60,-1),
		wxSP_ARROW_KEYS, 0, 10, 0);
	minSearchSpin->SetRange(10, 9999);
	sbSizer1->Add(minSearchSpin, 0, wxALL, 5);
	
	groupLabel = new wxStaticText(this, IDC_FAVGROUP, CTSTRING(GROUP), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	groupLabel->Wrap(-1);
	sbSizer1->Add(groupLabel, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	
	groupCombo = new wxComboBox(this, IDC_FAVGROUP_BOX, wxEmptyString, wxDefaultPosition, wxSize(200,-1), 0, NULL,
		wxCB_DROPDOWN | wxCB_READONLY); 
	sbSizer1->Add(groupCombo, 0, wxALL, 5);
	
	bSizer1->Add(sbSizer1, 0, wxBOTTOM | wxLEFT | wxRIGHT | wxEXPAND, 5);
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton(this, wxID_OK, CTSTRING(OK));
	m_sdbSizer1->AddButton(m_sdbSizer1OK);
	m_sdbSizer1Cancel = new wxButton(this, wxID_CANCEL, CTSTRING(CANCEL));
	m_sdbSizer1->AddButton(m_sdbSizer1Cancel);
	m_sdbSizer1->Realize();
	bSizer1->Add(m_sdbSizer1, 0, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);
	
	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);
	
	Center();

	// Filling values
	hubNameEdit->SetValue(Text::toT(_entry->getName()).c_str());
	hubDescEdit->SetValue(Text::toT(_entry->getDescription()).c_str());
	hubAddrEdit->SetValue(Text::toT(_entry->getServer()).c_str());
	idNickEdit->SetValue(Text::toT(_entry->getNick(false)).c_str());
	idPassEdit->SetValue(Text::toT(_entry->getPassword()).c_str());
	idDescEdit->SetValue(Text::toT(_entry->getUserDescription()).c_str());
	idEmulDcCheck->SetValue(_entry->getStealth());
	raw1Edit->SetValue(Text::toT(_entry->getRawOne()).c_str());
	raw2Edit->SetValue(Text::toT(_entry->getRawTwo()).c_str());
	raw3Edit->SetValue(Text::toT(_entry->getRawThree()).c_str());
	raw4Edit->SetValue(Text::toT(_entry->getRawFour()).c_str());
	raw5Edit->SetValue(Text::toT(_entry->getRawFive()).c_str());
	connIpEdit->SetValue(Text::toT(_entry->getIP()).c_str());
	minSearchSpin->SetValue(_entry->getSearchInterval());

	groupCombo->Append(_T("---"));
	groupCombo->SetSelection(0);

	const FavHubGroups& favHubGroups = FavoriteManager::getInstance()->getFavHubGroups();
	for(FavHubGroups::const_iterator i = favHubGroups.begin(); i != favHubGroups.end(); ++i)
	{
		const string& name = i->first;
		int pos = groupCombo->Append(wxString(Text::toT(name)).c_str());

		if(name == entry->getGroup())
			groupCombo->SetSelection(pos);
	}

	// TODO: add more encoding into wxWidgets version, this is enough now
	// FIXME: following names are Windows only!
	encodingCombo->AppendString(_("System default"));
	encodingCombo->AppendString(wxT("English_United Kingdom.1252"));
	encodingCombo->AppendString(wxT("Czech_Czech Republic.1250"));
	encodingCombo->AppendString(wxT("Russian_Russia.1251"));
	encodingCombo->AppendString(Text::toT(Text::utf8));

	if(strnicmp("adc://", entry->getServer().c_str(), 6) == 0 || strnicmp("adcs://", entry->getServer().c_str(), 7) == 0)
	{
		encodingCombo->SetSelection(4); // select UTF-8 for ADC hubs
		encodingCombo->Disable();
	}
	else
	{
		if(entry->getEncoding().empty())
			encodingCombo->SetSelection(0);
		else
			encodingCombo->SetValue(Text::toT(entry->getEncoding()));
	}

	switch (entry->getMode())
	{
		case 1:
			connDirRadio->SetValue(true);
			break;
		case 2:
			connFwRadio->SetValue(true);
			break;
		default:
			connDefRadio->SetValue(true);
			break;
	}
}

FavHubProperties::~FavHubProperties()
{
}

void FavHubProperties::onOk(wxCommandEvent &WXUNUSED(event))
{
	if (hubAddrEdit->GetValue().IsEmpty())
	{
		wxMessageBox(CTSTRING(INCOMPLETE_FAV_HUB), _T(""), wxOK | wxICON_ERROR);
	}
	else
	{
		_entry->setName(Text::fromT(hubNameEdit->GetValue()));
		_entry->setDescription(Text::fromT(hubDescEdit->GetValue()));
		_entry->setServer(Text::fromT(hubAddrEdit->GetValue()));
		_entry->setNick(Text::fromT(idNickEdit->GetValue()));
		_entry->setPassword(Text::fromT(idPassEdit->GetValue()));
		_entry->setUserDescription(Text::fromT(idDescEdit->GetValue()));
		_entry->setStealth(idEmulDcCheck->GetValue());
		_entry->setRawOne(Text::fromT(raw1Edit->GetValue()));
		_entry->setRawTwo(Text::fromT(raw2Edit->GetValue()));
		_entry->setRawThree(Text::fromT(raw3Edit->GetValue()));
		_entry->setRawFour(Text::fromT(raw4Edit->GetValue()));
		_entry->setRawFive(Text::fromT(raw5Edit->GetValue()));
		_entry->setIP(Text::fromT(connIpEdit->GetValue()));
		_entry->setSearchInterval(minSearchSpin->GetValue());

		if(groupCombo->GetSelection() == 0)
		{
			_entry->setGroup(Util::emptyString);
		}
		else
		{
			_entry->setGroup(std::string(groupCombo->GetValue()));
		}

		wxString encoding = encodingCombo->GetValue();
		if(_tcschr(encoding.c_str(), _T('.')) == NULL && _tcscmp(encoding.c_str(), Text::toT(Text::utf8).c_str()) != 0 && encodingCombo->GetSelection() != 0)
		{
			wxMessageBox(_("Invalid encoding!"), wxT(""), wxICON_WARNING | wxOK);
			return;
		}
		_entry->setEncoding(Text::fromT(encoding));

		EndModal(wxID_OK);
	}
}

void FavHubProperties::onTextUpdated(wxCommandEvent& event)
{
	wxTextCtrl *ctrl = (wxTextCtrl*)event.GetEventObject();
	wxString str = ctrl->GetValue();
	long curPos = ctrl->GetInsertionPoint();
	str.Replace(_T("$"), wxEmptyString);
	str.Replace(_T("|"), wxEmptyString);
	str.Replace(_T(" "), wxEmptyString);
	if (!ctrl->GetValue().IsSameAs(str))
	{
		ctrl->SetValue(str);
		ctrl->SetInsertionPoint(curPos);
	}
}