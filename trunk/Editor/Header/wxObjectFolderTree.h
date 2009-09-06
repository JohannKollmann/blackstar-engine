
#ifndef __wxObjectFolderTree_H__
#define __wxObjectFolderTree_H__

#include "EDTIncludes.h"
#include "wxFileTree.h"
#include "wxEditSGTGameObject.h"
#include "SGTGameObject.h"

class wxObjectFolderTree : public wxFileTree
{
public:
	wxObjectFolderTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = "wxVirtualDirTreeCtrl")
		: wxFileTree(parent, id, pos, size, style, validator, name)
	{
		wxArrayString extensions;
		extensions.Add("*.ot");
		extensions.Add("*.ocs");
		extensions.Add("*.static");
		SetExtensions(extensions);
		SetRootPath("Data/Editor/Objects");
	}
    
	~wxObjectFolderTree()
	{
	}

	void OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item);
	void OnMenuCallback(int id);
	void OnSelectItemCallback();
};

#endif

