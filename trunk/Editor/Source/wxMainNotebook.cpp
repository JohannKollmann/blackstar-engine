
#include "wxMainNotebook.h"

BEGIN_EVENT_TABLE(wxMainNotebook, wxAuiNotebook)
	EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, wxMainNotebook::OnNotebookChanged)
END_EVENT_TABLE() 

IMPLEMENT_CLASS(wxMainNotebook, wxAuiNotebook)

wxMainNotebook::wxMainNotebook(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                  const wxSize& size, long style)
				  : wxAuiNotebook(parent, id, pos, size, style)
{

	mOgreWindow = new wxOgre(this, -1);
	AddPage(mOgreWindow, "Scene", false);
	mScriptEditor = new wxScriptEditor(this);
	AddPage(mScriptEditor, "Empty Script *", false);
}

wxMainNotebook::~wxMainNotebook(void)
{
}

void wxMainNotebook::OnNotebookChanged(wxAuiNotebookEvent& event)
{
	if (GetSelection() == 0)
	{
	}
	if (GetSelection() == 1)
	{
	}
}