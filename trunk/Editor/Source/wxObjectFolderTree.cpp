
#include "../Header/wxObjectFolderTree.h"
#include "SGTScenemanager.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "SGTRagdoll.h"


enum
{
	ResTree_rename,
	ResTree_addDir,
	ResTree_del,
	ResTree_addGOC,
	ResTree_addRagdoll,
	ResTree_addNpc
};

BEGIN_EVENT_TABLE(wxObjectFolderTree, wxVirtualDirTreeCtrl)
	EVT_TREE_SEL_CHANGED(-1, wxObjectFolderTree::OnSelChanged)
	EVT_TREE_ITEM_MENU(-1, wxObjectFolderTree::OnItemMenu)
	EVT_TREE_ITEM_ACTIVATED(-1, wxObjectFolderTree::OnItemActivated)

	EVT_MENU(wxID_ANY, wxObjectFolderTree::OnMenuEvent)

END_EVENT_TABLE()


void wxObjectFolderTree::OnSetRootPath(const wxString &root)
{
	mCurrentItem = NULL;
}

void wxObjectFolderTree::OnMenuEvent(wxCommandEvent& event)
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

		/*TCHAR Buffer[100];
		GetCurrentDirectory(100, Buffer);
		Path = Ogre::String(Buffer) + "/" + Path;
		NewPath = Ogre::String(Buffer) + "/" + NewPath;
		Path.insert(3, "/");
		NewPath.insert(3, "/");
		for (int i = 0; i < Path.length(); i++)
		{
			if (Path[i] == '\\') Path[i] = '/';
		}
		for (int i = 0; i < NewPath.length(); i++)
		{
			if (NewPath[i] == '\\') NewPath[i] = '/';
		}*/

		//FullPath = FullPath.replace("\\", "/");
		//NewPath = NewPath.replace("\\", "/");

		//Ogre::LogManager::getSingleton().logMessage(Path);
		//Ogre::LogManager::getSingleton().logMessage(NewPath);

		boost::filesystem::path SourcePath(Path.c_str());
		boost::filesystem::path TargetPath(NewPath.c_str());
		boost::filesystem::rename(SourcePath, TargetPath);

		SetRootPath("Data/Editor/Objects/");
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
		SetRootPath("Data/Editor/Objects/");
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
		SetRootPath("Data/Editor/Objects/");
		ExpandToPath(wxString((BasePath).c_str()));
		return;
	}
	if (id == ResTree_addGOC)
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

		mCurrentPath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str()) + "/" + File;

		((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")))->NewResource(Path + File + ".ocs");
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

		mCurrentPath = Ogre::String(this->GetRelativePath(mCurrentItem->GetId()).GetFullPath().c_str()) + "/" + File;

		wxAuiPaneInfo& pane = wxEdit::Instance().GetAuiManager().GetPane(wxT("componentbar")).Show(false);
		wxEditSGTGameObject* page = ((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")));
		page->NewResource(Path + File + ".ocs", false);
		SGTDataMap map;
		SGTRagdoll::GetDefaultParameters(&map);
		page->AddGOCSection("Ragdoll", map);
	}

}

bool wxObjectFolderTree::OnAddDirectory(VdtcTreeItemBase &item, const wxFileName &name)
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

bool wxObjectFolderTree::OnAddFile(VdtcTreeItemBase &item, const wxFileName &name)
{
	return !(item.GetName().IsSameAs("descript.ion"));
}

void wxObjectFolderTree::OnItemMenu(wxTreeEvent &event)
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

void wxObjectFolderTree::ShowMenu(VdtcTreeItemBase *item, const wxPoint& pt)
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
		wxMenu *addMenu = new wxMenu("");
		addMenu->Append(ResTree_addGOC, "Component Assembly");
		addMenu->Append(ResTree_addRagdoll, "Ragdoll");
		addMenu->Append(ResTree_addNpc, "Npc");
		menu.AppendSubMenu(addMenu, "Add");

		menu.AppendSeparator();
		menu.Append(ResTree_addDir, "Add Filter");
	}

	if (item->IsDir() || item->IsFile())
	{
		if (item->IsDir()) menu.AppendSeparator();
		menu.Append(ResTree_rename, "Rename");
		menu.Append(ResTree_del, "Remove");
	}

	PopupMenu(&menu, pt);

};


void wxObjectFolderTree::OnItemActivated(wxTreeEvent &event)
{
};

void wxObjectFolderTree::OnSelChanged(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if(id.IsOk())
	{
		VdtcTreeItemBase *t = (VdtcTreeItemBase *)GetItemData(id);
		if (t->IsRoot()) return;
		mCurrentPath = Ogre::String(this->GetRelativePath(t->GetId()).GetPath().c_str()) + "/";
		if (t->IsFile())
		{
			Ogre::String Path = "Data\\Editor\\Objects\\" + Ogre::String(this->GetRelativePath(t->GetId()).GetPath().c_str()) + "\\";
			Ogre::String File = t->GetName().c_str();
			mCurrentPath += File;
			//Ogre::LogManager::getSingleton().logMessage("Object File Name: " + Path + File);

			Ogre::String extension = File.substr(File.find(".")+1, File.length());

			if (extension != "ot") ((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject")))->SetResource(Path + File);
		}
		mCurrentItem = (VdtcTreeItemBase *)GetItemData(id);
		//Ogre::LogManager::getSingleton().logMessage(mCurrentItem->GetName().c_str());
	}
};

Ogre::String wxObjectFolderTree::GetSelectedResource()
{
	return ((mCurrentItem == NULL) ? "None" : (mCurrentItem->IsFile() ? Ogre::String(GetFullPath(mCurrentItem->GetId()).GetFullPath().c_str()) : "None"));
}