
#pragma once

#include "EDTIncludes.h"
#include "wxFileTree.h"
#include "EntityTreeNotebookListener.h"

class wxScriptFileTree : public wxFileTree, public EntityTreeNotebookListener
{
protected:
	void OnItemActivated(wxTreeEvent &event);
public:
	wxScriptFileTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = "wxVirtualDirTreeCtrl");
	~wxScriptFileTree()
	{
	}

	void OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item);
	void OnMenuCallback(int id);
	void OnSelectItemCallback();

	static void OnToolbarEvent(int toolID, Ogre::String toolname);

	void OnEnterTab();
	void OnLeaveTab();
};