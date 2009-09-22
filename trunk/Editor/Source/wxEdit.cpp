
#include "../Header/wxEdit.h"
#include "wx/artprov.h"
#include "wxMaterialEditor.h"


BEGIN_EVENT_TABLE(wxEdit, wxFrame)
    EVT_ACTIVATE(wxEdit::OnActivate)
END_EVENT_TABLE() 


wxEdit::wxEdit(wxWindow* parent) : wxFrame(parent, -1, _("Blackstar Edit"),
		wxDefaultPosition, wxSize(1280,900),
		wxDEFAULT_FRAME_STYLE)
{
		// notify wxAUI which frame to use
		m_mgr.SetManagedWindow(this);

		mProgressBar = new wxProgressBar(this, -1);
		this->SetStatusBar(mProgressBar);

		wxInitAllImageHandlers();
		mMainToolbar = new wxSGTToolbar(this);
		mExplorerToolbar = new wxSGTToolbar(this);

	m_mgr.AddPane(mMainToolbar, wxAuiPaneInfo().
                  Name(wxT("maintoolbar")).Caption(wxT("")).
				  Top().Fixed().ToolbarPane().Layer(0));

	m_mgr.AddPane(mExplorerToolbar, wxAuiPaneInfo().
		Name(wxT("explorertoolbar")).Caption(wxT("")).
		Right().Fixed().ToolbarPane().Position(0).Layer(1));

		// ************************
		mMainNotebook = new wxMainNotebook(this, -1);
		mDummy = new wxOgre(this, -1);//, wxDefaultPosition, wxDefaultSize, 0, "dummypane");
		mDummy->Show(false);

		m_mgr.AddPane(mMainNotebook, wxCENTER, wxT("Main"));

		//Setup Menu
		mMenuBar = new wxMainMenu();
		SetMenuBar(mMenuBar);

		mPropertyWindow = new wxPropertyGridWindow(this, -1, wxDefaultPosition, wxSize(200,250));
		mPropertyWindow->AddPage(new wxEditSGTGameObject(), "EditGameObject");
		mPropertyWindow->AddPage(new wxEditSGTSceneParams(), "EditSceneParams");

		mPropertyWindow->AddPage(new wxMaterialEditor(), "material");

		mWorldExplorer = new wxEntityTreeNotebook(this, wxID_ANY,
									wxPoint(GetClientSize().x, GetClientSize().y),
                                    wxSize(300,300),
                                    wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

		mSettingsWindow = new wxSettingsWindow(this, wxID_ANY, wxDefaultPosition, wxSize(800,600));

		mMeshMagick = new wxMeshMagick(this, wxID_ANY, wxDefaultPosition, wxSize(800,600));

		// add the panes to the manager
		m_mgr.AddPane(mWorldExplorer, wxAuiPaneInfo().Name(wxT("World")).
			Right().
			Position(1).
			Layer(1).
			CloseButton(false).
			Caption("World"));
		m_mgr.AddPane(mPropertyWindow, wxAuiPaneInfo().Name(wxT("Properties")).
			Right().
			Position(2).
			Layer(1).
			CloseButton(false).
			Caption("Properties"));

    m_mgr.AddPane(mMeshMagick, wxAuiPaneInfo().
                  Name(wxT("meshmagick")).Caption(wxT("Mesh Magick Params")).
                  Dockable(false).Float().Hide());

    m_mgr.AddPane(mSettingsWindow, wxAuiPaneInfo().
                  Name(wxT("settings")).Caption(wxT("Editor Settings")).
                  Dockable(false).Float().Hide());

	m_mgr.Update();

	Ogre::String handle;
	handle = Ogre::StringConverter::toString((size_t)((HWND)this->GetHandle()));

	PushEventHandler(mMenuBar);
};

void wxEdit::RefreshToolbars()
{
	m_mgr.DetachPane(mMainToolbar);
	m_mgr.AddPane(mMainToolbar, wxAuiPaneInfo().
		Name(wxT("toolbar")).Caption(wxT("")).
		Top().Fixed().ToolbarPane().Layer(0));

	m_mgr.DetachPane(mExplorerToolbar);
	m_mgr.AddPane(mExplorerToolbar, wxAuiPaneInfo().
		Name(wxT("toolbar")).Caption(wxT("")).
		Right().Fixed().ToolbarPane().Layer(1).Position(0));

	m_mgr.Update();
}

void wxEdit::PostCreate()
{
	mComponentBar = new wxComponentBar(this, -1, wxDefaultPosition, wxSize(100,100));
	m_mgr.AddPane(	mComponentBar, wxAuiPaneInfo().
					Name(wxT("componentbar")).
					Caption(wxT("Components")).
					Fixed().
					Right().
					Position(3).
					Layer(1).
					Show(false).
					CloseButton(false));
	GetOgrePane()->setCamera(SGTMain::Instance().GetCamera());
	GetOgrePane()->initEdit();
	GetWorldExplorer()->GetSceneTree()->Update();
	GetWorldExplorer()->GetMaterialTree()->Update();
	GetOgrePane()->SetFocus();
	m_mgr.Update();
	SGTSceneManager::Instance().EnableClock(false);
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("ResourceMgr", false);	//Hack
}

wxPoint wxEdit::GetStartPosition()
{
    static int x = 0;
    x += 20;
    wxPoint pt = ClientToScreen(wxPoint(0,0));
    return wxPoint(pt.x + x, pt.y + x);
}


wxEdit& wxEdit::Instance()
{
	static wxEdit TheOneAndOnly(NULL); return TheOneAndOnly;
};

wxEdit::~wxEdit()
{
	std::cout << "~wxEdit" << std::endl;
	//PopEventHandler();

	// deinitialize the frame manager
	//m_mgr.UnInit();
	SGTMain::Instance().Shutdown();
	exit(0);	//HACK! FUCK YOU WX WIDGETS ICH WEISS WO DEIN HAUS WOHNT!!!11einseinself

};

wxOgre* wxEdit::GetOgrePane()
{
	return mMainNotebook->GetOgreWindow();
}

void wxEdit::UpdateOgre()
{
	GetOgrePane()->update();
};

void wxEdit::OnActivate(wxActivateEvent &event)
{
	GetOgrePane()->initOgre();
}