
#pragma once

#include "wxfiletree.h"
#include "wx/dnd.h"
#include "EntityTreeNotebookListener.h"

class wxMediaTree : public wxFileTree, public wxFileDropTarget, public EntityTreeNotebookListener
{
private:
	bool mShowMeshes;
	bool mShowTextures;
	bool mShowSounds;

	void RefreshFilters();

	void ApplyDefaultLightning(Ogre::String materialfile);
	Ogre::String Scan_Line_For_Material(Ogre::String line);

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
	void OnRenameItemCallback(Ogre::String oldpath, Ogre::String newpath);
	void OnCreateFolderCallback(Ogre::String path);

	static void OnToolbarEvent(int toolID, Ogre::String toolname);

	wxString GetDragName() { return "MediaDragged"; }

	bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString&  filenames);

	//Attention: MSW only!
	wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);

	void OnEnterTab();
	void OnLeaveTab();
};
