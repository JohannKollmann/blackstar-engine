
#include "propGridEditIceGOC.h"
#include "wxEdit.h"
#include "IceSceneManager.h"
#include "IceGOCView.h"
#include "Edit.h"
#include "wx/msw/winundef.h"
#include "IceObjectMessageIDs.h"
#include "IceComponentFactory.h"

 
/*void  
ComponentSectionVectorHandler ::Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName) 
{ 
	/*test what category the data is*//* 
	std::vector<int> dims; 
	std::vector< ComponentSection >* pVector=(std::vector< ComponentSection >*)pData; 
	dims.push_back(pVector->size()); 
 
	if(LoadSave::LoadSave::Instance().GetObjectID( "ComponentSection" )!=0) 
	{/*it is an object*/
		/*open an object-array*/ /*
		ss.OpenObjectArray( "ComponentSection" , dims, "_array"); 
		for(unsigned int i=0; i<pVector->size(); i++) 
			ss.AddObject((LoadSave::Saveable*)&((*pVector)[i])); 
	} 
	else 
	{
		IceAssert(false)
	} 
}  
 
void  
ComponentSectionVectorHandler ::Load(LoadSave::LoadSystem& ls, void* pDest) 
{ 
	std::vector<int> dims; 
	std::vector< ComponentSection >* pVector=(std::vector< ComponentSection >*)pDest; 
	if(LoadSave::LoadSave::Instance().GetObjectID( "ComponentSection" )!=0) 
	{/*it is an object*/ /*
		std::string str; 
		dims=ls.LoadObjectArray(&str); 
		if(str!="ComponentSection") 
			return; 
	} 
	else 
	{
		IceAssert(false)
	} 
	for(int i=0; i<dims[0]; i++) 
	{ 
		if(LoadSave::LoadSave::Instance().GetObjectID( "ComponentSection" )!=0) 
		{/*it is an object*/ /*
			pVector->push_back(*((ComponentSection*)ls.LoadArrayObject())); 
		} 
		else 
		{/*it must be an atom*/ /*
			IceAssert(false)
		} 
	} 
}*/


void wxEditGOCSections::OnLeave()
{
	//wxEdit::Instance().GetAuiManager().GetPane("Properties").Caption("Properties");
	wxAuiPaneInfo& pane = wxEdit::Instance().GetAuiManager().GetPane(("componentbar")).Show(false);
	wxEdit::Instance().GetAuiManager().Update();
}

void wxEditGOCSections::RemoveGOCSection(Ogre::String name)
{
	for (wxPropertyGridIterator it = mPropGrid->GetIterator(wxPG_ITERATE_CATEGORIES); !it.AtEnd(); it++)
	{
		wxPGProperty* p = *it;
		if (p->GetName() == name)
		{
			mPropGrid->DeleteProperty(p);
			return;
		}
	}
}

void wxEditGOCSections::AddGOCSection(Ogre::String name, Ice::DataMap &map, bool expand)
{
	AddDataMapSection(name, map, expand);
}

void wxEditGOCSections::OnActivate()
{
	mPropGrid->Clear();
}

void wxEditGOCSections::GetGOCSections(std::vector<ComponentSectionPtr> &sections)
{
		wxPropertyGridIterator it = mPropGrid->GetIterator(wxPG_ITERATE_ALL);
		if (it.AtEnd()) return;
		it++;
		Ice::DataMap transformdata;
		Ogre::String nextSectionname = SectionToDataMap(it, &transformdata);
		ComponentSectionPtr tcs = std::make_shared<ComponentSection>();
		tcs->mSectionName = "GameObject";
		tcs->mSectionData = transformdata;
		sections.push_back(tcs);
		while (nextSectionname != "")
		{
			it++;
			ComponentSectionPtr cs = std::make_shared<ComponentSection>();
			Ice::DataMap sd;
			cs->mSectionName = nextSectionname;
			nextSectionname = SectionToDataMap(it, &sd);
			cs->mSectionData = sd;
			sections.push_back(cs);
		}
}
bool wxEditGOCSections::OnDropText(const wxString& text)
{
	if (text == "ScriptDragged")
	{
		VdtcTreeItemBase *item = wxEdit::Instance().GetWorldExplorer()->GetScriptTree()->GetDraggedItem();
		if (item)
		{
			if (item->IsFile())
			{
				return true;
			}
		}
	}
	if (text == "MediaDragged")
	{
		VdtcTreeItemBase *item = wxEdit::Instance().GetWorldExplorer()->GetMediaTree()->GetDraggedItem();
		if (item)
		{
			if (item->IsFile())
			{
				if (item->GetName().find(".mesh") != wxString::npos)
				{
					bool found = false;
					wxPGProperty *currCat = 0;
					for (wxPropertyGridIterator it = mPropGrid->GetIterator(wxPG_ITERATE_ALL); !it.AtEnd(); it++)
					{
						wxPGProperty* p = *it;
						if (p->IsCategory())
						{
							currCat = p;
							continue;
						}
						Ogre::String sType = p->GetName().substr(0, p->GetName().find("|")).c_str();
						Ice::GenericProperty::PropertyTypes type = static_cast<Ice::GenericProperty::PropertyTypes>(Ogre::StringConverter::parseInt(sType));
						Ogre::String key = p->GetName().substr(p->GetName().find("|") + 1, p->GetName().find("--") - p->GetName().find("|") - 1).c_str();
						if (type == Ice::GenericProperty::PropertyTypes::STRING && key.find("Mesh") != Ogre::String::npos)
						{
							found = true;
							p->SetValueFromString(item->GetName().c_str());
							//if (currCat) currCat->SetExpanded(true);	TODO
						}
					}
					if (!found)
					{

						Ice::DataMap data;
						data.AddOgreString("MeshName", item->GetName().c_str().AsChar());
						data.AddBool("ShadowCaster", true);
						RemoveGOCSection("Mesh");
						AddGOCSection("Mesh", data, true);
						wxEdit::Instance().GetComponentBar()->SetSectionStatus("Mesh", true);
					}
					OnApply();
					wxEdit::Instance().GetAuiManager().Update();
					return true;
				}
			}
		}
	}
	return false;
}


wxEditIceGameObject::wxEditIceGameObject()
{
	mGameObject = std::weak_ptr<Ice::GameObject>();
}

Ice::GameObjectPtr wxEditIceGameObject::GetGameObject()
{
	return mGameObject.lock();
}

void wxEditIceGameObject::OnApply()
{
	STOP_MAINLOOP
	Ice::GameObjectPtr object = mGameObject.lock();
	IceAssert(object.get())
	std::vector<ComponentSectionPtr> sections;
	GetGOCSections(sections);
	bool select = false;
	if (wxEdit::Instance().GetOgrePane()->ObjectIsSelected(object))
	{
		select = true;
		wxEdit::Instance().GetOgrePane()->DeselectObject(object);
	}

	Ice::Msg msg;msg.typeID = Ice::ObjectMessageIDs::INTERN_RESET;
	object->BroadcastObjectMessage(msg, nullptr, true);

	std::list<Ice::GOCEditorInterface*> existingGOCs;
	for (auto ci = object->GetComponentIterator(); ci != object->GetComponentIteratorEnd(); ci++)
	{
		Ice::GOCEditorInterface* editor_interface = dynamic_cast<Ice::GOCEditorInterface*>((*ci).get());
		if (editor_interface)
			existingGOCs.push_back(editor_interface);
	}

	for (auto i = sections.begin(); i != sections.end(); i++)
	{
		if ((*i)->mSectionName == "GameObject")
		{
			object->SetName((*i)->mSectionData.GetOgreString("Name"));
			std::set<Ice::GameObject*> blacklist;
			object->SetGlobalPosition((*i)->mSectionData.GetOgreVec3("Position"), true, true, true, &blacklist);
			blacklist.clear();
			object->SetGlobalOrientation((*i)->mSectionData.GetOgreQuat("Orientation"), true, true, true, &blacklist);
			object->SetGlobalScale((*i)->mSectionData.GetOgreVec3("Scale"));
			continue;
		}     

		//Component data
		(*i)->mSectionData.AddOgreVec3("Scale", object->GetGlobalScale());	//HACK - Some components require scale value for initialisation (f.o. rigid bodies)

		Ice::GOCEditorInterface *proto = Ice::ComponentFactory::Instance().GetGOCPrototype((*i)->mSectionName);
		bool found = false;
		for (auto ie = existingGOCs.begin(); ie != existingGOCs.end(); ie++)
		{
			if ((*ie)->GetGOComponent()->GetComponentID() == proto->GetGOComponent()->GetComponentID())
			{
				(*ie)->SetParameters(&(*i)->mSectionData);
				existingGOCs.erase(ie);
				found = true;
				break;
			}
		}
		if (!found)
		{
			Ice::GOCEditorInterface *editor_interface = Ice::ComponentFactory::Instance().CreateGOC((*i)->mSectionName);
			editor_interface->SetParameters(&(*i)->mSectionData);
			object->RemoveComponent(editor_interface->GetGOComponent()->GetFamilyID());
			object->AddComponent(Ice::GOComponentPtr(editor_interface->GetGOComponent()));
		}
	}

	for (auto i = existingGOCs.begin(); i != existingGOCs.end(); i++)
		object->RemoveComponent((*i)->GetGOComponent()->GetFamilyID());

	object->SetGlobalPosition(object->GetGlobalPosition());
	object->SetGlobalOrientation(object->GetGlobalOrientation());
	object->SetGlobalScale(object->GetGlobalScale());

	object->FirePostInit();

	if (select == true)
	{
		wxEdit::Instance().GetOgrePane()->SelectObject(object);
	}
	wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->UpdateObject(object);
	wxEdit::Instance().GetOgrePane()->SetFocus();

	RESUME_MAINLOOP
}

void wxEditGOResource::OnApply()
{
	std::vector<ComponentSectionPtr> sections;
	GetGOCSections(sections);

	LoadSave::SaveSystem *ss=LoadSave::LoadSave::Instance().CreateSaveFile(mCurrentEditPath, mCurrentEditPath + ".xml");
	ss->SaveAtom("vector<ComponentSectionPtr>", &sections, "Sections");
	ss->CloseFiles();
	delete ss;

	wxFileName path = wxFileName(wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetCurrentPath().c_str());//GetRelativePath(wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelection());
	wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->SetRootPath("Data/Editor/Objects/");
	Ice::Log::Instance().LogMessage("Filename: " + Ogre::String(path.GetPath().c_str()));
	wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->ExpandToPath(path);
}

void wxEditIceGameObject::SetObject(Ice::GameObjectPtr object, bool update_ui)
{
	mPropGrid->Clear();
	mGameObject = std::weak_ptr<Ice::GameObject>(object);
	if (!object.get()) return;
	std::vector<ComponentSection> sections;
	Ice::DataMap transform_data;
	transform_data.AddOgreString("Name", object->GetName());
	transform_data.AddOgreVec3("Position", object->GetGlobalPosition());
	transform_data.AddOgreQuat("Orientation", object->GetGlobalOrientation());
	transform_data.AddOgreVec3("Scale", object->GetGlobalScale());
	AddGOCSection("GameObject", transform_data);
	for (auto i = object->GetComponentIterator(); i != object->GetComponentIteratorEnd(); i++)
	{
		Ice::GOCEditorInterface *editor_interface = dynamic_cast<Ice::GOCEditorInterface*>((*i).get());
		if (editor_interface)
		{
			Ice::DataMap data;
			editor_interface->GetParameters(&data);
			AddGOCSection(editor_interface->GetLabel(), data);
			ComponentSection cs;
			cs.mSectionName = editor_interface->GetLabel();
			sections.push_back(cs);
		}
	}
	if (update_ui)
	{
		wxAuiPaneInfo& pane = wxEdit::Instance().GetAuiManager().GetPane(("componentbar")).Show();
		wxEdit::Instance().GetComponentBar()->SetSections(sections);
		wxEdit::Instance().GetAuiManager().Update();
	}
}

void wxEditIceGameObject::OnUpdate()
{
	Ice::GameObjectPtr object = mGameObject.lock();
	if (object.get())
	{
		SetObject(object, false);
		wxEdit::Instance().GetpropertyWindow()->Refresh();
	}
}

void wxEditGOResource::SetResource(Ogre::String savepath)
{
	//wxEdit::Instance().GetAuiManager().GetPane("Properties").Caption("Properties");
	mPropGrid->Clear();
	mCurrentEditPath = savepath;
	LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile(mCurrentEditPath);
	std::vector<ComponentSectionPtr> sections;
	ls->LoadAtom("vector<ComponentSectionPtr>", &sections);
	for (auto i = sections.begin(); i != sections.end(); i++)
	{
		AddGOCSection((*i)->mSectionName, (*i)->mSectionData);
	}
	ls->CloseFile();
	delete ls;
	wxAuiPaneInfo& pane = wxEdit::Instance().GetAuiManager().GetPane(("componentbar")).Show();
	wxEdit::Instance().GetComponentBar()->SetSections(sections);
	wxEdit::Instance().GetAuiManager().Update();
}

void wxEditGOResource::NewResource(Ogre::String savepath, bool showcomponentbar)
{
	//wxEdit::Instance().GetAuiManager().GetPane("Properties").Caption("New Object Resource");
	mCurrentEditPath = savepath;
	Ice::DataMap data;
	data.AddOgreString("Name", "");
	data.AddOgreVec3("Position", Ogre::Vector3(0,0,0));
	data.AddOgreQuat("Orientation", Ogre::Quaternion());
	data.AddOgreVec3("Scale", Ogre::Vector3(1,1,1));
	AddGOCSection("Object", data);
	if (showcomponentbar)
	{
		wxAuiPaneInfo& pane = wxEdit::Instance().GetAuiManager().GetPane(("componentbar")).Show();
		wxEdit::Instance().GetComponentBar()->ResetCheckBoxes();
	}
	wxEdit::Instance().GetAuiManager().Update();
}