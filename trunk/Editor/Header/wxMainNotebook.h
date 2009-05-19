#pragma once

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include "wxOgre.h"
#include "../wxConnex/connexspace.h"

class wxMainNotebook : public wxAuiNotebook
{
	DECLARE_CLASS(wxMainNotebook)

private:
	wxOgre *mOgreWindow;
	wxConnexSpace* m_pCXSpace;

protected:
	DECLARE_EVENT_TABLE() 

	void OnNotebookChanged(wxAuiNotebookEvent& event);

public:
	wxMainNotebook(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT);
	~wxMainNotebook(void);

	wxOgre* GetOgreWindow() { return mOgreWindow; };

};
