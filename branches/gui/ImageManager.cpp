/* 
 * Copyright (C) 2010-2011 adrian_007 adrian-007 on o2 point pl
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
#include "ImageManager.h"
#include "../res/images.h"

#define LOAD_PNG(name) wxMEMORY_BITMAPEX(images::name, wxBITMAP_TYPE_PNG)

ImageManager::ImageManager(void)
{
	splash = LOAD_PNG(rsx_splash);
	toolbar20 = LOAD_PNG(toolbar20);
	toolbar = LOAD_PNG(toolbar);
	emoticon = LOAD_PNG(emoticon);
	users = LOAD_PNG(users);
	folders = LOAD_PNG(folders);
}

ImageManager::~ImageManager(void)
{
}
