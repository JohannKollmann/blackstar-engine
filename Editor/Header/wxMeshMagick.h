
#pragma once

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include "wx/checkbox.h"
#include "EDTIncludes.h"

class wxTreeCtrl;

class wxMeshMagick : public wxPanel
{

	DECLARE_CLASS(wxSettingsWindow)

protected:

	DECLARE_EVENT_TABLE()

	void OnApply(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);

	wxCheckBox *mOptimise;
	wxCheckBox *mScale;
	wxCheckBox *mTranslate;
	wxCheckBox *mRotate;
	wxCheckBox *mCenter;
	wxTextCtrl *mScaleEdt;
	wxTextCtrl *mTranslateEdt;
	wxTextCtrl *mRotateEdt;
	wxButton *mApplyBtn;

	wxArrayString mPaths;

public:
	wxMeshMagick(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);
	~wxMeshMagick(void);

	void SetPaths(wxArrayString paths);
};
