
#include "wxScriptFileTree.h"
#include "SGTScriptSystem.h"
#include "SGTIncludes.h"
#include "wxEdit.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "SGTAIManager.h"

enum
{
	ScriptTree_runScript = 3580,
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
	SetRootPath("Data/Scripts/");
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("ReloadScripts", "Scripts1", "Data/Editor/Intern/editor_scriptreload_01.png", wxScriptFileTree::OnToolbarEvent);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewScript", "Scripts2", "Data/Editor/Intern/editor_scripnew_01.png", wxScriptFileTree::OnToolbarEvent);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewNpcScript", "Scripts2", "Data/Editor/Intern/editor_scriptnpc_01.png", wxScriptFileTree::OnToolbarEvent);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewStateScript", "Scripts2", "Data/Editor/Intern/editor_scriptstatus_01.png", wxScriptFileTree::OnToolbarEvent);
	//wxEdit::Instance().GetExplorerToolbar()->RegisterTool("DeleteScript", "Scripts3", "Data/Editor/Intern/editor_scriptdelete_01.png", wxScriptFileTree::OnToolbarEvent);
}

void wxScriptFileTree::OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item)
{
	if (item->IsFile())
	{
		menu->Append(ScriptTree_runScript, "Run Script");
		menu->AppendSeparator();
	}
}

void wxScriptFileTree::OnItemActivated(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if(id.IsOk())
	{
		mCurrentItem = (VdtcTreeItemBase *)GetItemData(id);
	}
	if (mCurrentItem->IsFile())
	{
		wxMainNotebook *notebook = wxEdit::Instance().GetMainNotebook();
		notebook->AddScriptTab(mCurrentItem->GetName(), (mRootPath + "/" + GetRelativePath(mCurrentItem->GetId()).GetFullPath()));
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
		SGTScript script = SGTScriptSystem::GetInstance().CreateInstance(scriptFile);
//		SGTScriptSystem::GetInstance().KillScript(scriptFile);
	}
}

void wxScriptFileTree::OnToolbarEvent(int toolID, Ogre::String toolname)
{
	if (toolname == "ReloadScripts")
	{
		SGTMsg msg;
		msg.mNewsgroup = "REPARSE_SCRIPTS";
		SGTMessageSystem::Instance().SendMessage(msg);
		SGTScriptSystem::GetInstance().Clear();
	}
	if (toolname == "NewScript" || toolname == "NewNpcScript" || toolname == "NewStateScript")
	{
		Ogre::String insertpath = wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->GetInsertPath();
		Ogre::String file = wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->DoCreateFileDialog();
		if (file == "") return;
		if (file.find(".lua") == Ogre::String::npos) file = file + ".lua";
		Ogre::String fullPath = wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->mRootPath + "\\" + insertpath + file;
		std::fstream stream;
		stream.open(fullPath.c_str(), std::ios::out);
		stream << std::endl;
		stream << "function init()" << std::endl;
		stream << "end" << std::endl;
		stream << std::endl;
		if (toolname == "NewNpcScript")
		{
			stream << "function create(id)" << std::endl;
			stream << "end" << std::endl;
			stream << std::endl;
		}
		else if (toolname == "NewStateScript")
		{
			stream << "function onEnter()" << std::endl;
			stream << "end" << std::endl;
			stream << std::endl;
			stream << "function onUpdate()" << std::endl;
			stream << "end" << std::endl;
			stream << std::endl;
		}
		stream.close();
		wxFileName path(insertpath.c_str());
		wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->ExpandToPath(path);
		VdtcTreeItemBaseArray items;
		items.Add(wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->AddFileItem(file.c_str()));
		wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->AddItemsToTreeCtrl(wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->mCurrentItem, items);
		wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->ExpandToPath(wxFileName((insertpath + file).c_str()));
		wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->OnItemActivated(wxTreeEvent());
	}
}

void wxScriptFileTree::OnEnterTab()
{
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts1", true);
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts2", true);
	//wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts3", true);
}
void wxScriptFileTree::OnLeaveTab()
{
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts1", false);
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts2", false);
	//wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("Scripts3", false);
}