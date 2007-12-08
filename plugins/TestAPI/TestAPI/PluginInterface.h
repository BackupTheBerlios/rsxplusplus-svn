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

#ifndef PLUGIN_INTERFACE
#define PLUGIN_INTERFACE

class Client;
/* via this interface plugin recive:
 * 1. gui events
 * 2. hub incomming commands
 * 3. user outgoing commands (from hub frame)
 */

class iPlugin {
public:
	virtual bool onIncommingMessage(Client* c, const string& msg) = 0;
	virtual bool onOutgoingMessage(Client* c, const string& msg) = 0;
	virtual void onToolbarClick() = 0;
};
#endif