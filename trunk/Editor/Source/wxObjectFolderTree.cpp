
#include "wxObjectFolderTree.h"
#include "IceScenemanager.h"
#include "IceGOCAnimatedCharacter.h"
#include "IceMainLoop.h"
#include "Edit.h"
#include "IceGOCOgreNode.h"

enum
{
	ResTree_addGOC = 3570,
	ResTree_addRagdoll = 3571,
	ResTree_addNpc = 3572
};

wxObjectFolderTree::wxObjectFolderTree(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                  const wxSize& size, long style,
                  const wxValidator& validator,
                  const wxString& name)
		: wxFileTree(parent, id, pos, size, style, validator, name)
{
	wxArrayString extensions;
	extensions.Add("*.ot");
	extensions.Add("*.ocs");
	extensions.Add("*.static");
	SetExtensions(extensions);
	SetRootPath("Data\\Editor\\Objects\\");
	//wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewFolder", "ResourceMgr", "Data/Editor/Intern/1988.ico", wxObjectFolderTree::OnToolbarEvent);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewResource", "ResourceMgr", "Data/Editor/Intern/Engine_Icon07.png", wxObjectFolderTree::OnToolbarEvent, "New object resource");
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("ResourceMgr", true);
	//wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("ResourceMgr", false);
}

void wxObjectFolderTree::OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item)
{
	if (item->IsRoot() || item->IsDir())
	{
		wxMenu *addMenu = new wxMenu("");
		addMenu->Append(ResTree_addGOC, "Component Assembly");
		menu->AppendSubMenu(addMenu, "Add");
		menu->AppendSeparator();
	}
}

void wxObjectFolderTree::OnSelectItemCallback()
{
	ClearObjectPreview();
	if (mCurrentItem->IsRoot()) return;
	mCurrentPath = Ogre::String(GetRelativePath(mCurrentItem->GetId()).GetPath().c_str()) + PATH_SEPERATOR;
	if (mCurrentItem->IsFile())
	{
		Ogre::String Path = "Data/Editor/Objects/" + Ogre::String(GetRelativePath(mCurrentItem->GetId()).GetPath().c_str()) + PATH_SEPERATOR;
		Ogre::String File = mCurrentItem->GetName().c_str();
		mCurrentPath += File;
		Ogre::String extension = File.substr(File.find(".")+1, File.length());
		wxEdit::Instance().GetOgrePane()->OnSelectResource();

		if (extension == "ocs" && wxEdit::Instance().GetWorldExplorer()->GetSelection() == 1)
		{
			CreateObjectPreview(Path + File);
			((wxEditGOResource*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGOCRes")))->SetResource(Path + File);
		}
	}
}

void wxObjectFolderTree::ClearObjectPreview()
{
	wxEdit::Instance().GetPreviewWindow()->Reset();
}

void wxObjectFolderTree::CreateObjectPreview(Ogre::String file)
{
	LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile(file);
	std::vector<ComponentSectionPtr> sections;
	ls->LoadAtom("vector<ComponentSectionPtr>", (void*)(&sections));
	Ogre::Vector3 scale(1,1,1);
	for (auto i = sections.begin(); i != sections.end(); i++)
	{
		if ((*i)->mSectionName == "Mesh")
		{
			wxEdit::Instance().GetPreviewWindow()->ShowMesh((*i)->mSectionData.GetOgreString("MeshName"));
		}
	}
	ls->CloseFile();
	
}

void wxObjectFolderTree::OnMenuCallback(int id)
{
	if (id == ResTree_addGOC)
	{
		wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->ClearObjectPreview();

		Ogre::String relPath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str()) + PATH_SEPERATOR;
		Ogre::String file = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->DoCreateFileDialog();
		if (file == "") return;
		if (file.find(".ocs") == Ogre::String::npos) file = file + ".ocs";
		Ogre::String fullPath = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->mRootPath + PATH_SEPERATOR + relPath + file;

		wxEditGOResource *page = ((wxEditGOResource*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGOCRes")));
		page->NewResource(fullPath);
		page->OnApply();
		page->SetResource(fullPath);

		wxTreeItemId id = ExpandToPath(wxFileName(relPath + file));
	}
}

void wxObjectFolderTree::OnToolbarEvent(int toolID, Ogre::String toolname)
{
	if (toolname == "NewResource")
	{
		wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->ClearObjectPreview();
		Ogre::String insertpath = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetInsertPath();
		Ogre::String file = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->DoCreateFileDialog();

		if (file == "") return;
		if (file.find(".ocs") == Ogre::String::npos) file = file + ".ocs";
		Ogre::String fullPath = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->mRootPath + PATH_SEPERATOR + insertpath + file;

		wxEditGOResource *page = ((wxEditGOResource*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGOCRes")));
		page->NewResource(fullPath);
		page->OnApply();
		page->SetResource(fullPath);

		wxTreeItemId id = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->ExpandToPath(wxFileName(insertpath + file));
	}
}

void wxObjectFolderTree::OnEnterTab()
{
	wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject");
	if (GetSelectedResource() != "None")
	{
		if (GetSelectedResource().find(".ocs") != Ogre::String::npos)
		{
			((wxEditGOResource*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGOCRes")))->SetResource(GetSelectedResource());
		}
	}

	if (wxEdit::Instance().GetOgrePane())
	{
		if (mCurrentItem) OnSelectItemCallback();
	}

	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("ResourceMgr", true);
}
void wxObjectFolderTree::OnLeaveTab()
{
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("ResourceMgr", false);

	Ice::SceneManager::Instance().DestroyPreviewRender("EditorPreview");
	ClearObjectPreview();
}