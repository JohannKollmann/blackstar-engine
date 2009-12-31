
#include "wxSGTToolbar.h"
#include "wx/image.h"

BEGIN_EVENT_TABLE(wxSGTToolbar, wxToolBar)

	EVT_TOOL(wxID_ANY, wxSGTToolbar::OnToolEvent)

END_EVENT_TABLE()

wxSGTToolbar::wxSGTToolbar(wxWindow *parent)
: 	wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_NODIVIDER)// | wxTB_VERTICAL)
{
	SetToolBitmapSize(wxSize(30,25));
	mCurrentID = 101;
}

wxSGTToolbar::~wxSGTToolbar(void)
{
}

int wxSGTToolbar::NextID()
{
	return mCurrentID++;
}

void wxSGTToolbar::OnToolEvent(wxCommandEvent &event)
{
	for (std::vector<SGTEDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		for (std::vector<SGTEDTTool>::iterator x = i->mTools.begin(); x != i->mTools.end(); x++)
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

int wxSGTToolbar::RegisterTool(Ogre::String toolname, Ogre::String toolgroup, Ogre::String bitmapname, SGTToolCallback callback, Ogre::String tooltip, bool checktool, bool checked)
{
	SGTEDTTool tool;
	wxImage image(bitmapname);
	tool.mBitmap = wxBitmap(image.Scale(30,25, wxIMAGE_QUALITY_HIGH));
	tool.mCallbackFunc = callback;
	tool.mName = toolname;
	tool.mCheckTool = checktool;
	tool.mChecked = checked;
	tool.mActive = true;
	tool.mToolID = NextID();
	tool.mToolTip = tooltip;
	for (std::vector<SGTEDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		if (i->mGroupname == toolgroup)
		{
			i->mTools.push_back(tool);
			return tool.mToolID;
		}
	}
	SGTEDTToolGroup group;
	group.mActive = false;
	group.mGroupname = toolgroup;
	group.mTools.push_back(tool);
	mToolGroups.push_back(group);
	return tool.mToolID;
}

void wxSGTToolbar::SetGroupStatus(Ogre::String groupname, bool active)
{
	for (std::vector<SGTEDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		if (i->mGroupname == groupname)
		{
			i->mActive = active;
			break;
		}
	}
	RefreshAll();
}

void wxSGTToolbar::SetToolStatus(Ogre::String toolname, bool active)
{
	for (std::vector<SGTEDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		for (std::vector<SGTEDTTool>::iterator x = i->mTools.begin(); x != i->mTools.end(); x++)
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

void wxSGTToolbar::CheckTool(Ogre::String toolname, bool check)
{
	for (std::vector<SGTEDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		for (std::vector<SGTEDTTool>::iterator x = i->mTools.begin(); x != i->mTools.end(); x++)
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

void wxSGTToolbar::RefreshAll()
{
	ClearTools();
	for (std::vector<SGTEDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		if (i->mActive)
		{
			if (i != mToolGroups.begin())
			{
				AddSeparator();
			}
			for (std::vector<SGTEDTTool>::iterator x = i->mTools.begin(); x != i->mTools.end(); x++)
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