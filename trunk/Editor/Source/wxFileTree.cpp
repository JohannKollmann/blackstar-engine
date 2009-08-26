
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "wxFileTree.h"

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

	EVT_MENU(wxID_ANY, wxFileTree::OnMenuEvent)

END_EVENT_TABLE()


void wxFileTree::OnSetRootPath(const wxString &root)
{
	mCurrentItem = 0;
	mRootPath = root.c_str();
}

void wxFileTree::OnMenuEvent(wxCommandEvent& event)
{
	int id = event.GetId();
	if (id == ResTree_rename)
	{
		Ogre::String BasePath = "\\" + Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetPath().c_str());
		Ogre::String Path = "Data\\Editor\\Objects" + BasePath;
		if (mCurrentItem->IsDir())
		{
			int found = BasePath.find_last_of("\\");
			if (found != Ogre::String::npos)
			{
				BasePath = BasePath.substr(0, found) + "\\";
			}
		}

		wxTextEntryDialog dialog(this,
			_T("Enter file name:"),
			_T("Please enter a string"),
			_T(""),
			wxOK | wxCANCEL);

		Ogre::String File = "";
		Ogre::String extension = "";
		if (dialog.ShowModal() == wxID_OK)
		{
			File = dialog.GetValue().c_str();
		}
		if (mCurrentItem->IsFile())
		{
			Path = Path + "\\" + Ogre::String(mCurrentItem->GetName().c_str());
			extension = Path.substr(Path.find("."), Path.length());
		}

		Ogre::String NewPath = "Data\\Editor\\Objects" + BasePath + "\\" + File + extension;

		boost::filesystem::path SourcePath(Path.c_str());
		boost::filesystem::path TargetPath(NewPath.c_str());
		boost::filesystem::rename(SourcePath, TargetPath);

		SetRootPath(mRootPath);
		ExpandToPath(wxString((BasePath + "\\" + File + extension).c_str()));
		return;
	}
	if (id == ResTree_addDir)
	{
		Ogre::String BasePath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetPath().c_str()) + "\\";
		Ogre::String Path = "Data\\Editor\\Objects\\" + BasePath;
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
		TCHAR Buffer[100];
		GetCurrentDirectory(100, Buffer);
		Ogre::String FullPath = Ogre::String(Buffer) + "\\" + Path;

		Ogre::String newPath = FullPath + Folder;
		Ogre::LogManager::getSingleton().logMessage(newPath);
		if (!CreateDirectory(newPath.c_str(), NULL))
		{
			Ogre::LogManager::getSingleton().logMessage("Error while creating directory.");
		}
		wxFileName path = GetRelativePath(GetSelection());
		SetRootPath(mRootPath);
		ExpandToPath(wxString((BasePath + Folder).c_str()));
		return;
	}
	if (id == ResTree_del)
	{
		Ogre::String BasePath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetPath().c_str());
		Ogre::String Path = "Data\\Editor\\Objects\\" + BasePath;
		if (mCurrentItem->IsFile())
		{
			Path = Path + "\\" + Ogre::String(mCurrentItem->GetName().c_str());
		}
		Ogre::LogManager::getSingleton().logMessage(Path);

		boost::filesystem::path SourcePath(Path.c_str());
		boost::filesystem::remove_all(SourcePath);

		int found = BasePath.find_last_of("\\");
		if (found != Ogre::String::npos)
		{
			BasePath = BasePath.substr(0, found) + "\\";
		}
		SetRootPath(mRootPath);
		ExpandToPath(wxString((BasePath).c_str()));
		return;
	}
	OnMenuCallback(id);
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
		menu.Append(ResTree_rename, "Rename");
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