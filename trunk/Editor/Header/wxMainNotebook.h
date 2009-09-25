#pragma once

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include "wxOgre.h"
#include "wxScriptEditor.h"

struct ScriptEditorCaptionBind
{
	wxString mCaption;
	wxScriptEditor *mScriptEditor;
};

class wxMainNotebook : public wxAuiNotebook
{
	DECLARE_CLASS(wxMainNotebook)

private:
	wxOgre *mOgreWindow;
	std::list<ScriptEditorCaptionBind> mScriptTabs;

protected:
	DECLARE_EVENT_TABLE() 

	void OnNotebookChanged(wxAuiNotebookEvent& event);
	void OnPageClosed(wxAuiNotebookEvent& event);

public:
	wxMainNotebook(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT);
	~wxMainNotebook(void);

	wxOgre* GetOgreWindow() { return mOgreWindow; };

	static void OnToolbarEvent(int toolID, Ogre::String toolname);

	void AddScriptTab(wxString caption, wxString fullPath);
	void SetModified(wxScriptEditor *tab, bool modified);

};
