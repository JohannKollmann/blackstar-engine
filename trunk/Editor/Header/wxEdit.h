
#ifndef __wxEdit_H__
#define __wxEdit_H__

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include "wxObjectFolderTree.h"
#include "wxOgreSceneTree.h"
#include "wxMaterialTree.h"
#include "EDTIncludes.h"
#include "wxEditSGTGameObject.h"
#include "wxMainMenu.h"
#include "wxPropertyGridWindow.h"
#include "wxEntityTreeNotebook.h"
#include "wxSettingsWindow.h"
#include "wxProgressBar.h"
#include "wxMeshMagick.h"
#include "wxComponentBar.h"
#include "wxMainNotebook.h"
#include "wxSGTToolbar.h"

class wxEdit : public wxFrame
{
	DECLARE_EVENT_TABLE() 

private:

	wxAuiManager m_mgr;

	//Windows
	wxMainNotebook* mMainNotebook;
	wxOgre *mDummy;
	wxEntityTreeNotebook *mWorldExplorer;
	wxPropertyGridWindow *mPropertyWindow;
	wxSettingsWindow *mSettingsWindow;
	wxProgressBar *mProgressBar;
	wxMeshMagick *mMeshMagick;
	wxComponentBar *mComponentBar;
	wxSGTToolbar *mToolbar;

	//Menu
	wxMainMenu* mMenuBar;

public:

	wxEdit(wxWindow* parent);

	~wxEdit();

	void PostCreate();

	wxOgre* GetOgrePane();
	wxEntityTreeNotebook* GetWorldExplorer() { return mWorldExplorer; };
	wxPropertyGridWindow* GetpropertyWindow() { return mPropertyWindow; };
	wxSettingsWindow* GetSettingsWindow() { return mSettingsWindow; };
	wxMeshMagick* GetMeshMagick() { return mMeshMagick; };
	wxProgressBar* GetProgressBar() { return mProgressBar; };
	wxComponentBar* GetComponentBar() { return mComponentBar; };
	wxSGTToolbar* GetToolbar() { return mToolbar; };

	wxPoint GetStartPosition();

	wxAuiManager& GetAuiManager() { return m_mgr; };

	void UpdateOgre();

	void OnActivate(wxActivateEvent &event); 

	//Singleton
	static wxEdit& Instance();
};

#endif