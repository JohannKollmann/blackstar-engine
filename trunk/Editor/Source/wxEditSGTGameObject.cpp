
#include "wxEditSGTGameObject.h"
#include <wx/propgrid/manager.h>
// Needed for implementing custom properties.
#include <wx/propgrid/propdev.h>

// Extra property classes.
#include <wx/propgrid/advprops.h>

#include "SGTLoadSave.h"

#include "SGTSceneManager.h"
#include "SGTGOCEditorInterface.h"


enum
{
	edtRes_Apply,
	edtRes_Cancel,
};


Ogre::String wxEditSGTDataMap::SectionToDataMap(wxPropertyGridIterator &it, SGTDataMap *data)
{
	Ogre::Vector3 vec;
	Ogre::Degree yDeg, pDeg, rDeg; 
	int counter = 0;
	for (; !it.AtEnd(); it++)
	{
		wxPGProperty* p = *it;
		if (p->IsCategory() && p->GetName().find("|") == wxString::npos) return Ogre::String(p->GetName().c_str());
		Ogre::String type = p->GetName().substr(0, p->GetName().find("|")).c_str();
		Ogre::String key = p->GetName().substr(p->GetName().find("|") + 1, p->GetName().find("--") - p->GetName().find("|") - 1).c_str();
		if (type == "int")
		{
			if (!p->GetValue().IsNull()) data->AddInt(key, p->GetValue().GetInteger());
			else data->AddInt(key, 0);
		}
		if (type == "float")
		{
			if (!p->GetValue().IsNull()) data->AddFloat(key, p->GetValue().GetReal());
			else data->AddFloat(key, 0.0f);
		}
		if (type == "bool")
		{
			if (!p->GetValue().IsNull()) data->AddBool(key, p->GetValue().GetBool());
			else data->AddBool(key, false);
		}
		if (type == "Ogre::String")
		{
			data->AddOgreString(key, Ogre::String(p->GetValue().GetString().c_str()));
		}
		if (type == "Ogre::Vector3")
		{
			if (counter == 3)
			{
				if (!p->GetValue().IsNull())
					vec.z = p->GetValue().GetReal();
				else vec.z = 0;
				data->AddOgreVec3(key, vec);
				vec = Ogre::Vector3();
				counter = 0;
				continue;
			}
			if (counter == 2)
			{
				if (!p->GetValue().IsNull())
					vec.y = p->GetValue().GetReal();
				else vec.y = 0;
				counter++;
				continue;
			}
			if (counter == 1)
			{
				if (!p->GetValue().IsNull())
					vec.x = p->GetValue().GetReal();
				else vec.x = 0;
				counter++;
				continue;
			}
			if (counter == 0)
			{
				counter++;
				continue;
			}
		}
		if (type == "Ogre::Quaternion")
		{
			if (counter == 3)
			{
				if (!p->GetValue().IsNull()) rDeg = Ogre::Degree(p->GetValue().GetReal());
				Ogre::Matrix3 mat3;
				mat3.FromEulerAnglesYXZ(yDeg, pDeg, rDeg);
				Ogre::Quaternion q;
				q.FromRotationMatrix(mat3);
				data->AddOgreQuat(key, q);
				counter = 0;
				continue;
			}
			if (counter == 2)
			{
				if (!p->GetValue().IsNull()) yDeg = Ogre::Degree(p->GetValue().GetReal());
				counter++;
				continue;
			}
			if (counter == 1)
			{
				if (!p->GetValue().IsNull()) pDeg = Ogre::Degree(p->GetValue().GetReal());
				counter++;
				continue;
			}
			if (counter == 0)
			{
				counter++;
				continue;
			}
		}
	}
	return "";
}

void wxEditSGTDataMap::AddDataMapSection(Ogre::String name, SGTDataMap &map)
{
	wxPGProperty* csprop = mPropGrid->Append( new wxPropertyCategory(wxT(name.c_str()), wxT(name.c_str())));
	while (map.HasNext())
	{
		SGTGenericProperty entry = map.GetNext();
		Ogre::String key = entry.mKey;
		Ogre::String internname = entry.mType + "|" + key + "--" + name;
		if (entry.mType == "int") mPropGrid->AppendIn(csprop, new wxIntProperty(wxT(key.c_str()), wxT(internname.c_str()), Ogre::any_cast<int>(entry.mData) ));
		if (entry.mType == "float") mPropGrid->AppendIn(csprop, new wxFloatProperty(wxT(key.c_str()), wxT(internname.c_str()), Ogre::any_cast<float>(entry.mData) ));
		if (entry.mType == "bool") mPropGrid->AppendIn(csprop, new wxBoolProperty(wxT(key.c_str()), wxT(internname.c_str()), Ogre::any_cast<bool>(entry.mData) ));
		if (entry.mType == "Ogre::Vector3")
		{
			Ogre::Vector3 vec = Ogre::any_cast<Ogre::Vector3>(entry.mData);
			wxPGProperty* vecprop = mPropGrid->AppendIn(csprop, new wxStringProperty(wxT(key.c_str()), wxT(internname.c_str())));
			mPropGrid->AppendIn(vecprop, new wxFloatProperty(wxT("X"), wxT((internname + "__X").c_str()), vec.x) );
			mPropGrid->AppendIn(vecprop, new wxFloatProperty(wxT("Y"), wxT((internname + "__Y").c_str()), vec.y) );
			mPropGrid->AppendIn(vecprop, new wxFloatProperty(wxT("Z"), wxT((internname + "__Z").c_str()), vec.z) );
			vecprop->SetExpanded(false);
		}
		if (entry.mType == "Ogre::Quaternion")
		{
			Ogre::Quaternion quat = Ogre::any_cast<Ogre::Quaternion>(entry.mData);
			Ogre::Matrix3 mat3;
			quat.ToRotationMatrix(mat3);
			Ogre::Radian yRad, pRad, rRad;
			mat3.ToEulerAnglesYXZ(yRad, pRad, rRad);
			wxPGProperty* quatprop = mPropGrid->AppendIn(csprop, new wxStringProperty(wxT(key.c_str()), wxT(internname.c_str())));
			mPropGrid->AppendIn(quatprop, new wxFloatProperty(wxT("Pitch"), wxT((internname + "__Pitch").c_str()), pRad.valueDegrees()) );
			mPropGrid->AppendIn(quatprop, new wxFloatProperty(wxT("Yaw"), wxT((internname + "__Yaw").c_str()), yRad.valueDegrees()) );
			mPropGrid->AppendIn(quatprop, new wxFloatProperty(wxT("Roll"), wxT((internname + "__Roll").c_str()), rRad.valueDegrees()) );
			quatprop->SetExpanded(false);
		}
		if (entry.mType == "Ogre::String")
		{
			Ogre::String str = Ogre::any_cast<Ogre::String>(entry.mData);
			mPropGrid->AppendIn(csprop, new wxStringProperty(wxT(key.c_str()), wxT(internname.c_str()), wxT(str.c_str())) );
		}
	}
	csprop->SetExpanded(false);
	mPropGrid->Refresh();
}



wxEditSGTGameObject::wxEditSGTGameObject()
{
	mGameObject = 0;
	mFirstTimeOverideHack = true;
}

void wxEditSGTGameObject::OnLeave()
{
	wxAuiPaneInfo& pane = wxEdit::Instance().GetAuiManager().GetPane(wxT("componentbar")).Show(false);
	wxEdit::Instance().GetAuiManager().Update();
}

SGTGameObject* wxEditSGTGameObject::GetGameObject()
{
	return mGameObject;
}

void wxEditSGTGameObject::RemoveGOCSection(Ogre::String name)
{
	for (wxPropertyGridIterator it = mPropGrid->GetIterator(wxPG_ITERATE_CATEGORIES); !it.AtEnd(); it++)
	{
		wxPGProperty* p = *it;
		if (p->GetName() == name)
		{
			mPropGrid->Delete(p->GetId());
			return;
		}
	}
}

void wxEditSGTGameObject::AddGOCSection(Ogre::String name, SGTDataMap &map)
{
	AddDataMapSection(name, map);
}

Ogre::String wxEditSGTGameObject::SectionToDataMap(wxPropertyGridIterator &it, SGTDataMap *data)
{
	Ogre::Vector3 vec;
	Ogre::Degree yDeg, pDeg, rDeg; 
	int counter = 0;
	for (; !it.AtEnd(); it++)
	{
		wxPGProperty* p = *it;
		if (p->IsCategory() && p->GetName().find("|") == wxString::npos) return Ogre::String(p->GetName().c_str());
		Ogre::String type = p->GetName().substr(0, p->GetName().find("|")).c_str();
		Ogre::String key = p->GetName().substr(p->GetName().find("|") + 1, p->GetName().find("--") - p->GetName().find("|") - 1).c_str();
		if (type == "int")
		{
			if (!p->GetValue().IsNull()) data->AddInt(key, p->GetValue().GetInteger());
			else data->AddInt(key, 0);
		}
		if (type == "float")
		{
			if (!p->GetValue().IsNull()) data->AddFloat(key, p->GetValue().GetReal());
			else data->AddFloat(key, 0.0f);
		}
		if (type == "bool")
		{
			if (!p->GetValue().IsNull()) data->AddBool(key, p->GetValue().GetBool());
			else data->AddBool(key, false);
		}
		if (type == "Ogre::String")
		{
			if (mCurrentEditPath != "DoNotSave" && key == "Name") data->AddOgreString(key, "Default");
			else data->AddOgreString(key, Ogre::String(p->GetValue().GetString().c_str()));
		}
		if (type == "Ogre::Vector3")
		{
			if (counter == 3)
			{
				if (!p->GetValue().IsNull())
					vec.z = p->GetValue().GetReal();
				else vec.z = 0;
				data->AddOgreVec3(key, vec);
				vec = Ogre::Vector3();
				counter = 0;
				continue;
			}
			if (counter == 2)
			{
				if (!p->GetValue().IsNull())
					vec.y = p->GetValue().GetReal();
				else vec.y = 0;
				counter++;
				continue;
			}
			if (counter == 1)
			{
				if (!p->GetValue().IsNull())
					vec.x = p->GetValue().GetReal();
				else vec.x = 0;
				counter++;
				continue;
			}
			if (counter == 0)
			{
				counter++;
				continue;
			}
		}
		if (type == "Ogre::Quaternion")
		{
			if (counter == 3)
			{
				if (!p->GetValue().IsNull()) rDeg = Ogre::Degree(p->GetValue().GetReal());
				Ogre::Matrix3 mat3;
				mat3.FromEulerAnglesYXZ(yDeg, pDeg, rDeg);
				Ogre::Quaternion q;
				q.FromRotationMatrix(mat3);
				data->AddOgreQuat(key, q);
				counter = 0;
				continue;
			}
			if (counter == 2)
			{
				if (!p->GetValue().IsNull()) yDeg = Ogre::Degree(p->GetValue().GetReal());
				counter++;
				continue;
			}
			if (counter == 1)
			{
				if (!p->GetValue().IsNull()) pDeg = Ogre::Degree(p->GetValue().GetReal());
				counter++;
				continue;
			}
			if (counter == 0)
			{
				counter++;
				continue;
			}
		}
	}
	return "";
}

void wxEditSGTGameObject::OnActivate()
{
	if (!mFirstTimeOverideHack)
	{
		wxEdit::Instance().GetComponentBar()->ResetCheckBoxes();
		mFirstTimeOverideHack = false;
	}
	mPropGrid->Clear();
}

void wxEditSGTGameObject::OnApply()
{
	if (mPropGrid->GetChildrenCount() > 0)
	{
		wxPropertyGridIterator it = mPropGrid->GetIterator(wxPG_ITERATE_ALL);
		it++;
		SGTDataMap *transformdata = new SGTDataMap;
		Ogre::String sectionname = SectionToDataMap(it, transformdata);
		Ogre::String new_sectionname = "";
		std::list<ComponentSection> sections;
		ComponentSection tcs;
		tcs.mSectionName = "Transform";
		tcs.mSectionData = Ogre::SharedPtr<SGTDataMap>(transformdata);
		sections.push_back(tcs);
		while (sectionname != "")
		{
			it++;
			ComponentSection cs;
			SGTDataMap *sd = new SGTDataMap();
			cs.mSectionName = sectionname;
			sectionname = SectionToDataMap(it, sd);
			cs.mSectionData = Ogre::SharedPtr<SGTDataMap>(sd);
			sections.push_back(cs);
		}

		if (mCurrentEditPath == "DoNotSave")
		{
			bool select = false;
			if (wxEdit::Instance().GetOgrePane()->mEdit->ObjectIsSelected(mGameObject))
			{
				select = true;
				wxEdit::Instance().GetOgrePane()->mEdit->DeselectObject(mGameObject);
			}
			for (std::list<ComponentSection>::iterator i = sections.begin(); i != sections.end(); i++)
			{
				if ((*i).mSectionName == "Transform")
				{
					mGameObject->SetGlobalPosition((*i).mSectionData->GetOgreVec3("Position"));
					mGameObject->SetGlobalOrientation((*i).mSectionData->GetOgreQuat("Orientation"));
					mGameObject->SetGlobalScale((*i).mSectionData->GetOgreVec3("Scale"));
					continue;
				}        
				(*i).mSectionData->AddOgreVec3("Scale", mGameObject->GetGlobalScale());	//HACK - Some components require scale value for initialisation (f.o. rigid bodies)
				SGTGOCEditorInterface *editor_interface = SGTSceneManager::Instance().CreateComponent((*i).mSectionName, (*i).mSectionData.getPointer());
				if (editor_interface->IsViewComponent())
				{
					SGTGOCViewContainer *container = dynamic_cast<SGTGOCViewContainer*>(mGameObject->GetComponent("GOCView"));
					if (!container)
					{
						container = new SGTGOCViewContainer();
						mGameObject->AddComponent(container);
					}
					SGTGOCViewComponent *viewcomponent = dynamic_cast<SGTGOCViewComponent*>(editor_interface);
					Ogre::String gocTypeName = viewcomponent->GetTypeName();
					container->RemoveItem(gocTypeName);
					container->AddItem(viewcomponent);
				}
				else
				{
					SGTGOComponent* component = dynamic_cast<SGTGOComponent*>(editor_interface);
					Ogre::String familyID = component->GetFamilyID();
					SGTGOCEditorInterface *oldinterface = dynamic_cast<SGTGOCEditorInterface*>(mGameObject->GetComponent(familyID));
					void *userdata = oldinterface->GetUserData();
					mGameObject->RemoveComponent(familyID);
					mGameObject->AddComponent(component);
					editor_interface->InjectUserData(userdata);
				}
			}
			if (select == true)
			{
				wxEdit::Instance().GetOgrePane()->mEdit->SelectObject(mGameObject);
			}
			wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
			wxEdit::Instance().GetOgrePane()->SetFocus();
		}
		else
		{
			SGTSaveSystem *ss=SGTLoadSave::Instance().CreateSaveFile(mCurrentEditPath, mCurrentEditPath + ".xml");
			ss->SaveAtom("std::list<ComponentSection>", &sections, "Sections");
			ss->CloseFiles();
			delete ss;

			wxFileName path = wxFileName(wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetCurrentPath().c_str());//GetRelativePath(wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelection());
			wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->SetRootPath("Data/Editor/Objects/");
			Ogre::LogManager::getSingleton().logMessage("Filename: " + Ogre::String(path.GetPath().c_str()));
			wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->ExpandToPath(path);//path);
		}
	}
}

void wxEditSGTGameObject::SetObject(SGTGameObject *object, bool update_ui)
{
	mPropGrid->Clear();
	if (!object) return;
	mCurrentEditPath = "DoNotSave";
	mGameObject = object;
	std::list<ComponentSection> sections;
	SGTDataMap transform_data;
	transform_data.AddOgreVec3("Position", mGameObject->GetGlobalPosition());
	transform_data.AddOgreQuat("Orientation", mGameObject->GetGlobalOrientation());
	transform_data.AddOgreVec3("Scale", mGameObject->GetGlobalScale());
	AddGOCSection("Transform", transform_data);
	for (std::list<SGTGOComponent*>::iterator i = mGameObject->GetComponentIterator(); i != mGameObject->GetComponentIteratorEnd(); i++)
	{
		if ((*i)->GetComponentID() == "GOCViewContainer")
		{
			SGTGOCViewContainer *container = (SGTGOCViewContainer*)(*i);
			for (std::list<SGTGOCViewComponent*>::iterator x = container->GetItemIterator(); x != container->GetItemIteratorEnd(); x++)
			{
				SGTGOCEditorInterface *editor_interface = dynamic_cast<SGTGOCEditorInterface*>((*x));
				if (editor_interface)
				{
					SGTDataMap data;
					editor_interface->GetParameters(&data);
					AddGOCSection((*x)->GetTypeName().c_str(), data);
					ComponentSection cs;
					cs.mSectionName = (*x)->GetTypeName().c_str();
					sections.push_back(cs);
				}
			}
			continue;
		}
		SGTGOCEditorInterface *editor_interface = dynamic_cast<SGTGOCEditorInterface*>((*i));
		if (editor_interface)
		{
			SGTDataMap data;
			editor_interface->GetParameters(&data);
			AddGOCSection((*i)->GetComponentID(), data);
			ComponentSection cs;
			cs.mSectionName = (*i)->GetComponentID();
			sections.push_back(cs);
		}
	}
	if (update_ui)
	{
		wxAuiPaneInfo& pane = wxEdit::Instance().GetAuiManager().GetPane(wxT("componentbar")).Show();
		wxEdit::Instance().GetComponentBar()->SetSections(sections);
		wxEdit::Instance().GetAuiManager().Update();
	}
}

void wxEditSGTGameObject::OnUpdate()
{
	if (mGameObject != 0 && mCurrentEditPath == "DoNotSave")
	{
		SetObject(mGameObject, false);
		wxEdit::Instance().GetpropertyWindow()->Refresh();
	}
}

void wxEditSGTGameObject::SetResource(Ogre::String savepath)
{
	mPropGrid->Clear();
	mGameObject = 0;
	mCurrentEditPath = savepath;
	SGTLoadSystem *ls=SGTLoadSave::Instance().LoadFile(mCurrentEditPath);
	std::list<ComponentSection> sections;
	ls->LoadAtom("std::list<ComponentSection>", &sections);
	for (std::list<ComponentSection>::iterator i = sections.begin(); i != sections.end(); i++)
	{
		AddGOCSection((*i).mSectionName, *(*i).mSectionData);
	}
	ls->CloseFile();
	delete ls;
	wxAuiPaneInfo& pane = wxEdit::Instance().GetAuiManager().GetPane(wxT("componentbar")).Show();
	wxEdit::Instance().GetComponentBar()->SetSections(sections);
	wxEdit::Instance().GetAuiManager().Update();
}

void wxEditSGTGameObject::NewResource(Ogre::String savepath, bool showcomponentbar)
{
	mCurrentEditPath = savepath;
	SGTDataMap data;
	data.AddOgreVec3("Position", Ogre::Vector3(0,0,0));
	data.AddOgreQuat("Orientation", Ogre::Quaternion());
	data.AddOgreVec3("Scale", Ogre::Vector3(1,1,1));
	AddGOCSection("Transform", data);
	if (showcomponentbar)
	{
		wxAuiPaneInfo& pane = wxEdit::Instance().GetAuiManager().GetPane(wxT("componentbar")).Show();
		wxEdit::Instance().GetComponentBar()->ResetCheckBoxes();
	}
	wxEdit::Instance().GetAuiManager().Update();
}




void wxEditSGTSceneParams::OnActivate()
{
	mPropGrid->Clear();
	SGTDataMap map;
	SGTSceneManager::Instance().GetParameters(&map);
	AddDataMapSection("Scene params", map);
}

void wxEditSGTSceneParams::OnApply()
{
	wxPropertyGridIterator it = mPropGrid->GetIterator(wxPG_ITERATE_ALL);
	it++;
	SGTDataMap parameters;
	Ogre::String sectionname = SectionToDataMap(it, &parameters);
	SGTSceneManager::Instance().CreateFromDataMap(&parameters);
}