
#pragma once

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include <map>
#include "Ogre.h"
#include "SGTDataMap.h"

class wxComponentBar : public wxPanel
{
	DECLARE_CLASS(wxComponentBar)

protected:

	DECLARE_EVENT_TABLE()

	struct ComponentParameters
	{
		Ogre::String mName;
		SGTDataMap *mParameters;
		wxCheckBox *mCheckBox;
	};

	std::map<int, ComponentParameters> mCallbackMap;

public:
	wxComponentBar(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);
	~wxComponentBar(void);

	void OnCheckBoxClicked(wxCommandEvent& event);
	void ResetCheckBoxes();
	void SetSections(std::list<ComponentSection> sections);
	void SetSectionStatus(Ogre::String name, bool checked);
};
