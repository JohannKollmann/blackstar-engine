#pragma once

#include "wx/wx.h"
#include "wx/listctrl.h"
#include "Ogre.h"

class wxLogDisplay : public wxListCtrl, public Ogre::LogListener
{
	DECLARE_CLASS(wxLogDisplay)

private:
	long mIndex;

	DECLARE_EVENT_TABLE()

public:
	wxLogDisplay(wxWindow *parent);
	~wxLogDisplay(void);

	void messageLogged(const Ogre::String &message, Ogre::LogMessageLevel level, bool maskDebug, const Ogre::String &logName);

	void OnActivated(wxListEvent& event);

	void wxLogDisplay::OnShow(wxShowEvent& event);
};
