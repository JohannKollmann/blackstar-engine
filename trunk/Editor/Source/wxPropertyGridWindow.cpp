#include "wxPropertyGridWindow.h"

enum
{
	PropWindow_Apply
};

BEGIN_EVENT_TABLE(wxPropertyGridWindow, wxControl)
	EVT_BUTTON(PropWindow_Apply, wxPropertyGridWindow::OnApply)
	EVT_SIZE(wxPropertyGridWindow::OnResize)
	EVT_SET_FOCUS(wxPropertyGridWindow::OnSetFocus)
END_EVENT_TABLE()

// Required for WX
IMPLEMENT_CLASS(wxPropertyGridWindow, wxControl)


wxPropertyGridWindow::~wxPropertyGridWindow(void)
{
	//this->SetHWND(0);
}

wxPropertyGridWindow::wxPropertyGridWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                  const wxSize& size, long style,
                  const wxString& name) : 
	wxPanel(parent, wxID_ANY, pos, size)
{
       // Construct wxPropertyGrid control
       long propstyle = // default style
                //wxPG_SPLITTER_AUTO_CENTER |
                wxPG_TOOLTIPS |
                wxTAB_TRAVERSAL;
				//| wxPG_DESCRIPTION;
       mPropGrid = new wxPropertyGrid(
            this, // parent
            -1, // id
            wxDefaultPosition, // position
            wxDefaultSize, // size
			propstyle);

	mPropGrid->SetExtraStyle( wxPG_EX_HELP_AS_TOOLTIPS );

	mPropGrid->RegisterAdditionalEditors();
	mPropGrid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,wxVariant(true));

	mPropGrid->GetGrid()->SetVerticalSpacing( 2 );

	mApplyBtn = new wxButton(this,PropWindow_Apply,wxT("Apply"));

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(mPropGrid);
	sizer->Add(mApplyBtn);

	this->SetSizer(sizer);

	mCurrentPage = nullptr;

	SetDropTarget(new FileDropTarget(this));
}

void wxPropertyGridWindow::AddPage(wxPropertyGridListener* page, wxString name)
{
	page->Init(mPropGrid);
	mPages.insert(std::make_pair(name, page));
}

wxPropertyGridListener* wxPropertyGridWindow::SetPage(wxString name)
{
	mPropGrid->Clear();
	if (mCurrentPageName == name && name != "None")
	{
		mCurrentPage->OnActivate();
		return mCurrentPage;
	}
	mCurrentPageName = name;
	if (mCurrentPage) mCurrentPage->OnLeave();
	mCurrentPage = 0;
	if (name == "None")
	{
		Ogre::LogManager::getSingleton().logMessage("Warning: wxPropertyGridWindow::SetPage(wxString name) - return NULL");
		return 0;
	}
	if ((*mPages.find(name)).first == name)
	{
		mCurrentPage = mPages[name];
		mCurrentPage->OnActivate();
	}
	mPropGrid->Refresh();
	return mCurrentPage;
}

wxPropertyGridListener* wxPropertyGridWindow::GetPage(wxString name)
{
	return (*mPages.find(name)).second;
}

wxPropertyGridListener* wxPropertyGridWindow::GetCurrentPage()
{
	return mCurrentPage;
}

Ogre::String wxPropertyGridWindow::GetCurrentPageName()
{
	return mCurrentPageName;
}

void wxPropertyGridWindow::OnSetFocus(wxFocusEvent& event)
{
	if (mCurrentPage) mCurrentPage->OnUpdate();
}

void wxPropertyGridWindow::OnResize(wxSizeEvent& event)
{
	if (mPropGrid != NULL)
	{
		wxSize size = event.GetSize();
		size.y -= mApplyBtn->GetSize().GetHeight();
		mPropGrid->SetSize(size);
		mApplyBtn->SetPosition(wxPoint((size.x / 2) - (mApplyBtn->GetSize().GetWidth() / 2), size.y));
	}
}


void wxPropertyGridWindow::OnApply(wxCommandEvent& event)
{
	if (mCurrentPage != 0)
	{
		mCurrentPage->OnApply();
	}
}

bool wxPropertyGridWindow::FileDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString&  filenames)
{
	mPropWindow->mPropGrid->SetEmptySpaceColour(wxColour("white"));
	mPropWindow->mPropGrid->Refresh();
	if (filenames.IsEmpty()) return false;
	if (mPropWindow->mCurrentPage != 0) return mPropWindow->mCurrentPage->OnDropText(filenames[0]);
	return false;
}

wxDragResult wxPropertyGridWindow::FileDropTarget::OnEnter(wxCoord x, wxCoord y, wxDragResult def)
{
	if (mPropWindow->mCurrentPage != 0)
	{
		mPropWindow->mPropGrid->SetEmptySpaceColour(wxColour(0, 180, 20));
	}
	return def;
}

void wxPropertyGridWindow::FileDropTarget::OnLeave()
{
	mPropWindow->SetEmptySpaceColour(wxColour("white"));
}

void wxPropertyGridWindow::SetEmptySpaceColour(wxColour colour)
{
	mPropGrid->SetEmptySpaceColour(colour);
}