
#include "wxSGTToolbar.h"

BEGIN_EVENT_TABLE(wxSGTToolbar, wxToolBar)

	EVT_TOOL(wxID_ANY, wxSGTToolbar::OnToolEvent)

END_EVENT_TABLE()

wxSGTToolbar::wxSGTToolbar(wxWindow *parent)
: 	wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_NODIVIDER)// | wxTB_VERTICAL)
{
	SetToolBitmapSize(wxSize(28,24));
}

wxSGTToolbar::~wxSGTToolbar(void)
{
}

void wxSGTToolbar::OnToolEvent(wxCommandEvent &event)
{
}