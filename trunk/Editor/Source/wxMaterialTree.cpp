
#include "wxMaterialTree.h"
#include "../Misc/folder.xpm"
#include "../Misc/file.xpm"
#include "../Misc/root.xpm"

#include "windows.h"
#include <fstream>

#include "wxMaterialEditor.h"

#include "SGTGOCView.h"


BEGIN_EVENT_TABLE(wxMaterialTree, wxTreeCtrl)
	EVT_TREE_ITEM_MENU(-1, wxMaterialTree::OnItemMenu)
	EVT_MENU(wxID_ANY, wxMaterialTree::OnMenuEvent)
END_EVENT_TABLE()

wxMaterialTree::wxMaterialTree(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name)
	: wxTreeCtrl(parent, id, pos, size, style, validator, name)
	, _flags(wxVDTC_DEFAULT)
{
	// create an icon list for the tree ctrl
	_iconList = new wxImageList(16,16);
	
	mCallbackIDCounter = 10500;
};

wxMaterialTree::~wxMaterialTree()
{
	// first delete all OgreMaterialTreeItemBase items (client data)
	DeleteAllItems();

	// delete the icons
	delete _iconList;

}

bool wxMaterialTree::AddEntity(Ogre::Entity *entity)
{
	for (std::vector<Ogre::String>::iterator i = mAddedMeshes.begin(); i < mAddedMeshes.end(); i++)
	{
		if (entity->getMesh()->getName() == (*i)) return false;
	}
	mAddedMeshes.push_back(entity->getMesh()->getName());
	OgreMaterialTreeItemBase *item = OnCreateTreeFolder(entity->getMesh()->getName());
	AppendItem(mStart->GetId(), item->GetCaption(), item->GetIconId(), item->GetSelectedIconId(), item);
	for (unsigned short x = 0; x < entity->getNumSubEntities(); x++)
	{
		Ogre::SubEntity *subent = entity->getSubEntity(x);
		OgreMaterialTreeItemBase *material = OnCreateTreeMaterial(subent->getMaterial());
		AppendItem(item->GetId(), material->GetCaption(), material->GetIconId(), material->GetSelectedIconId(), material);
	}
	return true;
}

void wxMaterialTree::Update()
{
	// delete all items plus root first
	DeleteAllItems();
	OgreMaterialTreeItemBase *start = 0;

	mAddedMeshes.clear();

	// now call for icons management, the virtual
	// handler so the derived class can assign icons

	_iconList->RemoveAll();
	OnAssignIcons(*_iconList);

	SetImageList(_iconList);

	// create a root item
	start = OnCreateTreeRoot();
	mStart = start;

	if(start)
	{
		// add this item to the tree, with info of the developer
		wxTreeItemId id = AddRoot(start->GetCaption(), start->GetIconId(), start->GetSelectedIconId(), start);

		OgreMaterialTreeItemBase *othermaterials = OnCreateTreeFolder("Other");
		AppendItem(start->GetId(), othermaterials->GetCaption(), othermaterials->GetIconId(), othermaterials->GetSelectedIconId(), othermaterials);
		mOtherID = othermaterials->GetId();

		std::vector<Ogre::String> added;

		if (SGTSceneManager::Instance().HasLevelMesh())
		{
			Ogre::Entity *ent = SGTMain::Instance().GetOgreSceneMgr()->getEntity("LevelMesh-entity");	
			AddEntity(ent);
		}

		std::vector<Ogre::String> other_materials_added;

		for (std::list<SGTGameObject*>::iterator i = SGTSceneManager::Instance().mGameObjects.begin(); i != SGTSceneManager::Instance().mGameObjects.end(); i++)
		{
			SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)(*i)->GetComponent("GOCView");
			if (visuals != 0)
			{
				if (visuals->GetComponentID() == "GOCViewContainer")
				{
					SGTMeshRenderable *gocmesh = (SGTMeshRenderable*)((SGTGOCViewContainer*)visuals)->GetItem("MeshRenderable");
					if (gocmesh != 0)
					{
						AddEntity((Ogre::Entity*)gocmesh->GetEditorVisual());
					}
				}
			}
		}

		/*for (std::map<Ogre::String, std::list<SGTGameObject*> >::iterator i = SGTSceneManager::Instance().mEntities.begin(); i != SGTSceneManager::Instance().mEntities.end(); i++)
		{
			if ((*i).first == "Billboard")
			{
				for (std::list<SGTGameObject*>::iterator x = (*i).second.begin(); x != (*i).second.end(); x++)
				{

					SGTBillboard *billboard = (SGTBillboard*)(*x);

					bool stop = false;
					for (std::vector<Ogre::String>::iterator m = other_materials_added.begin(); m != other_materials_added.end(); m++)
					{
						if ((*m) == billboard->GetBillboard()->getMaterialName())
						{
							stop = true;
							break;
						}
					}
					if (!stop)
					{
						OgreMaterialTreeItemBase *material = this->OnCreateTreeMaterial(billboard->GetBillboard()->getMaterial());
						AppendItem(mOtherID, material->GetCaption(), material->GetIconId(), material->GetSelectedIconId(), material);	
						other_materials_added.push_back(billboard->GetBillboard()->getMaterialName());
					}
				}
			}

			if ((*i).first == "Body" || (*i).first == "StaticBody" || (*i).first == "StaticMesh")
			{
				for (std::list<SGTGameObject*>::iterator x = (*i).second.begin(); x != (*i).second.end(); x++)
				{
					bool stop = false;
					for (std::vector<Ogre::String>::iterator m = added.begin(); m != added.end(); m++)
					{
						if ((*m) == (*x)->GetVisual()->getMesh()->getName())
						{
							stop = true;
							break;
						}
					}
					if (!stop)
					{
						OgreMaterialTreeItemBase *item = OnCreateTreeFolder((*x)->GetVisual()->getMesh()->getName());
						AppendItem(start->GetId(), item->GetCaption(), item->GetIconId(), item->GetSelectedIconId(), item);
						for (unsigned short y = 0; y < (*x)->GetVisual()->getNumSubEntities(); y++)
						{
							Ogre::SubEntity *subent = (*x)->GetVisual()->getSubEntity(y);
							OgreMaterialTreeItemBase *material = this->OnCreateTreeMaterial(subent->getMaterial());
							AppendItem(item->GetId(), material->GetCaption(), material->GetIconId(), material->GetSelectedIconId(), material);
						}
						added.push_back((*x)->GetVisual()->getMesh()->getName());
					}
				}
			}
		}*/
		
		// expand root when allowed
		if(!(_flags & wxVDTC_NO_EXPAND))
			Expand(start->GetId());
	}
};

void wxMaterialTree::AddItemsToTreeCtrl(OgreMaterialTreeItemBase *item, OgreMaterialTreeItemBaseArray &items)
{
	//Ogre::LogManager::getSingleton().logMessage("AddItemsToTreeCtrl " + Ogre::String(item->GetName().c_str()) + " " + Ogre::StringConverter::toString(items.GetCount()));
	wxCHECK2(item, return);

	// now loop through all elements on this level and add them
	// to the tree ctrl pointed out by 'id'

	OgreMaterialTreeItemBase *t;
	wxTreeItemId id = item->GetId();
	for(size_t i = 0; i < items.GetCount(); i++)
	{
		t = items[i];
		if(t)
		{
			AppendItem(id, t->GetCaption(), t->GetIconId(), t->GetSelectedIconId(), t);
		}
	}
};

wxBitmap *wxMaterialTree::CreateRootBitmap()
{
	// create root and return
	return new wxBitmap(xpm_root);
};

wxBitmap *wxMaterialTree::CreateFolderBitmap()
{
	// create folder and return
	return new wxBitmap(xpm_folder);
};

wxBitmap *wxMaterialTree::CreateNodeBitmap()
{
	// create file and return
	return new wxBitmap(xpm_file);
};

void wxMaterialTree::OnAssignIcons(wxImageList &icons)
{
	wxBitmap *bmp;
	// default behaviour, assign three bitmaps

	bmp = CreateRootBitmap();
	icons.Add(*bmp);
	delete bmp;

	// 1 = folder
	bmp = CreateFolderBitmap();
	icons.Add(*bmp);
	delete bmp;

	// 2 = file
	bmp = CreateNodeBitmap();
	icons.Add(*bmp);
	delete bmp;
};

OgreMaterialTreeItemBase* wxMaterialTree::OnCreateTreeRoot()
{
	// return a default instance, no extra info needed in this item
	return new OgreMaterialTreeItemBase("Root", VDTC_TI_ROOT);
}; 

OgreMaterialTreeItemBase* wxMaterialTree::OnCreateTreeFolder(Ogre::String name)
{
	// return a default instance, no extra info needed in this item
	return new OgreMaterialTreeItemBase(name.c_str(), VDTC_TI_DIR);
}; 

OgreMaterialTreeItemBase* wxMaterialTree::OnCreateTreeMaterial(Ogre::MaterialPtr material)
{
	// return a default instance, no extra info needed in this item
	return new OgreMaterialTreeItemBase(material->getName().c_str(), material);
}; 

void wxMaterialTree::OnMenuEvent(wxCommandEvent& event)
{
	int id = event.GetId();
	for (std::list<MaterialOption>::iterator i = mMaterialOptions.begin(); i != mMaterialOptions.end(); i++)
	{
		if ((*i).mCallbackID == id)
		{
			wxMaterialEditor *editor = (wxMaterialEditor*)(wxEdit::Instance().GetpropertyWindow()->SetPage("material"));
			editor->SetMaterialTemplate((*i).mTemplateName, (*i).mTemplateFile);
			editor->EditMaterial(mSelectedMaterial);
			break;
		}
	}
	mCallbackIDCounter = 10500;
	mMaterialOptions.clear();
	event.Skip();
}

void wxMaterialTree::OnItemMenu(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if(id.IsOk())
	{
		OgreMaterialTreeItemBase *t = (OgreMaterialTreeItemBase *)GetItemData(id);
		wxPoint clientpt = event.GetPoint();
		wxPoint screenpt = ClientToScreen(clientpt);

		ShowMenu(t, clientpt);
	}
    event.Skip();
};

void wxMaterialTree::ShowMenu(OgreMaterialTreeItemBase *item, const wxPoint& pt)
{
	wxMenu *menu = new wxMenu("");

	if (item->IsFile())
	{
		wxEdit::Instance().GetOgrePane()->mEdit->DeselectMaterial();
		Ogre::Root::getSingleton().renderOneFrame();

		mSelectedMaterial = item->GetMaterial();

		HANDLE fHandle;
		WIN32_FIND_DATA wfd;
		fHandle=FindFirstFile("Data/Scripts/materials/scripts/Templates/*.material",&wfd);
		do
		{
			wxMenu *submenu = new wxMenu("");

			Ogre::String filename = Ogre::String("Data/Scripts/materials/scripts/Templates/") + Ogre::String(wfd.cFileName);
			std::fstream f;
			char cstring[256];
			f.open(filename.c_str(), std::ios::in);
			while (!f.eof())
			{
				f.getline(cstring, sizeof(cstring));
				Ogre::String line = cstring;
				if (line.find("abstract material") != Ogre::String::npos)
				{
					Ogre::String name = line.substr(line.find("abstract material") + 18, line.size());
					submenu->Append(mCallbackIDCounter, wxT(name.c_str()));
					AddOption(filename, name);
				}
			}
			f.close();


			menu->AppendSubMenu(submenu, wxT(Ogre::String(wfd.cFileName).substr(0, Ogre::String(wfd.cFileName).find(".material"))));
		}
		while (FindNextFile(fHandle,&wfd));
		FindClose(fHandle);

	}

	PopupMenu(menu, pt);

};
void wxMaterialTree::ExpandToMaterial(Ogre::String mesh, Ogre::String material)
{
	//Ogre::LogManager::getSingleton().logMessage("ExpandToMaterial - " + mesh + " - " + material);
	wxTreeItemIdValue cookie = 0;
	OgreMaterialTreeItemBase *b;
	wxTreeItemId child = GetFirstChild(mStart->GetId(), cookie);
	while(child.IsOk())
	{
		b = (OgreMaterialTreeItemBase *)GetItemData(child);
		if (Ogre::String(b->GetCaption().c_str()) == mesh)
		{
			Expand(b->GetId());
			wxTreeItemId subchild = GetFirstChild(b->GetId(), cookie);
			OgreMaterialTreeItemBase *b2;
			while(subchild.IsOk())
			{
				b2 = (OgreMaterialTreeItemBase *)GetItemData(subchild);
				if (Ogre::String(b2->GetCaption().c_str()) == material)
				{
					SelectItem(b2->GetId());
					return;
				}
				subchild = GetNextChild(b->GetId(), cookie);
			}
		}
		child = GetNextChild(mStart->GetId(), cookie);
	}
}

void wxMaterialTree::AddOption(Ogre::String file, Ogre::String name)
{
	MaterialOption option;
	option.mTemplateFile = file;
	option.mTemplateName = name;
	option.mCallbackID = mCallbackIDCounter;
	mMaterialOptions.push_back(option);
	mCallbackIDCounter++;
}