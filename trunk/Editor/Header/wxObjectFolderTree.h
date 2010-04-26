
#ifndef __wxObjectFolderTree_H__
#define __wxObjectFolderTree_H__

#include "EDTIncludes.h"
#include "wxFileTree.h"
#include "wxEditIceGameObject.h"
#include "IceGameObject.h"
#include "EntityTreeNotebookListener.h"
#include "wxEdit.h"

class wxObjectFolderTree : public wxFileTree, public EntityTreeNotebookListener
{
private:
	Ice::GameObject *mPreviewObject;
	void CreateObjectPreview(Ogre::String file);
	void ClearObjectPreview();

public:
	wxObjectFolderTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT | wxTR_EDIT_LABELS,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = "wxVirtualDirTreeCtrl");
    
	~wxObjectFolderTree()
	{
	}

	static void OnToolbarEvent(int toolID, Ogre::String toolname);

	void OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item);
	void OnMenuCallback(int id);
	void OnSelectItemCallback();

	void OnEnterTab();
	void OnLeaveTab();
};

#endif

