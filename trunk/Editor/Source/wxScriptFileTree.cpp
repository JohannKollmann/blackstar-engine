
#include "wxScriptFileTree.h"
#include "SGTScriptSystem.h"
#include "SGTIncludes.h"
#include "wxEdit.h"

enum
{
	ScriptTree_runScript = 3580,
	ScriptTree_editScript = 3581,
	ScriptTree_addScript = 3581
};

wxScriptFileTree::wxScriptFileTree(wxWindow* parent, wxWindowID id, const wxPoint& pos,
             const wxSize& size, long style,
             const wxValidator& validator,
             const wxString& name)
		: wxFileTree(parent, id, pos, size, style, validator, name)
{
	wxArrayString extensions;
	extensions.Add("*.lua");
	SetExtensions(extensions);
	SetRootPath("Data/Scripts/Game");
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("ReloadScripts", "Scripts1", "Data/Editor/Intern/editor_scriptreload_01.png", wxScriptFileTree::OnToolbarEvent);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewScript", "Scripts2", "Data/Editor/Intern/editor_scripnew_01.png", wxScriptFileTree::OnToolbarEvent);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewNpcScript", "Scripts2", "Data/Editor/Intern/editor_scriptnpc_01.png", wxScriptFileTree::OnToolbarEvent);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewStateScript", "Scripts2", "Data/Editor/Intern/editor_scriptstatus_01.png", wxScriptFileTree::OnToolbarEvent);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("DeleteScript", "Scripts3", "Data/Editor/Intern/editor_scriptdelete_01.png", wxScriptFileTree::OnToolbarEvent);
}

void wxScriptFileTree::OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item)
{
	if (item->IsFile())
	{
		menu->Append(ScriptTree_runScript, "Run Script");
		menu->Append(ScriptTree_editScript, "Edit Script");
		menu->AppendSeparator();
	}
	if (item->IsDir() || item->IsRoot())
	{
		menu->Append(ScriptTree_addScript, "Add Script");
		menu->AppendSeparator();
	}
}

void wxScriptFileTree::OnSelectItemCallback()
{
}

void wxScriptFileTree::OnMenuCallback(int id)
{
	if (id == ScriptTree_runScript)
	{
		Ogre::String scriptFile = Ogre::String(GetRelativePath(mCurrentItem->GetId()).GetFullPath());
		Ogre::LogManager::getSingleton().logMessage("Scriptfile: " + scriptFile);
		SGTScript script = SGTScriptSystem::GetInstance().CreateInstance(SCRIPT_BASE_DIR + scriptFile);
		SGTScriptSystem::GetInstance().KillScript(SCRIPT_BASE_DIR + scriptFile);
	}
}

void wxScriptFileTree::OnToolbarEvent(int toolID, Ogre::String toolname)
{
}

void wxScriptFileTree::OnEnterTab()
{
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts1", true);
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts2", true);
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts3", true);
}
void wxScriptFileTree::OnLeaveTab()
{
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts1", false);
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts2", false);
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts3", false);
}