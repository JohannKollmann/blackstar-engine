
#ifndef __wxEdit_H__
#define __wxEdit_H__

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include "wxOgreSceneTree.h"
#include "wxMaterialTree.h"
#include "EDTIncludes.h"
#include "propGridEditIceEditorInterface.h"
#include "wxPropertyGridWindow.h"
#include "wxEntityTreeNotebook.h"
#include "wxSettingsWindow.h"
#include "wxProgressBar.h"
#include "wxMeshMagick.h"
#include "wxComponentBar.h"
#include "wxMainNotebook.h"
#include "wxEditorToolbar.h"
#include "wxMediaTree.h"
#include "wxLogDisplay.h"
#include "EDTIncludes.h"
#include "wxSimpleOgreView.h"



class wxEdit : public wxFrame, public Ogre::Singleton<wxEdit>
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
	wxSimpleOgreView *mObjectPreviewWindow;
	wxLogDisplay *mLogDisplay;

public:

	enum
	{
		wxMainMenu_loadWorld,
		wxMainMenu_saveWorld,
		wxMainMenu_exit,
		wxMainMenu_loadMesh,
		wxMainMenu_Mode_Brush,
		wxMainMenu_Mode_Material,
		wxMainMenu_Meshes,
		wxMainMenu_Physics,
		wxMainMenu_ReloadScripts,
		wxMainMenu_EditorMeshes,
		wxMainMenu_ShowWayMesh,
		wxMainMenu_About,
		wxMainMenu_Settings,
		wxMainMenu_PreviewWindow,
		wxMainMenu_ShowLog
	};

	wxEdit();

	~wxEdit();

	void PostCreate();

	wxMainNotebook* GetMainNotebook() { return mMainNotebook; };
	wxMenuBar* GetMainMenu() { return mMenuBar; };
	Edit* GetOgrePane();
	wxEntityTreeNotebook* GetWorldExplorer() { return mWorldExplorer; };
	wxPropertyGridWindow* GetpropertyWindow() { return mPropertyWindow; };
	wxSettingsWindow* GetSettingsWindow() { return mSettingsWindow; };
	wxMeshMagick* GetMeshMagick() { return mMeshMagick; };
	wxSimpleOgreView* GetPreviewWindow() { return mObjectPreviewWindow; };
	wxProgressBar* GetProgressBar() { return mProgressBar; };
	wxComponentBar* GetComponentBar() { return mComponentBar; };
	wxEditorToolbar* GetMainToolbar() { return mMainToolbar; };
	wxEditorToolbar* GetExplorerToolbar() { return mExplorerToolbar; };

	wxPoint GetStartPosition();

	void RefreshToolbars();

	wxAuiManager& GetAuiManager() { return m_mgr; };

	void UpdateOgre();

	void OnActivate(wxActivateEvent &event); 

	void OnClose(wxCloseEvent &event);

	//Singleton
	static wxEdit& Instance();


protected:
	//menu stuff
	wxMenuBar* mMenuBar;
	wxMenu* mFileMenu;
	wxMenu* mToolsMenu;
	wxMenu* mWindowsMenu;
	wxMenu* mSettingsMenu;
	wxMenu* mAboutMenu;

	void OnLoadWorld(wxCommandEvent& WXUNUSED(event));
	void OnSaveWorld(wxCommandEvent& WXUNUSED(event));
	void OnLoadMesh(wxCommandEvent& WXUNUSED(event));
	void OnExit(wxCommandEvent& WXUNUSED(event));

	void OnEnableBrushMode(wxCommandEvent& WXUNUSED(event));
	void OnEnableMaterialMode(wxCommandEvent& WXUNUSED(event));
	void OnReloadScripts(wxCommandEvent& WXUNUSED(event));

	void OnEnablePhysics(wxCommandEvent& WXUNUSED(event));
	void OnEnableEditorMeshes(wxCommandEvent& WXUNUSED(event));
	void OnShowWayMesh(wxCommandEvent& WXUNUSED(event));

	void OnMeshEditor(wxCommandEvent& WXUNUSED(event));

	void OnSettings(wxCommandEvent& WXUNUSED(event));

	void OnPreviewWindow(wxCommandEvent& WXUNUSED(event));
	void OnShowLog(wxCommandEvent& WXUNUSED(event));

	void OnAbout(wxCommandEvent& WXUNUSED(event));
};

#endif