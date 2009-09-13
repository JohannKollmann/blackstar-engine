
#include "wxSGTToolbar.h"
#include "wx/image.h"

BEGIN_EVENT_TABLE(wxSGTToolbar, wxToolBar)

	EVT_TOOL(wxID_ANY, wxSGTToolbar::OnToolEvent)

END_EVENT_TABLE()

wxSGTToolbar::wxSGTToolbar(wxWindow *parent)
: 	wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_NODIVIDER)// | wxTB_VERTICAL)
{
	SetToolBitmapSize(wxSize(28,24));
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
				x->mCallbackFunc(event.GetId(), x->mName);
				return;
			}
		}
	}
}

void wxSGTToolbar::RegisterTool(Ogre::String toolname, Ogre::String toolgroup, Ogre::String bitmapname, SGTToolCallback callback, bool checktool)
{
	SGTEDTTool tool;
	wxImage image(bitmapname);
	tool.mBitmap = wxBitmap(image.Scale(28,24, wxIMAGE_QUALITY_HIGH));
	tool.mCallbackFunc = callback;
	tool.mName = toolname;
	tool.mCheckTool = checktool;
	tool.mToolID = NextID();
	for (std::vector<SGTEDTToolGroup>::iterator i = mToolGroups.begin(); i != mToolGroups.end(); i++)
	{
		if (i->mGroupname == toolgroup)
		{
			i->mTools.push_back(tool);
			return;
		}
	}
	SGTEDTToolGroup group;
	group.mActive = false;
	group.mGroupname = toolgroup;
	group.mTools.push_back(tool);
	mToolGroups.push_back(group);
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
				if (x->mCheckTool) AddCheckTool(x->mToolID, x->mName, x->mBitmap);
				else AddTool(x->mToolID, x->mName, x->mBitmap);
			}
		}
	}
	Realize();
	wxEdit::Instance().GetAuiManager().DetachPane(this);
	wxEdit::Instance().GetAuiManager().AddPane(this, wxAuiPaneInfo().
		Name(wxT("toolbar")).Caption(wxT("")).
		Top().Fixed().ToolbarPane().Layer(1));
	wxEdit::Instance().GetAuiManager().Update();
}