
#ifndef __wxEdit_H__
#define __wxEdit_H__

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include "wxOgreSceneTree.h"
#include "wxMaterialTree.h"
#include "EDTIncludes.h"
#include "wxEditIceGameObject.h"
#include "wxMainMenu.h"
#include "wxPropertyGridWindow.h"
#include "wxEntityTreeNotebook.h"
#include "wxSettingsWindow.h"
#include "wxProgressBar.h"
#include "wxMeshMagick.h"
#include "wxComponentBar.h"
#include "wxMainNotebook.h"
#include "wxEditorToolbar.h"
#include "wxMediaTree.h"
#include "wxTextureDisplay.h"
#include "wxLogDisplay.h"
#include "EDTIncludes.h"

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
	wxEditorToolbar *mMainToolbar;
	wxEditorToolbar *mExplorerToolbar;
	wxTextureDisplay *mPreviewWindow;
	wxLogDisplay *mLogDisplay;

	//Menu
	wxMainMenu* mMenuBar;

public:

	wxEdit(wxWindow* parent);

	~wxEdit();

	void PostCreate();

	wxMainNotebook* GetMainNotebook() { return mMainNotebook; };
	wxMainMenu* GetMainMenu() { return mMenuBar; };
	wxOgre* GetOgrePane();
	wxEntityTreeNotebook* GetWorldExplorer() { return mWorldExplorer; };
	wxPropertyGridWindow* GetpropertyWindow() { return mPropertyWindow; };
	wxSettingsWindow* GetSettingsWindow() { return mSettingsWindow; };
	wxMeshMagick* GetMeshMagick() { return mMeshMagick; };
	wxTextureDisplay* GetPreviewWindow() { return mPreviewWindow; };
	wxProgressBar* GetProgressBar() { return mProgressBar; };
	wxComponentBar* GetComponentBar() { return mComponentBar; };
	wxEditorToolbar* GetMainToolbar() { return mMainToolbar; };
	wxEditorToolbar* GetExplorerToolbar() { return mExplorerToolbar; };

	wxPoint GetStartPosition();

	void RefreshToolbars();

	wxAuiManager& GetAuiManager() { return m_mgr; };

	void UpdateOgre();

	void OnActivate(wxActivateEvent &event); 

	//Singleton
	static wxEdit& Instance();
};

#endif