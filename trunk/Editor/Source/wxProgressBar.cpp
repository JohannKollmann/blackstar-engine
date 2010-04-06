
#include "wxProgressBar.h"
#include "wxEdit.h"

IMPLEMENT_CLASS(wxProgressBar, wxStatusBar)

BEGIN_EVENT_TABLE(wxProgressBar, wxStatusBar)
    EVT_SIZE(wxProgressBar::OnSize)
END_EVENT_TABLE()

wxProgressBar::wxProgressBar(wxWindow* parent, wxWindowID id)
: wxStatusBar(parent, id)
{
	SetFieldsCount(2);
	static const int widthsFor2Fields[] = { -1, 200 };
	SetStatusWidths(2, widthsFor2Fields);

	//mStatusMessage = new wxStaticText(this, -1, "Blackstar Editor - Ready", wxPoint(5, 5));
	SetStatusMessage("Blackstar Editor - Ready");
	mGauge = new wxGauge(this, -1, 100);
	mGauge->SetRange(100);
	mGauge->Show(false);
	SetMinHeight(15);

	Reset();
	
}

wxProgressBar::~wxProgressBar(void)
{
}


void wxProgressBar::Reset()
{
	//mGauge->Show(false);
	mGauge->SetValue(0);
	SetStatusMessage("Blackstar Editor - Ready");
	mGauge->Show(false);
    //wxEdit::Instance().GetAuiManager().Update();
}


void wxProgressBar::SetStatusMessage(wxString message)
{
	SetStatusText(message);
	//mStatusMessage->SetLabel(message);
	//wxEdit::Instance().GetAuiManager().Update();
}

void wxProgressBar::SetProgress(float progress)
{
	mGauge->Show((progress > 0));
	int pos = static_cast<int>(progress * 100.0f);
	mGauge->SetValue(pos);
	mGauge->Refresh();
	mGauge->Update();
}

void wxProgressBar::SetShowBar( bool show )
{
	mGauge->Show(show);
	mGauge->Refresh();
	wxEdit::Instance().GetAuiManager().Update();
}

void wxProgressBar::OnSize( wxSizeEvent& event )
{
    wxRect rect;
    GetFieldRect(1, rect);

    mGauge->SetSize(rect.x + 2, rect.y + 2, rect.width - 4, rect.height - 4);
}
