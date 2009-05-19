
#ifndef __wxObjectFolderTree_H__
#define __wxObjectFolderTree_H__

#include "virtualdirtreectrl.h"
#include "EDTIncludes.h"
#include "wxEditSGTGameObject.h"
#include "SGTGameObject.h"

/* This class is an example of inheriting the wxVirtualDirTreeCtrl. It has the following added functionality:
	
	- Directories starting with a "." are not added, Debug, Release, CVS and SVN dir are also not added
	- The file descript.ion is not added, but the file description is read, and used as caption for files / dirs
	- what else ???
*/
	
class wxObjectFolderTree : public wxVirtualDirTreeCtrl
{
public:
	wxObjectFolderTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = "wxVirtualDirTreeCtrl")
		: wxVirtualDirTreeCtrl(parent, id, pos, size, style, validator, name)
	{
		mCurrentItem = NULL;
	}
    
	virtual ~wxObjectFolderTree()
	{
		// TODO:
	}

	void OnMenuEvent(wxCommandEvent& event);

	// overridden handlers

	virtual bool OnAddDirectory(VdtcTreeItemBase &item, const wxFileName &name);

	virtual bool OnAddFile(VdtcTreeItemBase &item, const wxFileName &name);

	void ShowMenu(VdtcTreeItemBase *item, const wxPoint& pt);

	Ogre::String GetSelectedResource();
	Ogre::String GetCurrentPath() { return mCurrentPath; };

protected:
    DECLARE_EVENT_TABLE()

	void OnItemMenu(wxTreeEvent &event);
	void OnSelChanged(wxTreeEvent &event);
	void OnItemActivated(wxTreeEvent &event);

	void OnSetRootPath(const wxString &root);

	VdtcTreeItemBase *mCurrentItem;
	Ogre::String mCurrentPath;

};

#endif

