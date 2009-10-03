
#pragma once

#include "EDTIncludes.h"
#include "virtualdirtreectrl.h"
#include "Ogre.h"
#include <wx/dnd.h>

class wxDnDTreeItemData : public wxDataObjectSimple
{
protected:
	VdtcTreeItemBase *mTreeItem;

public:
	wxDnDTreeItemData() : wxDataObjectSimple(wxDataFormat("wxDnDTreeItemData"))
	{
		mTreeItem = 0;
	}
	wxDnDTreeItemData(VdtcTreeItemBase* item) : wxDataObjectSimple(wxDataFormat("wxDnDTreeItemData"))
	{
		mTreeItem = item;
	}
	~wxDnDTreeItemData() {}

	size_t GetDataSize() const;
	bool GetDataHere(void *buf) const;
	bool SetData(size_t len, const void *buf);

	VdtcTreeItemBase* GetItem();

	DECLARE_NO_COPY_CLASS(wxDnDTreeItemData)
};

class wxFileTree : public wxVirtualDirTreeCtrl
{
protected:
    DECLARE_EVENT_TABLE()

	virtual void OnItemMenu(wxTreeEvent &event);
	virtual void OnSelChanged(wxTreeEvent &event);
	virtual void OnItemActivated(wxTreeEvent &event);
	virtual void OnBeginLabelEdit(wxTreeEvent& event);
	virtual void OnEndLabelEdit(wxTreeEvent& event);
	virtual void OnBeginDrag(wxTreeEvent& event);

	virtual wxString GetDragName() { return ""; }

	void OnLeftDown(wxMouseEvent& ev);

	void OnSetRootPath(const wxString &root);

	VdtcTreeItemBase *mCurrentItem;
	VdtcTreeItemBase* mCurrentlyDragged;
	bool mDraggingFile;
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
		mCurrentlyDragged = 0;
		mDraggingFile = false;
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
	virtual void OnRenameItemCallback(Ogre::String oldpath, Ogre::String newpath) {};

	Ogre::String GetSelectedResource();
	VdtcTreeItemBase* GetDraggedItem();
	Ogre::String GetCurrentPath() { return mCurrentPath; } 
};