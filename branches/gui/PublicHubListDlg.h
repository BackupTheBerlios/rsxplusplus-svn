#ifndef _PUBLICHUBLISTDLG_H
#define _PUBLICHUBLISTDLG_H

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

class PublicHubListDlg : public wxDialog
{
private:
	DECLARE_EVENT_TABLE()

	enum
	{
		IDD_HUB_LIST
	};

	void EnableControls();

protected:

	wxTextCtrl* hubListEdit;
	wxButton* addButton;
	wxListBox* hubListsList;
	wxButton* upButton;
	wxButton* downButton;
	wxButton* editButton;
	wxButton* removeButton;

	wxButton* OKButton;
	wxButton* cancelButton;

	void onSelectItem(wxCommandEvent &WXUNUSED(event));
	void onAddButton(wxCommandEvent &WXUNUSED(event));
	void onUpButton(wxCommandEvent &WXUNUSED(event));
	void onDownButton(wxCommandEvent &WXUNUSED(event));
	void onEditButton(wxCommandEvent &WXUNUSED(event));
	void onRemoveButton(wxCommandEvent &WXUNUSED(event));
	void onOK(wxCommandEvent &WXUNUSED(event));


public:
	PublicHubListDlg(wxWindow* parent, wxWindowID id = IDD_HUB_LIST, const wxString& title = CTSTRING(CONFIGURED_HUB_LISTS),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
	~PublicHubListDlg();
};

#endif // _PUBLICHUBLISTDLG_H
