
#pragma once

#include "EDTIncludes.h"
#include "wxFileTree.h"

class wxScriptFileTree : public wxFileTree
{
public:
	wxScriptFileTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = "wxVirtualDirTreeCtrl")
		: wxFileTree(parent, id, pos, size, style, validator, name)
	{
		wxArrayString extensions;
		extensions.Add("*.lua");
		SetExtensions(extensions);
		SetRootPath("Data/Scripts/Game");
	}
	~wxScriptFileTree()
	{
	}

	void OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item);
	void OnMenuCallback(int id);
	void OnSelectItemCallback();
};