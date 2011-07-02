#ifndef _SYSTEMLOG_H
#define _SYSTEMLOG_H

#include "AsyncHandler.h"
#include "ListViewCtrl.h"

//#include "../client/LogManager.h"
#include "../client/LogManagerListener.h"
#include "../client/Util.h"

#include <wx/artprov.h>

class SystemLog : 
	public ListViewCtrl,
	public AsyncHandler<SystemLog>,
	private LogManagerListener
{
public:

	SystemLog(wxWindow* parent) :
		ListViewCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxLC_REPORT)
	{
		SetFont(WinUtil::font);
		SetBackgroundColour(WinUtil::bgColor);
		SetForegroundColour(WinUtil::textColor);
		InsertColumn(0, _("Time"));
		InsertColumn(1, _("Message"));

		wxImageList* logIcons = new wxImageList(16, 16);
		logIcons->Add(wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_BUTTON));
		logIcons->Add(wxArtProvider::GetBitmap(wxART_WARNING, wxART_BUTTON));
		logIcons->Add(wxArtProvider::GetBitmap(wxART_ERROR, wxART_BUTTON));
		
		AssignImageList(logIcons, wxIMAGE_LIST_SMALL);

		LogManager::List oldMessages = LogManager::getInstance()->getLastLogs();
		// Technically, we might miss a message or two here, but who cares...
		LogManager::getInstance()->addListener(this);

		for(LogManager::List::const_iterator i = oldMessages.begin(); i != oldMessages.end(); ++i) 
		{
			callAsync(std::bind(&SystemLog::addLog, this, i->first, Text::toT(i->second.first), i->second.second));
		}
	}

	~SystemLog()
	{
		LogManager::getInstance()->removeListener(this);
	}

private:

	void addLog(time_t t, const tstring& msg, LogManager::Severity severity)
	{
		TStringList l;
		l.push_back(Text::toT(Util::getShortTimeString(t)));
		l.push_back(msg);
			
		int i = insert(GetItemCount(), l, severity);
			
		// set column size according to widest text
		SetColumnWidth(1, wxLIST_AUTOSIZE);
		// simulate scroll to bottom
		Focus(i);
	}

	void on(LogManagerListener::Message, time_t time, const string& msg, LogManager::Severity severity) throw()
	{
		callAsync(std::bind(&SystemLog::addLog, this, time, Text::toT(msg), severity));
	}
};

#endif	// _SYSTEMLOG_H