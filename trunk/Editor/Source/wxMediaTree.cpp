
#include "wxMediaTree.h"
#include "wxEdit.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "TransformTool.h"
#include "OgreEnvironment.h"
#include "OgreScriptCompiler.h"
#include "SGTSceneManager.h"

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

	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("ShowMeshes", "MediaTree", "Data/Editor/Intern/Editor_meshbox_01.png", wxMediaTree::OnToolbarEvent, "Show mesh files.", true, true);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("ShowTextures", "MediaTree", "Data/Editor/Intern/Editor_textur_01.png", wxMediaTree::OnToolbarEvent, "Show texture files.", true, true);
	wxEdit::Instance().GetExplorerToolbar()->RegisterTool("ShowSounds", "MediaTree", "Data/Editor/Intern/Editor_music_01_small.png", wxMediaTree::OnToolbarEvent, "Show sound files.", true, true);
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
		extensions.Add("*.scene");
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
	if (mCurrentItem->IsDir())
	{
		Ogre::ResourceGroupManager::getSingleton().removeResourceLocation("Data/Media/" + oldpath, "FileSystem");
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Data/Media/" + newpath, "FileSystem");
	}
}

void wxMediaTree::OnShowMenuCallback(wxMenu *menu, VdtcTreeItemBase *item)
{
}

void wxMediaTree::OnMenuCallback(int id)
{
}

bool wxMediaTree::IsTexture(wxString filename)
{
	if	(		filename.find(".tga")	!= Ogre::String::npos
			||	filename.find(".png")	!= Ogre::String::npos
			||	filename.find(".tiff")	!= Ogre::String::npos
			||	filename.find(".dds")	!= Ogre::String::npos
			||	filename.find(".jpg")	!= Ogre::String::npos
			||	filename.find(".psd")	!= Ogre::String::npos)
	{
		return true;
	}
	return false;
}
bool wxMediaTree::IsMaterial(wxString filename)
{
	if (filename.find(".material") != Ogre::String::npos)
	{
		return true;
	}
	return false;
}
bool wxMediaTree::IsMesh(wxString filename)
{
	if (filename.find(".mesh") != Ogre::String::npos)
	{
		return true;
	}
	return false;
}
bool wxMediaTree::IsAudio(wxString filename)
{
	if (filename.find(".ogg") != Ogre::String::npos)
	{
		return true;
	}
	return false;
}

void wxMediaTree::OnSelectItemCallback()
{
	if (mCurrentItem->IsFile())
	{
		SGTSceneManager::Instance().DestroyPreviewRender("EditorPreview");
		wxEdit::Instance().GetPreviewWindow()->ClearDisplay();
		if (IsMesh(mCurrentItem->GetName()))
		{
			Ogre::SceneNode *node = SGTMain::Instance().GetPreviewSceneMgr()->getRootSceneNode()->createChildSceneNode("EditorPreview");
			Ogre::Entity *entity = SGTMain::Instance().GetPreviewSceneMgr()->createEntity("EditorPreview_Mesh", mCurrentItem->GetName().c_str());
			node->attachObject(entity);
			float width = 256;//wxEdit::Instance().GetAuiManager().GetPane("preview").floating_size.GetWidth();
			float height = 256;//wxEdit::Instance().GetAuiManager().GetPane("preview").floating_size.GetHeight();
			SGTSceneManager::Instance().CreatePreviewRender(node, "EditorPreview", width, height);
			Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName("EditorPreview_Tex");
			wxEdit::Instance().GetPreviewWindow()->SetTexture(texture);
			wxEdit::Instance().GetPreviewWindow()->SetPreviewNode(node);
		}
		else if	(IsTexture(mCurrentItem->GetName()))
		{
			Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().load(mCurrentItem->GetName().c_str(), "General");
			wxEdit::Instance().GetPreviewWindow()->SetTexture(texture);
		}
		else if (IsAudio(mCurrentItem->GetName()))
		{
			int id = SGTSceneManager::Instance().RequestID();
			Ogre::SceneNode *node = SGTMain::Instance().GetPreviewSceneMgr()->getRootSceneNode()->createChildSceneNode("EditorPreview");
			OgreOggSound::OgreOggISound *sound = SGTMain::Instance().GetSoundManager()->createSound(Ogre::StringConverter::toString(id), mCurrentItem->GetName().c_str(), true, false);
			if (sound) sound->play();
			node->attachObject(sound);
			wxEdit::Instance().GetPreviewWindow()->SetPreviewNode(node);
		}
	}
}

void wxMediaTree::OnCreateFolderCallback(Ogre::String path)
{
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Data/Media/" + path, "FileSystem");
}

void wxMediaTree::OnDropExternFilesCallback(const wxArrayString& filenames)
{
	if (mCurrentItem->IsFile()) return;
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

		if (IsMesh(target.leaf()))
		{
			Ogre::ResourcePtr rp = Ogre::MeshManager::getSingleton().getByName(target.leaf().c_str());
			if (!rp.isNull() && !removed)
			{
				wxMessageDialog dialog( 0, _T(target.leaf() + " already exists somewehere else!\nRename and try again."),
					_T("Error"), wxOK);
				dialog.ShowModal();
				boost::filesystem::remove(target);
				return;
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
		else if (IsTexture(source.leaf()))
		{
			Ogre::ResourcePtr rp = Ogre::TextureManager::getSingleton().getByName(target.leaf().c_str());
			if (!rp.isNull() && !removed)
			{
				wxMessageDialog dialog( 0, _T(target.leaf() + " already exists somewehere else!\nRename and try again."),
					_T("Error"), wxOK);
				dialog.ShowModal();
				boost::filesystem::remove(target);
				continue;
			}
			Ogre::TextureManager::getSingleton().load(target.leaf().c_str(), "General");
		}
		else if (IsMaterial(source.leaf()))
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
							return;
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
	return;
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
	bool texture_added = false;
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
				texture_added = true;
				newfile.push_back("\tset $Diffuse " + line.substr(line.find("texture ") + 8, line.size()));
			}
		}
		if (line.find("{") != Ogre::String::npos) bracket_counter++;
		if (line.find("}") != Ogre::String::npos)
		{
			bracket_counter--;
			if (curmat != "" && bracket_counter == 0)
			{
				if (!texture_added) newfile.push_back("\tset $Diffuse blank.tga");
				texture_added = false;
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

void wxMediaTree::OnEnterTab()
{
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("MediaTree", true);

	if (wxEdit::Instance().GetOgrePane()->GetEdit())
	{
		if (mCurrentItem) OnSelectItemCallback();
	}
}
void wxMediaTree::OnLeaveTab()
{
	wxEdit::Instance().GetExplorerToolbar()->SetGroupStatus("MediaTree", false);

	SGTSceneManager::Instance().DestroyPreviewRender("EditorPreview");
	wxEdit::Instance().GetPreviewWindow()->ClearDisplay();
}

void wxMediaTree::OnSetupDragCursor(wxDropSource &dropSource)
{
	if (mCurrentlyDragged->IsFile())
	{
		if (IsMesh(mCurrentlyDragged->GetName()))
		{
			wxImage image("Data/Editor/Intern/Editor_meshbox_01.png");
			wxCursor cursor(image.Scale(32,32, wxIMAGE_QUALITY_HIGH));
			dropSource.SetCursor(wxDragResult::wxDragMove, cursor);
		}
		else if (IsTexture(mCurrentlyDragged->GetName()))
		{
			wxImage image("Data/Editor/Intern/Editor_TextureIcon02.png");
			wxCursor cursor(image.Scale(32,32, wxIMAGE_QUALITY_HIGH));
			dropSource.SetCursor(wxDragResult::wxDragMove, cursor);
		}
	}
}