
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "wxFileTree.h"
#include "Commctrl.h"
#include "wxEdit.h"
#include "Edit.h"

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

	EVT_KEY_DOWN(wxFileTree::OnKeyDown)

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

void wxFileTree::OnKeyDown(wxKeyEvent& key)
{
	if (key.GetKeyCode() == WXK_F5)
		SetRootPath(mRootPath);
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

	wxFileName relativePath = GetRelativePath(id);
	Ogre::String basePath = Ogre::String(relativePath.GetPath().c_str());
	Ogre::String oldPath = mRootPath + PATH_SEPERATOR + basePath;
	if (t->IsDir())
	{
		int found = basePath.find_last_of(PATH_SEPERATOR);
		if (found != Ogre::String::npos)
		{
			basePath = basePath.substr(0, found) + PATH_SEPERATOR;
		}
	}
	if (t->IsFile())
	{
		Ogre::String oldextension = "";
		Ogre::String newextension = "";
		oldextension = oldName.substr(oldName.find("."), oldName.length());
		newextension = newName.substr(newName.find("."), newName.length());
		if (oldextension != newextension)
		{
			event.Veto();
			return;
		}
	}
	t->SetName(newName);

	Ogre::String newPath = mRootPath + PATH_SEPERATOR + basePath;

	boost::filesystem::path SourcePath((oldPath + PATH_SEPERATOR + oldName).c_str());
	boost::filesystem::path TargetPath((newPath + + PATH_SEPERATOR + newName).c_str());
	boost::filesystem::rename(SourcePath, TargetPath);

	OnRenameItemCallback(oldPath, newPath, oldName, newName);
}

void wxFileTree::OnMenuEvent(wxCommandEvent& event)
{
	int id = event.GetId();
	if (id == ResTree_rename)
	{
		/*wxTreeEvent renameevent(wxEVT_COMMAND_Tree_BEGIN_LABEL_EDIT, this, mCurrentItem->GetId());
		ProcessEvent(renameevent);*/
	}
	if (id == ResTree_addDir)
	{
		if (mCurrentItem->IsFile()) return;
		Ogre::String BasePath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetPath().c_str());
		Ogre::String Path = mRootPath + PATH_SEPERATOR + BasePath;
		wxTextEntryDialog dialog(this,
			"Enter folder name:",
			"Please enter a string",
			"",
			wxOK | wxCANCEL);

		Ogre::String Folder = "";
		wxEdit::Instance().GetMainNotebook()->GetOgreWindow()->SetPaused(true);
		if (dialog.ShowModal() == wxID_OK)
		{
			Folder = dialog.GetValue().c_str();
		}
		boost::filesystem::path newPath((Path + PATH_SEPERATOR + Folder.c_str()));
		boost::filesystem::create_directory(newPath);
		SetRootPath(mRootPath);
		ExpandToPath(wxString((BasePath + PATH_SEPERATOR + Folder).c_str()));
		OnCreateFolderCallback(BasePath + PATH_SEPERATOR + Folder);
		wxEdit::Instance().GetMainNotebook()->GetOgreWindow()->SetPaused(false);
		return;
	}
	if (id == ResTree_del)
	{
		Ogre::String BasePath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetPath().c_str());
		Ogre::String Path = mRootPath + PATH_SEPERATOR + BasePath;
		if (mCurrentItem->IsFile())
		{
			Path = Path + PATH_SEPERATOR + Ogre::String(mCurrentItem->GetName().c_str());
		}
		Ice::Log::Instance().LogMessage(Path);
		OnRemoveItemCallback();
		boost::filesystem::path SourcePath(Path.c_str());
		unsigned long success = boost::filesystem::remove_all(SourcePath);

		int found = BasePath.find_last_of(PATH_SEPERATOR);
		if (found != Ogre::String::npos)
		{
			BasePath = BasePath.substr(0, found) + PATH_SEPERATOR;
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
			relPath = Ogre::String(GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str());
		}
	}
	return relPath;
}

Ogre::String wxFileTree::DoCreateFileDialog()
{
	wxTextEntryDialog dialog(this,
		"Enter file name:",
		"Please enter a string",
		"",
		wxOK | wxCANCEL);

	Ogre::String File = "";
	wxEdit::Instance().GetMainNotebook()->GetOgreWindow()->SetPaused(true);
	if (dialog.ShowModal() == wxID_OK)
	{
		File = dialog.GetValue().c_str();
	}
	wxEdit::Instance().GetMainNotebook()->GetOgreWindow()->SetPaused(false);
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

	//Ice::Main::Instance().AddOgreResourcePath(name.GetFullPath().c_str().AsChar());

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

bool wxFileTree::FileDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
	if (!mFileTree->mHighlightedItem.IsOk()) return false;
	mFileTree->SelectItem(mFileTree->mHighlightedItem);
	mFileTree->ClearHighlightedItem();
	if (filenames.IsEmpty()) return false;
	if (filenames[0] == mFileTree->GetDragName())
	{
	}
	else if (mFileTree->IsExternFileDropTarget()) mFileTree->OnDropExternFilesCallback(filenames);
	return false;
}

wxDragResult wxFileTree::FileDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	//if (mDraggingFile) return def;
	wxPoint pt = wxPoint(x, y);
	TV_HITTESTINFO tvhti;
	tvhti.pt.x = pt.x;
	tvhti.pt.y = pt.y;
	wxTreeItemId item;
	if (TreeView_HitTest((HWND)mFileTree->GetHWND(), &tvhti) )
	{
		item = wxTreeItemId(tvhti.hItem);
		VdtcTreeItemBase *t = (VdtcTreeItemBase *)mFileTree->GetItemData(item);
		if (!t->IsFile())
		{
			if (mFileTree->mHighlightedItem != item)
			{
				mFileTree->ClearHighlightedItem();
				mFileTree->SetItemBackgroundColour(item, wxColour(0, 180, 20));
				mFileTree->mHighlightedItem = item;
			}
		}
		else mFileTree->ClearHighlightedItem();
	}
	else mFileTree->ClearHighlightedItem();
	return def;
}

void wxFileTree::FileDropTarget::OnLeave()
{
	mFileTree->ClearHighlightedItem();
}

void wxFileTree::ClearHighlightedItem()
{
	if (mHighlightedItem.IsOk())
	{
		SetItemBackgroundColour(mHighlightedItem, "white");
		mHighlightedItem.Unset();
	}
}