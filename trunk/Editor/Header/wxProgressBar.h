
#pragma once

#include "wx/aui/aui.h"
#include "wx/wx.h"

class wxProgressBar : public wxStatusBar
{
	DECLARE_CLASS(wxProgressBar)

private:
	wxStaticText *mStatusMessage;
	wxGauge *mGauge;

public:
	wxProgressBar(wxWindow* parent, wxWindowID id);
	~wxProgressBar(void);

	/*
	Hide window and reset status messege.
	*/
	void Reset();

	void SetStatusMessage(wxString message);

	void SetProgress(float progress);
};
