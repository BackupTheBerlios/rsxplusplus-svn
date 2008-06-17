/* Copyright (C) 2007-2008 iceman50
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

/* Crise:
 *  Derives from ATL's CImage now, for the sake of easy usage 
 *  and not having to worry about transparency...
 */

#if !defined(EX_CIMAGE_H)
#define EX_CIMAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef __ATLMISC_H__
#define __ATLTYPES_H__
#endif

#include "../client/Singleton.h"
#include "../client/Pointer.h"
#include "../client/FastAlloc.h"
#include <atlimage.h>

class ExCImage : public FastAlloc<ExCImage>, public CImage, public intrusive_ptr_base
{
public:
	typedef ExCImage* Ptr;
	typedef unordered_map<unsigned int, Ptr> CahceList;

	ExCImage() throw() { m_hBuffer = NULL; }

	ExCImage(LPCTSTR pszFileName) throw() {
		m_hBuffer = NULL; Load(pszFileName);
	}
	ExCImage(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL) throw() {
		m_hBuffer = NULL; LoadFromResource(id, pType, hInst);
	}
	ExCImage(UINT id, UINT type, HMODULE hInst = NULL) throw() { 
		m_hBuffer = NULL; LoadFromResource(id, MAKEINTRESOURCE(type), hInst);
	}

	~ExCImage() throw() { Destroy(); }
	void Destroy() throw();

	bool LoadFromResource(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL) throw();

private:
	HGLOBAL m_hBuffer;
};

inline void ExCImage::Destroy() throw() {
	CImage::Destroy();
	if(m_hBuffer) {
		::GlobalUnlock(m_hBuffer);
		::GlobalFree(m_hBuffer);
		m_hBuffer = NULL;
	}
}

inline bool ExCImage::LoadFromResource(UINT id, LPCTSTR pType, HMODULE hInst) throw() {
	HRSRC hResource = ::FindResource(hInst, MAKEINTRESOURCE(id), pType);
	if(!hResource)
		return false;
	
	DWORD imageSize = ::SizeofResource(hInst, hResource);
	if(!imageSize)
		return false;

	const void* pResourceData = ::LockResource(::LoadResource(hInst, hResource));
	if(!pResourceData)
		return false;

	HRESULT res = E_FAIL;
	m_hBuffer  = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
	if(m_hBuffer) {
		void* pBuffer = ::GlobalLock(m_hBuffer);
		if(pBuffer) {
			CopyMemory(pBuffer, pResourceData, imageSize);

			IStream* pStream = NULL;
			if(::CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream) == S_OK) {
				res = Load(pStream);
				pStream->Release();
				pStream = NULL;
			}
			::GlobalUnlock(m_hBuffer);
		}
		::GlobalFree(m_hBuffer);
		m_hBuffer = NULL;
	}
	return (res == S_OK);
}

/* Crise:
 *  Idea here is to load each *internal* resource only once
 *  Good, bad... useless?
 */

class ResourceLoader : public Singleton<ResourceLoader>
{
public:
	~ResourceLoader() {
		ClearCache();
		ExCImage::ReleaseGDIPlus();
	}

	ExCImage::Ptr Load(LPCTSTR pszFileName) {
		ExCImage* img = NULL;
		try {
			img = new ExCImage(pszFileName);
			img->inc();
		} catch(const Exception& e) {
			dcdebug("ResourceLoader::Load(): %s", e.getError().c_str());
		}

		return img;
	}

	ExCImage::Ptr LoadFromResource(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL) {
		// If exists in cache use that one...
		ExCImage::CahceList::const_iterator i;
		if((i = resCache.find(id)) != resCache.end()) {
			i->second->inc();
			return i->second;
		}

		// We are still here so we have new resource to handle...
		ExCImage* img = NULL;
		try {
			img = new ExCImage(id, pType, hInst);
			img->inc();

			resCache[id] = img;
			img->inc();
		} catch(const Exception& e) {
			dcdebug("ResourceLoader::LoadFromResource(): %s", e.getError().c_str());
		}

		return img;
	}

	static void Destroy(ExCImage::Ptr &aImg) {
		if(aImg != NULL) {
			if(aImg->unique()) {
				aImg->dec();
				aImg = NULL;
			} else {
				aImg->dec();
			}
		}
	}

private:
	ExCImage::CahceList resCache;

	void ClearCache() {
		ExCImage::CahceList::iterator i = resCache.begin();
		for(; !resCache.empty(); i = resCache.begin()) {
			ExCImage* img = i->second;
			resCache.erase(i);
			Destroy(img);
		}
	}
};
#define RL_LoadFromResource(resId) ResourceLoader::getInstance()->LoadFromResource(resId, _T("PNG"))
#define RL_LoadFromResourceInst(resId) ResourceLoader::getInstance()->LoadFromResource(resId, _T("PNG"), _Module.get_m_hInst())
#define RL_Load(path) ResourceLoader::getInstance()->Load(path)
#define RL_CreateImageList(img, list, id, size) img = RL_LoadFromResource(id);\
	list.Create(size, size, ILC_COLOR32 | ILC_MASK, 0, size);\
	list.Add(*img);
#define RL_DeleteObject(resPtr) ResourceLoader::Destroy(resPtr)
#endif // !defined(EX_CIMAGE_H)
