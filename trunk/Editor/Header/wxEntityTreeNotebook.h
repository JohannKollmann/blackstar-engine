#pragma once

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include "wxObjectFolderTree.h"
#include "wxOgreSceneTree.h"
#include "wxMaterialTree.h"
#include "wxScriptFileTree.h"
#include "wxMediaTree.h"

class wxEntityTreeNotebook : public wxAuiNotebook
{

	DECLARE_CLASS(wxEntityTreeNotebook)

private:
	std::vector<EntityTreeNotebookListener*> mTabs;
	EntityTreeNotebookListener *mLastTab;
	wxObjectFolderTree *mResourcesTree;
	wxOgreSceneTree *mSceneTree;
	wxMaterialTree *mMaterialTree;
	wxScriptFileTree *mScriptTree;
	wxMediaTree* mMediaTree;

protected:
	DECLARE_EVENT_TABLE() 

	void OnNotebookChanged(wxAuiNotebookEvent& event);

public:
	wxEntityTreeNotebook(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT);
	~wxEntityTreeNotebook(void);

	void AddTab(EntityTreeNotebookListener* tab, wxWindow *window, Ogre::String caption);

	wxObjectFolderTree* GetResourceTree() { return mResourcesTree; };
	wxOgreSceneTree* GetSceneTree() { return mSceneTree; };
	wxMaterialTree* GetMaterialTree() { return mMaterialTree; };
	wxScriptFileTree* GetScriptTree() { return mScriptTree; };
	wxMediaTree* GetMediaTree() { return mMediaTree; };

};
