
#include "wxMainNotebook.h"

BEGIN_EVENT_TABLE(wxMainNotebook, wxAuiNotebook)
	EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, wxMainNotebook::OnNotebookChanged)
	EVT_AUINOTEBOOK_PAGE_CLOSED(wxID_ANY, wxMainNotebook::OnPageClosed)
END_EVENT_TABLE() 

IMPLEMENT_CLASS(wxMainNotebook, wxAuiNotebook)

wxMainNotebook::wxMainNotebook(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                  const wxSize& size, long style)
				  : wxAuiNotebook(parent, id, pos, size, style)
{

	mOgreWindow = new wxOgre(this, -1);
	AddPage(mOgreWindow, "Scene", false);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("SaveScript", "ScriptEditor", "Data/Editor/Intern/Icon_Save_Klein.png", &wxMainNotebook::OnToolbarEvent);
}

wxMainNotebook::~wxMainNotebook(void)
{
	//delete mOgreWindow;
}

void wxMainNotebook::OnToolbarEvent(int toolID, Ogre::String toolname)
{
	if (toolname == "SaveScript")
	{
		wxMainNotebook *this_ = wxEdit::Instance().GetMainNotebook();
		int sel = this_->GetSelection();
		std::list<ScriptEditorCaptionBind>::iterator iter = this_->mScriptTabs.begin();
		for (int i = 1; i < sel; i++)
			iter++;
		iter->mScriptEditor->SaveScript();

	}
}

void wxMainNotebook::OnNotebookChanged(wxAuiNotebookEvent& event)
{
	if (GetSelection() == 0)
	{
		if (GetWindowStyle() & 2048) ToggleWindowStyle(wxAUI_NB_CLOSE_ON_ACTIVE_TAB);
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("WorldSettings", true);
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Transform", true);
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("EditMode", true);
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("ObjectMgr", true);

		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("ScriptEditor", false);
	}
	else if (event.old_selection == 0)
	{
		ToggleWindowStyle(wxAUI_NB_CLOSE_ON_ACTIVE_TAB);
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("WorldSettings", false);
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Transform", false);
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("EditMode", false);
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("ObjectMgr", false);

		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("ScriptEditor", true);

	}
}

void wxMainNotebook::OnPageClosed(wxAuiNotebookEvent& event)
{
	int tab = event.GetSelection();
	if (tab == 0) return;
	std::list<ScriptEditorCaptionBind>::iterator i = mScriptTabs.begin();
	for (int a = 1; a < tab; a++)
		i++;
	mScriptTabs.erase(i);
}

void wxMainNotebook::AddScriptTab(wxString caption, wxString fullPath)
{
	int a = 0;
	for (std::list<ScriptEditorCaptionBind>::iterator i = mScriptTabs.begin(); i != mScriptTabs.end(); i++)
	{
		a++;
		if ((*i).mCaption == caption)
		{
			SetSelection(a);
			return;
		}
	}
	wxScriptEditor *editor = new wxScriptEditor(this);
	AddPage(editor, caption);
	editor->LoadScript(fullPath);
	ScriptEditorCaptionBind bind;
	bind.mCaption = caption;
	bind.mScriptEditor = editor;
	mScriptTabs.push_back(bind);
	SetSelection(mScriptTabs.size());
}

void wxMainNotebook::SetModified(wxScriptEditor *tab, bool modified)
{
	int a = 0;
	for (std::list<ScriptEditorCaptionBind>::iterator i = mScriptTabs.begin(); i != mScriptTabs.end(); i++)
	{
		a++;
		if (i->mScriptEditor == tab) SetPageText(a, modified ? i->mCaption + " *" : i->mCaption);
	}
}