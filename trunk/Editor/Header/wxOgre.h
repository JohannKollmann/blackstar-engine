
#pragma once

#include "boost/thread.hpp"
#include "Ogre.h"
#include "wx/wx.h"
#include "EDTIncludes.h"

	/**
	@brief WX widget for and Ogre rendering window

	This WX widget is a self-contained SINGLETON Ogre rendering window; 
	meaning it contains all Ogre objects necessary to create a rendering 
	window and currently supports only one rendering window at a time!
	This is due to the limitation of the self contained class.
	
	@usage Simply create a new wxOgre object and pass a wxFrame as its 
	parent window. Then work with it just like ay other wxControl object.
	It can even be passed to an wxAUI pane.
	*/

	class wxOgre : public wxControl
	{
		DECLARE_CLASS(wxOgre)

	private:
		boost::mutex mLoopMutex;
		int mBlockingCounter;

	public:
		/** A new wxOgre must receive a parent frame to which to attach 
		itself to */
		wxOgre(wxWindow* parent, wxWindowID id,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxSize(960, 600), long style = 0,
           const wxString& name = wxPanelNameStr); 
		virtual ~wxOgre();

		void initOgre(Ogre::String renderWindowName);

		/** Renders a single Ogre frame */
		void update();
		
		Ogre::RenderWindow* getRenderWindow() { return mRenderWindow; };
		Ogre::Camera* getCamera() { return mCamera; };
		void setCamera(Ogre::Camera* camera) { mCamera = camera;};

		void SetPaused(bool paused);
		void IncBlockingCounter() { mBlockingCounter++; }
		void DecBlockingCounter() { mBlockingCounter--; }

		boost::mutex& GetLoopMutex() { return mLoopMutex; }
		

	protected:
		DECLARE_EVENT_TABLE()

		virtual void OnRender() {}
		virtual void OnInit() {}

		/* Ogre members */
		/** Local Ogre::Root pointer */
		Ogre::Root* mRoot;
		/** Local Ogre::RenderWindow pointer */
		Ogre::RenderWindow* mRenderWindow;
		Ogre::Camera *mCamera;

		bool mPaused;
		bool mInitialized;

	protected:

		/** Creates an ogre rendering window and all other default objects
		such as the Ogre Root, default camera, default scene manager etc */ 
		void createOgreRenderWindow(Ogre::String renderWindowName);
		/** Toggles the rendering timer on/off */
		void toggleTimerRendering();

		/** Callback function to a window resize event */
		virtual void OnSize(wxSizeEvent& event);
		/** Callback function to a window paint event */
		void OnPaint(wxPaintEvent& event);
		/** Callback function to an EraseBackground event */
		void OnEraseBackground( wxEraseEvent& );
		/** Callback function to a timer "tick" event */
		void OnRenderTimer(wxTimerEvent& event);

		/* WX members */
		/** Rendering timer */
		wxTimer	mTimer;

	};