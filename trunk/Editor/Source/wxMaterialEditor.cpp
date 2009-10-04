
#include "wxMaterialEditor.h"
#include <fstream>
#include "windows.h"
#include "OgreScriptCompiler.h"
#include "MaterialReload.h"

wxMaterialEditor::wxMaterialEditor(void)
{
}

wxMaterialEditor::~wxMaterialEditor(void)
{
}

Ogre::String wxMaterialEditor::FindResource(Ogre::MaterialPtr material)
{
	return SGTSceneManager::Instance().FindResourcePath("Data", material->getOrigin());
}

Ogre::String wxMaterialEditor::Scan_Line_For_Material(Ogre::String line)
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

		if (line[i] == ':') break;
		if (line[i] == ' ' || line[i] == 9) continue;
		temp += line[i];
	}
	return temp;
}

void wxMaterialEditor::OnApply()
{
	if (mCurrentFile != "" && mCurrentTemplate != "")
	{
	std::fstream f;
	char cstring[256];
	f.open(mCurrentFile.c_str(), std::ios::in);
	bool found = false;
	bool replaced = false;
	bool addimport = true;
	Ogre::String import_file = mCurrentTemplateFile.substr(mCurrentTemplateFile.find("Templates/") + 10, mCurrentTemplateFile.size());
	int bracket_counter = 0;
	std::vector<Ogre::String> newfile;
	while (!f.eof())
	{
		f.getline(cstring, sizeof(cstring));
		Ogre::String line = cstring;
		if (line.find("import * from " + import_file) != Ogre::String::npos) addimport = false;
		if (found)
		{
			if (line.find("{") != Ogre::String::npos) bracket_counter++;
			if (line.find("}") != Ogre::String::npos) bracket_counter--;
			if (bracket_counter <= 0)
			{
				replaced = true;
				found = false;
			}
			continue;
		}
		else if (!replaced)
		{
			Ogre::String temp = Scan_Line_For_Material(line);
			if (temp == mCurrentMaterial->getName())
			{
				if (line.find("{") != Ogre::String::npos) bracket_counter++;
				if (line.find("}") != Ogre::String::npos) bracket_counter--;
				found = true;

				newfile.push_back("material " + mCurrentMaterial->getName() + " : " + mCurrentTemplate);
				newfile.push_back("{");
				for (wxPropertyGridIterator it = mPropGrid->GetIterator(); !it.AtEnd(); it++)
				{
					wxPGProperty* p = *it;
					if (p->IsCategory()) continue;
					Ogre::String oValue = p->GetValue().GetString().c_str();
					if (oValue == "") oValue = "1";
					Ogre::String newline = Ogre::String("\tset $") + Ogre::String(p->GetName().c_str()) + " " + oValue;
					newfile.push_back(newline);
				}
				newfile.push_back("}");
				continue;
			}
		}
		newfile.push_back(line);
	}
	f.close();
	std::fstream f2;
	f2.open(mCurrentFile.c_str(), std::ios::out | std::ios::trunc);
	if (addimport)
	{
		f2 << (Ogre::String("import * from ") + import_file).c_str() << std::endl;
	}
	for (std::vector<Ogre::String>::iterator i = newfile.begin(); i != newfile.end(); i++)
	{
		f2 << (*i).c_str() << std::endl;
	}
	f2.close();

	//Das Material darf nirgendwo genutzt werden beim Reload, deswegen muss es temprär durch BaseWhiteNoLightning ersetzt werden.
	Ogre::SceneManager::MovableObjectIterator ents = SGTMain::Instance().GetOgreSceneMgr()->getMovableObjectIterator("Entity");
	while (ents.hasMoreElements())
	{
		Ogre::Entity *ent = (Ogre::Entity*)(ents.getNext());
		for (unsigned short x = 0; x < ent->getNumSubEntities(); x++)
		{
			Ogre::SubEntity *subent = ent->getSubEntity(x);
			if (subent->getMaterialName() == mCurrentMaterial->getName()) subent->setMaterialName("BaseWhiteNoLighting");
		}
	}

	Ogre::SceneManager::MovableObjectIterator decals = SGTMain::Instance().GetOgreSceneMgr()->getMovableObjectIterator("BillboardSet");
	while (decals.hasMoreElements())
	{
		Ogre::BillboardSet *decal = (Ogre::BillboardSet*)(decals.getNext());
		if (decal->getMaterialName() == mCurrentMaterial->getName()) decal->setMaterialName("BaseWhiteNoLighting");
	}
	/*Ogre::SharedPtr<Ogre::GpuProgramParameters> params;
	mCurrentMaterial->getTechnique(0)->getPass("SkyLight")->getFragmentProgramParameters()->copyConstantsFrom(params);
	Ogre::MaterialPtr tempmaterial = mCurrentMaterial->clone("EditMaterial_Temp");
	tempmaterial->copyDetailsTo(mCurrentMaterial);*/
	ReloadMaterial(mCurrentMaterial->getName(), "General", mCurrentMaterial->getOrigin(), true);
	ents = SGTMain::Instance().GetOgreSceneMgr()->getMovableObjectIterator("Entity");
	while (ents.hasMoreElements())
	{
		Ogre::Entity *ent = (Ogre::Entity*)(ents.getNext());
		for (unsigned short x = 0; x < ent->getNumSubEntities(); x++)
		{
			Ogre::SubEntity *subent = ent->getSubEntity(x);
			if (subent->getMaterialName() == "BaseWhiteNoLighting") subent->setMaterialName(mCurrentMaterial->getName());
		}
	}

	decals = SGTMain::Instance().GetOgreSceneMgr()->getMovableObjectIterator("BillboardSet");
	while (decals.hasMoreElements())
	{
		Ogre::BillboardSet *decal = (Ogre::BillboardSet*)(decals.getNext());
		if (decal->getMaterialName() == "BaseWhiteNoLighting") decal->setMaterialName(mCurrentMaterial->getName());
	}

	wxEdit::Instance().GetOgrePane()->SetFocus();
	}
}

void wxMaterialEditor::OnActivate()
{
}

void wxMaterialEditor::OnLeave()
{
	wxEdit::Instance().GetAuiManager().GetPane("Properties").Caption("Properties");
	wxEdit::Instance().GetAuiManager().Update();
}

void wxMaterialEditor::SetMaterialTemplate(Ogre::String Name, Ogre::String File)
{
	wxEdit::Instance().GetAuiManager().GetPane("Properties").Caption(Name);
	wxEdit::Instance().GetAuiManager().Update();
	mCurrentTemplate = Name;
	mCurrentTemplateFile = File;

	wxPGProperty* parameters = mPropGrid->Append( new wxPropertyCategory(wxT("Shader Params")) );
	parameters->SetExpanded(true);

	std::fstream f;
	char cstring[256];
	f.open(File.c_str(), std::ios::in);
	bool found = false;
	int bracket_counter = 0;
	std::vector<Ogre::String> added;
	while (!f.eof())
	{
		f.getline(cstring, sizeof(cstring));
		Ogre::String line = cstring;
		if (found)
		{
			if (line.find("$") != Ogre::String::npos)
			{
				Ogre::String prop = ""; 
				bool start = false;
				for (unsigned int i = 0; i < line.size(); i++)
				{
					if (start == true)
					{
						if (line[i] == ' ' || line[i] == 9)
						{
							bool addstop = false;
							for (std::vector<Ogre::String>::iterator i = added.begin(); i != added.end(); i++)
							{
								if ((*i) == prop)
								{
									addstop = true;
								}
							}
							if (!addstop)
							{
								added.push_back(prop);
								if (line.find(" float ") != Ogre::String::npos) mPropGrid->Append( new wxFloatProperty(wxT(prop.c_str()), wxT(prop.c_str()), 1.0f));
								else mPropGrid->Append( new wxStringProperty(wxT(prop.c_str()), wxT(prop.c_str())) );
							}

							start = false;
							prop = "";
							continue;
						}
						prop += line[i];
					}
					if (line[i] == '$') start = true;
				}
				if (prop != "")
				{
					bool addstop = false;
					for (std::vector<Ogre::String>::iterator i = added.begin(); i != added.end(); i++)
					{
						if ((*i) == prop)
						{
							addstop = true;
						}
					}
					if (!addstop)
					{
						added.push_back(prop);
						if (line.find(" float ") != Ogre::String::npos) mPropGrid->Append( new wxFloatProperty(wxT(prop.c_str()), wxT(prop.c_str())));
						else mPropGrid->Append( new wxStringProperty(wxT(prop.c_str()), wxT(prop.c_str())) );
					}
				}
			}
			if (line.find("{") != Ogre::String::npos) bracket_counter++;
			if (line.find("}") != Ogre::String::npos) bracket_counter--;
			if (bracket_counter <= 0) found = false;
		}
		else
		{
			Ogre::String temp = Scan_Line_For_Material(line);
			if (temp == Name)
			{
				if (line.find("{") != Ogre::String::npos) bracket_counter++;
				if (line.find("}") != Ogre::String::npos) bracket_counter--;
				found = true;
			}
		}
	}
	f.close();

	mPropGrid->Refresh();
}

void wxMaterialEditor::EditMaterial(Ogre::MaterialPtr material, bool detect_template)
{
	mCurrentMaterial = material;
	mCurrentFile = "";
	mCurrentFile = FindResource(mCurrentMaterial);
	if (mCurrentFile == "") return;
	if (detect_template) mCurrentTemplate = "";

	if (detect_template) wxEdit::Instance().GetAuiManager().GetPane("Properties").Caption("Drop Maps here or choose template");
	//Ogre::LogManager::getSingleton().logMessage("Edit Material: " + mCurrentFile);

	std::fstream f;
	char cstring[256];
	f.open(mCurrentFile.c_str(), std::ios::in);
	bool found = false;
	bool IsCustom = false;
	int bracket_counter = 0;
	std::vector<Ogre::String> newfile;
	while (!f.eof())
	{
		f.getline(cstring, sizeof(cstring));
		Ogre::String line = cstring;

		if (found)
		{
			if (IsCustom)
			{
			if (line.find("$") != Ogre::String::npos)
			{
				Ogre::String prop = ""; 
				Ogre::String value = ""; 
				bool propstart = false, valstart = false;
				for (unsigned int i = 0; i < line.size(); i++)
				{
					if (valstart == true)
					{
						if (line[i] != ' ' && line[i] != 9) value += line[i];
					}
					if (propstart == true)
					{
						if (line[i] == ' ' || line[i] == 9)
						{
							propstart = false;
							valstart = true;
							continue;
						}
						prop += line[i];
					}
					if (line[i] == '$') propstart = true;
				}
				for (wxPropertyGridIterator it = mPropGrid->GetIterator(); !it.AtEnd(); it++)
				{
					wxPGProperty* p = *it;
					if (p->IsCategory()) continue;
					if (Ogre::String(p->GetName().c_str()) == prop) p->SetValue(wxVariant(wxString(value.c_str())));
				}
			}
			}
			else
			{
				//ein bisschen hacky: wir gehen davon aus, dass es sich um ein normal exportiertes material handelt und suchen
				//lediglich nach dem schlüsselwort "texture".
				if (line.find("texture ") != Ogre::String::npos)
				{
					for (wxPropertyGridIterator it = mPropGrid->GetIterator(); !it.AtEnd(); it++)
					{
						wxPGProperty* p = *it;
						if (p->IsCategory()) continue;
						if (p->GetName() == "scale") p->SetValue(wxVariant(wxT("1.0")));
						if (p->GetName() == "Diffuse") p->SetValue(wxVariant(wxT(line.substr(line.find("texture ") + 8, line.size()))));
					}
				}
			}
			if (line.find("{") != Ogre::String::npos) bracket_counter++;
			if (line.find("}") != Ogre::String::npos) bracket_counter--;
			if (bracket_counter <= 0)
			{
				found = false;
				break;
			}
		}
		else
		{
			Ogre::String temp = Scan_Line_For_Material(line);
			if (temp == mCurrentMaterial->getName())
			{
				if (line.find(":") != Ogre::String::npos)
				{
					Ogre::String templatename = line.substr(line.find(":")+1, line.size());
					int blank_index = templatename.find(" ");
					while (blank_index != Ogre::String::npos)
					{
						templatename = templatename.substr(0, blank_index) + templatename.substr(blank_index+1);
						blank_index = templatename.find(" ");
					}
					Ogre::String templatelocation = wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->GetTemplateLocation(templatename);
					if (templatelocation != "" && detect_template) SetMaterialTemplate(templatename, templatelocation);
					IsCustom = true; 
				}
				if (line.find("{") != Ogre::String::npos) bracket_counter++;
				if (line.find("}") != Ogre::String::npos) bracket_counter--;
				found = true;
			}
		}
	}
	f.close();
}

void wxMaterialEditor::RegisterDefaultMapTemplate(Ogre::String map_type, Ogre::String tname, Ogre::String tfile)
{
	MaterialTemplate mt;
	mt.mName = tname;
	mt.mFile = tfile;
	mMapTemplates.insert(std::make_pair<Ogre::String, MaterialTemplate>(map_type, mt));
}

bool wxMaterialEditor::OnDropText(const wxString& text)
{
	if (text == "MediaDragged" && !mCurrentMaterial.isNull())
	{
		VdtcTreeItemBase *item = wxEdit::Instance().GetWorldExplorer()->GetMediaTree()->GetDraggedItem();
		if (item)
		{
			if (item->IsFile())
			{
				if (item->GetName().find(".tga") != Ogre::String::npos
					|| item->GetName().find(".png") != Ogre::String::npos
					|| item->GetName().find(".tiff") != Ogre::String::npos
					|| item->GetName().find(".dds") != Ogre::String::npos
					|| item->GetName().find(".jpg") != Ogre::String::npos
					|| item->GetName().find(".psd") != Ogre::String::npos)
				{
					wxArrayString choices;
					for (std::map<Ogre::String, MaterialTemplate>::iterator i = mMapTemplates.begin(); i != mMapTemplates.end(); i++) choices.Add(i->first.c_str());

					wxSingleChoiceDialog dialog(wxEdit::Instance().GetWorldExplorer()->GetMaterialTree(),
						_T("Please select a map type:"),
						"What's that?",
						choices);

					dialog.SetSelection(0);

					if (dialog.ShowModal() == wxID_OK)
					{
						int selection = dialog.GetSelection();
						std::map<Ogre::String, MaterialTemplate>::iterator it = mMapTemplates.begin();
						for (int n = 0; n < selection; n++) it++;
						bool found = false;
						for (wxPropertyGridIterator pgi = mPropGrid->GetIterator(); !pgi.AtEnd(); pgi++)
						{
							wxPGProperty* p = *pgi;
							if (p->IsCategory()) continue;
							if (Ogre::String(p->GetName().c_str()) == it->first)
							{
								found = true;
								p->SetValue(wxVariant(wxString(item->GetName().c_str())));
								break;
							}
						}
						if (!found)
						{
							mPropGrid->Clear();
							SetMaterialTemplate(it->second.mName, it->second.mFile);
							EditMaterial(mCurrentMaterial, false);
							for (wxPropertyGridIterator pgi = mPropGrid->GetIterator(); !pgi.AtEnd(); pgi++)
							{
								wxPGProperty* p = *pgi;
								if (p->IsCategory()) continue;
								if (Ogre::String(p->GetName().c_str()) == it->first)
								{
									p->SetValue(wxVariant(wxString(item->GetName().c_str())));
									break;
								}
							}
						}
						OnApply();
						return true;
					}
				}
			}
		}
	}
	return false;
}