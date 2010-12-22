
#include "wxEdit.h"
#include "wx/artprov.h"
#include "wxMaterialEditor.h"
#include "IceMainLoop.h"
#include "IceSceneManager.h"
#include "Edit.h"
#include "propGridEditIceGOC.h"
#include "DotSceneLoader.h"
#include "IceAIManager.h"
#include "IceLevelMesh.h"
#include "NavMeshEditorNode.h"

BEGIN_EVENT_TABLE(wxEdit, wxFrame)
    EVT_ACTIVATE(wxEdit::OnActivate)
	EVT_CLOSE(wxEdit::OnClose)

	EVT_MENU(wxMainMenu_loadWorld, wxEdit::OnLoadWorld)
	EVT_MENU(wxMainMenu_saveWorld, wxEdit::OnSaveWorld)
	EVT_MENU(wxMainMenu_loadMesh, wxEdit::OnLoadMesh)
	EVT_MENU(wxMainMenu_exit, wxEdit::OnExit)
	EVT_MENU(wxMainMenu_Mode_Brush, wxEdit::OnEnableBrushMode)
	EVT_MENU(wxMainMenu_Mode_Material, wxEdit::OnEnableMaterialMode)
	EVT_MENU(wxMainMenu_Meshes, wxEdit::OnMeshEditor)
	EVT_MENU(wxMainMenu_Physics, wxEdit::OnEnablePhysics)
	EVT_MENU(wxMainMenu_ReloadScripts, wxEdit::OnReloadScripts)
	EVT_MENU(wxMainMenu_EditorMeshes, wxEdit::OnEnableEditorMeshes)
	EVT_MENU(wxMainMenu_ShowWayMesh, wxEdit::OnShowWayMesh)
	EVT_MENU(wxMainMenu_ShowWalkableAreas, wxEdit::OnShowWalkableAreas)
	EVT_MENU(wxMainMenu_About, wxEdit::OnAbout)
	EVT_MENU(wxMainMenu_Settings, wxEdit::OnSettings)
	EVT_MENU(wxMainMenu_PreviewWindow, wxEdit::OnPreviewWindow)
	EVT_MENU(wxMainMenu_ShowLog, wxEdit::OnShowLog)
END_EVENT_TABLE() 


wxEdit::wxEdit() : wxFrame(nullptr, -1, _("Blackspace Editor"),
		wxDefaultPosition, wxSize(1280,900),
		wxDEFAULT_FRAME_STYLE)
{
		// notify wxAUI which frame to use
		m_mgr.SetManagedWindow(this);

		mProgressBar = new wxProgressBar(this, -1);
		SetStatusBar(mProgressBar);
		PositionStatusBar();

		wxInitAllImageHandlers();
		mMainToolbar = new wxEditorToolbar(this);
		mExplorerToolbar = new wxEditorToolbar(this);

	m_mgr.AddPane(mMainToolbar, wxAuiPaneInfo().
                  Name(("maintoolbar")).Caption(("")).
				  Top().Fixed().ToolbarPane().Layer(0));

	m_mgr.AddPane(mExplorerToolbar, wxAuiPaneInfo().
		Name(("explorertoolbar")).Caption(("")).
		Right().Fixed().ToolbarPane().Position(0).Layer(1));

		// ************************
		mMainNotebook = new wxMainNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
			wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS);
		mDummy = new wxOgre(this, -1);//, wxDefaultPosition, wxDefaultSize, 0, "dummypane");
		mDummy->Show(false);

		m_mgr.AddPane(mMainNotebook, wxCENTER, ("Main"));

		mPropertyWindow = new wxPropertyGridWindow(this, -1, wxDefaultPosition, wxSize(200,250));
		mPropertyWindow->AddPage(new wxEditIceGameObject(), "EditGameObject");
		mPropertyWindow->AddPage(new wxEditGOResource(), "EditGOCRes");
		mPropertyWindow->AddPage(new wxEditIceSceneParams(), "EditSceneParams");

		mPropertyWindow->AddPage(new wxMaterialEditor(), "material");

		mWorldExplorer = new wxEntityTreeNotebook(this, wxID_ANY,
									wxPoint(GetClientSize().x, GetClientSize().y),
                                    wxSize(330,400),
                                    wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

		mSettingsWindow = new wxSettingsWindow(this, wxID_ANY, wxDefaultPosition, wxSize(800,600));

		mMeshMagick = new wxMeshMagick(this, wxID_ANY, wxDefaultPosition, wxSize(800,600));

		// add the panes to the manager
		m_mgr.AddPane(mWorldExplorer, wxAuiPaneInfo().Name(("World")).
			Right().
			Position(1).
			Layer(1).
			CloseButton(false).
			Caption("World"));
		m_mgr.AddPane(mPropertyWindow, wxAuiPaneInfo().Name(("Properties")).
			Right().
			Position(2).
			Layer(1).
			CloseButton(false).
			Caption("Properties"));

    m_mgr.AddPane(mMeshMagick, wxAuiPaneInfo().
                  Name(("meshmagick")).Caption(("Mesh Magick Params")).
				  Dockable(false).Float().Hide());

	mObjectPreviewWindow = new wxSimpleOgreView(this, -1);
    m_mgr.AddPane(mObjectPreviewWindow, wxAuiPaneInfo().
                  Name(("preview")).Caption(("Preview")).
				 Dockable(false).Float().FloatingPosition(wxPoint(GetPosition().x + GetSize().GetWidth(),GetPosition().y)).FloatingSize(256,256));

    m_mgr.AddPane(mSettingsWindow, wxAuiPaneInfo().
                  Name(("settings")).Caption(("Editor Settings")).
                  Dockable(false).Float().Hide());

	mLogDisplay = new wxLogDisplay(this);
	m_mgr.AddPane(mLogDisplay, wxAuiPaneInfo().
					Name(("logdisplay")).
					Caption(("Log")).Bottom().BestSize(200, 125));

	/*mMediaTree = new wxMediaTree(this, wxID_ANY, wxDefaultPosition, wxSize(300,500));
    m_mgr.AddPane(mMediaTree, wxAuiPaneInfo().
                  Name(("mediaTree")).Caption(("Loaded media files - drop new media here.")).
				  Dockable(false).Float().Hide());*/

	m_mgr.Update();
	Refresh();

	Ogre::String handle;
	handle = Ogre::StringConverter::toString((size_t)((HWND)this->GetHandle()));

	//Setup Menu
	mMenuBar = new wxMenuBar(wxMB_DOCKABLE);
	SetMenuBar(mMenuBar);
	mFileMenu = new wxMenu;
	mFileMenu->Append(wxMainMenu_loadWorld, "Load World");
	mFileMenu->Append(wxMainMenu_saveWorld, "Save World");
	mFileMenu->AppendSeparator();
	mFileMenu->Append(wxMainMenu_loadMesh, "Import world");
	mFileMenu->AppendSeparator();
	mFileMenu->Append(wxMainMenu_exit, "Exit");
	mToolsMenu = new wxMenu;
	wxMenuItem *meshtool = new wxMenuItem(mToolsMenu, wxMainMenu_Meshes, "MeshMagick (by Haffax)", "Mesh Editor which allows you to perform some basic operations on your meshes such as scaling or center align.");
	mToolsMenu->Append(meshtool);
	mSettingsMenu = new wxMenu;
	wxMenuItem *showeditormeshes = new wxMenuItem(mSettingsMenu, wxMainMenu_EditorMeshes, "Show Editor Meshes", "Enables/Disables Editor Meshes.", true);
	mSettingsMenu->Append(wxMainMenu_Settings, "Settings");
	mSettingsMenu->AppendSeparator();
	mSettingsMenu->Append(showeditormeshes);
	wxMenuItem *showwaymesh = new wxMenuItem(mSettingsMenu, wxMainMenu_ShowWayMesh, "Show Waymesh", "Displays/Hides the waymesh.", true);
	mSettingsMenu->Append(showwaymesh);
	wxMenuItem *showwalkables = new wxMenuItem(mSettingsMenu, wxMainMenu_ShowWalkableAreas, "Show Walkable Areas", "Displays/Hides walkable areas.", true);
	mSettingsMenu->Append(showwalkables);
	mAboutMenu = new wxMenu;
	mAboutMenu->Append(wxMainMenu_About, "About");
	mWindowsMenu = new wxMenu;
	wxMenuItem *showpreview = new wxMenuItem(mWindowsMenu, wxMainMenu_PreviewWindow, "Preview Window", "Enables/Disables the preview window.", true);
	mWindowsMenu->Append(showpreview);
	wxMenuItem *showlog = new wxMenuItem(mWindowsMenu, wxMainMenu_ShowLog, "Log", "Enables/Disables the log.", true);
	mWindowsMenu->Append(showlog);
	mMenuBar->Append(mFileMenu, "File");
	mMenuBar->Append(mToolsMenu, "Tools");
	mMenuBar->Append(mWindowsMenu, "Windows");
	mMenuBar->Append(mSettingsMenu, "Settings");
	mMenuBar->Append(mAboutMenu, "About");
	mMenuBar->Check(wxMainMenu_EditorMeshes, true);
	mMenuBar->Check(wxMainMenu_PreviewWindow, true);

	GetOgrePane()->initOgre("MainRenderWindow");
};

void wxEdit::RefreshToolbars()
{
	m_mgr.DetachPane(mMainToolbar);
	m_mgr.AddPane(mMainToolbar, wxAuiPaneInfo().
		Name(("maintoolbar")).Caption(("")).
		Top().Fixed().ToolbarPane().Layer(0));

	m_mgr.DetachPane(mExplorerToolbar);
	m_mgr.AddPane(mExplorerToolbar, wxAuiPaneInfo().
		Name(("explorertoolbar")).Caption(("")).
		Right().Fixed().ToolbarPane().Layer(1).Position(0));

	m_mgr.Update();
}

void wxEdit::PostCreate()
{
	mComponentBar = new wxComponentBar(this, -1, wxDefaultPosition, wxSize(100,100));
	m_mgr.AddPane(	mComponentBar, wxAuiPaneInfo().
					Name(("componentbar")).
					Caption(("Components")).
					Fixed().
					Right().
					Position(3).
					Layer(1).
					Show(false).
					CloseButton(false));
	GetOgrePane()->setCamera(Ice::Main::Instance().GetCamera());
	GetOgrePane()->PostInit();
	GetWorldExplorer()->GetSceneTree()->Update();
	GetWorldExplorer()->GetMaterialTree()->Update();
	GetOgrePane()->SetFocus();
	m_mgr.Update();
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("ResourceMgr", false);	//Hack
	Ice::MainLoop::Instance().doLoop();
	Ice::SceneManager::Instance().EnableClock(false);

	mObjectPreviewWindow->initOgre("PreviewWindow");

	LoadSave::LoadSave::Instance().RegisterObject(&ComponentSection::Register);
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new ComponentSectionVectorHandler());
}

wxPoint wxEdit::GetStartPosition()
{
    static int x = 0;
    x += 20;
    wxPoint pt = ClientToScreen(wxPoint(0,0));
    return wxPoint(pt.x + x, pt.y + x);
}

template<> wxEdit* Ogre::Singleton<wxEdit>::ms_Singleton = nullptr;
wxEdit& wxEdit::Instance()
{
	return ( *ms_Singleton );
};

wxEdit::~wxEdit()
{
	std::cout << "~wxEdit" << std::endl;
	Ice::SceneManager::Instance().Reset();	//Reset scene
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
	wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
	m_mgr.UnInit();
	Ice::Main::Instance().Shutdown();

};

Edit* wxEdit::GetOgrePane()
{
	return mMainNotebook->GetOgreWindow();
}

void wxEdit::UpdateOgre()
{
	GetOgrePane()->update();
};

void wxEdit::OnActivate(wxActivateEvent &event)
{
}

void wxEdit::OnClose(wxCloseEvent &event)
{
	//delete mMainNotebook;
	//Ice::Main::Instance().Shutdown();
	Destroy();
}

void wxEdit::OnLoadWorld(wxCommandEvent& WXUNUSED(event))
{
	//Ogre::LogManager::getSingleton().logMessage("OnLoadWorld");
    wxFileDialog dialog
                 (
                    this,
                    "Load World",
                    wxEmptyString,
                    wxEmptyString,
                    "Blackstar Engine World files (*.eew)|*.eew"
                 );

    dialog.CentreOnParent();
	dialog.SetPath("Data/Editor/Worlds/");

    if (dialog.ShowModal() == wxID_OK)
    {
		GetOgrePane()->OnLoadWorld(dialog.GetPath().c_str().AsChar());
	}
	wxEdit::Instance().GetProgressBar()->Reset();
	Ice::SceneManager::Instance().ShowEditorMeshes(mMenuBar->IsChecked(wxMainMenu_EditorMeshes));
};

void wxEdit::OnSaveWorld(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetOgrePane()->DeselectAllObjects();

	//Ogre::LogManager::getSingleton().logMessage("OnSaveWorld");
    wxFileDialog dialog
                 (
                    this,
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
		GetOgrePane()->OnSaveWorld(dialog.GetPath().c_str().AsChar());
	}
	wxEdit::Instance().GetProgressBar()->Reset();
	Ice::SceneManager::Instance().ShowEditorMeshes(mMenuBar->IsChecked(wxMainMenu_EditorMeshes));
};

void wxEdit::OnLoadMesh(wxCommandEvent& WXUNUSED(event))
{
	//Ogre::LogManager::getSingleton().logMessage("OnLoadMesh");
    wxFileDialog dialog
                 (
                    this,
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
		Ogre::String sPath = dialog.GetPath().c_str();
		wxEdit::Instance().GetProgressBar()->SetStatusMessage("Importing world...");
		wxEdit::Instance().GetProgressBar()->SetProgress(0.1);
		Ogre::String extension = sFile.substr(sFile.find_last_of("."), sFile.size());
		if (extension == ".mesh")
		{
			Ice::SceneManager::Instance().LoadLevelMesh(sFile);
		}
		else DotSceneLoader::Instance().ImportScene(sFile);

		wxEdit::Instance().GetpropertyWindow()->SetPage("None");
		//Ice::Main::Instance().GetCamera()->setPosition(Ogre::Vector3(0,0,0));
		wxEdit::Instance().GetProgressBar()->SetStatusMessage("Updating material Tree...");
		wxEdit::Instance().GetProgressBar()->SetProgress(0.8);
		wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();

		Ice::AIManager::Instance().GetNavigationMesh()->ImportOgreMesh(Ice::SceneManager::Instance().GetLevelMesh()->GetEntity()->getMesh());
		//NavMeshEditorNode::FromMesh(Ice::AIManager::Instance().GetNavigationMesh());
    }
	wxEdit::Instance().GetProgressBar()->Reset();
};

void wxEdit::OnExit(wxCommandEvent& WXUNUSED(event))
{
	Ogre::LogManager::getSingleton().logMessage("wxEdit::OnExit");
	Close();
};

void wxEdit::OnMeshEditor(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dialog(this, "Select meshes",
                        wxEmptyString, wxEmptyString, "Ogre binary Mesh files (*.mesh;*.skeleton)|*.mesh;*.skeleton",
                        wxFD_OPEN|wxFD_MULTIPLE);

    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString paths;

        dialog.GetPaths(paths);

		wxEdit::Instance().GetMeshMagick()->SetPaths(paths);

		wxAuiPaneInfo& floating_pane = wxEdit::Instance().GetAuiManager().GetPane(("meshmagick")).Float().Show();

		if (floating_pane.floating_pos == wxDefaultPosition)
			floating_pane.FloatingPosition(wxEdit::Instance().GetStartPosition());

		wxEdit::Instance().GetAuiManager().Update();
	}
}

void wxEdit::OnReloadScripts(wxCommandEvent& WXUNUSED(event))
{
	STOP_MAINLOOP
	Ice::ScriptSystem::GetInstance().ReparseAllScripts();
	RESUME_MAINLOOP
}

void wxEdit::OnEnableBrushMode(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetOgrePane()->mBrushMode = (wxEdit::Instance().GetOgrePane()->mBrushMode ? false : true);
	if (wxEdit::Instance().GetOgrePane()->mBrushMode)
	{
		wxEdit::Instance().GetOgrePane()->mMaterialMode = false;
		mMenuBar->Check(wxMainMenu_Mode_Material, false);
		wxEdit::Instance().GetOgrePane()->OnSelectResource();
	}
	else wxEdit::Instance().GetOgrePane()->ClearPreviewObject();
	wxEdit::Instance().GetOgrePane()->DeselectMaterial();
	wxEdit::Instance().GetProgressBar()->Reset();
};

void wxEdit::OnEnableMaterialMode(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetOgrePane()->mMaterialMode = (wxEdit::Instance().GetOgrePane()->mMaterialMode ? false : true);
	if (wxEdit::Instance().GetOgrePane()->mMaterialMode)
	{
		wxEdit::Instance().GetOgrePane()->mBrushMode = false;
		mMenuBar->Check(wxMainMenu_Mode_Brush, false);
	}
	wxEdit::Instance().GetOgrePane()->DeselectMaterial();
	wxEdit::Instance().GetProgressBar()->Reset();
};

void wxEdit::OnEnablePhysics(wxCommandEvent& WXUNUSED(event))
{
	Ice::MainLoop::Instance().SetPhysics(mMenuBar->IsChecked(wxMainMenu_Physics));
	wxEdit::Instance().GetProgressBar()->Reset();
}

void wxEdit::OnEnableEditorMeshes(wxCommandEvent& WXUNUSED(event))
{
	Ice::SceneManager::Instance().ShowEditorMeshes(mMenuBar->IsChecked(wxMainMenu_EditorMeshes));
}

void wxEdit::OnShowWalkableAreas(wxCommandEvent& WXUNUSED(event))
{
	Ice::AIManager::Instance().GetNavigationMesh()->VisualiseWalkableAreas(mMenuBar->IsChecked(wxMainMenu_ShowWalkableAreas));
}
void wxEdit::OnShowWayMesh(wxCommandEvent& WXUNUSED(event))
{
	Ice::AIManager::Instance().GetNavigationMesh()->VisualiseWaymesh(mMenuBar->IsChecked(wxMainMenu_ShowWayMesh));
}

void wxEdit::OnSettings(wxCommandEvent& WXUNUSED(event))
{
	wxAuiPaneInfo& floating_pane = wxEdit::Instance().GetAuiManager().GetPane(("settings")).Float().Show();

    if (floating_pane.floating_pos == wxDefaultPosition)
        floating_pane.FloatingPosition(wxEdit::Instance().GetStartPosition());

	wxEdit::Instance().GetProgressBar()->Reset();
    wxEdit::Instance().GetAuiManager().Update();
}

void wxEdit::OnPreviewWindow(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetAuiManager().GetPane(("preview")).Show(mMenuBar->IsChecked(wxMainMenu_PreviewWindow));
	wxEdit::Instance().GetAuiManager().Update();
}

void wxEdit::OnShowLog(wxCommandEvent& WXUNUSED(event))
{
	wxEdit::Instance().GetAuiManager().GetPane(("logdisplay")).Show(mMenuBar->IsChecked(wxMainMenu_ShowLog));
	wxEdit::Instance().GetAuiManager().Update();
}

void wxEdit::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox("Blackspacer (C) 2010 Andreas Henne (Caphalor)\nBlackspace Engine (C) 2010 Benedikt (1nsane) / Andreas Henne\n\nDependencies: Ogre 1.8, PhysX 2.8, wxWidgets 2.87, Caelum, OpenAL, OgreOggSound, boost, Lua, MeshMagick\n\nContact: heandreas@live.de\nJOIN US! ;)",
                       "About Blackspace Editor 'Weathertop' PRE ALPHA",
                       wxICON_INFORMATION);
	wxEdit::Instance().GetProgressBar()->Reset();
}