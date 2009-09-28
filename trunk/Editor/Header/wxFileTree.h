
#pragma once

#include "EDTIncludes.h"
#include "virtualdirtreectrl.h"
#include "Ogre.h"

class wxFileTree : public wxVirtualDirTreeCtrl
{
protected:
    DECLARE_EVENT_TABLE()

	void OnItemMenu(wxTreeEvent &event);
	void OnSelChanged(wxTreeEvent &event);
	virtual void OnItemActivated(wxTreeEvent &event);
	void OnBeginLabelEdit(wxTreeEvent& event);
	void OnEndLabelEdit(wxTreeEvent& event);

	void OnSetRootPath(const wxString &root);

	VdtcTreeItemBase *mCurrentItem;
	Ogre::String mCurrentPath;
	Ogre::String mRootPath;

public:
	wxFileTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT | wxTR_EDIT_LABELS,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = "wxVirtualDirTreeCtrl")
		: wxVirtualDirTreeCtrl(parent, id, pos, size, style, validator, name)
	{
		mCurrentItem = 0;
	}
	virtual ~wxFileTree()
	{
		// TODO:
	}

	void OnMenuEvent(wxCommandEvent& event);

	virtual bool OnAddDirectory(VdtcTreeItemBase &item, const wxFileName &name);

	virtual bool OnAddFile(VdtcTreeItemBase &item, const wxFileName &name);

	void ShowMenu(VdtcTreeItemBase *item, const wxPoint& pt);

	virtual void OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item) {};
	virtual void OnMenuCallback(int id) {};
	virtual void OnSelectItemCallback() {};

	Ogre::String GetSelectedResource();
	Ogre::String GetCurrentPath() { return mCurrentPath; } 
};