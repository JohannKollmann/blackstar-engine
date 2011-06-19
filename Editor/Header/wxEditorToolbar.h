
#pragma once

#include "EDTIncludes.h"
#include "wxEdit.h"

typedef void (*ToolCallback)(int toolID, Ogre::String toolname);

struct EDTTool
{
	Ogre::String mName;
	wxBitmap mBitmap;
	bool mActive;
	bool mChecked;
	ToolCallback mCallbackFunc;
	int mToolID;
	bool mCheckTool;
	Ogre::String mToolTip;
};

struct EDTToolGroup
{
	bool mActive;
	Ogre::String mGroupname;
	std::vector<EDTTool> mTools;
};

class wxEditorToolbar : public wxToolBar
{
protected:
	DECLARE_EVENT_TABLE()

	int mCurrentID;
	int NextID();

	void OnToolEvent(wxCommandEvent &event);

	std::vector<EDTToolGroup> mToolGroups;

public:
	wxEditorToolbar(wxWindow *parent);
	~wxEditorToolbar(void);

	int RegisterTool(Ogre::String toolname, Ogre::String toolgroup, Ogre::String bitmapname, ToolCallback callback, Ogre::String tooltip = "", bool checktool = false, bool checked = false);
	void SetGroupStatus(Ogre::String groupname, bool active);
	void SetToolStatus(Ogre::String tool, bool active);
	void CheckTool(Ogre::String toolname, bool check = true);
	bool GetToolIsChecked(Ogre::String toolname);
	void RefreshAll();
};
