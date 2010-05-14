
#pragma once

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include <map>
#include "Ogre.h"
#include "IceDataMap.h"
#include "EDTIncludes.h"
#include "propGridEditIceGOC.h"

class wxComponentBar : public wxPanel
{
	DECLARE_CLASS(wxComponentBar)

protected:

	DECLARE_EVENT_TABLE()

	struct ComponentParameters
	{
		Ogre::String mFamily;
		Ogre::String mName;
		Ice::DataMap *mParameters;
		wxCheckBox *mCheckBox;
	};

	std::map<int, ComponentParameters> mCallbackMap;

	void NotifyGroupCheck(bool checked, Ogre::String checked_name, Ogre::String group);

public:
	wxComponentBar(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);
	~wxComponentBar(void);

	void OnCheckBoxClicked(wxCommandEvent& event);
	void ResetCheckBoxes();
	void SetSections(std::vector<ComponentSection> &sections);
	void SetSectionStatus(Ogre::String name, bool checked);
};
