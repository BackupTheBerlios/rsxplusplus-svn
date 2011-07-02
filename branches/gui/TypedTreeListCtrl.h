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

#ifndef _TYPEDTREELISTCTRL_H
#define _TYPEDTREELISTCTRL_H

#include "TypedDataViewCtrl.h"

/*
 * TypedTreeListCtrl class
 */
template<class T, class K, class hashFunc, class equalKey>
class TypedTreeListCtrl : public TypedDataViewCtrl<T> 
{
	/*
	 * Model designed for sorting items in TypedTreeListCtrl
	 */
	class TypedTreeListModel : public TypedListModel<T>
	{
	public:
		TypedTreeListModel(wxDataViewCtrl* _dataView) : TypedListModel<T>(_dataView) { }

		void Resort() { std::sort(items.begin(), items.end(), CompareStruct(dataView)); }

	private:
		struct CompareStruct : public binary_function<T*, T*, bool>
		{
			CompareStruct(wxDataViewCtrl* _list) : list(_list) { }
			bool operator()(const T* a, const T* b) const 
			{
				wxDataViewColumn* sortingColumn = list->GetSortingColumn();
				int result = TypedTreeListCtrl<T, K, hashFunc, equalKey>::compareItems(a, b, sortingColumn->GetModelColumn());

				if(result == 2)
					result = (sortingColumn->IsSortOrderAscending() ? 1 : -1);
				else if(result == -2)
					result = (sortingColumn->IsSortOrderAscending() ? -1 : 1);

				return (sortingColumn->IsSortOrderAscending() ? (result < 0) : (result > 0));
			}

		private:
			wxDataViewCtrl* list;
		};
	};

	/*
	 * Class for rendering state icons into TypedTreeListCtrl
	 */
	class TreeRenderer : public wxDataViewCustomRenderer
	{
	public:
	
		TreeRenderer(TypedTreeListCtrl<T, K, hashFunc, equalKey>* _treeList) : 
			wxDataViewCustomRenderer(wxT("string"), wxDATAVIEW_CELL_ACTIVATABLE), treeList(_treeList)
		{
			states.Create(16, 16, false);
			states.Add(wxBitmap(wxT("IDB_STATE")));
		}

		wxSize GetSize() const { return wxSize(16, 16); } // <-- what is this for?
		bool GetValue(wxVariant&) const { return true; }

		bool SetValue(const wxVariant& _value) 
		{ 
			value << _value; 
			return true;
		}

		bool Render(wxRect rect, wxDC *dc, int state)
		{
			T* item = (T*)value.data;
			int offset = rect.GetLeft();
			if(item->parent == NULL)
			{
				// render parent item
				offset += 24;

				// do we have any children?
				const vector<T*>& children = treeList->findChildren(item->getGroupCond());
				if(children.size() > (size_t)(treeList->uniqueParent ? 1 : 0))
					states.Draw(((T*)value.data)->collapsed ? 0 : 1, *dc, 3, rect.GetY());
			}
			else
			{
				// render child item
				offset += 40;
			}
			
			if(value.GetIcon().IsOk())
				dc->DrawIcon(value.GetIcon(), offset, rect.GetY());
			RenderText(value.GetText(), offset + 16, rect, dc, state);

			return true;
		}
	
		bool LeftClick(wxPoint cursor, wxRect /*cell*/, wxDataViewModel* /*model*/, const wxDataViewItem& item, unsigned int /*col*/)
		{
			if(cursor.x < 16)
				treeList->OnExpander(treeList->GetRowByItem(item));

			return true;
		}

		bool Activate(wxRect /*cell*/, wxDataViewModel *model, const wxDataViewItem &item, unsigned int /*col*/)
		{
		    // process this event in treeList else we wouldn't able to catch doubleclicks in first column
            wxDataViewEvent le(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, treeList->GetId());
            le.SetItem(item);
            le.SetEventObject(treeList);
            le.SetModel(model);

			treeList->GetEventHandler()->ProcessEvent(le);

			return true;
		}

	private:
		
		wxDataViewIconTextData value;

		/** +/- images */
		wxImageList	states;

		TypedTreeListCtrl<T, K, hashFunc, equalKey>* treeList;
	};

public:

	TypedTreeListCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) :
		TypedDataViewCtrl(parent, id, pos, size, style)
	{
		TypedTreeListModel* dataModel = new TypedTreeListModel(this);
		AssociateModel(dataModel);
		dataModel->DecRef();
	}

	~TypedTreeListCtrl() { }

	wxDataViewRenderer* getDefaultRenderer(int column) 
	{ 
		if(column == 0)
			return new TreeRenderer(this);
		else
			return new wxDataViewIconTextRenderer();
	}

	typedef TypedTreeListCtrl<T, K, hashFunc, equalKey> thisClass;
	
	struct ParentPair 
	{
		T* parent;
		vector<T*> children;
	};

	typedef unordered_map<K*, ParentPair, hashFunc, equalKey> ParentMap;

	void Collapse(T* parent, int itemPos) 
	{
		Freeze();
		const vector<T*>& children = findChildren(parent->getGroupCond());
		for(vector<T*>::const_iterator i = children.begin(); i != children.end(); i++) 
		{
			deleteItem(*i);
		}
		parent->collapsed = true;
		// TODO: SetItemState(itemPos, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
		Thaw();
	}

	void Expand(T* parent, int itemPos) 
	{
		Freeze();
		const vector<T*>& children = findChildren(parent->getGroupCond());
		if(children.size() > (size_t)(uniqueParent ? 1 : 0)) 
		{
			parent->collapsed = false;
			for(vector<T*>::const_iterator i = children.begin(); i != children.end(); i++) 
			{
				insertChild(*i, itemPos + 1);
			}
			// TODO SetItemState(itemPos, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
			resort();
		}
		Thaw();
	}

	void insertChild(const T* item, int idx) 
	{
		/*LV_ITEM lvi;
		lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_INDENT;
		lvi.iItem = idx;
		lvi.iSubItem = 0;
		lvi.iIndent = 1;
		lvi.pszText = LPSTR_TEXTCALLBACK;
		lvi.iImage = item->getImageIndex();
		lvi.lParam = (LPARAM)item;
		lvi.state = 0;
		lvi.stateMask = 0;
		InsertItem(&lvi);*/
		insertItem(idx, item);
	}

	inline T* findParent(const K& groupCond) const 
	{
		ParentMap::const_iterator i = parents.find(const_cast<K*>(&groupCond));
		return i != parents.end() ? (*i).second.parent : NULL;
	}

	static const vector<T*> emptyVector;
	inline const vector<T*>& findChildren(const K& groupCond) const 
	{
		ParentMap::const_iterator i = parents.find(const_cast<K*>(&groupCond));
		return i != parents.end() ? (*i).second.children : emptyVector;
	}

	inline ParentPair* findParentPair(const K& groupCond) 
	{
		ParentMap::iterator i = parents.find(const_cast<K*>(&groupCond));
		return i != parents.end() ? &((*i).second) : NULL;
	}

	void insertGroupedItem(T* item, bool autoExpand) 
	{
		T* parent = NULL;
		ParentPair* pp = findParentPair(item->getGroupCond());

		int pos = -1;

		if(pp == NULL) 
		{
			parent = item;

			ParentPair newPP = { parent };
			parents.insert(make_pair(const_cast<K*>(&parent->getGroupCond()), newPP));

			parent->parent = NULL; // ensure that parent of this item is really NULL
			insertItem(getSortPos(parent), parent);
			return;
		} 
		else if(pp->children.empty()) 
		{
			T* oldParent = pp->parent;
			parent = oldParent->createParent();
			if(parent != oldParent) 
			{
				uniqueParent = true;
				parents.erase(const_cast<K*>(&oldParent->getGroupCond()));
				deleteItem(oldParent);

				ParentPair newPP = { parent };
				pp = &(parents.insert(make_pair(const_cast<K*>(&parent->getGroupCond()), newPP)).first->second);

				parent->parent = NULL; // ensure that parent of this item is really NULL
				oldParent->parent = parent;
				pp->children.push_back(oldParent); // mark old parent item as a child
				parent->hits++;

				pos = insertItem(getSortPos(parent), parent);
			} 
			else 
			{
				uniqueParent = false;
				pos = findItem(parent);
			}

			if(pos != -1)
			{
				if(autoExpand)
				{
					// TODO SetItemState(pos, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
					parent->collapsed = false;
				} 
				else 
				{
					// TODO SetItemState(pos, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
				}
			}
		} else {
			parent = pp->parent;
			pos = findItem(parent);
		}

		pp->children.push_back(item);
		parent->hits++;
		item->parent = parent;

		if(pos != -1) 
		{
			if(!parent->collapsed) 
			{
				insertChild(item, pos + pp->children.size());
			}
			updateItem(pos);
		}
	}

	void removeParent(T* parent) {
		ParentPair* pp = findParentPair(parent->getGroupCond());
		if(pp) 
		{
			for(vector<T*>::iterator i = pp->children.begin(); i != pp->children.end(); i++) 
			{
				deleteItem(*i);
				delete *i;
			}
			pp->children.clear();
			parents.erase(const_cast<K*>(&parent->getGroupCond()));
		}
		deleteItem(parent);
	}

	void removeGroupedItem(T* item, bool removeFromMemory = true) 
	{
		if(!item->parent) 
		{
			removeParent(item);
		} 
		else 
		{
			T* parent = item->parent;
			ParentPair* pp = findParentPair(parent->getGroupCond());

			deleteItem(item);

			vector<T*>::iterator n = find(pp->children.begin(), pp->children.end(), item);
			if(n != pp->children.end()) 
			{
				pp->children.erase(n);
				pp->parent->hits--;
			}
	
			if(uniqueParent) 
			{
				dcassert(!pp->children.empty());
				if(pp->children.size() == 1) 
				{
					const T* oldParent = parent;
					parent = pp->children.front();

					deleteItem(oldParent);
					parents.erase(const_cast<K*>(&oldParent->getGroupCond()));
					delete oldParent;

					ParentPair newPP = { parent };
					parents.insert(make_pair(const_cast<K*>(&parent->getGroupCond()), newPP));

					parent->parent = NULL; // ensure that parent of this item is really NULL
					deleteItem(parent);
					insertItem(getSortPos(parent), parent);
				}
			} 
			else 
			{
				if(pp->children.empty()) 
				{
					// TODO SetItemState(findItem(parent), INDEXTOSTATEIMAGEMASK(0), LVIS_STATEIMAGEMASK);
				}
			}

			updateItem(parent);
		}

		if(removeFromMemory)
			delete item;
	}

	void deleteAllItems(bool clear = true) 
	{
		if(clear)
		{
			// HACK: ugly hack but at least it doesn't crash and there's no memory leak
			for(ParentMap::iterator i = parents.begin(); i != parents.end(); ++i) 
			{
				T* ti = (*i).second.parent;
				for(vector<T*>::iterator j = (*i).second.children.begin(); j != (*i).second.children.end(); ++j) 
				{
					deleteItem(*j);
					delete *j;
				}
				deleteItem(ti);
				delete ti;
			}
			for(unsigned int i = 0; i < getItemCount(); ++i) 
			{
				T* si = getItemData(i);
				delete si;
			}

 			parents.clear();
		}
		TypedDataViewCtrl<T>::deleteAllItems();
	}

	int getSortPos(const T* a) 
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
		while( low <= high ) 
		{
			mid = (low + high) / 2;
			b = getItemData(mid);
			comp = compareItems(a, b, static_cast<uint8_t>(sortingColumn->GetModelColumn()));
			
			if(!sortingColumn->IsSortOrderAscending())
				comp = -comp;

			if(comp == 0) 
			{
				return mid;
			} 
			else if(comp < 0) 
			{
				high = mid - 1;
			} 
			else if(comp > 0) 
			{
				low = mid + 1;
			} 
			else if(comp == 2)
			{
				if(sortingColumn->IsSortOrderAscending())
					low = mid + 1;
				else
					high = mid -1;
			} 
			else if(comp == -2)
			{
				if(!sortingColumn->IsSortOrderAscending())
					low = mid + 1;
				else
					high = mid -1;
			}
		}

		comp = compareItems(a, b, static_cast<uint8_t>(sortingColumn->GetModelColumn()));
		if(!sortingColumn->IsSortOrderAscending())
			comp = -comp;
		if(comp > 0)
			mid++;

		return mid;
	}

	ParentMap& getParents() { return parents; }

private:

	void OnExpander(int item) 
	{
		T* i = getItemData(item);
		if(i->parent == NULL) 
		{
			if(i->collapsed) 
			{
				Expand(i, item);
			}
			else 
			{
				Collapse(i, item);
			}
		}
	}

   	/** map of all parent items with their associated children */
	ParentMap	parents;

	/** is extra item needed for parent items? */
	bool		uniqueParent;

	// Copyright (C) Liny, RevConnect
	static int compareItems(const T* a, const T* b, uint8_t col) 
	{
		// both are children
		if(a->parent && b->parent)
		{
			// different parent
			if(a->parent != b->parent)
				return compareItems(a->parent, b->parent, col);			
		}
		else
		{
			if(a->parent == b)
				return 2;  // a should be displayed below b

			if(b->parent == a)
				return -2; // b should be displayed below a

			if(a->parent)
				return compareItems(a->parent, b, col);	

			if(b->parent)
				return compareItems(a, b->parent, col);	
		}

		return T::compareItems(a, b, col);
	}
};

template<class T, class K, class hashFunc, class equalKey>
const vector<T*> TypedTreeListCtrl<T, K, hashFunc, equalKey>::emptyVector;

#endif	// _TYPEDTREELISTCTRL_H
