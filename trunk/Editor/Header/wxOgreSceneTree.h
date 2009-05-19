
#ifndef __wxOgreSceneTree_H__
#define __wxOgreSceneTree_H__

/*
Zeigt ein SGTGameObject mit allen children geordnet nach Typ (Licht, Geometrie, Partikeleffekt usw.) an.
*/

#include <wx/dynarray.h>
#include <wx/treectrl.h>
#include <wx/filesys.h>
#include <wx/imaglist.h>
#include "virtualdirtreectrl.h"
#include "Ogre.h"
#include "wxEdit.h"
#include "SGTGameObject.h"


class OgreTreeItemBase : public wxTreeItemData
{
protected:
	wxString _name;
	int _type;
	SGTGameObject *mNode;

public:
	OgreTreeItemBase(int type, const wxString &name, SGTGameObject *node)
		: _type(type)
		, _name(name)
		, mNode(node)
	{
	};

	/** Default destructor */
	~OgreTreeItemBase()
	{
		// NOTE: do not delete the tree item
		// because the tree item deletes this item data
	};

	/** Virtual function to report the caption back to the wxTreeCtrl to be added. If the caption should be
	    something else then the default name it gets from the file (or the root path when this node points
		to a root item, inherit this class and redefine GetCaption
	*/
	virtual const wxString &GetCaption() const {
		return _name;
	};

	SGTGameObject* getNode() { return mNode; };

	/** Virtual function to return the icon ID this node should get. Per default it gets the ID of the
	    default image list. If you assigned more bitmaps (or different bitmaps) to the image list, return
		the proper indices based upon the class it refers to. The ID's returned are:

		- VDTC_ICON_ROOT: For root
		- VDTC_ICON_DIR: For a directory
		- VDTC_ICON_FILE: For a file
	*/
	virtual int GetIconId() const {
		switch(_type)
		{
			case VDTC_TI_ROOT:
				return VDTC_ICON_ROOT;
			case VDTC_TI_DIR:
				return VDTC_ICON_DIR;
			case VDTC_TI_FILE:
				return VDTC_ICON_FILE;
		}
		return -1;
	};

	/** Virtual function to return the selected icon ID this node should get. Per default there is no icon
	    associated with a selection. If you would like a selection, inherit this class and redefine this function
		to return a proper id.
	*/
	virtual int GetSelectedIconId() const {
		return -1;
	};

	/** Gets this name. The name of the root is the base path of the whole directory, the
	   name of a file node is the filename, and from a dir node the directory name.
	   \sa IsDir, IsFile, IsRoot */
	const wxString &GetName() {
		return _name;
	};

	/** Returns true if this is of type VDTC_TI_DIR */
	bool IsDir() const {
		return _type == VDTC_TI_DIR;
	};

	/** Returns true if this is of type VDTC_TI_ROOT */
	bool IsRoot() const {
		return _type == VDTC_TI_ROOT;
	};

	/** Returns true if this is of type VDTC_TI_FILE */
	bool IsFile() const {
		return _type == VDTC_TI_FILE;
	};

};

WX_DEFINE_ARRAY(OgreTreeItemBase *, OgreTreeItemBaseArray);

class wxOgreSceneTree : public wxTreeCtrl
{
private:
	/** Icons image list */
	wxImageList *_iconList;
	/** Extra flags */
	int _flags;

	OgreTreeItemBase *mStart;

	OgreTreeItemBase *mSelectedItem;

protected:
	/** This method can be used in the method OnAssignIcons. It returns a pointer to a newly created bitmap
	    holding the default icon image for a root node. NOTE: When this bitmap is assigned to the icon list,
		don't forget to delete it! */
	wxBitmap *CreateRootBitmap();

	/** This method can be used in the method OnAssignIcons. It returns a pointer to a newly created bitmap
	    holding the default icon image for a folder node. NOTE: When this bitmap is assigned to the icon list,
		don't forget to delete it! */
	wxBitmap *CreateFolderBitmap();

	/** This method can be used in the method OnAssignIcons. It returns a pointer to a newly created bitmap
	    holding the default icon image for a file node. NOTE: When this bitmap is assigned to the icon list,
		don't forget to delete it! */
	wxBitmap *CreateNodeBitmap();


	void OnItemMenu(wxTreeEvent &event);
	void OnItemActivated(wxTreeEvent &event);

	bool ExpandToObject(OgreTreeItemBase *from, SGTGameObject *object);

public:
    /** Default constructor of this control. It is similar to the wxTreeCtrl */
    wxOgreSceneTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT,
                  const wxValidator& validator = wxDefaultValidator,
                  const wxString& name = "wxOgreSceneTree");
    virtual ~wxOgreSceneTree();

	SGTGameObject* GetSelectedItem();

	void Update();

	void ScanFromNode(OgreTreeItemBase *item, SGTGameObject *scanFrom, bool subScan = true);

	void AddItemsToTreeCtrl(OgreTreeItemBase *item, OgreTreeItemBaseArray &items);

	void OnExpanding(wxTreeEvent &event);

	bool _itemHasFolder(OgreTreeItemBase *item, wxString& FolderName, wxTreeItemId *destfolderID);

	bool _itemHasChild(wxTreeItemId itemID, SGTGameObject *item, SGTGameObject* node);

	virtual void OnAssignIcons(wxImageList &icons);
	virtual OgreTreeItemBase *OnCreateTreeItem(int type, SGTGameObject *node);

	void OnSelChanged(wxTreeEvent &event);

	void ExpandToObject(SGTGameObject *object);


private:
    DECLARE_EVENT_TABLE()
};

#endif