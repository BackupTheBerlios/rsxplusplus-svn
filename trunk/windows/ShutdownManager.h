/*
 * Copyright (C) 2007-2010 adrian_007, adrian-007 on o2 point pl
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

#ifndef RSXPLUSPLUS_SHUTDOWN_MANAGER
#define RSXPLUSPLUS_SHUTDOWN_MANAGER

#include "stdafx.h"
#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/QueueManager.h"

class ShutdownManager {
public:
	enum Action {
		SHUTDOWN = 0,
		LOGOFF, 
		REBOOT,
		SUSPEND,
		HIBERNATE,
		LOCK,
		CLOSE_APP
	};

	enum Type {
		DL_COMPLETE = 0,
		TRANSFERS_COMPLETE,
		QUEUE_EMPTY,
		DELAY
	};

	ShutdownManager(int a, int t, uint64_t d) : act((Action)a), type((Type)t), delay(d) { 
		if(type == DELAY && delay <= 0) delay = 1;
	};
	~ShutdownManager() { };

	uint64_t getTimeLeft(bool& close, bool& shutdown) {
		switch(type) {
			case DL_COMPLETE: {
				shutdown = DownloadManager::getInstance()->getDownloadCount() == 0;
				break;
			}
			case TRANSFERS_COMPLETE: {
				shutdown = DownloadManager::getInstance()->getDownloadCount() == 0 && UploadManager::getInstance()->getUploadCount() == 0;
				break;
			}
			case QUEUE_EMPTY: {
				shutdown = QueueManager::getInstance()->getQueueSize(false) == 0;
				break;
			}
			case DELAY: {
				shutdown = --delay == 0;
				break;
			}
			default: shutdown = false;
		}

		close = act == CLOSE_APP;
		return delay;
	}

	inline void shutdown() {
		WinUtil::shutDown((int)act);
	}
private:
	uint64_t delay;
	Action act;
	Type type;
};

#endif

/**
 * @file
 * $Id$
 */
