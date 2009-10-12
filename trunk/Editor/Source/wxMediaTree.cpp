
#include "wxMediaTree.h"
#include "wxEdit.h"
#include "Commctrl.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "TransformTool.h"
#include "OgreEnvironment.h"
#include "OgreScriptCompiler.h"
#include "GUISystem.h"

wxMediaTree::wxMediaTree(wxWindow* parent, wxWindowID id, const wxPoint& pos,
             const wxSize& size, long style,
             const wxValidator& validator,
             const wxString& name)
		: wxFileTree(parent, id, pos, size, style, validator, name)
{
	mShowMeshes = true;
	mShowTextures = true;
	mShowSounds = true;
	RefreshFilters();
	SetDropTarget(this);

	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("ShowMeshes", "MediaTree", "Data/Editor/Intern/Editor_meshbox_01.png", wxMediaTree::OnToolbarEvent, true, true);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("ShowTextures", "MediaTree", "Data/Editor/Intern/Editor_textur_01.png", wxMediaTree::OnToolbarEvent, true, true);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("ShowSounds", "MediaTree", "Data/Editor/Intern/Editor_music_01_small.png", wxMediaTree::OnToolbarEvent, true, true);
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("MediaTree", false);
}

void wxMediaTree::OnToolbarEvent(int toolID, Ogre::String toolname)
{
	bool checked = wxEdit::Instance().GetExplorerToolbar()->GetToolState(toolID);
	if (toolname == "ShowMeshes") wxEdit::Instance().GetWorldExplorer()->GetMediaTree()->mShowMeshes = checked;
	if (toolname == "ShowTextures") wxEdit::Instance().GetWorldExplorer()->GetMediaTree()->mShowTextures = checked;
	if (toolname == "ShowSounds") wxEdit::Instance().GetWorldExplorer()->GetMediaTree()->mShowSounds = checked;
	wxEdit::Instance().GetWorldExplorer()->GetMediaTree()->RefreshFilters();
}

void wxMediaTree::RefreshFilters()
{
	Ogre::String relative_base;
	Ogre::String relPath = Ogre::String(mExpandedPath.c_str());
	wxArrayString extensions;
	if (mShowMeshes)
	{
		extensions.Add("*.mesh");
		extensions.Add("*.skeleton");
	}
	if (mShowTextures)
	{
		extensions.Add("*.tga");
		extensions.Add("*.jpg");
		extensions.Add("*.png");
		extensions.Add("*.dds");
		extensions.Add("*.psd");
		extensions.Add("*.material");
	}
	if (mShowSounds)
	{
		extensions.Add("*.ogg");
	}
	SetExtensions(extensions);
	SetRootPath("Data/Media/");
	wxFileName expandto = wxFileName(relPath.c_str());
	ExpandToPath(expandto);
}

void wxMediaTree::OnRenameItemCallback(Ogre::String oldpath, Ogre::String newpath)
{
}

void wxMediaTree::OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item)
{
}

void wxMediaTree::OnMenuCallback(int id)
{
}

void wxMediaTree::OnSelectItemCallback()
{
	if (mCurrentItem->IsFile())
	{
		SGTSceneManager::Instance().DestroyPreviewRender("EditorPreview");
		if (mCurrentItem->GetName().find(".mesh") != Ogre::String::npos)
		{
			Ogre::SceneNode *node = SGTMain::Instance().GetPreviewSceneMgr()->getRootSceneNode()->createChildSceneNode("EditorPreview");
			Ogre::Entity *entity = SGTMain::Instance().GetPreviewSceneMgr()->createEntity("EditorPreview_Mesh", mCurrentItem->GetName().c_str());
			node->attachObject(entity);
			SGTSceneManager::Instance().CreatePreviewRender(node);
			Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName("gui/runtime");
			material->getTechnique(0)->getPass(0)->removeAllTextureUnitStates();
			material->getTechnique(0)->getPass(0)->createTextureUnitState("EditorPreview_Tex");
			wxEdit::Instance().GetOgrePane()->GetEdit()->mPreviewWindow.SetMaterial(material->getName());
		}
		else if	(		mCurrentItem->GetName().find(".tga")	!= Ogre::String::npos
					||	mCurrentItem->GetName().find(".png")	!= Ogre::String::npos
					||	mCurrentItem->GetName().find(".tiff")	!= Ogre::String::npos
					||	mCurrentItem->GetName().find(".dds")	!= Ogre::String::npos
					||	mCurrentItem->GetName().find(".jpg")	!= Ogre::String::npos
					||	mCurrentItem->GetName().find(".psd")	!= Ogre::String::npos)
		{
			//Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().create(mCurrentItem->GetName(), "General");
			Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName("gui/runtime");
			material->getTechnique(0)->getPass(0)->removeAllTextureUnitStates();
			material->getTechnique(0)->getPass(0)->createTextureUnitState(mCurrentItem->GetName().c_str())->setTextureAddressingMode(Ogre::TextureUnitState::TextureAddressingMode::TAM_CLAMP);
			wxEdit::Instance().GetOgrePane()->GetEdit()->mPreviewWindow.SetMaterial(material->getName());
		}
	}
}

bool wxMediaTree::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString&  filenames)
{
	if (mCurrentItem->IsFile()) return false;
	Ogre::String relative_base(GetRelativePath(mCurrentItem->GetId()).GetFullPath());
	Ogre::String target_base("Data\\Media\\" + relative_base);
	//VdtcTreeItemBaseArray items;
	for (unsigned int i = 0; i < filenames.size(); i++)
	{
		boost::filesystem::path source(filenames[i]);
		if (!source.has_leaf()) continue;
		Ogre::String ext = "*" + source.leaf().substr(source.leaf().find_last_of("."));
		if (_extensions.Index(ext.c_str()) == wxNOT_FOUND) continue;
		/*VdtcTreeItemBase *item = AddFileItem(source.leaf());
		items.Add(item);*/
		boost::filesystem::path target(target_base + source.leaf());
		target = boost::filesystem::complete(target);
		bool removed = false;
		try
		{
			if (boost::filesystem::exists(target))
			{
				boost::filesystem::remove(target);
				removed = true;
			}
			boost::filesystem::copy_file(source, target);
		}
		catch (std::exception e)
		{
			Ogre::LogManager::getSingleton().logMessage(Ogre::String("Exception: ") + e.what());
		}

		if (target.leaf().find(".mesh") != wxString::npos)
		{
			Ogre::ResourcePtr rp = Ogre::MeshManager::getSingleton().getByName(target.leaf().c_str());
			if (!rp.isNull() && !removed)
			{
				wxMessageDialog dialog( 0, _T(target.leaf() + " already exists somewehere else!\nRename and try again."),
					_T("Error"), wxOK);
				dialog.ShowModal();
				boost::filesystem::remove(target);
				return false;
			}
			Ogre::MeshPtr meshpt = Ogre::MeshManager::getSingleton().load(target.leaf().c_str(), "General");
			float height = meshpt->getBounds().getSize().y;
			float scale_factor = 1.0f;
			if (height < 0.1f)
			{
				const wxString choices[] = { _T(Ogre::StringConverter::toString(height) + "m [Original]"), _T(Ogre::StringConverter::toString(height*10.0f)+"m"), _T(Ogre::StringConverter::toString(height*100.0f)+"m"), _T(Ogre::StringConverter::toString(height*39.3700787f)+"m")} ;

				wxSingleChoiceDialog dialog(this,
											_T(meshpt->getName() + " is really small!\n")
											_T("Which height sounds realistic to you?"),
											_T("Epic exporter fail - Rescale?"),
											WXSIZEOF(choices), choices);

				dialog.SetSelection(1);

				if (dialog.ShowModal() == wxID_OK)
				{
					if (dialog.GetSelection() == 1) scale_factor = 10.0f;
					if (dialog.GetSelection() == 2) scale_factor = 100.0f;
					if (dialog.GetSelection() == 3) scale_factor = 39.3700787f;
				}
			}
			else if (height > 10.0f)
			{
				const wxString choices[] = { _T(Ogre::StringConverter::toString(height) + "m [Original]"), _T(Ogre::StringConverter::toString(height/10.0f)+"m"), _T(Ogre::StringConverter::toString(height/100.0f)+"m"), _T(Ogre::StringConverter::toString(height*0.0254f)+"m")} ;

				wxSingleChoiceDialog dialog(this,
					_T(meshpt->getName() + " is really big!\n")
											_T("Which height sounds realistic to you?"),
											_T("Epic exporter fail - Rescale?"),
											WXSIZEOF(choices), choices);

				dialog.SetSelection(1);

				if (dialog.ShowModal() == wxID_OK)
				{
					if (dialog.GetSelection() == 1) scale_factor = 0.1f;
					if (dialog.GetSelection() == 2) scale_factor = 0.01f;
					if (dialog.GetSelection() == 3) scale_factor = 0.0254f;
				}
			}
			if (scale_factor != 1.0f)
			{
				meshmagick::OptionList transformOptions;
				Ogre::Any value = Ogre::Any(Ogre::Vector3(scale_factor, scale_factor, scale_factor));
				transformOptions.push_back(meshmagick::Option("scale", value));
				transformOptions.push_back(meshmagick::Option("xalign", Ogre::Any(Ogre::String("center"))));
				transformOptions.push_back(meshmagick::Option("yalign", Ogre::Any(Ogre::String("center"))));
				transformOptions.push_back(meshmagick::Option("zalign", Ogre::Any(Ogre::String("center"))));
				meshmagick::TransformTool tt;
				meshmagick::OptionList globalOptions;
				Ogre::StringVector sv;
				sv.push_back(target.file_string().c_str());
				tt.invoke(globalOptions, transformOptions, sv, sv);
				meshpt->reload();
			}
		}
		else if (source.leaf().find(".tga") != wxString::npos
					|| source.leaf().find(".png") != wxString::npos
					|| source.leaf().find(".tiff") != wxString::npos
					|| source.leaf().find(".dds") != wxString::npos
					|| source.leaf().find(".jpg") != wxString::npos
					|| source.leaf().find(".psd") != wxString::npos)
		{
			Ogre::ResourcePtr rp = Ogre::TextureManager::getSingleton().getByName(target.leaf().c_str());
			if (!rp.isNull() && !removed)
			{
				wxMessageDialog dialog( 0, _T(target.leaf() + " already exists somewehere else!\nRename and try again."),
					_T("Error"), wxOK);
				dialog.ShowModal();
				boost::filesystem::remove(target);
				return false;
			}
			Ogre::TextureManager::getSingleton().load(target.leaf().c_str(), "General");
		}
		else if (source.leaf().find(".material") != wxString::npos)
		{
			if (!removed)
			{
				std::fstream f;
				char cstring[256];
				f.open(target.file_string().c_str(), std::ios::in);
				while (!f.eof())
				{
					f.getline(cstring, sizeof(cstring));
					Ogre::String line = cstring;
					if (line.find("material") != Ogre::String::npos)
					{
						Ogre::String name = line.substr(line.find("material") + 9);
						if (!Ogre::MaterialManager::getSingleton().getByName(name).isNull())
						{
							wxMessageDialog dialog( 0, _T(name + " already exists!\nRename and try again."),
							_T("Error"), wxOK);
							dialog.ShowModal();
							f.close();
							boost::filesystem::remove(target);
							return false;
						}
					}
					f.close();
				}
			}
			ApplyDefaultLightning(target.file_string().c_str());
			Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(target.leaf().c_str());
			Ogre::ScriptCompilerManager::getSingleton().parseScript(stream, "General");
		}

	}
	SetRootPath("Data/Media/");
	wxFileName expandto = wxFileName(relative_base.c_str());
	ExpandToPath(expandto);
	//AddItemsToTreeCtrl(mCurrentItem, items);
	return true;
}


Ogre::String wxMediaTree::Scan_Line_For_Material(Ogre::String line)
{
	Ogre::String temp = "";
	bool found_material_statement = false;
	for (unsigned int i = 0; i < line.size(); i++)
	{
		if (temp.find("material") != Ogre::String::npos || temp.find("Material") != Ogre::String::npos && !found_material_statement)
		{
			found_material_statement = true;
			temp = "";
			continue;
		}

		if (line[i] == ':') return "";		//Das ist kein default material
		if (line[i] == ' ' || line[i] == 9) continue;
		temp += line[i];
	}
	return temp;
}

void wxMediaTree::ApplyDefaultLightning(Ogre::String materialfile)
{
	std::fstream f;
	char cstring[256];
	f.open(materialfile.c_str(), std::ios::in);
	std::vector<Ogre::String> newfile;
	int bracket_counter = 0;
	Ogre::String curmat = "";
	while (!f.eof())
	{
		f.getline(cstring, sizeof(cstring));
		Ogre::String line = cstring;
		if (curmat == "" && bracket_counter == 0)
		{
			curmat = Scan_Line_For_Material(line);
			if (curmat != "")
			{
				newfile.push_back(line + " : Standard/PerPixelLightening");
				newfile.push_back("{");
				newfile.push_back("\tset $scale 1.0");
				newfile.push_back("\tset $specular_factor 1.0");
			}
		}
		if (curmat == "") newfile.push_back(line);
		else
		{
			if (line.find("texture ") != Ogre::String::npos)
			{
				newfile.push_back("\tset $Diffuse " + line.substr(line.find("texture ") + 8, line.size()));
			}
		}
		if (line.find("{") != Ogre::String::npos) bracket_counter++;
		if (line.find("}") != Ogre::String::npos)
		{
			bracket_counter--;
			if (curmat != "" && bracket_counter == 0)
			{
				curmat = "";
				newfile.push_back("}");
			}
		}
	}
	f.close();
	std::fstream f2;
	f2.open(materialfile.c_str(), std::ios::out | std::ios::trunc);
	f2 << Ogre::String("import * from Standard.material").c_str() << std::endl;
	f2 << "" << std::endl;
	for (std::vector<Ogre::String>::iterator i = newfile.begin(); i != newfile.end(); i++)
	{
		f2 << (*i).c_str() << std::endl;
	}
	f2.close();
}

wxDragResult wxMediaTree::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	if (mDraggingFile) return def;
	wxPoint pt = wxPoint(x, y);
	TV_HITTESTINFO tvhti;
    tvhti.pt.x = pt.x;
    tvhti.pt.y = pt.y;
	wxTreeItemId item;
	if (TreeView_HitTest((HWND)GetHWND(), &tvhti) )
	{
		item = wxTreeItemId(tvhti.hItem);
		VdtcTreeItemBase *t = (VdtcTreeItemBase *)GetItemData(item);
		if (!t->IsFile()) SelectItem(item);
	}
	return def;
}

void wxMediaTree::OnEnterTab()
{
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("MediaTree", true);

	if (wxEdit::Instance().GetOgrePane()->GetEdit())
	{
		SGTGUISystem::GetInstance().SetVisible(wxEdit::Instance().GetOgrePane()->GetEdit()->mPreviewWindow.GetHandle(), true);
		if (mCurrentItem) OnSelectItemCallback();
	}
}
void wxMediaTree::OnLeaveTab()
{
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("MediaTree", false);

	Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName("gui/runtime");
	material->getTechnique(0)->getPass(0)->removeAllTextureUnitStates();
	SGTSceneManager::Instance().DestroyPreviewRender("EditorPreview");
	SGTGUISystem::GetInstance().SetVisible(wxEdit::Instance().GetOgrePane()->GetEdit()->mPreviewWindow.GetHandle(), false);
}