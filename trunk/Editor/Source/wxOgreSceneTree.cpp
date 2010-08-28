
#include "../Header/wxOgreSceneTree.h"
#include "../Misc/folder.xpm"
#include "../Misc/file.xpm"
#include "../Misc/root.xpm"
#include "IceCameraController.h"
#include "IceSceneManager.h"
#include "Edit.h"

BEGIN_EVENT_TABLE(wxOgreSceneTree, wxTreeCtrl)
	EVT_TREE_ITEM_EXPANDING(-1, wxOgreSceneTree::OnExpanding)

	EVT_TREE_SEL_CHANGED(-1, wxOgreSceneTree::OnSelChanged)
	EVT_TREE_ITEM_MENU(-1, wxOgreSceneTree::OnItemMenu)
	EVT_TREE_ITEM_ACTIVATED(-1, wxOgreSceneTree::OnItemActivated)
END_EVENT_TABLE()

wxOgreSceneTree::wxOgreSceneTree(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name)
	: wxTreeCtrl(parent, id, pos, size, style, validator, name)
{
	mSelectedItem = nullptr;
};

wxOgreSceneTree::~wxOgreSceneTree()
{
	// first delete all OgreTreeItemBase items (client data)
	DeleteAllItems();

}

Ice::GameObject* wxOgreSceneTree::GetSelectedItem()
{
	if (mSelectedItem != nullptr)
	{
		return mSelectedItem->getGO();
	}
	return nullptr;
}

void wxOgreSceneTree::Update()
{
	// delete all items plus root first
	DeleteAllItems();
	OgreTreeItemBase *start = 0;

	mAllItems.clear();

	// create a root item
	start = new OgreTreeItemBase("Root");
	mStart = start;

	if(mStart)
	{
		wxTreeItemId id = AddRoot(mStart->GetCaption(), mStart->GetIconId(), mStart->GetSelectedIconId(), mStart);

		for (std::map<int, Ice::ManagedGameObject*>::iterator i = Ice::SceneManager::Instance().GetGameObjects().begin(); i != Ice::SceneManager::Instance().GetGameObjects().end(); i++)
		{
			if (!i->second->GetParent())
			{
				AppendGameObject(mStart->GetId(), i->second);
			}
		}

		Expand(id);
	}

	mSelectedItem = nullptr;

	wxEdit::Instance().GetpropertyWindow()->SetPage("EditSceneParams");
	wxEdit::Instance().GetpropertyWindow()->Refresh();
};

OgreTreeItemBase* wxOgreSceneTree::AppendGameObject(wxTreeItemId parent, Ice::GameObject *object)
{
	OgreTreeItemBase *item = new OgreTreeItemBase(object);
	mAllItems.push_back(item);
	AppendItem(parent, item->GetName(), item->GetIconId(), item->GetSelectedIconId(), item);
	ScanFromNode(item, object);
	return item;
}

OgreTreeItemBase* wxOgreSceneTree::FindItemByObject(Ice::GameObject *object)
{
	for (auto i = mAllItems.begin(); i != mAllItems.end(); i++)
		if ((*i)->getGO() == object) return *i;
	return nullptr;
}

void wxOgreSceneTree::UpdateObject(Ice::GameObject* object)
{
	OgreTreeItemBase *item = FindItemByObject(object);
	if (item) SetItemText(item->GetId(), object->GetName());
}

void wxOgreSceneTree::NotifyObject(Ice::GameObject *object)
{
	Ice::GameObject *parent = object->GetParent();
	if (parent)
	{
		OgreTreeItemBase *parentItem = FindItemByObject(parent);
		if (parentItem) AppendGameObject(parentItem->GetId(), object);
		else NotifyObject(parent);
	}
	else
	{
		AppendGameObject(mStart->GetId(), object);
	}
}

void wxOgreSceneTree::ScanFromNode(OgreTreeItemBase *item, Ice::GameObject *scanFrom)
{
	if (scanFrom == nullptr) return;
	OgreTreeItemBase *currentItem = 0;
	Ice::GameObject *currentNode = nullptr;
	//Ogre::LogManager::getSingleton().logMessage(scanFrom->GetName() + " " + Ogre::StringConverter::toString(scanFrom->GetNumChildren()));
	if (scanFrom->GetNumChildren() > 0)
	{
		for (unsigned short i = 0; i < scanFrom->GetNumChildren(); i++)
		{
			currentNode = scanFrom->GetChild(i);
			if (currentNode->IsManagedByParent()) AppendGameObject(item->GetId(), currentNode);
		}
	}
};

void wxOgreSceneTree::OnExpanding(wxTreeEvent &event)
{
	event.Skip();
} 

void wxOgreSceneTree::OnItemMenu(wxTreeEvent &event)
{
}

void wxOgreSceneTree::OnItemActivated(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if(id.IsOk())
	{
		OgreTreeItemBase *t = (OgreTreeItemBase *)GetItemData(id);
		if (t->IsFile())
		{
			wxEdit::Instance().GetOgrePane()->SelectObject(t->getGO());
			Ice::Main::Instance().GetCamera()->setPosition(t->getGO()->GetGlobalPosition() - Ogre::Vector3(0,0,5));
			Ice::Main::Instance().GetCamera()->lookAt(t->getGO()->GetGlobalPosition());
			wxEdit::Instance().GetOgrePane()->update();
		}
	}
}

void wxOgreSceneTree::OnSelChanged(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if(id.IsOk())
	{
		OgreTreeItemBase *t = (OgreTreeItemBase *)GetItemData(id);
		if (t->IsRoot())
		{
			wxEdit::Instance().GetpropertyWindow()->SetPage("EditSceneParams");
			wxEdit::Instance().GetpropertyWindow()->Refresh();
		}
		if (t->IsFile())
		{
			mSelectedItem = t;
		}
	}
}

void wxOgreSceneTree::ExpandToObject(Ice::GameObject *object)
{
	ExpandToObject(mStart, object);
}

bool wxOgreSceneTree::ExpandToObject(OgreTreeItemBase *from, Ice::GameObject *object)
{
	wxTreeItemIdValue cookie = 0;
	OgreTreeItemBase *b;
	wxTreeItemId child = GetFirstChild(from->GetId(), cookie);
	while(child.IsOk())
	{
		b = (OgreTreeItemBase *)GetItemData(child);
		if (b->IsFile())
		{
			if (b->getGO() == object)
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

void wxOgreSceneTree::OnEnterTab()
{
	wxEditIceGameObject *page = (wxEditIceGameObject*)wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject");
	if (GetSelectedItem() != 0)
	{
		page->SetObject(GetSelectedItem());
	}
}
void wxOgreSceneTree::OnLeaveTab()
{
}