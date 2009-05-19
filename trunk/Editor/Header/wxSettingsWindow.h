#pragma once

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include "wx/spinctrl.h"
#include "wx/checkbox.h"
#include "wx/radiobox.h"
#include "wx/slider.h"

class wxSettingsWindow : public wxPanel
{
	DECLARE_CLASS(wxSettingsWindow)

protected:

	DECLARE_EVENT_TABLE()

	void OnSetCameraMovementSpeed(wxScrollEvent& event);
	void OnSetCameraRotationSpeed(wxScrollEvent& event);
	void OnSetObjectMovementSpeed(wxScrollEvent& event);
	void OnSetObjectRotationSpeed(wxScrollEvent& event);
	void OnSetCaelumTimeScale(wxScrollEvent& event);

private:
	wxSlider* mCameraMovementSpeed;
	wxSlider* mCameraRotationSpeed;

	wxSlider* mObjectMovementSpeed;
	wxSlider* mObjectRotationSpeed;

	wxSlider* mCaelumTimeScale;

public:
	wxSettingsWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);
	~wxSettingsWindow(void);
};
