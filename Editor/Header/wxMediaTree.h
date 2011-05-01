
#pragma once

#include "wxfileTree.h"
#include "wx/dnd.h"
#include "EntityTreeNotebookListener.h"
#include "EDTIncludes.h"

class wxMediaTree : public wxFileTree, public EntityTreeNotebookListener
{
private:
	bool mShowMeshes;
	bool mShowTextures;
	bool mShowSounds;

	void RefreshFilters();

	void ApplyDefaultLightning(Ogre::String materialfile);
	Ogre::String Scan_Line_For_Material(Ogre::String line);

	//Helper functions
	bool IsTexture(wxString filename);
	bool IsMaterial(wxString filename);
	bool IsMesh(wxString filename);
	bool IsAudio(wxString filename);


public:
	wxMediaTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT | wxTR_EDIT_LABELS,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = "wxMediaTree");
	~wxMediaTree()
	{
	}

	void OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item);
	void OnMenuCallback(int id);
	void OnSelectItemCallback();
	void OnRemoveItemCallback();
	void OnRenameItemCallback(Ogre::String oldPath, Ogre::String newPath, Ogre::String oldFile, Ogre::String newFile) override;
	void OnCreateFolderCallback(Ogre::String path);
	void OnDropExternFilesCallback(const wxArrayString& filenames);
	bool IsExternFileDropTarget() { return true; }
	void OnSetupDragCursor(wxDropSource &dropSource);

	static void OnToolbarEvent(int toolID, Ogre::String toolname);

	wxString GetDragName() { return "MediaDragged"; }

	void OnEnterTab();
	void OnLeaveTab();
};
