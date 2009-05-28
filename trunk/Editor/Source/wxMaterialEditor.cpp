
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
	return ScanPath("Data/Scripts/materials/scripts", material->getOrigin());
}

Ogre::String wxMaterialEditor::ScanPath(Ogre::String path, Ogre::String filename)
{
	HANDLE fHandle;
	WIN32_FIND_DATA wfd;
	fHandle=FindFirstFile((path + "/*").c_str(),&wfd);
	do
	{
		if((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			Ogre::String dir = wfd.cFileName;
			if (dir == "." || dir == "..") continue;
			Ogre::String result = "";
			result = ScanPath(path + "/" + Ogre::String(wfd.cFileName),  filename);
			if (result != "") return result;
		}
		if (Ogre::String(wfd.cFileName) == filename)
		{
			return path + "/" + Ogre::String(wfd.cFileName);
		}
	}
	while (FindNextFile(fHandle,&wfd));
	FindClose(fHandle);
	return "";
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
	if (mCurrentFile != "")
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
					if (oValue == "") oValue = "0";
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
	mCurrentFile = "";
}

void wxMaterialEditor::SetMaterialTemplate(Ogre::String Name, Ogre::String File)
{
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
								if (line.find(" float ") != Ogre::String::npos) mPropGrid->Append( new wxFloatProperty(wxT(prop.c_str()), wxT(prop.c_str())));
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

void wxMaterialEditor::EditMaterial(Ogre::MaterialPtr material)
{
	mCurrentMaterial = material;
	mCurrentFile = FindResource(mCurrentMaterial);
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
						if (p->GetName() == "diffuse_map") p->SetValue(wxVariant(wxT(line.substr(line.find("texture ") + 8, line.size()))));
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
				if (line.find(":") != Ogre::String::npos) IsCustom = true; 
				if (line.find("{") != Ogre::String::npos) bracket_counter++;
				if (line.find("}") != Ogre::String::npos) bracket_counter--;
				found = true;
			}
		}
	}
	f.close();
}