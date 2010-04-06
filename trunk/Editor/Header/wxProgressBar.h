
#pragma once

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include "EDTIncludes.h"

class wxProgressBar : public wxStatusBar
{
	DECLARE_CLASS(wxProgressBar)
	DECLARE_EVENT_TABLE()

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
	void SetShowBar(bool show);

	void OnSize(wxSizeEvent& event);
};
