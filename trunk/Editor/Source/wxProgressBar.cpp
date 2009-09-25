
#include "wxProgressBar.h"
#include "wxEdit.h"

IMPLEMENT_CLASS(wxProgressBar, wxStatusBar)

wxProgressBar::wxProgressBar(wxWindow* parent, wxWindowID id)
: wxStatusBar(parent, id)
{
	//mStatusMessage = new wxStaticText(this, -1, "Blackstar Editor - Ready", wxPoint(5, 5));
	SetStatusMessage("Blackstar Editor - Ready");
	mGauge = new wxGauge(this, -1, 100, wxPoint(200,2), wxSize(200, 20), wxGA_HORIZONTAL | wxGA_SMOOTH);
	SetProgress(0);
	
}

wxProgressBar::~wxProgressBar(void)
{
}


void wxProgressBar::Reset()
{
	SetProgress(0);
	SetStatusMessage("Blackstar Editor - Ready");
    wxEdit::Instance().GetAuiManager().Update();
}


void wxProgressBar::SetStatusMessage(wxString message)
{
	SetStatusText(message);
	//mStatusMessage->SetLabel(message);
	//wxEdit::Instance().GetAuiManager().Update();
}

void wxProgressBar::SetProgress(float progress)
{
	mGauge->Show((progress > 0.0f));
	mGauge->SetValue(progress * 100);
	wxEdit::Instance().GetAuiManager().Update();
}