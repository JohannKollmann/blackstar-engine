
#include "wxScriptFileTree.h"
#include "SGTScriptSystem.h"
#include "SGTIncludes.h"

enum
{
	ScriptTree_runScript = 3580,
	ScriptTree_editScript = 3581,
	ScriptTree_addScript = 3581
};

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
	}
}