
#pragma once

#include "EDTIncludes.h"
#include "wxEdit.h"

typedef void (*SGTToolCallback)(int toolID, Ogre::String toolname);

struct SGTEDTTool
{
	Ogre::String mName;
	wxBitmap mBitmap;
	bool mActive;
	bool mChecked;
	SGTToolCallback mCallbackFunc;
	int mToolID;
	bool mCheckTool;
};

struct SGTEDTToolGroup
{
	bool mActive;
	Ogre::String mGroupname;
	std::vector<SGTEDTTool> mTools;
};

class wxSGTToolbar : public wxToolBar
{
protected:
	DECLARE_EVENT_TABLE()

	int mCurrentID;
	int NextID();

	void OnToolEvent(wxCommandEvent &event);

	std::vector<SGTEDTToolGroup> mToolGroups;

public:
	wxSGTToolbar(wxWindow *parent);
	~wxSGTToolbar(void);

	int RegisterTool(Ogre::String toolname, Ogre::String toolgroup, Ogre::String bitmapname, SGTToolCallback callback, bool checktool = false, bool checked = false);
	void SetGroupStatus(Ogre::String groupname, bool active);
	void SetToolStatus(Ogre::String tool, bool active);
	void CheckTool(Ogre::String toolname, bool check = true);
	void RefreshAll();
};
