#ifndef RSXPLUSPLUS_DETECTION_LIST_ITEM
#define RSXPLUSPLUS_DETECTION_LIST_ITEM

#include "ExListViewCtrl.h"
#include "CRawCombo.h"

class DetectionListItem : public CDialogImpl<DetectionListItem> {
public:
	enum { IDD = IDD_DETECTION_LIST_ITEM };
	DetectionListItem() : isNew(false) { }
	
	bool isNew;
	DetectionEntry tmpEntry;

	BEGIN_MSG_MAP(DetectionListItem)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_DETECTION_ITEM_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_DETECTION_ITEM_CHANGE, onChange)
		COMMAND_ID_HANDLER(IDC_DETECTION_ITEM_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDOK, onClose)
		COMMAND_ID_HANDLER(IDCANCEL, onClose)
		NOTIFY_HANDLER(IDC_INF_LIST, LVN_ITEMCHANGED, onItemChanged)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
private:
	CRawCombo cRaw;

	void initData();
	bool saveData();

	void addItem(const tstring& first, const tstring& second, int pos) {
		TStringList l;
		l.push_back(first);
		l.push_back(second);
		ctrlList.insert(pos, l, 0, 0);
	}
	void enableFieldsEdit(bool enable);

	CEdit cCheat, cComment, cId, cField, cPattern;
	ExListViewCtrl ctrlList;
};
#endif