#include "../Header/wxOgre.h"
#include "IceMainLoop.h"
#include "IceWeatherController.h"
#include "IceSceneManager.h"

#ifdef __WXGTK__
#include <gdk/gdk.h>
#include <gtk/gtk.h> // just this should suffice as it should include gdk.h itself
#endif

// Required for the timer
const long ID_RENDERTIMER = wxNewId();


// Required for WX
IMPLEMENT_CLASS(wxOgre, wxControl)

// Required for WX
BEGIN_EVENT_TABLE(wxOgre, wxControl)
    EVT_KEY_DOWN(wxOgre::OnKeyDown)
    EVT_KEY_UP(wxOgre::OnKeyUp)

	EVT_SIZE(wxOgre::OnSize)
	// EVT_PAINT(wxOgre::OnPaint) // Produces flickers and runs too fast!
	EVT_ERASE_BACKGROUND( wxOgre::OnEraseBackground )
	EVT_TIMER( ID_RENDERTIMER, wxOgre::OnRenderTimer )

	EVT_SET_FOCUS(wxOgre::OnSetFocus)
	EVT_KILL_FOCUS(wxOgre::OnKillFocus)

	EVT_MOUSE_EVENTS(wxOgre::OnMouseEvent)

	EVT_MENU(wxOgre_insertObject, wxOgre::OnInsertObject)
	EVT_MENU(wxOgre_insertObjectAsChild, wxOgre::OnInsertObjectAsChild)
	EVT_MENU(wxOgre_deleteObject, wxOgre::OnDeleteObject)
	EVT_MENU(wxOgre_createObjectgroup, wxOgre::OnCreateObjectGroup)
	EVT_MENU(wxOgre_saveObjectgroup, wxOgre::OnSaveObjectGroup)
	EVT_MENU(wxOgre_createChain, wxOgre::OnCreateChain)
	EVT_MENU(wxOgre_connectWaypoints, wxOgre::OnConnectWaypoints)
	EVT_MENU(wxOgre_saveBones, wxOgre::OnSaveBones)
	EVT_MENU(wxOgre_createWayTriangle, wxOgre::OnCreateWayTriangle)

END_EVENT_TABLE()

wxOgre::wxOgre(wxWindow* parent, wxWindowID id,
               const wxPoint& pos, const wxSize& size, long style,
               const wxString& name) :
    wxControl(parent, id, pos, size, style, wxDefaultValidator, name),
    mRenderWindow(0),
    mRoot(0),
	mCamera(0),
	mInitialized(false),
	mEdit(0)
{
	//Ogre::LogManager::getSingleton().logMessage("new wxOgre");
}


void wxOgre::initOgre()
{
    if (!mInitialized)
    {
		Ogre::LogManager::getSingleton().logMessage("initOgre");

		mRoot = Ogre::Root::getSingletonPtr();

        mInitialized = true; 
		// Create all Ogre objects
		createOgreRenderWindow();
		// Start the rendering timer
		toggleTimerRendering();

		this->SetEvtHandlerEnabled(true);

		mTimer.SetOwner(this, ID_RENDERTIMER);
	}
}

void wxOgre::createOgreRenderWindow()
{
	Ogre::LogManager::getSingleton().logMessage("createOgreRenderWindow 1");

	// --------------------
	// Create a new parameters list according to compiled OS
	Ogre::NameValuePairList params;
	Ogre::String handle;
#ifdef __WXMSW__
	handle = Ogre::StringConverter::toString((size_t)((HWND)GetHandle()));
#elif defined(__WXGTK__)
	// TODO: Someone test this. you might to use "parentWindowHandle" if this
	// does not work.  Ogre 1.2 + Linux + GLX platform wants a string of the
	// format display:screen:window, which has variable types ulong:uint:ulong.
	GtkWidget* widget = GetHandle();
	gtk_widget_realize( widget );	// Mandatory. Otherwise, a segfault happens.
	std::stringstream handleStream;
	Display* display = GDK_WINDOW_XDISPLAY( widget->window );
	Window wid = GDK_WINDOW_XWINDOW( widget->window );	// Window is a typedef for XID, which is a typedef for unsigned int
	/* Get the right display (DisplayString() returns ":display.screen") */
	std::string displayStr = DisplayString( display );
	displayStr = displayStr.substr( 1, ( displayStr.find( ".", 0 ) - 1 ) );
	/* Put all together */
	handleStream << displayStr << ':' << DefaultScreen( display ) << ':' << wid;
	handle = handleStream.str();
#else
	#error Not supported on this platform.
#endif
	params["externalWindowHandle"] = handle;

	for (std::vector<Ice::Main::KeyVal>::iterator i = Ice::Main::Instance().mSettings["Graphics"].begin(); i != Ice::Main::Instance().mSettings["Graphics"].end(); i++)
	{
		if (i->Key == "AA")
		{
			params["FSAA"] = i->Val;
			break;
		}
	}

	Ogre::LogManager::getSingletonPtr()->logMessage(handle);

	// Get wx control window size
	int width;
	int height;
	GetSize(&width, &height);
	width -= width%4;
	height -= height%4;
	// Create the render window
	Ogre::LogManager::getSingleton().logMessage("createOgreRenderWindow 3");
	mRenderWindow = Ogre::Root::getSingleton().createRenderWindow("OgreRenderWindow", width, height, false, &params);
	Ogre::LogManager::getSingleton().logMessage("createOgreRenderWindow 4");
}

void wxOgre::toggleTimerRendering()
{
	// Toggle Start/Stop
	if (mTimer.IsRunning())
		mTimer.Stop();
	mTimer.Start(30);
}

wxOgre::~wxOgre()
{
	if (mInitialized)
	{
		delete mEdit;
		//Ice::Main::Instance().Shutdown();
		//exit(0);		//HACK! FUCK YOU WX WIDGETS ICH WEISS WO DEIN HAUS WOHNT!!!11einseinself
	}
}

void wxOgre::initEdit()
{
	Ogre::LogManager::getSingleton().logMessage("init Edit");
	/*std::vector<Ogre::String> entityTypes = Ice::SceneManager::Instance().GetAvailableEntityTypes();
	for (std::vector<Ogre::String>::iterator i = entityTypes.begin(); i != entityTypes.end(); i++)
	{
		extensions.Add("*." + wxString((*i).c_str()));
		Ogre::LogManager::getSingleton().logMessage("Adding extension " + (*i));
	}*/
	/*
	wxArrayString extensions;
	extensions.Add("*.ot");
	extensions.Add("*.ocs");
	wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->SetExtensions(extensions);
	wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->SetRootPath("Data/Editor/Objects");*/
	mEdit = new Edit();
};

void wxOgre::OnSize(wxSizeEvent& event)
{
	if (mInitialized)
	{
		// Setting new size;
		int width;
		int height;
		GetSize(&width, &height);
		Ice::Main::Instance().GetWindow()->resize( width, height );
		// Letting Ogre know the window has been resized;
		Ice::Main::Instance().GetWindow()->windowMovedOrResized();
		// Set the aspect ratio for the new size;
		if (Ice::Main::Instance().GetCamera())
			Ice::Main::Instance().GetCamera()->setAspectRatio(Ogre::Real(width) / Ogre::Real(height));

		if (Ice::SceneManager::Instance().GetWeatherController()) Ice::SceneManager::Instance().GetWeatherController()->UpdateViewport();
		//update();
	}
}
void wxOgre::OnPaint(wxPaintEvent& event)
{
//	update(); // Produces flickers and runs too fast!
}
void wxOgre::OnEraseBackground( wxEraseEvent& )
{
	//update();
}
void wxOgre::OnRenderTimer(wxTimerEvent& event)
{
	//Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode();
	/*if (mEdit->mPerformingObjMov || mEdit->mPerformingObjRot)	//Hack...
	{
		//Ogre::LogManager::getSingleton().logMessage("Hack: SetFocus to Ogre!");
		wxEdit::Instance().GetOgrePane()->SetFocus();
	}*/
	//if (wxEdit::Instance().FindFocus() == this || wxEdit::Instance().FindFocus() == wxEdit::Instance().GetSettingsWindow()) update();
	/*if (wxEdit::Instance().FindFocus())
	{
		if (wxEdit::Instance().FindFocus()->GetName() == "panel" || wxEdit::Instance().FindFocus()->GetName() == "frame" || wxEdit::Instance().FindFocus()->GetName() == "slider") update();
	}*/
	update();
	//else Ogre::LogManager::getSingleton().logMessage(wxEdit::Instance().FindFocus()->GetName().c_str());
}

void wxOgre::OnSetFocus(wxFocusEvent& event)
{
	if (mInitialized)
	{
		mEdit->OnSetFocus(true);
	}
}
void wxOgre::OnKillFocus(wxFocusEvent& event)
{
	if (mInitialized)
	{
		mEdit->OnSetFocus(false);
	}
}

void wxOgre::update()
{
	if (mInitialized)
	{
		Ice::MainLoop::Instance().doLoop();
	}
}

void wxOgre::OnKeyDown(wxKeyEvent& key)
{
	if (mEdit != NULL) mEdit->OnKeyDown(key);
};

void wxOgre::OnKeyUp(wxKeyEvent& key)
{
	if (mEdit != NULL) mEdit->OnKeyUp(key);
};

void wxOgre::OnMouseEvent(wxMouseEvent& ev)
{
	if (mEdit != NULL) mEdit->OnMouseEvent(ev);
};

void wxOgre::OnInsertObject(wxCommandEvent& WXUNUSED(event))
{
	mEdit->OnInsertObject();
};

void wxOgre::OnInsertObjectAsChild(wxCommandEvent& WXUNUSED(event))
{
	mEdit->OnInsertObjectAsChild();
};

void wxOgre::OnDeleteObject(wxCommandEvent& WXUNUSED(event))
{
	mEdit->OnDeleteObject();
};

void wxOgre::OnCreateObjectGroup(wxCommandEvent& WXUNUSED(event))
{
	mEdit->OnCreateObjectGroup();
};

void wxOgre::OnSaveObjectGroup(wxCommandEvent& WXUNUSED(event))
{
	mEdit->OnSaveObjectGroup();
};

void wxOgre::OnCreateChain(wxCommandEvent& WXUNUSED(event))
{
	mEdit->OnCreateChain();
};

void wxOgre::OnConnectWaypoints(wxCommandEvent& WXUNUSED(event))
{
	mEdit->OnConnectWaypoints();
};

void wxOgre::OnSaveBones(wxCommandEvent& WXUNUSED(event))
{
	mEdit->OnSaveBones();
};

void wxOgre::OnCreateWayTriangle(wxCommandEvent& WXUNUSED(event))
{
	mEdit->OnCreateWayTriangle();
}