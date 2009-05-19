
#include "wxMeshMagick.h"
#include "Ogre.h"
#include "TransformTool.h"
#include "OptimiseTool.h"
#include "wxProgressBar.h"
#include "wxEdit.h"
#include "OgreEnvironment.h"

enum
{
	ID_MeshMagickApply,
};

IMPLEMENT_CLASS(wxMeshMagick, wxPanel)

BEGIN_EVENT_TABLE(wxMeshMagick, wxPanel)
	EVT_BUTTON(ID_MeshMagickApply, wxMeshMagick::OnApply)
END_EVENT_TABLE()

wxMeshMagick::wxMeshMagick(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: wxPanel(parent, id, pos, size)
{
        wxBoxSizer* s1 = new wxBoxSizer(wxHORIZONTAL);
        mOptimise = new wxCheckBox(this, -1, "Optimise");
        s1->Add(1, 1, 1, wxEXPAND);
        s1->Add(mOptimise);
        s1->Add(1, 1, 1, wxEXPAND);
        s1->SetItemMinSize((size_t)1, 75, 25);

        wxBoxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
        mCenter = new wxCheckBox(this, -1, "Center");
        s2->Add(1, 1, 1, wxEXPAND);
        s2->Add(mCenter);
        s2->Add(1, 1, 1, wxEXPAND);
        s2->SetItemMinSize((size_t)1, 75, 25);

        wxBoxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
        mScale = new wxCheckBox(this, -1, "Scale");
		mScaleEdt = new wxTextCtrl(this, -1, "1.0/1.0/1.0", wxDefaultPosition, wxSize(150,20));
        s3->Add(1, 1, 1, wxEXPAND);
        s3->Add(mScale);
		s3->Add(mScaleEdt);
        s3->Add(1, 1, 1, wxEXPAND);
        s3->SetItemMinSize((size_t)1, 75, 25);

        wxBoxSizer* s4 = new wxBoxSizer(wxHORIZONTAL);
        mRotate = new wxCheckBox(this, -1, "Rotate");
		mRotateEdt = new wxTextCtrl(this, -1, "90/1/0/0", wxDefaultPosition, wxSize(150,20));
        s4->Add(1, 1, 1, wxEXPAND);
        s4->Add(mRotate);
		s4->Add(mRotateEdt);
        s4->Add(1, 1, 1, wxEXPAND);
        s4->SetItemMinSize((size_t)1, 75, 25);

		wxBoxSizer* s5 = new wxBoxSizer(wxHORIZONTAL);
		mApplyBtn = new wxButton(this, ID_MeshMagickApply, "Go");
        s5->Add(1, 1, 1, wxEXPAND);
        s5->Add(mApplyBtn);
        s5->Add(1, 1, 1, wxEXPAND);
        s5->SetItemMinSize((size_t)1, 75, 25);

        wxGridSizer* grid_sizer = new wxGridSizer(1);
        grid_sizer->SetHGap(5);
        grid_sizer->Add(s1);
		grid_sizer->Add(s2);
		grid_sizer->Add(s3);
		grid_sizer->Add(s4);

        wxBoxSizer* cont_sizer = new wxBoxSizer(wxVERTICAL);
        cont_sizer->Add(grid_sizer, 1, wxEXPAND | wxALL, 5);
		wxSizerFlags sf;
		cont_sizer->Add(s5, wxSizerFlags().Centre());
        SetSizer(cont_sizer);
        GetSizer()->SetSizeHints(this);

		meshmagick::OgreEnvironment* ogreEnv = new meshmagick::OgreEnvironment();
		ogreEnv->initialize(false, Ogre::LogManager::getSingleton().getLog("ogre.graphics.log"));
}

wxMeshMagick::~wxMeshMagick(void)
{
}

void wxMeshMagick::SetPaths(wxArrayString paths)
{
	mPaths = paths;
}

void wxMeshMagick::OnApply(wxCommandEvent& event)
{
	wxEdit::Instance().GetProgressBar()->Reset();
	wxEdit::Instance().GetProgressBar()->SetStatusMessage("Processing meshes...");
	size_t count = mPaths.GetCount();
	for ( size_t n = 0; n < count; n++ )
	{
		wxEdit::Instance().GetProgressBar()->SetProgress((n+1) * (1.0f / (float)((mPaths.GetCount() + 1))));

		Ogre::StringVector sv;
		sv.push_back(Ogre::String(mPaths[n].c_str()));

		meshmagick::OptionList globalOptions;

		if (mOptimise->IsChecked())
		{
			meshmagick::OptionList toolOptions;

			meshmagick::OptimiseTool ot;// = new meshmagick::OptimiseTool();
			ot.invoke(globalOptions, toolOptions, sv, sv);
		}

		bool transform = false;
		meshmagick::OptionList transformOptions;

		if (mScale->IsChecked())
		{
			transform = true;

			Ogre::StringVector components = Ogre::StringUtil::split(Ogre::String(mScaleEdt->GetLineText(0).c_str()), "/");
			if (components.size() != 3)
			{
				Ogre::LogManager::getSingleton().logMessage("Wrong Params for Ogre::Vector3!");
				return;
			}
			Ogre::Any value = Ogre::Any(Ogre::Vector3(
                    Ogre::StringConverter::parseReal(components[0]),
                    Ogre::StringConverter::parseReal(components[1]),
                    Ogre::StringConverter::parseReal(components[2])));
			transformOptions.push_back(meshmagick::Option("scale", value));
		}
		if (mCenter->IsChecked())
		{
			transform = true;

			transformOptions.push_back(meshmagick::Option("xalign", Ogre::Any(Ogre::String("center"))));
			transformOptions.push_back(meshmagick::Option("yalign", Ogre::Any(Ogre::String("center"))));
			transformOptions.push_back(meshmagick::Option("zalign", Ogre::Any(Ogre::String("center"))));
		}
		if (mRotate->IsChecked())
		{
			transform = true;

			Ogre::StringVector components = Ogre::StringUtil::split(Ogre::String(mRotateEdt->GetLineText(0).c_str()), "/");
			if (components.size() != 4)
			{
				Ogre::LogManager::getSingleton().logMessage("Wrong Params for Ogre::Quaternion!");
				return;
			}
			Ogre::Any value = Ogre::Any(Ogre::Quaternion(
								Ogre::Degree(Ogre::StringConverter::parseReal(components[0])),
									Ogre::Vector3(Ogre::StringConverter::parseReal(components[1]),
									Ogre::StringConverter::parseReal(components[2]),
									Ogre::StringConverter::parseReal(components[3])).normalisedCopy()));
			transformOptions.push_back(meshmagick::Option("rotate", value));
		}

		if (transform)
		{
			meshmagick::TransformTool tt;// = new meshmagick::TransformTool();
			tt.invoke(globalOptions, transformOptions, sv, sv);
		}

		Ogre::ResourceManager::ResourceMapIterator RI = Ogre::MeshManager::getSingleton().getResourceIterator();
		while (RI.hasMoreElements())
		{
			Ogre::MeshPtr mesh = RI.getNext();
			if (mesh->getName() == Ogre::String(mPaths[n].c_str()))
			{
				mesh->reload();
			}
		}
	}

	wxEdit::Instance().GetProgressBar()->Reset();
	wxEdit::Instance().GetAuiManager().GetPane(wxT("meshmagick")).Float().Hide();
	wxEdit::Instance().GetAuiManager().Update();

}

void wxMeshMagick::OnCancel(wxCommandEvent& event)
{
}