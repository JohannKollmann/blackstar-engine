
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

	mLastTab = 0;

	mResourcesTree = new wxObjectFolderTree(this,  -1, wxDefaultPosition, wxSize(250,400));

	mScriptTree = new wxScriptFileTree(this,  -1, wxDefaultPosition, wxSize(250,400));

	mSceneTree = new wxOgreSceneTree(this, -1, wxDefaultPosition,
		wxSize(250,500));
	mMaterialTree = new wxMaterialTree(this, -1, wxDefaultPosition,
		wxSize(250,500));
	mMediaTree = new wxMediaTree(this, wxID_ANY, wxDefaultPosition, wxSize(300,500));
	AddTab(mMediaTree, mMediaTree, "Assets");
	AddTab(mResourcesTree, mResourcesTree, "Resources");
	AddTab(mScriptTree, mScriptTree, "Scripts");
	AddTab(mMaterialTree, mMaterialTree, "Materials");
	AddTab(mSceneTree, mSceneTree, "Scene");
}

wxEntityTreeNotebook::~wxEntityTreeNotebook(void)
{
}

void wxEntityTreeNotebook::AddTab(EntityTreeNotebookListener* tab, wxWindow *window, Ogre::String caption)
{
	mTabs.push_back(tab);
	AddPage(window, caption, false);
}

void wxEntityTreeNotebook::OnNotebookChanged(wxAuiNotebookEvent& event)
{
	if (mLastTab) mLastTab->OnLeaveTab();
	mLastTab = mTabs[GetSelection()];
	mLastTab->OnEnterTab();
}