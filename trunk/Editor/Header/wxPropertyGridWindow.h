
#pragma once

#include "wx/wx.h"
#include "wxEdit.h"
#include "wx/propgrid/propgrid.h"
#include <map>
#include "EDTIncludes.h"
#include "wxPropertyGridListener.h"
#include <wx/dnd.h>
#include "wxFileTree.h"


class wxPropertyGridWindow : public wxPanel
{
	DECLARE_CLASS(wxPropertygridWindow)

	class FileDropTarget : public wxFileDropTarget
	{
	private:
		wxPropertyGridWindow *mPropWindow;
	public:

		FileDropTarget(wxPropertyGridWindow *propWindow) : wxFileDropTarget(), mPropWindow(propWindow) {}
		~FileDropTarget() {}

		bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString&  filenames);
		wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def);
		void OnLeave();
	};

public:
	wxPropertyGridWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT,
                  const wxString& name = "wxPropertygridWindow");
	~wxPropertyGridWindow();
	
	void AddPage(wxPropertyGridListener* page, wxString name);
	wxPropertyGridListener* SetPage(wxString name);
	wxPropertyGridListener* GetPage(wxString name);
	wxPropertyGridListener* GetCurrentPage();
	Ogre::String GetCurrentPageName();

	void UpdateCurrentPage();

	void SetEmptySpaceColour(wxColour colour);

	void OnSetFocus(wxFocusEvent& event);

	void OnApply(wxCommandEvent& event);
	void OnResize(wxSizeEvent& event);
	void OnActivate(wxActivateEvent& event);
	void OnMouseEnterWindow(wxMouseEvent& event);	

protected:
	DECLARE_EVENT_TABLE()

private:

	wxPropertyGrid *mPropGrid;
	wxButton *mApplyBtn;

	wxPropertyGridListener* mCurrentPage;
	Ogre::String mCurrentPageName;
	std::map<wxString, wxPropertyGridListener*> mPages;
};