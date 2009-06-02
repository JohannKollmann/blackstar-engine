
#include "../Header/wxOgreSceneTree.h"
#include "../Misc/folder.xpm"
#include "../Misc/file.xpm"
#include "../Misc/root.xpm"
#include "SGTCameraController.h"
#include "SGTSceneManager.h"

BEGIN_EVENT_TABLE(wxOgreSceneTree, wxTreeCtrl)
	EVT_TREE_ITEM_EXPANDING(-1, wxOgreSceneTree::OnExpanding)

	EVT_TREE_SEL_CHANGED(-1, wxOgreSceneTree::OnSelChanged)
	EVT_TREE_ITEM_MENU(-1, wxOgreSceneTree::OnItemMenu)
	EVT_TREE_ITEM_ACTIVATED(-1, wxOgreSceneTree::OnItemActivated)
END_EVENT_TABLE()

wxOgreSceneTree::wxOgreSceneTree(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name)
	: wxTreeCtrl(parent, id, pos, size, style, validator, name)
	, _flags(wxVDTC_DEFAULT)
{
	// create an icon list for the tree ctrl
	_iconList = new wxImageList(16,16);

	mSelectedItem = NULL;
};

wxOgreSceneTree::~wxOgreSceneTree()
{
	// first delete all OgreTreeItemBase items (client data)
	DeleteAllItems();

	// delete the icons
	delete _iconList;

}

SGTGameObject* wxOgreSceneTree::GetSelectedItem()
{
	if (mSelectedItem != NULL)
	{
		return mSelectedItem->getNode();
	}
	return NULL;
}

void wxOgreSceneTree::Update()
{

	Ogre::LogManager::getSingleton().logMessage("OgreScenetree wird geleert");
	// delete all items plus root first
	DeleteAllItems();
	OgreTreeItemBase *start = 0;

	// now call for icons management, the virtual
	// handler so the derived class can assign icons

	_iconList->RemoveAll();
	OnAssignIcons(*_iconList);

	SetImageList(_iconList);

	// create a root item
	start = OnCreateTreeItem(VDTC_TI_ROOT, 0);
	mStart = start;

	if(start)
	{

		// add this item to the tree, with info of the developer
		wxTreeItemId id = AddRoot(start->GetCaption(), start->GetIconId(), start->GetSelectedIconId(), start);

		// scan directory, either the smart way or not at all
		for (std::list<SGTGameObject*>::iterator i = SGTSceneManager::Instance().mGameObjects.begin(); i != SGTSceneManager::Instance().mGameObjects.end(); i++)
		{
			if (!(*i)->GetParent())
			{
				OgreTreeItemBase *item = OnCreateTreeItem(VDTC_TI_FILE, (*i));
				OgreTreeItemBaseArray addedItems;
				addedItems.Add(item);
				AddItemsToTreeCtrl(mStart, addedItems);
				ScanFromNode(item, (*i));
			}
		}

		// expand root when allowed
		if(!(_flags & wxVDTC_NO_EXPAND))
			Expand(id);
	}

	mSelectedItem = 0;
};

void wxOgreSceneTree::ScanFromNode(OgreTreeItemBase *item, SGTGameObject *scanFrom, bool subScan)
{
	if (scanFrom == 0) return;
	OgreTreeItemBase *currentItem = 0;
	SGTGameObject *currentNode = NULL;
	OgreTreeItemBaseArray addedItems;
	//Ogre::LogManager::getSingleton().logMessage(scanFrom->GetName() + " " + Ogre::StringConverter::toString(scanFrom->GetNumChildren()));
	if (scanFrom->GetNumChildren() > 0)
	{
		for (unsigned short i = 0; i < scanFrom->GetNumChildren(); i++)
		{
			currentNode = scanFrom->GetChild(i);
			currentItem = OnCreateTreeItem(VDTC_TI_FILE, currentNode);
			addedItems.Add(currentItem);
		}
		AddItemsToTreeCtrl(item, addedItems);
	}

	if (subScan == true)
	{
		OgreTreeItemBase *b;
		wxTreeItemIdValue cookie = 0;

		wxTreeItemId child = GetFirstChild(item->GetId(), cookie);
		while(child.IsOk())
		{
			b = (OgreTreeItemBase *)GetItemData(child);
			if(b)
			{
				if (b->IsFile()) ScanFromNode(b, b->getNode(), false);
			}

			child = GetNextChild(item->GetId(), cookie);
		}
	}
};

bool wxOgreSceneTree::_itemHasFolder(OgreTreeItemBase *item, wxString& FolderName, wxTreeItemId *destfolderID)
{
	OgreTreeItemBase *b;
	wxTreeItemIdValue cookie = 0;

	wxTreeItemId child = GetFirstChild(item->GetId(), cookie);
	while(child.IsOk())
	{
		b = (OgreTreeItemBase *)GetItemData(child);
		if ((b->IsDir()) && (b->GetCaption() == FolderName))
		{
			*destfolderID = b->GetId();
			return true;
		}
		child = GetNextChild(item->GetId(), cookie);
	}
	return false;
};

bool wxOgreSceneTree::_itemHasChild(wxTreeItemId itemID, SGTGameObject *item, SGTGameObject* node)
{
	OgreTreeItemBase *b;
	wxTreeItemIdValue cookie = 0;

	wxTreeItemId child = GetFirstChild(itemID, cookie);
	while(child.IsOk())
	{
		b = (OgreTreeItemBase *)GetItemData(child);
		if (b->getNode() == node) return true;
		child = GetNextChild(itemID, cookie);
	}
	return false;
};

void wxOgreSceneTree::OnExpanding(wxTreeEvent &event)
{
	// check for collapsing item, and scan from there
	wxTreeItemId id = event.GetItem();
	if(id.IsOk())
	{
		OgreTreeItemBase *t = (OgreTreeItemBase *)GetItemData(id);
		if(t)
		{
			//Ogre::LogManager::getSingleton().logMessage("OnExpanding " + Ogre::String(t->GetName().c_str()));
			if (t->IsDir())
			{
				//Item ist nur Filter ohne Node
				OgreTreeItemBase *b;
				wxTreeItemIdValue cookie = 0;
				wxTreeItemId child = GetFirstChild(t->GetId(), cookie);
				while(child.IsOk())
				{
					b = (OgreTreeItemBase *)GetItemData(child);
					if(b)
					{
						ScanFromNode(b, b->getNode(), false);
					}
					child = GetNextChild(t->GetId(), cookie);
				}
			}

			// extract data element belonging to it, and scan.
			else ScanFromNode(t, t->getNode());
		}
	}

	// be kind, and let someone else also handle this event
	event.Skip();
}


void wxOgreSceneTree::AddItemsToTreeCtrl(OgreTreeItemBase *item, OgreTreeItemBaseArray &items)
{
	//Ogre::LogManager::getSingleton().logMessage("AddItemsToTreeCtrl " + Ogre::String(item->GetName().c_str()) + " " + Ogre::StringConverter::toString(items.GetCount()));
	wxCHECK2(item, return);

	// now loop through all elements on this level and add them
	// to the tree ctrl pointed out by 'id'

	OgreTreeItemBase *t;
	wxTreeItemId id = item->GetId();
	for(size_t i = 0; i < items.GetCount(); i++)
	{
		id = item->GetId();
		t = items[i];
		if(t)
		{
			if (!_itemHasChild(id, item->getNode(), t->getNode())) AppendItem(id, t->GetCaption(), t->GetIconId(), t->GetSelectedIconId(), t);
			/*wxTreeItemId folderID;
			if (!_itemHasFolder(item, wxString(t->getNode()->TellName().c_str()), &folderID))
			{
				OgreTreeItemBase *folder = new OgreTreeItemBase(VDTC_TI_DIR, wxString(t->getNode()->TellName().c_str()), t->getNode());
				AppendItem(id, folder->GetCaption(), folder->GetIconId(), folder->GetSelectedIconId(), folder);
				folderID = folder->GetId();
			}
			if (!_itemHasChild(folderID, item->getNode(), t->getNode())) AppendItem(folderID, t->GetCaption(), t->GetIconId(), t->GetSelectedIconId(), t);*/
		}
	}
};

wxBitmap *wxOgreSceneTree::CreateRootBitmap()
{
	// create root and return
	return new wxBitmap(xpm_root);
};

wxBitmap *wxOgreSceneTree::CreateFolderBitmap()
{
	// create folder and return
	return new wxBitmap(xpm_folder);
};

wxBitmap *wxOgreSceneTree::CreateNodeBitmap()
{
	// create file and return
	return new wxBitmap(xpm_file);
};

void wxOgreSceneTree::OnAssignIcons(wxImageList &icons)
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

OgreTreeItemBase* wxOgreSceneTree::OnCreateTreeItem(int type, SGTGameObject *node)
{
	// return a default instance, no extra info needed in this item
	if (node == 0) return new OgreTreeItemBase(type, "Root", node);
	return new OgreTreeItemBase(type, node->GetName(), node);
}; 

void wxOgreSceneTree::OnItemMenu(wxTreeEvent &event)
{
};

void wxOgreSceneTree::OnItemActivated(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if(id.IsOk())
	{
		OgreTreeItemBase *t = (OgreTreeItemBase *)GetItemData(id);
		if (t->IsFile())
		{
			wxEdit::Instance().GetOgrePane()->mEdit->SelectObject(t->getNode());
			SGTMain::Instance().GetCamera()->setPosition(t->getNode()->GetGlobalPosition() - Ogre::Vector3(0,0,5));
			SGTMain::Instance().GetCamera()->lookAt(t->getNode()->GetGlobalPosition());
			wxEdit::Instance().GetOgrePane()->update();
		}
	}
};

void wxOgreSceneTree::OnSelChanged(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if(id.IsOk())
	{
		OgreTreeItemBase *t = (OgreTreeItemBase *)GetItemData(id);
		if (t->IsFile())
		{
			mSelectedItem = t;
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")))->SetObject(t->getNode());
			wxEdit::Instance().GetpropertyWindow()->Refresh();
		}
	}
}

void wxOgreSceneTree::ExpandToObject(SGTGameObject *object)
{
	ExpandToObject(mStart, object);
}

bool wxOgreSceneTree::ExpandToObject(OgreTreeItemBase *from, SGTGameObject *object)
{
	wxTreeItemIdValue cookie = 0;
	OgreTreeItemBase *b;
	wxTreeItemId child = GetFirstChild(from->GetId(), cookie);
	while(child.IsOk())
	{
		b = (OgreTreeItemBase *)GetItemData(child);
		if (b->IsFile())
		{
			if (b->getNode() == object)
			{
				Expand(from->GetId());
				SelectItem(b->GetId());
				return true;
			}
			else
			{
				if (ExpandToObject(b, object)) return true;
			}
		}
		if (b->IsDir())
		{
			if (ExpandToObject(b, object))
			{
				Expand(from->GetId());
				return true;
			}
		}

		child = GetNextChild(from->GetId(), cookie);
	}
	return false;
}