
#ifndef __wxOgreSceneTree_H__
#define __wxOgreSceneTree_H__

/*
Zeigt ein Ice::GameObject mit allen children geordnet nach Typ (Licht, Geometrie, Partikeleffekt usw.) an.
*/

#include <wx/dynarray.h>
#include <wx/Treectrl.h>
#include <wx/filesys.h>
#include <wx/imaglist.h>
#include "virtualdirTreectrl.h"
#include "Ogre.h"
#include "wxEdit.h"
#include "IceGameObject.h"
#include "EntityTreeNotebookListener.h"
#include "EDTIncludes.h"
#include <set>


class OgreTreeItemBase : public wxTreeItemData
{
protected:
	wxString mName;
	int mType;
	std::weak_ptr<Ice::GameObject> mGameObject;
	bool mIsDir;

public:
	OgreTreeItemBase(const wxString &name)
		: mIsDir(true), mName(name) {}
	OgreTreeItemBase(std::weak_ptr<Ice::GameObject> object)
		: mIsDir(false) { mGameObject = object; }

	~OgreTreeItemBase() {}

	const wxString &GetName()
	{
		Ice::GameObjectPtr object = mGameObject.lock();
		if (object.get()) mName = wxString(object->GetName().c_str());
		return mName;
	}
	virtual const wxString &GetCaption()
	{
		return GetName();
	}

	Ice::GameObjectPtr GetGameObject() { return mGameObject.lock(); }

	virtual int GetIconId() const
	{
		return mIsDir ? -1 : -1;
	}
	virtual int GetSelectedIconId() const
	{
		return -1;
	}

	bool IsDir() const { return mIsDir; }
	bool IsRoot() const { return mIsDir; }
	bool IsFile() const { return !mIsDir; }

};

class wxOgreSceneTree : public wxTreeCtrl, public EntityTreeNotebookListener
{
protected:
	OgreTreeItemBase *mStart;

	std::vector<OgreTreeItemBase*> mAllItems;

	OgreTreeItemBase* FindItemByObject(Ice::GameObjectPtr object);

	OgreTreeItemBase *mSelectedItem;

	OgreTreeItemBase* AppendGameObject(wxTreeItemId parent, Ice::GameObjectPtr object, std::set<int> &expandBlacklist = std::set<int>());

	void OnItemMenu(wxTreeEvent &event);
	void OnItemActivated(wxTreeEvent &event);

	bool ExpandToObject(OgreTreeItemBase *from, Ice::GameObjectPtr object);

public:
    /** Default constructor of this control. It is similar to the wxTreeCtrl */
    wxOgreSceneTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT,
                  const wxValidator& validator = wxDefaultValidator,
                  const wxString& name = "wxOgreSceneTree");
    virtual ~wxOgreSceneTree();

	Ice::GameObjectPtr GetSelectedItem();

	void Update();
	void NotifyObject(Ice::GameObjectPtr object);
	void UpdateObject(Ice::GameObjectPtr object);

	void ScanFromNode(std::queue<OgreTreeItemBase*> &expandQueue, std::set<int> &expandBlacklist = std::set<int>());

	void OnExpanding(wxTreeEvent &event);

	void OnSelChanged(wxTreeEvent &event);

	void ExpandToObject(Ice::GameObjectPtr object);

	void OnEnterTab();
	void OnLeaveTab();


private:
    DECLARE_EVENT_TABLE()
};

#endif