#ifndef RSXPLUSPLUS_RAW_COMBO_BOX
#define RSXPLUSPLUS_RAW_COMBO_BOX

class CRawCombo : public CComboBox {
public:
	CRawCombo() : settingId(0) { }
	~CRawCombo();
	
	void attach(HWND hWnd, const int actionId);
	int getActionId() const;
	tstring getActionName(const int actionId) const;

	inline void setPos(const int actionId) {
		SetCurSel(getActionPos(actionId));
	}
	inline void setSettingId(const int Id) {
		settingId = Id;
	}
private:
	typedef std::tr1::unordered_map<int, int> ActionMap;

	int getActionPos(const int actionId) const;

	ActionMap map;
	int settingId;
};

#endif