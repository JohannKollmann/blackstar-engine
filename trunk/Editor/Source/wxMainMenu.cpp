
#include "../Header/wxMainMenu.h"
#include "SGTMainLoop.h"
#include "SGTCameraController.h"
#include "SGTDotSceneLoader.h"
#include "SGTScriptSystem.h"
#include "SGTAIManager.h"


// Required for WX
IMPLEMENT_CLASS(wxMainMenu, wxMenuBar)

// Required for WX
BEGIN_EVENT_TABLE(wxMainMenu, wxMenuBar)
	EVT_MENU(wxMainMenu_loadWorld, wxMainMenu::OnLoadWorld)
	EVT_MENU(wxMainMenu_saveWorld, wxMainMenu::OnSaveWorld)
	EVT_MENU(wxMainMenu_loadMesh, wxMainMenu::OnLoadMesh)
	EVT_MENU(wxMainMenu_exit, wxMainMenu::OnExit)
	EVT_MENU(wxMainMenu_Mode_Brush, wxMainMenu::OnEnableBrushMode)
	EVT_MENU(wxMainMenu_Mode_Material, wxMainMenu::OnEnableMaterialMode)
	EVT_MENU(wxMainMenu_Meshes, wxMainMenu::OnMeshEditor)
	EVT_MENU(wxMainMenu_Physics, wxMainMenu::OnEnablePhysics)
	EVT_MENU(wxMainMenu_ReloadScripts, wxMainMenu::OnReloadScripts)
	EVT_MENU(wxMainMenu_EditorMeshes, wxMainMenu::OnEnableEditorMeshes)
	EVT_MENU(wxMainMenu_About, wxMainMenu::OnAbout)
	EVT_MENU(wxMainMenu_Settings, wxMainMenu::OnSettings)
	EVT_MENU(wxMainMenu_PreviewWindow, wxMainMenu::OnPreviewWindow)
	EVT_MENU(wxMainMenu_ShowLog, wxMainMenu::OnShowLog)
END_EVENT_TABLE()

wxMainMenu::wxMainMenu()
{
	wxMenuBar(wxMB_DOCKABLE);

	mFileMenu = new wxMenu;
	mFileMenu->Append(wxMainMenu_loadWorld, _T("Load World"));
	mFileMenu->Append(wxMainMenu_saveWorld, "Save World");
	mFileMenu->AppendSeparator();
	mFileMenu->Append(wxMainMenu_loadMesh, "Import world");
	mFileMenu->AppendSeparator();
	mFileMenu->Append(wxMainMenu_exit, "Exit");

	mToolsMenu = new wxMenu;

	/*wxMenuItem *brushmode = new wxMenuItem(mToolsMenu, wxMainMenu_Mode_Brush, "Brush Mode", "Enables/Disables the Brush Mode, which allows you to \"paint\" objects on Geometry.", true);
	wxMenuItem *materialmode = new wxMenuItem(mToolsMenu, wxMainMenu_Mode_Material, "Material Mode", "Enables/Disables the Material Mode, which allows you to edit Materials. (Double Click)", true);
	mToolsMenu->Append(brushmode);
	mToolsMenu->Append(materialmode);
	mToolsMenu->AppendSeparator();

	wxMenuItem *reloadscripts = new wxMenuItem(mToolsMenu, wxMainMenu_ReloadScripts, "Reload Scripts");
	mToolsMenu->Append(reloadscripts);
	mToolsMenu->AppendSeparator();*/

	wxMenuItem *meshtool = new wxMenuItem(mToolsMenu, wxMainMenu_Meshes, "MeshMagick (by Haffax)", "Mesh Editor which allows you to perform some basic operations on your meshes such as scaling or center align.");
	mToolsMenu->Append(meshtool);

	/*mToolsMenu->AppendSeparator();
	mToolsMenu->Append(-1, "Goto Position (Str + p)");
	mToolsMenu->AppendSeparator();
	mToolsMenu->Append(-1, "Stop Game System");
	mToolsMenu->Append(-1, "Play the Game");*/

	mSettingsMenu = new wxMenu;
	//wxMenuItem *physics = new wxMenuItem(mToolsMenu, wxMainMenu_Physics, "Physics", "Enables/Disables the Physics Simulation.", true);
	wxMenuItem *showeditormeshes = new wxMenuItem(mToolsMenu, wxMainMenu_EditorMeshes, "Show Editor Meshes", "Enables/Disables Editor Meshes.", true);
	mSettingsMenu->Append(wxMainMenu_Settings, "Settings");
	mSettingsMenu->AppendSeparator();
	//mSettingsMenu->Append(physics);
	mSettingsMenu->Append(showeditormeshes);

	mAboutMenu = new wxMenu;
	mAboutMenu->Append(wxMainMenu_About, "About");

	mWindowsMenu = new wxMenu;
	wxMenuItem *showpreview = new wxMenuItem(mWindowsMenu, wxMainMenu_PreviewWindow, "Preview Window", "Enables/Disables the preview window.", true);
	mWindowsMenu->Append(showpreview);

	wxMenuItem *showlog = new wxMenuItem(mWindowsMenu, wxMainMenu_ShowLog, "Log", "Enables/Disables the log.", true);
	mWindowsMenu->Append(showlog);

	Append(mFileMenu, _T("File"));
	Append(mToolsMenu, _T("Tools"));
	Append(mWindowsMenu, _T("Windows"));
	Append(mSettingsMenu, _T("Settings"));
	Append(mAboutMenu, _T("About"));

	//Check(wxMainMenu_Physics, true);
	Check(wxMainMenu_EditorMeshes, true);
	Check(wxMainMenu_PreviewWindow, true);

	mEdit = &wxEdit::Instance();
};

void wxMainMenu::OnLoadWorld(wxCommandEvent& WXUNUSED(event))
{
	//Ogre::LogManager::getSingleton().logMessage("OnLoadWorld");
    wxFileDialog dialog
                 (
                    mEdit,
                    "Load World",
                    wxEmptyString,
                    wxEmptyString,
                    "Blackstar Engine World files (*.eew)|*.eew"
                 );

    dialog.CentreOnParent();
	dialog.SetPath("Data/Editor/Worlds/");

    if (dialog.ShowModal() == wxID_OK)
    {
		mEdit->GetOgrePane()->GetEdit()->OnLoadWorld(dialog.GetPath().c_str());
	}
	wxEdit::Instance().GetProgressBar()->Reset();
	SGTSceneManager::Instance().ShowEditorMeshes(IsChecked(wxMainMenu_EditorMeshes));
};

void wxMainMenu::OnSaveWorld(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetOgrePane()->GetEdit()->DeselectAllObjects();

	//Ogre::LogManager::getSingleton().logMessage("OnSaveWorld");
    wxFileDialog dialog
                 (
                    mEdit,
                    "Save World",
                    wxEmptyString,
                    wxEmptyString,
                    "Blackstar Engine World files (*.eew)|*.eew",
					wxFD_SAVE|wxFD_OVERWRITE_PROMPT
                 );

    dialog.CentreOnParent();
	dialog.SetPath("Data/Editor/Worlds/");

	dialog.SetFilterIndex(1);

    if (dialog.ShowModal() == wxID_OK)
    {
		mEdit->GetOgrePane()->GetEdit()->OnSaveWorld(dialog.GetPath().c_str());
	}
	wxEdit::Instance().GetProgressBar()->Reset();
	SGTSceneManager::Instance().ShowEditorMeshes(IsChecked(wxMainMenu_EditorMeshes));
};

void wxMainMenu::OnLoadMesh(wxCommandEvent& WXUNUSED(event))
{
	//Ogre::LogManager::getSingleton().logMessage("OnLoadMesh");
    wxFileDialog dialog
                 (
                    mEdit,
                    "Import World",
                    wxEmptyString,
                    wxEmptyString,
                    "Mesh and scene files (*.mesh;*.scene)|*.mesh;*.scene"
                 );

    dialog.CentreOnParent();
	dialog.SetPath("Data/Media/Meshes/");

    if (dialog.ShowModal() == wxID_OK)
    {
		Ogre::String sFile = dialog.GetFilename().c_str();
		wxEdit::Instance().GetProgressBar()->SetStatusMessage("Importing world...");
		wxEdit::Instance().GetProgressBar()->SetProgress(0.1);
		Ogre::String extension = sFile.substr(sFile.find("."), sFile.size());
		if (extension == ".mesh") SGTSceneManager::Instance().LoadLevelMesh(sFile);
		else SGTDotSceneLoader::Instance().ImportScene(sFile);

		wxEdit::Instance().GetpropertyWindow()->SetPage("None");
		SGTMain::Instance().GetCamera()->setPosition(Ogre::Vector3(0,0,0));
		wxEdit::Instance().GetProgressBar()->SetStatusMessage("Updating material tree...");
		wxEdit::Instance().GetProgressBar()->SetProgress(0.8);
		wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
    }
	wxEdit::Instance().GetProgressBar()->Reset();
};

void wxMainMenu::OnExit(wxCommandEvent& WXUNUSED(event))
{
	Ogre::LogManager::getSingleton().logMessage("wxMainMenu::OnExit");
	mEdit->Close();
};

void wxMainMenu::OnMeshEditor(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dialog(this, _T("Select meshes"),
                        wxEmptyString, wxEmptyString, "Ogre binary Mesh files (*.mesh)|*.mesh",
                        wxFD_OPEN|wxFD_MULTIPLE);

    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString paths;

        dialog.GetPaths(paths);

		wxEdit::Instance().GetMeshMagick()->SetPaths(paths);

		wxAuiPaneInfo& floating_pane = wxEdit::Instance().GetAuiManager().GetPane(wxT("meshmagick")).Float().Show();

		if (floating_pane.floating_pos == wxDefaultPosition)
			floating_pane.FloatingPosition(wxEdit::Instance().GetStartPosition());

		wxEdit::Instance().GetAuiManager().Update();
	}
}

void wxMainMenu::OnReloadScripts(wxCommandEvent& WXUNUSED(event))
{
	SGTScriptSystem::GetInstance().Clear();
	SGTAIManager::Instance().ReloadScripts();
}

void wxMainMenu::OnEnableBrushMode(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetOgrePane()->GetEdit()->mBrushMode = (wxEdit::Instance().GetOgrePane()->GetEdit()->mBrushMode ? false : true);
	if (wxEdit::Instance().GetOgrePane()->GetEdit()->mBrushMode)
	{
		wxEdit::Instance().GetOgrePane()->GetEdit()->mMaterialMode = false;
		Check(wxMainMenu_Mode_Material, false);
		wxEdit::Instance().GetOgrePane()->GetEdit()->OnSelectResource();
	}
	else wxEdit::Instance().GetOgrePane()->GetEdit()->ClearPreviewObject();
	wxEdit::Instance().GetOgrePane()->GetEdit()->DeselectMaterial();
	wxEdit::Instance().GetProgressBar()->Reset();
};

void wxMainMenu::OnEnableMaterialMode(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetOgrePane()->GetEdit()->mMaterialMode = (wxEdit::Instance().GetOgrePane()->GetEdit()->mMaterialMode ? false : true);
	if (wxEdit::Instance().GetOgrePane()->GetEdit()->mMaterialMode)
	{
		wxEdit::Instance().GetOgrePane()->GetEdit()->mBrushMode = false;
		Check(wxMainMenu_Mode_Brush, false);
	}
	wxEdit::Instance().GetOgrePane()->GetEdit()->DeselectMaterial();
	wxEdit::Instance().GetProgressBar()->Reset();
};

void wxMainMenu::OnEnablePhysics(wxCommandEvent& WXUNUSED(event))
{
	SGTMainLoop::Instance().SetPhysics(IsChecked(wxMainMenu_Physics));
	wxEdit::Instance().GetProgressBar()->Reset();
}

void wxMainMenu::OnEnableEditorMeshes(wxCommandEvent& WXUNUSED(event))
{
	SGTSceneManager::Instance().ShowEditorMeshes(IsChecked(wxMainMenu_EditorMeshes));
}

void wxMainMenu::OnSettings(wxCommandEvent& WXUNUSED(event))
{
	wxAuiPaneInfo& floating_pane = wxEdit::Instance().GetAuiManager().GetPane(wxT("settings")).Float().Show();

    if (floating_pane.floating_pos == wxDefaultPosition)
        floating_pane.FloatingPosition(wxEdit::Instance().GetStartPosition());

	wxEdit::Instance().GetProgressBar()->Reset();
    wxEdit::Instance().GetAuiManager().Update();
}

void wxMainMenu::OnPreviewWindow(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetAuiManager().GetPane(wxT("preview")).Show(IsChecked(wxMainMenu_PreviewWindow));
	wxEdit::Instance().GetAuiManager().Update();
}

void wxMainMenu::OnShowLog(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetAuiManager().GetPane(wxT("logdisplay")).Show(IsChecked(wxMainMenu_ShowLog));
	wxEdit::Instance().GetAuiManager().Update();
}

void wxMainMenu::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(_T("Blackstar Editor (C) 2009 Andreas Henne (Caphalor)\nBlackstar Engine (C) 2009 Benedikt (1nsane) / Andreas Henne\n\nDependencies: Ogre 1.6, NxOgre 1.022, PhysX 2.8, wxWidgets 2.87, Caelum, OpenAL, OgreOggSound, boost, Lua, MeshMagick\n\nContact: heandreas@live.de\nJOIN US! ;)"),
                       _T("About Blackstar Editor 'Weathertop' PRE ALPHA"),
                       wxICON_INFORMATION);
	wxEdit::Instance().GetProgressBar()->Reset();
}

wxMainMenu::~wxMainMenu()
{
};