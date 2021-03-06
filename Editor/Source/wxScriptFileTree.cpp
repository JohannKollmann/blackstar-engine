
#include "wxScriptFileTree.h"
#include "IceScriptSystem.h"
#include "IceIncludes.h"
#include "wxEdit.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "IceAIManager.h"
#include "Edit.h"

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
	extensions.Add("*.material");
	extensions.Add("*.compositor");
	extensions.Add("*.program");
	extensions.Add("*.hlsl");
	extensions.Add("*.cg");
	extensions.Add("*.glsl");
	SetExtensions(extensions);
	SetRootPath("Data\\Scripts");
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("ReloadScripts", "Scripts1", "Data/Editor/Intern/editor_scriptreload_01.png", wxScriptFileTree::OnToolbarEvent, "Reload all scripts");
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewScript", "Scripts2", "Data/Editor/Intern/editor_scripnew_01.png", wxScriptFileTree::OnToolbarEvent, "Add empty script");
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewObjectScript", "Scripts2", "Data/Editor/Intern/editor_scriptnpc_01.png", wxScriptFileTree::OnToolbarEvent, "Add Object script");
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
		notebook->AddScriptTab(mCurrentItem->GetName(), (mRootPath + PATH_SEPERATOR + GetRelativePath(mCurrentItem->GetId()).GetFullPath()));
	}
}

void wxScriptFileTree::OnSelectItemCallback()
{
}

void wxScriptFileTree::OnMenuCallback(int id)
{
	if (id == ScriptTree_runScript)
	{
		STOP_MAINLOOP
		Ogre::String scriptFile = mCurrentItem->GetName().c_str();
		Ice::Log::Instance().LogMessage("Scriptfile: " + scriptFile);
		Ice::Script script = Ice::ScriptSystem::GetInstance().CreateInstance(scriptFile);
		RESUME_MAINLOOP
//		Ice::ScriptSystem::GetInstance().KillScript(scriptFile);
	}
}

void wxScriptFileTree::OnToolbarEvent(int toolID, Ogre::String toolname)
{
	if (toolname == "ReloadScripts")
	{
		STOP_MAINLOOP
		Ice::ScriptSystem::GetInstance().ReparseAllScripts();
		RESUME_MAINLOOP
	}
	if (toolname == "NewScript" || toolname == "NewObjectScript")
	{
		Ogre::String insertpath = wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->GetInsertPath();
		Ogre::String file = wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->DoCreateFileDialog();
		if (file == "") return;
		if (file.find(".lua") == Ogre::String::npos) file = file + ".lua";
		Ogre::String fullPath = wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->mRootPath + PATH_SEPERATOR + insertpath + file;
		std::fstream stream;
		stream.open(fullPath.c_str(), std::ios::out);
		stream << std::endl;
		stream << "function init()" << std::endl;
		stream << "    bindc(\"LogMessage\")" << std::endl;
		if (toolname == "NewObjectScript")
		{
			stream << "    bindc(\"This\")" << std::endl;
		}
		stream << "end" << std::endl;
		stream << std::endl;
		stream << "function create()" << std::endl;
		stream << "end" << std::endl;
		stream << std::endl;
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

void wxScriptFileTree::OnRenameItemCallback(Ogre::String oldPath, Ogre::String newPath, Ogre::String oldFile, Ogre::String newFile)
{
	STOP_MAINLOOP
	if (mCurrentItem->IsDir())
	{
		Ogre::ResourceGroupManager::getSingleton().removeResourceLocation(oldPath, "FileSystem");
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation(newPath, "FileSystem");
	}
	else if (mCurrentItem->IsFile())
	{
		//Ogre::ResourceGroupManager::getSingleton().deleteResource(oldFile, "General", oldPath);
	}
	RESUME_MAINLOOP
}