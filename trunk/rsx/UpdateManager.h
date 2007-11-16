/*
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

#ifndef UPDATE_MANAGER_H
#define UPDATE_MANAGER_H

#include "../client/Singleton.h"
#include "../client/HttpConnection.h"
#include "../client/SimpleXML.h"

typedef map<int8_t, string> UpdateMap;

class UpdateManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Complete;
	typedef X<1> Failed;

	virtual void on(Complete, int /*file*/) throw() { };
	virtual void on(Failed, int /*file*/, const string& /*reason*/) throw() { };
};

class UpdateManager : public Singleton<UpdateManager>, public Speaker<UpdateManagerListener>, private HttpConnectionListener {
public:
	UpdateManager();
	~UpdateManager();	
	enum Types { 
		CLIENT = 0, 
		MYINFO, 
		IPWATCH
	};

	void reloadFile(int file);
	void restoreOld(int file);
	void updateFiles(UpdateMap& aUrls);
	void startDownload();

private:
	friend class Singleton<UpdateManager>;

	HttpConnection c;
	UpdateMap profileList;
	int currentlyUpdating;
	string downBuf;

	// HttpConnectionListener
	void on(HttpConnectionListener::Complete, HttpConnection* /*conn*/, const string& /*aLine*/) throw();
	void on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw();	
	void on(HttpConnectionListener::Failed, HttpConnection* /*conn*/, const string& aLine) throw();
};
#endif