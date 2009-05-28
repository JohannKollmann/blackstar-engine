
#include "../Header/wxMainMenu.h"
#include "SGTGamestate.h"
#include "SGTCameraController.h"
#include "SGTDotSceneLoader.h"

enum
{
	wxMainMenu_loadWorld,
	wxMainMenu_saveWorld,
	wxMainMenu_loadMesh,
	wxMainMenu_exit,
	wxMainMenu_Mode_Normal,
	wxMainMenu_Mode_Brush,
	wxMainMenu_Mode_Material,
	wxMainMenu_Meshes,
	wxMainMenu_Physics,
	wxMainMenu_EditorMeshes,
	wxMainMenu_About,
	wxMainMenu_Settings
};


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
	EVT_MENU(wxMainMenu_EditorMeshes, wxMainMenu::OnEnableEditorMeshes)
	EVT_MENU(wxMainMenu_About, wxMainMenu::OnAbout)
	EVT_MENU(wxMainMenu_Settings, wxMainMenu::OnSettings)
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

	wxMenuItem *brushmode = new wxMenuItem(mToolsMenu, wxMainMenu_Mode_Brush, "Brush Mode", "Enables/Disables the Brush Mode, which allows you to \"paint\" objects on Geometry.", true);
	wxMenuItem *materialmode = new wxMenuItem(mToolsMenu, wxMainMenu_Mode_Material, "Material Mode", "Enables/Disables the Material Mode, which allows you to edit Materials. (Double Click)", true);
	mToolsMenu->Append(brushmode);
	mToolsMenu->Append(materialmode);
	mToolsMenu->AppendSeparator();

	wxMenuItem *meshtool = new wxMenuItem(mToolsMenu, wxMainMenu_Meshes, "MeshMagick (by Haffax)", "Mesh Editor which allows you to perform some basic operations on your meshes such as scaling or center align.");
	mToolsMenu->Append(meshtool);

	/*mToolsMenu->AppendSeparator();
	mToolsMenu->Append(-1, "Goto Position (Str + p)");
	mToolsMenu->AppendSeparator();
	mToolsMenu->Append(-1, "Stop Game System");
	mToolsMenu->Append(-1, "Play the Game");*/

	mSettingsMenu = new wxMenu;
	wxMenuItem *physics = new wxMenuItem(mToolsMenu, wxMainMenu_Physics, "Physics", "Enables/Disables the Physics Simulation.", true);
	wxMenuItem *showeditormeshes = new wxMenuItem(mToolsMenu, wxMainMenu_EditorMeshes, "Show Editor Meshes", "Enables/Disables Editor Meshes.", true);
	mSettingsMenu->Append(wxMainMenu_Settings, "Settings");
	mSettingsMenu->AppendSeparator();
	mSettingsMenu->Append(physics);
	mSettingsMenu->Append(showeditormeshes);

	mAboutMenu = new wxMenu;
	mAboutMenu->Append(wxMainMenu_About, "About");

	Append(mFileMenu, _T("File"));
	Append(mToolsMenu, _T("Tools"));
	Append(mSettingsMenu, _T("Settings"));
	Append(mAboutMenu, _T("About"));

	Check(wxMainMenu_Physics, true);
	Check(wxMainMenu_EditorMeshes, true);

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
		mEdit->GetOgrePane()->mEdit->OnLoadWorld(dialog.GetPath().c_str());
	}
	wxEdit::Instance().GetProgressBar()->Reset();
};

void wxMainMenu::OnSaveWorld(wxCommandEvent& WXUNUSED(event))
{
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
		mEdit->GetOgrePane()->mEdit->OnSaveWorld(dialog.GetPath().c_str());
	}
	wxEdit::Instance().GetProgressBar()->Reset();
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

void wxMainMenu::OnEnableBrushMode(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetOgrePane()->mEdit->mBrushMode = (wxEdit::Instance().GetOgrePane()->mEdit->mBrushMode ? false : true);
	if (wxEdit::Instance().GetOgrePane()->mEdit->mBrushMode)
	{
		wxEdit::Instance().GetOgrePane()->mEdit->mMaterialMode = false;
		Check(wxMainMenu_Mode_Material, false);
	}
	wxEdit::Instance().GetOgrePane()->mEdit->DeselectMaterial();
	wxEdit::Instance().GetProgressBar()->Reset();
};

void wxMainMenu::OnEnableMaterialMode(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetOgrePane()->mEdit->mMaterialMode = (wxEdit::Instance().GetOgrePane()->mEdit->mMaterialMode ? false : true);
	if (wxEdit::Instance().GetOgrePane()->mEdit->mMaterialMode)
	{
		wxEdit::Instance().GetOgrePane()->mEdit->mBrushMode = false;
		Check(wxMainMenu_Mode_Brush, false);
	}
	wxEdit::Instance().GetOgrePane()->mEdit->DeselectMaterial();
	wxEdit::Instance().GetProgressBar()->Reset();
};

void wxMainMenu::OnEnablePhysics(wxCommandEvent& WXUNUSED(event))
{
	SGTKernel::Instance().SetPhysics(IsChecked(wxMainMenu_Physics));
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

void wxMainMenu::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(_T("Blackstar Editor (C) 2008 Caphalor\nBlackstar Engine (C) 2008 1nsane / Caphalor\n\nDependencies: Ogre 1.6, NxOgre 1.022, PhysX 2.8, wxWidgets 2.87, Caelum, Hydrax,OpenAL, OgreOggSound, boost, Lua, MeshMagick"),
                       _T("About Blackstar Editor"),
                       wxICON_INFORMATION);
	wxEdit::Instance().GetProgressBar()->Reset();
}

wxMainMenu::~wxMainMenu()
{
};