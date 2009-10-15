

#ifndef __wxMainMenu_H__
#define __wxMainMenu_H__

#include "wxEdit.h"
#include "wx/wx.h"
#include "SGTMain.h"

enum
{
	wxMainMenu_loadWorld,
	wxMainMenu_saveWorld,
	wxMainMenu_loadMesh,
	wxMainMenu_exit,
	wxMainMenu_Mode_Normal,
	wxMainMenu_Mode_Brush,
	wxMainMenu_Mode_Material,
	wxMainMenu_ReloadScripts,
	wxMainMenu_Meshes,
	wxMainMenu_Physics,
	wxMainMenu_EditorMeshes,
	wxMainMenu_About,
	wxMainMenu_Settings,
	wxMainMenu_PreviewWindow
};

class wxMainMenu : public wxMenuBar
{
public:
	DECLARE_CLASS(wxMainMenu)

	wxMainMenu();
	~wxMainMenu();

	wxMenu* mFileMenu;
	wxMenu* mToolsMenu;
	wxMenu* mWindowsMenu;
	wxMenu* mSettingsMenu;
	wxMenu* mAboutMenu;

	DECLARE_EVENT_TABLE()

	void OnLoadWorld(wxCommandEvent& WXUNUSED(event));
	void OnSaveWorld(wxCommandEvent& WXUNUSED(event));
	void OnLoadMesh(wxCommandEvent& WXUNUSED(event));
	void OnExit(wxCommandEvent& WXUNUSED(event));

	void OnEnableBrushMode(wxCommandEvent& WXUNUSED(event));
	void OnEnableMaterialMode(wxCommandEvent& WXUNUSED(event));
	void OnReloadScripts(wxCommandEvent& WXUNUSED(event));

	void OnEnablePhysics(wxCommandEvent& WXUNUSED(event));
	void OnEnableEditorMeshes(wxCommandEvent& WXUNUSED(event));

	void OnMeshEditor(wxCommandEvent& WXUNUSED(event));

	void OnSettings(wxCommandEvent& WXUNUSED(event));

	void OnPreviewWindow(wxCommandEvent& WXUNUSED(event));

	void OnAbout(wxCommandEvent& WXUNUSED(event));

private:
	wxEdit *mEdit;
	SGTMain *mRoot;
};

#endif