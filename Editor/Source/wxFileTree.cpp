
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "wxFileTree.h"
#include "Commctrl.h"
#include "wxEdit.h"

enum
{
	ResTree_rename = 3560,
	ResTree_addDir = 3561,
	ResTree_del = 3562
};

//IMPLEMENT_CLASS(wxFileTree, wxVirtualDirTreeCtrl)

BEGIN_EVENT_TABLE(wxFileTree, wxVirtualDirTreeCtrl)
	EVT_TREE_SEL_CHANGED(-1, wxFileTree::OnSelChanged)
	EVT_TREE_ITEM_MENU(-1, wxFileTree::OnItemMenu)
	EVT_TREE_ITEM_ACTIVATED(-1, wxFileTree::OnItemActivated)
	EVT_TREE_BEGIN_DRAG(-1, wxFileTree::OnBeginDrag)
	EVT_TREE_BEGIN_LABEL_EDIT(-1, wxFileTree::OnBeginLabelEdit) 
	EVT_TREE_END_LABEL_EDIT(-1, wxFileTree::OnEndLabelEdit) 

	EVT_MENU(wxID_ANY, wxFileTree::OnMenuEvent)

END_EVENT_TABLE()


bool wxDnDTreeItemData::SetData(size_t len, const void *buf)
{
	mTreeItem = (VdtcTreeItemBase*)buf;
	return true;
}

size_t wxDnDTreeItemData::GetDataSize() const
{
	return 4;
}

bool wxDnDTreeItemData::GetDataHere(void *buf) const
{
	memcpy((char*)buf, (char*)mTreeItem, 4);
	return true;
}
VdtcTreeItemBase* wxDnDTreeItemData::GetItem()
{
	return mTreeItem;
}


void wxFileTree::OnLeftDown(wxMouseEvent& ev)
{
	ev.Skip();
}

VdtcTreeItemBase* wxFileTree::GetDraggedItem()
{
	return mCurrentlyDragged;
}
void wxFileTree::OnBeginDrag(wxTreeEvent& event)
{
	wxTreeItemId id = event.GetItem();
	mCurrentlyDragged = (VdtcTreeItemBase *)GetItemData(id);
	SelectItem(id);

	wxFileDataObject data;
	data.AddFile(GetDragName().c_str());
	wxDropSource dropSource(this);
	dropSource.SetData(data);
	OnSetupDragCursor(dropSource);
	mDraggingFile = true;
	wxDragResult result = dropSource.DoDragDrop( TRUE );
	mDraggingFile = false;
	ClearHighlightedItem();
}

void wxFileTree::OnSetRootPath(const wxString &root)
{
	mCurrentItem = 0;
	mRootPath = root.c_str();
}

void wxFileTree::OnBeginLabelEdit(wxTreeEvent& event)
{
	wxTreeItemId id = event.GetItem();
	VdtcTreeItemBase *t = (VdtcTreeItemBase *)GetItemData(id);
	if (t->IsRoot()) event.Veto();
}
void wxFileTree::OnEndLabelEdit(wxTreeEvent& event)
{
	wxTreeItemId id = event.GetItem();
	Ogre::String newName = event.GetLabel();
	if (newName == "")
	{
		event.Veto();
		return;
	}
	VdtcTreeItemBase *t = (VdtcTreeItemBase *)GetItemData(id);
	Ogre::String oldName = t->GetName();

	Ogre::String BasePath = "\\" + Ogre::String(this->GetRelativePath(id).GetPath().c_str());
	Ogre::String OldPath = mRootPath + BasePath;
	if (t->IsDir())
	{
		int found = BasePath.find_last_of("\\");
		if (found != Ogre::String::npos)
		{
			BasePath = BasePath.substr(0, found) + "\\";
		}
	}
	if (t->IsFile())
	{
		Ogre::String oldextension = "";
		Ogre::String newextension = "";
		OldPath = OldPath + "\\" + Ogre::String(oldName);
		oldextension = oldName.substr(oldName.find("."), oldName.length());
		newextension = newName.substr(newName.find("."), newName.length());
		if (oldextension != newextension)
		{
			event.Veto();
			return;
		}
	}
	t->SetName(newName);

	Ogre::String NewPath = mRootPath + BasePath + "\\" + newName;

	boost::filesystem::path SourcePath(OldPath.c_str());
	boost::filesystem::path TargetPath(NewPath.c_str());
	boost::filesystem::rename(SourcePath, TargetPath);

	OnRenameItemCallback(OldPath, NewPath);
}

void wxFileTree::OnMenuEvent(wxCommandEvent& event)
{
	int id = event.GetId();
	if (id == ResTree_rename)
	{
		/*wxTreeEvent renameevent(wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT, this, mCurrentItem->GetId());
		ProcessEvent(renameevent);*/
	}
	if (id == ResTree_addDir)
	{
		if (mCurrentItem->IsFile()) return;
		Ogre::String BasePath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetPath().c_str());
		Ogre::String Path = mRootPath + "/" + BasePath;
		wxTextEntryDialog dialog(this,
			_T("Enter folder name:"),
			_T("Please enter a string"),
			_T(""),
			wxOK | wxCANCEL);

		Ogre::String Folder = "";
		if (dialog.ShowModal() == wxID_OK)
		{
			Folder = dialog.GetValue().c_str();
		}
		boost::filesystem::path newPath((Path + "/" + Folder.c_str()));
		boost::filesystem::create_directory(newPath);
		SetRootPath(mRootPath);
		ExpandToPath(wxString((BasePath + "/" + Folder).c_str()));
		OnCreateFolderCallback(BasePath + "/" + Folder);
		return;
	}
	if (id == ResTree_del)
	{
		Ogre::String BasePath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetPath().c_str());
		Ogre::String Path = mRootPath + "/" + BasePath;
		if (mCurrentItem->IsFile())
		{
			Path = Path + "/" + Ogre::String(mCurrentItem->GetName().c_str());
		}
		Ogre::LogManager::getSingleton().logMessage(Path);
		OnRemoveItemCallback();
		boost::filesystem::path SourcePath(Path.c_str());
		unsigned long success = boost::filesystem::remove_all(SourcePath);

		int found = BasePath.find_last_of("/");
		if (found != Ogre::String::npos)
		{
			BasePath = BasePath.substr(0, found) + "/";
		}
		SetRootPath(mRootPath);
		ExpandToPath(wxString((BasePath).c_str()));
		return;
	}
	OnMenuCallback(id);
}

Ogre::String wxFileTree::GetInsertPath()
{
	Ogre::String relPath = mExpandedPath.c_str();
	if (mCurrentItem)
	{
		if (mCurrentItem->IsDir())// || mCurrentItem->IsRoot())
		{
			relPath = Ogre::String(GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str()) + "\\";
		}
	}
	Ogre::String Path = relPath + "\\";
	return Path;
}

Ogre::String wxFileTree::DoCreateFileDialog()
{
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
	return File;
}

bool wxFileTree::OnAddDirectory(VdtcTreeItemBase &item, const wxFileName &name)
{
	wxString str = item.GetName();

	// no . in begin of dir
	if(str.StartsWith("."))
		return false;

	// no Debug, Release, CVS
	if(!str.CmpNoCase("Debug") || !str.CmpNoCase("Release") || !str.CmpNoCase("CVS"))
		return false;

	return true;
}

bool wxFileTree::OnAddFile(VdtcTreeItemBase &item, const wxFileName &name)
{
	return !(item.GetName().IsSameAs("descript.ion"));
}

void wxFileTree::OnItemMenu(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if(id.IsOk())
	{
		VdtcTreeItemBase *t = (VdtcTreeItemBase *)GetItemData(id);
		mCurrentItem = t;
		wxPoint clientpt = event.GetPoint();
		wxPoint screenpt = ClientToScreen(clientpt);

		ShowMenu(t, clientpt);
    event.Skip();
	}
};

void wxFileTree::ShowMenu(VdtcTreeItemBase *item, const wxPoint& pt)
{
	wxString title;
	if (item->IsRoot())
	{
		title = "";
	}
	if (item->IsDir())
	{
		title = "";
	}
	if (item->IsFile())
	{
		title = "";
	}
	wxMenu menu(title);

	if (item->IsRoot() || item->IsDir())
	{
		menu.Append(ResTree_addDir, "Add Folder");
		menu.AppendSeparator();
	}

	OnShowMenuCallback(&menu, item);

	if (item->IsDir() || item->IsFile())
	{
		//menu.Append(ResTree_rename, "Rename");
		menu.Append(ResTree_del, "Remove");
	}

	PopupMenu(&menu, pt);

};

void wxFileTree::OnItemActivated(wxTreeEvent &event)
{
};

void wxFileTree::OnSelChanged(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if(id.IsOk())
	{
		mCurrentItem = (VdtcTreeItemBase *)GetItemData(id);
		OnSelectItemCallback();
	}
};

Ogre::String wxFileTree::GetSelectedResource()
{
	return ((mCurrentItem == NULL) ? "None" : (mCurrentItem->IsFile() ? Ogre::String(GetFullPath(mCurrentItem->GetId()).GetFullPath().c_str()) : "None"));
}

bool wxFileTree::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
	if (!mHighlightedItem.IsOk()) return false;
	SelectItem(mHighlightedItem);
	ClearHighlightedItem();
	if (filenames.IsEmpty()) return false;
	if (filenames[0] == GetDragName())
	{
	}
	else if (IsExternFileDropTarget()) OnDropExternFilesCallback(filenames);
	return false;
}

wxDragResult wxFileTree::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	//if (mDraggingFile) return def;
	wxPoint pt = wxPoint(x, y);
	TV_HITTESTINFO tvhti;
    tvhti.pt.x = pt.x;
    tvhti.pt.y = pt.y;
	wxTreeItemId item;
	if (TreeView_HitTest((HWND)GetHWND(), &tvhti) )
	{
		item = wxTreeItemId(tvhti.hItem);
		VdtcTreeItemBase *t = (VdtcTreeItemBase *)GetItemData(item);
		if (!t->IsFile())
		{
			if (mHighlightedItem != item)
			{
				ClearHighlightedItem();
				SetItemBackgroundColour(item, wxColour(0, 180, 20));
				mHighlightedItem = item;
			}
		}
		else ClearHighlightedItem();
	}
	else ClearHighlightedItem();
	return def;
}

void wxFileTree::OnLeave()
{
	ClearHighlightedItem();
}

void wxFileTree::ClearHighlightedItem()
{
	if (mHighlightedItem.IsOk())
	{
		SetItemBackgroundColour(mHighlightedItem, "white");
		mHighlightedItem.Unset();
	}
}