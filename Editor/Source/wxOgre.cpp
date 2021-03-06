#include "wxOgre.h"
#include "IceMainLoop.h"
#include "IceWeatherController.h"
#include "IceSceneManager.h"
#include "IceMain.h"
#include "Edit.h"

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
	EVT_SIZE(wxOgre::OnSize)
	//EVT_PAINT(wxOgre::OnPaint) // Produces flickers and runs too fast!
	EVT_ERASE_BACKGROUND( wxOgre::OnEraseBackground )
	EVT_TIMER( ID_RENDERTIMER, wxOgre::OnRenderTimer )



END_EVENT_TABLE()

wxOgre::wxOgre(wxWindow* parent, wxWindowID id,
               const wxPoint& pos, const wxSize& size, long style,
               const wxString& name) :
    wxControl(parent, id, pos, size, style, wxDefaultValidator, name),
    mRenderWindow(0),
    mRoot(0),
	mCamera(0),
	mInitialized(false),
	mPaused(false)
{
	//Ice::Log::Instance().LogMessage("new wxOgre");
}


void wxOgre::initOgre(Ogre::String renderWindowName)
{
    if (!mInitialized)
    {
		Ice::Log::Instance().LogMessage("initOgre");

		mRoot = Ogre::Root::getSingletonPtr();

        mInitialized = true; 
		// Create all Ogre objects
		createOgreRenderWindow(renderWindowName);
		// Start the rendering timer
		toggleTimerRendering();

		this->SetEvtHandlerEnabled(true);

		mTimer.SetOwner(this, ID_RENDERTIMER);
	}
}

void wxOgre::createOgreRenderWindow(Ogre::String renderWindowName)
{
	Ice::Log::Instance().LogMessage("createOgreRenderWindow 1");

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

	// Get wx control window size
	int width;
	int height;
	GetSize(&width, &height);
	/*width -= width%4;
	height -= height%4;*/
	// Create the render window
	Ice::Log::Instance().LogMessage("createOgreRenderWindow 3");
	mRenderWindow = Ogre::Root::getSingleton().createRenderWindow(renderWindowName, width, height, false, &params);
	Ice::Log::Instance().LogMessage("createOgreRenderWindow 4");

	OnInit();
}

void wxOgre::toggleTimerRendering()
{
	// Toggle Start/Stop
	if (mTimer.IsRunning())
		mTimer.Stop();
	mTimer.Start(20);
}

wxOgre::~wxOgre()
{
}

void wxOgre::OnSize(wxSizeEvent& event)
{
	if (mInitialized && !mPaused)
	{	
		// Setting new size;
		int width;
		int height;
		GetSize(&width, &height);
		mRenderWindow->resize( width, height );
		// Letting Ogre know the window has been resized;
		mRenderWindow->windowMovedOrResized();
		// Set the aspect ratio for the new size;
		if (mCamera)
			mCamera->setAspectRatio(Ogre::Real(width) / Ogre::Real(height));
	}
}
void wxOgre::OnPaint(wxPaintEvent& event)
{
}
void wxOgre::OnEraseBackground( wxEraseEvent& )
{
	//update();
}
void wxOgre::OnRenderTimer(wxTimerEvent& event)
{
	update();
}

void wxOgre::update()
{
	if (mInitialized && !mPaused)
	{
		OnRender();
	}
}

void wxOgre::SetPaused(bool paused)
{
	mPaused = paused;
}
