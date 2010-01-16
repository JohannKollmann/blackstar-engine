
#include "wxSettingsWindow.h"
#include "IceSceneManager.h"
#include "IceWeatherController.h"
#include "wxEdit.h"

enum
{
	ID_SetCameraMovementSpeed,
	ID_SetCameraRotationSpeed,
	ID_SetObjectMovementSpeed,
	ID_SetObjectRotationSpeed,
	ID_SetCaelumTimeScale,
};

IMPLEMENT_CLASS(wxSettingsWindow, wxPanel)

BEGIN_EVENT_TABLE(wxSettingsWindow, wxPanel)
	EVT_COMMAND_SCROLL(ID_SetCameraMovementSpeed, wxSettingsWindow::OnSetCameraMovementSpeed)
	EVT_COMMAND_SCROLL(ID_SetCameraRotationSpeed, wxSettingsWindow::OnSetCameraRotationSpeed)
	EVT_COMMAND_SCROLL(ID_SetObjectMovementSpeed, wxSettingsWindow::OnSetObjectMovementSpeed)
	EVT_COMMAND_SCROLL(ID_SetObjectRotationSpeed, wxSettingsWindow::OnSetObjectRotationSpeed)
	EVT_COMMAND_SCROLL(ID_SetCaelumTimeScale, wxSettingsWindow::OnSetCaelumTimeScale)
END_EVENT_TABLE()

wxSettingsWindow::wxSettingsWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: wxPanel(parent, id, pos, size)
{
        wxBoxSizer* s1 = new wxBoxSizer(wxHORIZONTAL);
        mCameraMovementSpeed = new wxSlider(this, ID_SetCameraMovementSpeed, 10, 0, 100, wxDefaultPosition, wxSize(200,30), wxSL_AUTOTICKS);
		mCameraMovementSpeed->SetTickFreq(10,0);
        s1->Add(1, 1, 1, wxEXPAND);
        s1->Add(new wxStaticText(this, wxID_ANY, wxT("Camera Movement Speed:")));
        s1->Add(mCameraMovementSpeed);
        s1->Add(1, 1, 1, wxEXPAND);
        s1->SetItemMinSize((size_t)1, 150, 30);

        wxBoxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
        mCameraRotationSpeed = new wxSlider(this, ID_SetCameraRotationSpeed, 20, 0, 100, wxDefaultPosition, wxSize(200,30), wxSL_AUTOTICKS);
		mCameraRotationSpeed->SetTickFreq(10,0);
        s2->Add(1, 1, 1, wxEXPAND);
        s2->Add(new wxStaticText(this, wxID_ANY, wxT("Camera Rotation Speed:")));
        s2->Add(mCameraRotationSpeed);
        s2->Add(1, 1, 1, wxEXPAND);
        s2->SetItemMinSize((size_t)1, 150, 30);

        wxBoxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
        mObjectMovementSpeed = new wxSlider(this, ID_SetObjectMovementSpeed, 20, 0, 100, wxDefaultPosition, wxSize(200,30), wxSL_AUTOTICKS);
		mObjectMovementSpeed->SetTickFreq(10,0);
        s3->Add(1, 1, 1, wxEXPAND);
        s3->Add(new wxStaticText(this, wxID_ANY, wxT("Object Movement Speed:")));
        s3->Add(mObjectMovementSpeed);
        s3->Add(1, 1, 1, wxEXPAND);
        s3->SetItemMinSize((size_t)1, 150, 30);

        wxBoxSizer* s4 = new wxBoxSizer(wxHORIZONTAL);
        mObjectRotationSpeed = new wxSlider(this, ID_SetObjectRotationSpeed, 30, 0, 100, wxDefaultPosition, wxSize(200,30), wxSL_AUTOTICKS);
		mObjectRotationSpeed->SetTickFreq(10,0);
        s4->Add(1, 1, 1, wxEXPAND);
        s4->Add(new wxStaticText(this, wxID_ANY, wxT("Object Rotation Speed:")));
        s4->Add(mObjectRotationSpeed);
        s4->Add(1, 1, 1, wxEXPAND);
        s4->SetItemMinSize((size_t)1, 150, 30);

        wxBoxSizer* s5 = new wxBoxSizer(wxHORIZONTAL);
        mCaelumTimeScale = new wxSlider(this, ID_SetCaelumTimeScale, 0, -50, 50, wxDefaultPosition, wxSize(200,30), wxSL_AUTOTICKS);
		mCaelumTimeScale->SetTickFreq(10,0);
        s5->Add(1, 1, 1, wxEXPAND);
        s5->Add(new wxStaticText(this, wxID_ANY, wxT("Time Scale:")));
        s5->Add(mCaelumTimeScale);
        s5->Add(1, 1, 1, wxEXPAND);
        s5->SetItemMinSize((size_t)1, 150, 30);

        wxGridSizer* grid_sizer = new wxGridSizer(1);
        grid_sizer->SetHGap(5);
        grid_sizer->Add(s1);
		grid_sizer->Add(s2);
		grid_sizer->Add(s3);
		grid_sizer->Add(s4);
		grid_sizer->Add(s5);

        wxBoxSizer* cont_sizer = new wxBoxSizer(wxVERTICAL);
        cont_sizer->Add(grid_sizer, 1, wxEXPAND | wxALL, 5);
        SetSizer(cont_sizer);
        GetSizer()->SetSizeHints(this);
}

wxSettingsWindow::~wxSettingsWindow(void)
{
}

void wxSettingsWindow::OnSetCameraMovementSpeed(wxScrollEvent& event)
{
	float val = (float)(event.GetInt());
	float factor = (val / 100.0) * 100.0f;
	wxEdit::Instance().GetOgrePane()->GetEdit()->SetCameraMoveSpeed(factor);
}

void wxSettingsWindow::OnSetCameraRotationSpeed(wxScrollEvent& event)
{
	float val = (float)(event.GetInt());
	float factor = (val / 100.0) * 1.0;
	wxEdit::Instance().GetOgrePane()->GetEdit()->SetCameraRotationSpeed(factor);
}

void wxSettingsWindow::OnSetObjectMovementSpeed(wxScrollEvent& event)
{
	float val = (float)(event.GetInt());
	float factor = (val / 100.0) * 5.0;
	wxEdit::Instance().GetOgrePane()->GetEdit()->SetObjectMoveSpeed(factor);
}

void wxSettingsWindow::OnSetObjectRotationSpeed(wxScrollEvent& event)
{
	float val = (float)(event.GetInt());
	float factor = (val / 100.0) * 1.0;
	wxEdit::Instance().GetOgrePane()->GetEdit()->SetObjectRotationSpeed(factor);
}

void wxSettingsWindow::OnSetCaelumTimeScale(wxScrollEvent& event)
{
	float val = (float)(event.GetInt());
	float factor = (val / 50.0) * 4096.0;
	Ice::SceneManager::Instance().GetWeatherController()->GetCaelumSystem()->getUniversalClock()->setTimeScale(factor);
}
