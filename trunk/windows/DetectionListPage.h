#ifndef RSXPLUSPLUS_DETECTION_PAGE
#define RSXPLUSPLUS_DETECTION_PAGE

#include "../client/DetectionEntry.h"

class DetectionListPage : public CPropertyPage<IDD_DETECTION_LIST_PAGE>, public PropPage {
public:
	DetectionListPage(SettingsManager *s) : PropPage(s) {
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT) + _T('\\') + TSTRING(SETTINGS_CLIENTS)).c_str());
		SetTitle(title);
		m_psp.dwFlags |= PSP_RTLREADING;
	};

	~DetectionListPage() { 
		ctrlList.Detach();
		free(title);
	};

	BEGIN_MSG_MAP(DetectionListPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_DETECTION_LIST_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_DETECTION_LIST_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_DETECTION_LIST_EDIT, onEdit)
		//COMMAND_ID_HANDLER(IDC_MOVE_CLIENT_UP, onMoveClientUp)
		//COMMAND_ID_HANDLER(IDC_MOVE_CLIENT_DOWN, onMoveClientDown)
		//COMMAND_ID_HANDLER(IDC_RELOAD_CLIENTS, onReload)
		//MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu) //RSX++
		//NOTIFY_HANDLER(IDC_CLIENT_ITEMS, NM_CUSTOMDRAW, onCustomDraw)
		//NOTIFY_HANDLER(IDC_CLIENT_ITEMS, NM_DBLCLK, onDblClick)
		//NOTIFY_HANDLER(IDC_CLIENT_ITEMS, LVN_GETINFOTIP, onInfoTip)
		NOTIFY_HANDLER(IDC_DETECTION_LIST, LVN_ITEMCHANGED, onItemChanged)
		//NOTIFY_HANDLER(IDC_CLIENT_ITEMS, LVN_KEYDOWN, onKeyDown)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	//LRESULT onMoveClientUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//LRESULT onMoveClientDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//LRESULT onReload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//LRESULT onInfoTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	//LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/); //RSX++
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	//LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);

	//LRESULT onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled) {
	//	return onChangeClient(0, 0, 0, bHandled);
	//}
	//LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);

	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();

private:
	void updateColumns(const tstring& name, const tstring& cheat, const tstring& action);

	ExListViewCtrl ctrlList;
	static TextItem texts[];
	TCHAR* title;

	void addEntry(const DetectionEntry& entry, int pos);
private:
	//void reload();
};

#endif