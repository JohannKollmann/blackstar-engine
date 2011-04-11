
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

Ice::GameObjectPtr wxOgreSceneTree::GetSelectedItem()
{
	if (mSelectedItem != nullptr)
	{
		return mSelectedItem->GetGameObject();
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

		for (auto i = Ice::SceneManager::Instance().GetGameObjects().begin(); i != Ice::SceneManager::Instance().GetGameObjects().end(); i++)
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

OgreTreeItemBase* wxOgreSceneTree::AppendGameObject(wxTreeItemId parent, Ice::GameObjectPtr object, std::set<int> &expandBlacklist)
{
	OgreTreeItemBase *item = new OgreTreeItemBase(std::weak_ptr<Ice::GameObject>(object));
	mAllItems.push_back(item);
	AppendItem(parent, item->GetName(), item->GetIconId(), item->GetSelectedIconId(), item);
	ScanFromNode(item, object, expandBlacklist);
	return item;
}

OgreTreeItemBase* wxOgreSceneTree::FindItemByObject(Ice::GameObjectPtr object)
{
	for (auto i = mAllItems.begin(); i != mAllItems.end(); i++)
		if ((*i)->GetGameObject().get() == object.get()) return *i;
	return nullptr;
}

void wxOgreSceneTree::UpdateObject(Ice::GameObjectPtr object)
{
	OgreTreeItemBase *item = FindItemByObject(object);
	if (item) SetItemText(item->GetId(), object->GetName());
}

void wxOgreSceneTree::NotifyObject(Ice::GameObjectPtr object)
{
	Ice::GameObjectPtr parent = object->GetParent();
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

void wxOgreSceneTree::ScanFromNode(OgreTreeItemBase *item, Ice::GameObjectPtr scanFrom, std::set<int> &expandBlacklist)
{
	if (!scanFrom.get()) return;
	expandBlacklist.insert(scanFrom->GetID());
	OgreTreeItemBase *currentItem = nullptr;
	Ice::GameObjectPtr currentNode;
	//Ogre::LogManager::getSingleton().logMessage(scanFrom->GetName() + " " + Ogre::StringConverter::toString(scanFrom->GetNumChildren()));
	while (scanFrom->HasNextObjectReference())
	{
		currentNode = scanFrom->GetNextObjectReference()->Object.lock();
		if (currentNode.get() && expandBlacklist.find(currentNode->GetID()) == expandBlacklist.end())
			AppendGameObject(item->GetId(), currentNode, expandBlacklist);
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
			wxEdit::Instance().GetOgrePane()->SelectObject(t->GetGameObject());
			Ice::Main::Instance().GetCamera()->setPosition(t->GetGameObject()->GetGlobalPosition() - Ogre::Vector3(0,0,5));
			Ice::Main::Instance().GetCamera()->lookAt(t->GetGameObject()->GetGlobalPosition());
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

void wxOgreSceneTree::ExpandToObject(Ice::GameObjectPtr object)
{
	ExpandToObject(mStart, object);
}

bool wxOgreSceneTree::ExpandToObject(OgreTreeItemBase *from, Ice::GameObjectPtr object)
{
	wxTreeItemIdValue cookie = 0;
	OgreTreeItemBase *b;
	wxTreeItemId child = GetFirstChild(from->GetId(), cookie);
	while(child.IsOk())
	{
		b = (OgreTreeItemBase *)GetItemData(child);
		if (b->IsFile())
		{
			if (b->GetGameObject().get() == object.get())
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
	if (GetSelectedItem() != nullptr)
	{
		page->SetObject(GetSelectedItem());
	}
}
void wxOgreSceneTree::OnLeaveTab()
{
}