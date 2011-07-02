/*
 * Copyright (C) 2010 Big Muscle, http://strongdc.sf.net
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

#ifndef _TYPEDDATAVIEWCTRL_H
#define	_TYPEDDATAVIEWCTRL_H

#include <wx/dataview.h>
#include <wx/wx.h>

BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_LOCAL_EVENT_TYPE(GET_ATTRIBUTES, 13000)
END_DECLARE_EVENT_TYPES()

class wxDataViewIconTextData : public wxDataViewIconText
{
public:
		
	wxDataViewIconTextData() { }
	wxDataViewIconTextData(const wxString& text, const wxIcon& icon, void* _data) :
		wxDataViewIconText(text, icon), data(_data) { }

	~wxDataViewIconTextData() { }

	void* data;

private:
    DECLARE_DYNAMIC_CLASS(wxDataViewIconTextData)
};

DECLARE_VARIANT_OBJECT(wxDataViewIconTextData)

template <class T>
class TypedListModel : public wxDataViewVirtualListModel
{

public:
	
	TypedListModel(wxDataViewCtrl* _dataView) : dataView(_dataView), imageList(NULL) { }
	~TypedListModel() { }

	// implemented abstract methods
	unsigned int GetRowCount() const				{ return items.size(); }
	unsigned int GetColumnCount() const				{ return dataView->GetColumnCount(); }
	wxString GetColumnType(unsigned int col) const	{ return dataView->GetColumn(col)->GetRenderer()->GetVariantType(); }
	bool IsContainer(const wxDataViewItem&) const	{ return false; }

	bool SetValueByRow(const wxVariant& v, unsigned int row, unsigned int col) 
	{ 
		if(GetColumnType(col) == wxT("bool"))
		{
			Check(row, v.GetBool());
			dataView->Refresh();
			return true;
		}
		return false; 
	}

	void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const
	{
		wxIcon icon;

		// FIXME when first column is hidden we need to display icon somewhere
		if(col == 0 && imageList != NULL)
			icon = imageList->GetIcon(items[row]->getImageIndex());

		wxString colType = GetColumnType(col);
		if(colType == wxT("wxDataViewIconText"))
			variant << wxDataViewIconText(items[row]->getText(col), icon);
		else if(colType == wxT("bool"))
			variant = IsChecked(row);
		else
			variant << wxDataViewIconTextData(items[row]->getText(col), icon, items[row]);
	}

	bool GetAttrByRow(unsigned int row, unsigned int /*col*/, wxDataViewItemAttr &attr) const
	{
		wxCommandEvent evt(GET_ATTRIBUTES);
		evt.SetId(dataView->GetId());
		evt.SetClientData(&attr);
		evt.SetInt(row);
		
		// let parent to assign item colour
		return dataView->GetParent()->GetEventHandler()->ProcessEvent(evt);
	}

	void InsertItem(unsigned int index, const T* item)
	{
		items.insert(items.begin() + index, (T*)item);
		RowInserted(index);
	}

	void DeleteItem(unsigned int i)
	{
		items.erase(items.begin() + i);
		RowDeleted(i);
	}

	void DeleteAllItems()
	{
		// delete from end
		for(int i = items.size() - 1; i >= 0; --i)
			RowDeleted(i);

		items.clear();
	}

	int FindItem(const T* item) const
	{
		for(unsigned int i = 0; i < items.size(); ++i)
		{
			if(items[i] == item)
				return i;
		}

		return -1;
	}

	T* GetItemData(unsigned int index) const			{ return items[index]; }
	void SetItemData(unsigned int index, const T* item) { items[index] = (T*)item; RowChanged(index); }
	void SetImageList(wxImageList* list)				{ imageList = list; }
	virtual void Resort()								{ std::sort(items.begin(), items.end(), CompareStruct(dataView)); }

	bool IsChecked(int row) const { map<int, bool>::const_iterator i = checkboxes.find(row); return (i != checkboxes.end()) ? i->second : false; }
	void Check(int row, bool value) { checkboxes[row] = value; }

protected:

	std::vector<T*> items;
	wxDataViewCtrl* dataView;
	wxImageList*	imageList;

	map<int, bool>	checkboxes;

	// compare tool
	struct CompareStruct : public binary_function<T*, T*, bool>
	{
		CompareStruct(wxDataViewCtrl* _dataView) : dataView(_dataView) { }
		bool operator()(const T* a, const T* b) const 
		{
			wxDataViewColumn* sortingColumn = dataView->GetSortingColumn();
			if(sortingColumn == NULL) return 0;

			if(sortingColumn->IsSortOrderAscending())
				return T::compareItems(a, b, sortingColumn->GetModelColumn()) < 0;
			else
				return T::compareItems(a, b, sortingColumn->GetModelColumn()) > 0;
		}

	private:
		wxDataViewCtrl* dataView;
	};
};

template <class T>
class TypedDataViewCtrl :
	public wxDataViewCtrl
{

public:

	typedef TypedDataViewCtrl<T> thisClass;

	TypedDataViewCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) :
		wxDataViewCtrl(parent, id, pos, size, style | wxDV_MULTIPLE)
	{
		TypedListModel<T>* dataModel = new TypedListModel<T>(this);
		AssociateModel(dataModel);
		dataModel->DecRef();

		this->SetIndent(6);
	}

	~TypedDataViewCtrl(void)
	{
	}

	class iterator : public ::iterator<random_access_iterator_tag, T*> {
	public:
		iterator() : typedList(NULL), cur(0), cnt(0) { }
		iterator(const iterator& rhs) : typedList(rhs.typedList), cur(rhs.cur), cnt(rhs.cnt) { }
		iterator& operator=(const iterator& rhs) { typedList = rhs.typedList; cur = rhs.cur; cnt = rhs.cnt; return *this; }

		bool operator==(const iterator& rhs) const { return cur == rhs.cur; }
		bool operator!=(const iterator& rhs) const { return !(*this == rhs); }
		bool operator<(const iterator& rhs) const { return cur < rhs.cur; }

		int operator-(const iterator& rhs) const { 
			return cur - rhs.cur;
		}

		iterator& operator+=(int n) { cur += n; return *this; }
		iterator& operator-=(int n) { return (cur += -n); }
		
		T& operator*() { return *typedList->getItemData(cur); }
		T* operator->() { return &(*(*this)); }
		T& operator[](int n) { return *typedList->getItemData(cur + n); }
		
		iterator operator++(int) {
			iterator tmp(*this);
			operator++();
			return tmp;
		}
		iterator& operator++() {
			++cur;
			return *this;
		}

	private:
		iterator(thisClass* aTypedList) : typedList(aTypedList), cur(0), cnt(aTypedList->getItemCount()) { 
			if(cur == -1)
				cur = cnt;
		}
		iterator(thisClass* aTypedList, int first) : typedList(aTypedList), cur(first), cnt(aTypedList->getItemCount()) { 
			if(cur == -1)
				cur = cnt;
		}
		friend class thisClass;
		thisClass* typedList;
		int cur;
		int cnt;
	};

	iterator begin() { return iterator(this); }
	iterator end() { return iterator(this, getModel()->GetRowCount()); }

	void InsertColumn(unsigned int index, const tstring& title, wxAlignment format, int width, wxDataViewRenderer* renderer = NULL)
	{
		if(width == 0) 
			width = 80;
			
		if(renderer == NULL)
			renderer = getDefaultRenderer(index);

        wxDataViewColumn* column = new wxDataViewColumn(title, renderer, index, width, format,
			wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE | wxDATAVIEW_COL_RESIZABLE );

		AppendColumn(column);
	}

	virtual wxDataViewRenderer* getDefaultRenderer(int /*column*/) { return new wxDataViewIconTextRenderer; }

	int insertItem(const T* item) 
	{
		return insertItem(getSortPos(item), item);
	}

	int insertItem(int index, const T* item)
	{
		getModel()->InsertItem(index, item);
		return index;
	}

	void deleteItem(unsigned int index)
	{
		getModel()->DeleteItem(index);
	}

	void deleteItem(const T* item) 
	{ 
		int i = findItem(item); 
		if(i != -1) 
			deleteItem(i); 
	}

	void deleteAllItems()								{ getModel()->DeleteAllItems(); }
	void updateItem(unsigned int index)					{ /*getModel()->RowChanged(index);*/ Refresh(); }
	void updateItem(unsigned int index, int col)		{ /*getModel()->RowChanged(index, col);*/ Refresh(); }
	void updateItem(const T* item)						{ int i = findItem(item); if(i != -1) updateItem(i); }
	void setItemData(unsigned int index, const T* item) { getModel()->SetItemData(index, item); }

	int findItem(const T* item) const					{ return getModel()->FindItem(item); }
	T* getItemData(unsigned int index) const			{ return getModel()->GetItemData(index);	}
	T* getItemData(const wxDataViewItem& item) const	{ return getModel()->GetItemData(GetRowByItem(item)); }
	unsigned int getItemCount() const					{ return getModel()->GetRowCount(); }
	int getSortColumn() const							{ return GetSortingColumnIndex(); }

	bool isChecked(int row) const						{ return getModel()->IsChecked(row); }
	void check(int row, bool value)						{ getModel()->Check(row, value); }

	void setSortColumn(int column, bool ascending = true) 
	{ 
		this->SetSortingColumnIndex(column);
		this->GetColumn(column)->SetSortOrder(ascending);
		this->GetColumn(column)->SetAsSortKey();
	}

	void resort() 
	{
		if(GetSortingColumnIndex() != -1) 
		{
			getModel()->Resort();
		}
	}

	void forEach(void (T::*func)()) 
	{
		unsigned int n = GetItemCount();
		for(unsigned int i = 0; i < n; ++i)
			(getItemData(i)->*func)();
	}

	void forEachSelected(void (T::*func)()) 
	{
		wxDataViewItemArray sel;
		GetSelections(sel);
		for(unsigned int i = 0; i < sel.size(); ++i)
			(getItemData(GetRowByItem(sel[i]))->*func)();
	}

	template<class _Function>
	_Function forEachT(_Function pred) 
	{
		unsigned int n = getItemCount();
		for(unsigned int i = 0; i < n; ++i)
			pred(getItemData(i));
		return pred;
	}

	template<class _Function>
	_Function forEachSelectedT(_Function pred) 
	{
		wxDataViewItemArray sel;
		GetSelections(sel);
		for(unsigned int i = 0; i < sel.size(); ++i)
			pred(getItemData(GetRowByItem(sel[i])));
		return pred;
	}

	void forEachAtPos(int iIndex, void (T::*func)()) 
	{
		(getItemData(iIndex)->*func)();
	}

	void setImageList(wxImageList* list) { getModel()->SetImageList(list); }

	void select(int index) { this->Select(index); }
	void unselect(int index) { this->Unselect(index); }
	void ensureVisible(int index) { this->EnsureVisible(index, 0); }
	int getFirstSelectedItem() const { return GetRowByItem(this->GetSelection()); }

	wxMenu* getCopyMenu() const
	{
		wxMenu* copyMenu = new wxMenu();
		copyMenu->SetTitle(TSTRING(COPY));

		for(unsigned int i = 0; i < GetColumnCount(); ++i) 
		{
			copyMenu->Append(IDC_COPY + i, GetColumn(i)->GetTitle());
		}

		return copyMenu;
	}

private:

	DECLARE_EVENT_TABLE();
		
	// methods
	TypedListModel<T>* getModel() { return (TypedListModel<T>*)GetModel(); }
	const TypedListModel<T>* getModel() const { return (const TypedListModel<T>*)GetModel(); }

	int getSortPos(const T* a) const 
	{
		wxDataViewColumn* sortingColumn = this->GetSortingColumn();

		int high = getModel()->GetRowCount();
		if((sortingColumn == NULL) || (high == 0))
			return high;

		high--;

		int low = 0;
		int mid = 0;
		T* b = NULL;
		int comp = 0;
		while( low <= high ) {
			mid = (low + high) / 2;
			b = getItemData(mid);
			comp = T::compareItems(a, b, static_cast<uint8_t>(sortingColumn->GetModelColumn()));

			if(!sortingColumn->IsSortOrderAscending())
				comp = -comp;

			if(comp == 0) {
				return mid;
			} else if(comp < 0) {
				high = mid - 1;
			} else if(comp > 0) {
					low = mid + 1;
			}
		}

		comp = T::compareItems(a, b, static_cast<uint8_t>(sortingColumn->GetModelColumn()));
		if(!sortingColumn->IsSortOrderAscending())
			comp = -comp;
		if(comp > 0)
			mid++;

		return mid;
	}

	void OnHeaderContextMenu(wxDataViewEvent& /*event*/)
	{
		wxMenu headerMenu;

		for(unsigned int i = 0; i < GetColumnCount(); ++i)
		{
			wxDataViewColumn* column = GetColumn(i);
			wxMenuItem* item = headerMenu.AppendCheckItem(IDC_HEADER_MENU + i, column->GetTitle());
			if(!column->IsHidden())
				item->Check(true);

			Connect(IDC_HEADER_MENU + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(TypedDataViewCtrl<T>::OnHeaderMenuItem));
		}
		PopupMenu(&headerMenu);	
	}

	void OnHeaderMenuItem(wxCommandEvent& event)
	{
		wxDataViewColumn* column = GetColumn(event.GetId() - IDC_HEADER_MENU);
		column->SetHidden(!column->IsHidden());
	}
};

BEGIN_EVENT_TABLE_TEMPLATE1(TypedDataViewCtrl, wxDataViewCtrl, T)
	EVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICKED(wxID_ANY, TypedDataViewCtrl<T>::OnHeaderContextMenu)
END_EVENT_TABLE()

#endif