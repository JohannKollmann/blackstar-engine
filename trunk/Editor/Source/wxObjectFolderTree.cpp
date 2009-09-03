
#include "wxObjectFolderTree.h"
#include "SGTScenemanager.h"
#include "SGTGOCAnimatedCharacter.h"

enum
{
	ResTree_addGOC = 3570,
	ResTree_addRagdoll = 3571,
	ResTree_addNpc = 3572
};

void wxObjectFolderTree::OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item)
{
	if (item->IsRoot() || item->IsDir())
	{
		wxMenu *addMenu = new wxMenu("");
		addMenu->Append(ResTree_addGOC, "Component Assembly");
		addMenu->Append(ResTree_addRagdoll, "AnimatedCharacter");
		addMenu->Append(ResTree_addNpc, "Npc");
		menu->AppendSubMenu(addMenu, "Add");
		menu->AppendSeparator();
	}
}

void wxObjectFolderTree::OnSelectItemCallback()
{
	if (mCurrentItem->IsRoot()) return;
	mCurrentPath = Ogre::String(GetRelativePath(mCurrentItem->GetId()).GetPath().c_str()) + "/";
	if (mCurrentItem->IsFile())
	{
		Ogre::String Path = "Data\\Editor\\Objects\\" + Ogre::String(GetRelativePath(mCurrentItem->GetId()).GetPath().c_str()) + "\\";
		Ogre::String File = mCurrentItem->GetName().c_str();
		mCurrentPath += File;
		Ogre::String extension = File.substr(File.find(".")+1, File.length());

		if (extension != "ot") ((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")))->SetResource(Path + File);
	}
}

void wxObjectFolderTree::OnMenuCallback(int id)
{
	if (id == ResTree_addGOC)
	{
		Ogre::String Path = "Data\\Editor\\Objects\\" + Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str()) + "\\";
		wxTextEntryDialog dialog(this,
			_T("Enter file name:"),
			_T("Please enter a string"),
			_T(""),
			wxOK | wxCANCEL);

		Ogre::String File = "";
		if (dialog.ShowModal() == wxID_OK)
		{
			File = dialog.GetValue().c_str();
		}
		else return;

		mCurrentPath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str()) + "/" + File;

		wxEditSGTGameObject *page = ((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")));
		page->NewResource(Path + File + ".ocs");
		page->OnApply();
		page->SetResource(Path + File + ".ocs");
	}
	if (id == ResTree_addRagdoll)
	{
		Ogre::String Path = "Data\\Editor\\Objects\\" + Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str()) + "\\";
		wxTextEntryDialog dialog(this,
			_T("Enter file name:"),
			_T("Please enter a string"),
			_T(""),
			wxOK | wxCANCEL);

		Ogre::String File = "";
		if (dialog.ShowModal() == wxID_OK)
		{
			File = dialog.GetValue().c_str();
		}
		else return;

		mCurrentPath = Ogre::String(GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str()) + "/" + File;

		wxAuiPaneInfo& pane = wxEdit::Instance().GetAuiManager().GetPane(wxT("componentbar")).Show(false);
		wxEditSGTGameObject* page = ((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")));
		page->NewResource(Path + File + ".ocs", false);
		SGTDataMap map;
		SGTGOCAnimatedCharacter::GetDefaultParameters(&map);
		page->AddGOCSection("AnimatedCharacter", map);
	}
}