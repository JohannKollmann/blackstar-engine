#ifndef __WXOGRE_H__
#define __WXOGRE_H__

#include "Ogre.h"
#include "wx/wx.h"
#include "EDTIncludes.h"
#include "Edit.h"

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

enum
{
	wxOgre_insertObject,
	wxOgre_insertObjectAsChild,
	wxOgre_editObject,
	wxOgre_deleteObject,
	wxOgre_createObjectgroup,
	wxOgre_saveObjectgroup,
	wxOgre_createChain,
	wxOgre_connectWaypoints,
	wxOgre_saveBones
};


	class wxOgre : 
		public wxControl/*, 
		public Ogre::Singleton<wxOgre>*/
	{
		DECLARE_CLASS(wxOgre) 
	public:
		/** A new wxOgre must receive a parent frame to which to attach 
		itself to */
		wxOgre(wxWindow* parent, wxWindowID id,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize, long style = 0,
           const wxString& name = wxPanelNameStr); 
		~wxOgre();

		void initOgre();

		Edit* GetEdit() { return mEdit; }

		/** Renders a single Ogre frame */
		void update();
		
		inline Ogre::RenderWindow* getRenderWindow() { return mRenderWindow; };
		inline Ogre::Camera* getCamera() { return mCamera; };
		inline void setCamera(Ogre::Camera* camera) { mCamera = camera;};
		void initEdit();
		
		/*
		Input stuff
		OIS überwacht das gesamte Fenster, während hier die Möglichkeit besteht, nur den Input des Ogre widgets zu überwachen.
		*/
		void OnKeyDown(wxKeyEvent& key);
		void OnKeyUp(wxKeyEvent& key);

		void OnSetFocus(wxFocusEvent& event);
		void OnKillFocus(wxFocusEvent& event);
		

	protected:
		DECLARE_EVENT_TABLE()

		void OnMouseEvent(wxMouseEvent& ev);

		void OnInsertObject(wxCommandEvent& WXUNUSED(event));
		void OnInsertObjectAsChild(wxCommandEvent& WXUNUSED(event));
		void OnDeleteObject(wxCommandEvent& WXUNUSED(event));
		void OnCreateObjectGroup(wxCommandEvent& WXUNUSED(event));
		void OnSaveObjectGroup(wxCommandEvent& WXUNUSED(event));
		void OnCreateChain(wxCommandEvent& WXUNUSED(event));
		void OnConnectWaypoints(wxCommandEvent& WXUNUSED(event));
		void OnSaveBones(wxCommandEvent& WXUNUSED(event));

	private:
		/** Creates an ogre rendering window and all other default objects
		such as the Ogre Root, default camera, default scene manager etc */ 
		void createOgreRenderWindow();
		/** Toggles the rendering timer on/off */
		void toggleTimerRendering();

		/** Callback function to a window resize event */
		void OnSize(wxSizeEvent& event);
		/** Callback function to a window paint event */
		void OnPaint(wxPaintEvent& event);
		/** Callback function to an EraseBackground event */
		void OnEraseBackground( wxEraseEvent& );
		/** Callback function to a timer "tick" event */
		void OnRenderTimer(wxTimerEvent& event);

		/* WX members */
		/** Rendering timer */
		wxTimer	mTimer;

		/* Ogre members */
		/** Local Ogre::Root pointer */
		Ogre::Root* mRoot;
		/** Local Ogre::RenderWindow pointer */
		Ogre::RenderWindow* mRenderWindow;
		Ogre::Camera *mCamera;

		Edit *mEdit;

		bool mInitialized;


	};


#endif // __WXOGRE_H__