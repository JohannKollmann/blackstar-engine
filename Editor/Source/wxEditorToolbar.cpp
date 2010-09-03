
#include "wxEditorToolbar.h"
#include "wx/image.h"

BEGIN_EVENT_TABLE(wxEditorToolbar, wxToolBar)

	EVT_TOOL(wxID_ANY, wxEditorToolbar::OnToolEvent)

END_EVENT_TABLE()

wxEditorToolbar::wxEditorToolbar(wxWindow *parent)
: 	wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_NODIVIDER)// | B_VERTICAL)
{
	SetToolBitmapSize(wxSize(30,25));
	mCurrentID = 101;
}

wxEditorToolbar::~wxEditorToolbar(void)
{
}

int wxEditorToolbar::NextID()
{
	return mCurrentID++;
}

void wxEditorToolbar::OnToolEvent(wxCommandEvent &event)
{
	for (std::vector<EDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		for (std::vector<EDTTool>::iterator x = i->mTools.begin(); x != i->mTools.end(); x++)
		{
			if (x->mToolID == event.GetId())
			{
				x->mChecked = GetToolState(x->mToolID);
				x->mCallbackFunc(event.GetId(), x->mName);
				return;
			}
		}
	}
}

int wxEditorToolbar::RegisterTool(Ogre::String toolname, Ogre::String toolgroup, Ogre::String bitmapname, ToolCallback callback, Ogre::String tooltip, bool checktool, bool checked)
{
	EDTTool tool;
	wxImage image(bitmapname);
	tool.mBitmap = wxBitmap(image.Scale(30,25, wxIMAGE_QUALITY_HIGH));
	tool.mCallbackFunc = callback;
	tool.mName = toolname;
	tool.mCheckTool = checktool;
	tool.mChecked = checked;
	tool.mActive = true;
	tool.mToolID = NextID();
	tool.mToolTip = tooltip;
	for (std::vector<EDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		if (i->mGroupname == toolgroup)
		{
			i->mTools.push_back(tool);
			return tool.mToolID;
		}
	}
	EDTToolGroup group;
	group.mActive = false;
	group.mGroupname = toolgroup;
	group.mTools.push_back(tool);
	mToolGroups.push_back(group);
	return tool.mToolID;
}

void wxEditorToolbar::SetGroupStatus(Ogre::String groupname, bool active)
{
	for (std::vector<EDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		if (i->mGroupname == groupname)
		{
			i->mActive = active;
			break;
		}
	}
	RefreshAll();
}

void wxEditorToolbar::SetToolStatus(Ogre::String toolname, bool active)
{
	for (std::vector<EDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		for (std::vector<EDTTool>::iterator x = i->mTools.begin(); x != i->mTools.end(); x++)
		{
			if (x->mName == toolname)
			{
				x->mActive = active;
				RefreshAll();
				return;
			}
		}
	}
}

void wxEditorToolbar::CheckTool(Ogre::String toolname, bool check)
{
	for (std::vector<EDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		for (std::vector<EDTTool>::iterator x = i->mTools.begin(); x != i->mTools.end(); x++)
		{
			if (x->mName == toolname)
			{
				x->mChecked = check;
				ToggleTool(x->mToolID, check);
				return;
			}
		}
	}
}

void wxEditorToolbar::RefreshAll()
{
	ClearTools();
	for (std::vector<EDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		if (i->mActive)
		{
			if (i != mToolGroups.begin())
			{
				AddSeparator();
			}
			for (std::vector<EDTTool>::iterator x = i->mTools.begin(); x != i->mTools.end(); x++)
			{
				if (x->mActive)
				{
					if (x->mCheckTool)
					{
						AddCheckTool(x->mToolID, x->mName, x->mBitmap, wxNullBitmap, x->mToolTip.c_str());
						if (x->mChecked) ToggleTool(x->mToolID, true);
					}
					else AddTool(x->mToolID, x->mName, x->mBitmap, x->mToolTip.c_str());
				}
			}
		}
	}
	Realize();
	wxEdit::Instance().RefreshToolbars();
}