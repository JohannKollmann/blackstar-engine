
#include "wxObjectFolderTree.h"
#include "IceScenemanager.h"
#include "IceGOCAnimatedCharacter.h"

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
	SetRootPath("Data/Editor/Objects");
	//wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewFolder", "ResourceMgr", "Data/Editor/Intern/1988.ico", wxObjectFolderTree::OnToolbarEvent);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewResource", "ResourceMgr", "Data/Editor/Intern/Engine_Icon07.png", wxObjectFolderTree::OnToolbarEvent, "New object resource");
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("ResourceMgr", true);
	//wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("ResourceMgr", false);
	mPreviewObject = 0;
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
	mCurrentPath = Ogre::String(GetRelativePath(mCurrentItem->GetId()).GetPath().c_str()) + "/";
	if (mCurrentItem->IsFile())
	{
		Ogre::String Path = "Data\\Editor\\Objects\\" + Ogre::String(GetRelativePath(mCurrentItem->GetId()).GetPath().c_str()) + "\\";
		Ogre::String File = mCurrentItem->GetName().c_str();
		mCurrentPath += File;
		Ogre::String extension = File.substr(File.find(".")+1, File.length());
		wxEdit::Instance().GetOgrePane()->GetEdit()->OnSelectResource();

		if (extension == "ocs" && wxEdit::Instance().GetWorldExplorer()->GetSelection() == 1)
		{
			CreateObjectPreview(Path + File);
			((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")))->SetResource(Path + File);
		}
	}
}

void wxObjectFolderTree::ClearObjectPreview()
{
	if (mPreviewObject)
	{
		Ice::Main::Instance().SetSceneMgr(false);
		delete mPreviewObject;
		Ice::Main::Instance().SetSceneMgr(true);
		mPreviewObject = 0;
	}
	wxEdit::Instance().GetPreviewWindow()->SetPreviewNode(0);
	wxEdit::Instance().GetPreviewWindow()->ClearDisplay();
	Ice::SceneManager::Instance().DestroyPreviewRender("EditorPreview");
}

void wxObjectFolderTree::CreateObjectPreview(Ogre::String file)
{
	ClearObjectPreview();
	Ice::Main::Instance().SetSceneMgr(false);
	mPreviewObject = new Ice::GameObject(-1);
	LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile(file);
	std::list<Ice::ComponentSection> sections;
	ls->LoadAtom("std::list<ComponentSection>", (void*)(&sections));
	Ogre::Vector3 scale(1,1,1);
	for (std::list<Ice::ComponentSection>::iterator i = sections.begin(); i != sections.end(); i++)
	{
		if ((*i).mSectionName == "GameObject") continue;
		(*i).mSectionData->AddOgreVec3("Scale", scale);
		Ice::GOCEditorInterface *component = Ice::SceneManager::Instance().CreateComponent((*i).mSectionName, (*i).mSectionData.getPointer());
		if (!component) continue;
		Ice::GOComponent *test1 = dynamic_cast<Ice::GOComponent*>(component);
		Ice::GOCNodeRenderable *test2 = dynamic_cast<Ice::GOCNodeRenderable*>(component);
		if (test1 && !test2) delete test1;
		else component->AttachToGO(mPreviewObject);
	}
	ls->CloseFile();
	Ice::GOCNodeRenderable *renderable = (Ice::GOCNodeRenderable*)mPreviewObject->GetComponent("View");
	if (!renderable)
	{
		delete mPreviewObject;
		mPreviewObject = 0;
		Ice::Main::Instance().SetSceneMgr(true);
		return;
	}
	Ice::Main::Instance().SetSceneMgr(true);
	float width = 256;//wxEdit::Instance().GetAuiManager().GetPane("preview").floating_size.GetWidth();
	float height = 256;//wxEdit::Instance().GetAuiManager().GetPane("preview").floating_size.GetHeight();
	Ice::SceneManager::Instance().CreatePreviewRender(renderable->GetNode(), "EditorPreview", width, height);
	Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName("EditorPreview_Tex");
	wxEdit::Instance().GetPreviewWindow()->SetPreviewNode(renderable->GetNode());
	wxEdit::Instance().GetPreviewWindow()->SetTexture(texture);
	
}

void wxObjectFolderTree::OnMenuCallback(int id)
{
	if (id == ResTree_addGOC)
	{
		wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->ClearObjectPreview();

		Ogre::String relPath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str()) + "\\";
		Ogre::String file = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->DoCreateFileDialog();
		if (file == "") return;
		if (file.find(".ocs") == Ogre::String::npos) file = file + ".ocs";
		Ogre::String fullPath = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->mRootPath + "\\" + relPath + file;

		wxEditIceGameObject *page = ((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")));
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
		Ogre::String fullPath = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->mRootPath + "\\" + insertpath + file;

		wxEditIceGameObject *page = ((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")));
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
			((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetResource(GetSelectedResource());
		}
	}

	if (wxEdit::Instance().GetOgrePane()->GetEdit())
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