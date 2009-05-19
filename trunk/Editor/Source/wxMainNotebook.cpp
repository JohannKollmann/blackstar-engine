
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

	m_pCXSpace = new wxConnexSpace( this, wxID_ANY, wxPoint(0,0), wxSize(150, 150), wxNO_BORDER);
	m_pCXSpace->SetBackgroundColour( wxColor(128,128,128,255) );
	AddPage(m_pCXSpace, "Game Logic", false);
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