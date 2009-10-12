
#include "wxObjectFolderTree.h"
#include "SGTScenemanager.h"
#include "SGTGOCAnimatedCharacter.h"
#include "GUISystem.h"

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
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("NewResource", "ResourceMgr", "Data/Editor/Intern/Engine_Icon07.png", wxObjectFolderTree::OnToolbarEvent);
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
		wxEdit::Instance().GetOgrePane()->GetEdit()->OnSelectResource();

		if (extension == "ocs")
		{
			CreateObjectPreview(Path + File);
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")))->SetResource(Path + File);
		}
	}
}

void wxObjectFolderTree::ClearObjectPreview()
{
	if (mPreviewObject)
	{
		SGTMain::Instance().SetSceneMgr(false);
		delete mPreviewObject;
		SGTMain::Instance().SetSceneMgr(true);
		mPreviewObject = 0;
	}
	SGTSceneManager::Instance().DestroyPreviewRender("EditorPreview");
}

void wxObjectFolderTree::CreateObjectPreview(Ogre::String file)
{
	ClearObjectPreview();
	SGTMain::Instance().SetSceneMgr(false);
	mPreviewObject = new SGTGameObject(0);
	SGTLoadSystem *ls=SGTLoadSave::Instance().LoadFile(file);
	std::list<ComponentSection> sections;
	ls->LoadAtom("std::list<ComponentSection>", (void*)(&sections));
	Ogre::Vector3 scale(1,1,1);
	SGTGOCViewContainer *container = 0;
	for (std::list<ComponentSection>::iterator i = sections.begin(); i != sections.end(); i++)
	{
		if ((*i).mSectionName == "GameObject") continue;
		(*i).mSectionData->AddOgreVec3("Scale", scale);
		SGTGOCEditorInterface *component = SGTSceneManager::Instance().CreateComponent((*i).mSectionName, (*i).mSectionData.getPointer());
		if (!component) continue;
		if (component->IsViewComponent())
		{
			container = (SGTGOCViewContainer*)mPreviewObject->GetComponent("GOCView");
			if (!container)
			{
				container = new SGTGOCViewContainer();
				mPreviewObject->AddComponent(container);
			}
			container->AddItem(dynamic_cast<SGTGOCViewComponent*>(component));
		}
		else
		{
			delete component;
		}
	}
	ls->CloseFile();
	if (!container)
	{
		delete mPreviewObject;
		mPreviewObject = 0;
		return;
	}
	SGTMain::Instance().SetSceneMgr(true);
	SGTSceneManager::Instance().CreatePreviewRender(container->GetNode(), "EditorPreview");
	Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName("gui/runtime");
	material->getTechnique(0)->getPass(0)->removeAllTextureUnitStates();
	material->getTechnique(0)->getPass(0)->createTextureUnitState("EditorPreview_Tex");
	wxEdit::Instance().GetOgrePane()->GetEdit()->mPreviewWindow.SetMaterial(material->getName());
}

void wxObjectFolderTree::OnMenuCallback(int id)
{
	if (id == ResTree_addGOC)
	{
		Ogre::String relPath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str()) + "\\";
		Ogre::String Path = "Data\\Editor\\Objects\\" + relPath;
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

		wxEditSGTGameObject *page = ((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")));
		page->NewResource(Path + File + ".ocs");
		page->OnApply();
		page->SetResource(Path + File + ".ocs");

		wxTreeItemId id = ExpandToPath(wxFileName(relPath + File + ".ocs"));
		SelectItem(id);
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

void wxObjectFolderTree::OnToolbarEvent(int toolID, Ogre::String toolname)
{
	if (toolname == "NewResource")
	{
		Ogre::String relPath = Ogre::String(wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->mExpandedPath.c_str());
		if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->mCurrentItem->IsDir() || wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->mCurrentItem->IsRoot())
		{
			relPath = Ogre::String(wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetRelativePath(wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->mCurrentItem->GetId()).GetFullPath().c_str()) + "\\";
		}
		Ogre::String Path = "Data\\Editor\\Objects\\" + relPath;
		wxTextEntryDialog dialog(wxEdit::Instance().GetWorldExplorer()->GetResourceTree(),
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

		wxEditSGTGameObject *page = ((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")));
		page->NewResource(Path + File + ".ocs");
		page->OnApply();
		page->SetResource(Path + File + ".ocs");

		wxTreeItemId id = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->ExpandToPath(wxFileName(relPath + File + ".ocs"));
		wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->SelectItem(id);
	}
}

void wxObjectFolderTree::OnEnterTab()
{
	wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject");
	if (GetSelectedResource() != "None")
	{
		if (GetSelectedResource().find(".ocs") != Ogre::String::npos)
		{
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetResource(GetSelectedResource());
		}
	}

	if (wxEdit::Instance().GetOgrePane()->GetEdit())
	{
		SGTGUISystem::GetInstance().SetVisible(wxEdit::Instance().GetOgrePane()->GetEdit()->mPreviewWindow.GetHandle(), true);
		if (mCurrentItem) OnSelectItemCallback();
	}

	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("ResourceMgr", true);
}
void wxObjectFolderTree::OnLeaveTab()
{
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("ResourceMgr", false);

	Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName("gui/runtime");
	material->getTechnique(0)->getPass(0)->removeAllTextureUnitStates();
	SGTSceneManager::Instance().DestroyPreviewRender("EditorPreview");
	SGTGUISystem::GetInstance().SetVisible(wxEdit::Instance().GetOgrePane()->GetEdit()->mPreviewWindow.GetHandle(), false);
}