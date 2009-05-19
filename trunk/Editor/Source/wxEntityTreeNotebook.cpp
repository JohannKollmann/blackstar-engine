
#include "wxEntityTreeNotebook.h"
#include "Ogre.h"

BEGIN_EVENT_TABLE(wxEntityTreeNotebook, wxAuiNotebook)
	EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, wxEntityTreeNotebook::OnNotebookChanged)
END_EVENT_TABLE() 

IMPLEMENT_CLASS(wxEntityTreeNotebook, wxAuiNotebook)

wxEntityTreeNotebook::wxEntityTreeNotebook(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                  const wxSize& size, long style)
				  : wxAuiNotebook(parent, id, pos, size, style)
{

	mResourcesTree = new wxObjectFolderTree(this,  -1, wxDefaultPosition, wxSize(250,400));

	mSceneTree = new wxOgreSceneTree(this, -1, wxDefaultPosition,
		wxSize(250,500));
	mMaterialTree = new wxMaterialTree(this, -1, wxDefaultPosition,
		wxSize(250,500));
	AddPage(mSceneTree, "Scenetree", false);
	AddPage(mResourcesTree, "Resources", false);
	AddPage(mMaterialTree, "Materials", false);
}

wxEntityTreeNotebook::~wxEntityTreeNotebook(void)
{
}

void wxEntityTreeNotebook::OnNotebookChanged(wxAuiNotebookEvent& event)
{
	if (GetSelection() == 0)
	{
		wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject");
		if (mSceneTree->GetSelectedItem() != 0)
		{
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(mSceneTree->GetSelectedItem());
		}
	}
	if (GetSelection() == 1)
	{
		wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject");
		if (mResourcesTree->GetSelectedResource() != "None")
		{
			if (mResourcesTree->GetSelectedResource().find(".ot") == Ogre::String::npos)
			{
				((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetResource(mResourcesTree->GetSelectedResource());
			}
		}
	}
	if (GetSelection() == 2)
	{
		wxEdit::Instance().GetpropertyWindow()->SetPage("material");
	}
}