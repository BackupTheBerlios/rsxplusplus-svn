#include "stdafx.h"

#include "../client/SettingsManager.h"
#include "../client/Text.h"
#include "../client/FavoriteManager.h"

#include "PublicHubListDlg.h"

BEGIN_EVENT_TABLE(PublicHubListDlg, wxDialog)
	EVT_LISTBOX(IDC_LIST_LIST, PublicHubListDlg::onSelectItem)
	EVT_BUTTON(IDC_LIST_ADD, PublicHubListDlg::onAddButton)
	EVT_BUTTON(IDC_LIST_UP, PublicHubListDlg::onUpButton)
	EVT_BUTTON(IDC_LIST_DOWN, PublicHubListDlg::onDownButton)
	EVT_BUTTON(IDC_LIST_EDIT, PublicHubListDlg::onEditButton)
	EVT_BUTTON(IDC_LIST_REMOVE, PublicHubListDlg::onRemoveButton)
	EVT_BUTTON(wxID_OK, PublicHubListDlg::onOK)
END_EVENT_TABLE()

PublicHubListDlg::PublicHubListDlg(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos,
	const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(2, 2, 0, 0);
	fgSizer1->AddGrowableCol(0);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	hubListEdit = new wxTextCtrl(this, IDC_LIST_EDIT_BOX, wxEmptyString, wxDefaultPosition, wxSize(270,-1), 0);
	fgSizer1->Add(hubListEdit, 0, wxALL | wxEXPAND, 5);

	addButton = new wxButton(this, IDC_LIST_ADD, CTSTRING(ADD), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(addButton, 0, wxALL, 5);

	hubListsList = new wxListBox(this, IDC_LIST_LIST, wxDefaultPosition, wxSize(270,224), 0, NULL, wxLB_SINGLE);
	fgSizer1->Add(hubListsList, 0, wxALL, 5);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	upButton = new wxButton(this, IDC_LIST_UP, CTSTRING(MOVE_UP), wxDefaultPosition, wxDefaultSize, 0);
	upButton->Enable(false);
	bSizer1->Add(upButton, 0, wxALL, 5);

	downButton = new wxButton(this, IDC_LIST_DOWN, CTSTRING(MOVE_DOWN), wxDefaultPosition, wxDefaultSize, 0);
	downButton->Enable(false);
	bSizer1->Add(downButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	editButton = new wxButton(this, IDC_LIST_EDIT, CTSTRING(EDIT_ACCEL), wxDefaultPosition, wxDefaultSize, 0);
	editButton->Enable(false);
	bSizer1->Add(editButton, 0, wxALL, 5);

	removeButton = new wxButton(this, IDC_LIST_REMOVE, CTSTRING(REMOVE), wxDefaultPosition, wxDefaultSize, 0);
	removeButton->Enable(false);
	bSizer1->Add(removeButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	bSizer1->Add(0, 0, 1, wxEXPAND, 5);

	OKButton = new wxButton(this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(OKButton, 0, wxALL, 5);

	cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(cancelButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

	fgSizer1->Add(bSizer1, 1, wxEXPAND, 5);

	this->SetSizer(fgSizer1);
	this->Layout();
	fgSizer1->Fit(this);

	StringList lists(FavoriteManager::getInstance()->getHubLists());
	for(StringList::const_iterator idx = lists.begin(); idx != lists.end(); ++idx)
	{
		hubListsList->Append(Text::toT(*idx));
	}
}

PublicHubListDlg::~PublicHubListDlg()
{
}

void PublicHubListDlg::onSelectItem(wxCommandEvent &WXUNUSED(event))
{
	EnableControls();
}

void PublicHubListDlg::onAddButton(wxCommandEvent &WXUNUSED(event))
{
	if (!hubListEdit->GetValue().IsEmpty())
	{
		hubListsList->Append(hubListEdit->GetValue());
	}
}

void PublicHubListDlg::onUpButton(wxCommandEvent &WXUNUSED(event))
{
	int index = hubListsList->GetSelection();
	if (index > 0)
	{
		wxString itemToMove = hubListsList->GetStringSelection();
		hubListsList->Delete(index);
		hubListsList->Insert(itemToMove, index - 1);
		hubListsList->SetSelection(index - 1);
	}
}

void PublicHubListDlg::onDownButton(wxCommandEvent &WXUNUSED(event))
{
	unsigned int index = hubListsList->GetSelection();
	if (index < hubListsList->GetCount() - 1)
	{
		wxString itemToMove = hubListsList->GetStringSelection();
		hubListsList->Delete(index);
		hubListsList->Insert(itemToMove, index + 1);
		hubListsList->SetSelection(index + 1);
	}
}

void PublicHubListDlg::onEditButton(wxCommandEvent &WXUNUSED(event))
{
	wxString newList = wxGetTextFromUser(CTSTRING(HUB_LIST_EDIT), CTSTRING(HUB_LIST), hubListsList->GetStringSelection());
	if (!newList.IsEmpty())
	{
		int index = hubListsList->GetSelection();
		hubListsList->Delete(index);
		hubListsList->Insert(newList, index);
		EnableControls();
	}
}

void PublicHubListDlg::onRemoveButton(wxCommandEvent &WXUNUSED(event))
{
	int index = hubListsList->GetSelection();
	if (index != -1)
	{
		hubListsList->Delete(index);
		EnableControls();
	}
}

void PublicHubListDlg::onOK(wxCommandEvent &WXUNUSED(event))
{
	string tmp;
	unsigned int count = hubListsList->GetCount();
	for (unsigned int i = 0; i < count; ++i)
	{
		tmp += string(hubListsList->GetString(i).mb_str()) + ';';
	}
	if(count > 0)
	{
		tmp.erase(tmp.size() - 1);
	}
	SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, tmp);
	EndModal(wxID_OK);
}

void PublicHubListDlg::EnableControls()
{
	if (hubListsList->GetSelection() == -1)
	{
		upButton->Enable(false);
		downButton->Enable(false);
		editButton->Enable(false);
		removeButton->Enable(false);
	}
	else
	{
		upButton->Enable(true);
		downButton->Enable(true);
		editButton->Enable(true);
		removeButton->Enable(true);
	}
}
