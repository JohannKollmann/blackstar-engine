
#ifndef __wxPropertyGridWindow_H__
#define __wxPropertyGridWindow_H__

#include "wx/wx.h"
#include "wxEdit.h"
#include "wx/propgrid/propgrid.h"
#include <map>
#include "EDTIncludes.h"
#include "wxPropertyGridListener.h"
#include <wx/dnd.h>
#include "wxFileTree.h"


class wxPropertyGridWindow : public wxPanel, public wxTextDropTarget
{
	DECLARE_CLASS(wxPropertygridWindow)

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

protected:
	DECLARE_EVENT_TABLE()

	  void OnApply(wxCommandEvent& event);
	  void OnResize(wxSizeEvent& event);
	  void OnSetFocus(wxFocusEvent& event);

	bool OnDropText(wxCoord x, wxCoord y, const wxString&  data);

private:

	wxPropertyGrid *mPropGrid;
	wxButton *mApplyBtn;

	wxPropertyGridListener* mCurrentPage;
	Ogre::String mCurrentPageName;
	std::map<wxString, wxPropertyGridListener*> mPages;
};

#endif