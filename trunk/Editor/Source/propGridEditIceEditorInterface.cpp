
#include "propGridEditIceEditorInterface.h"
#include "IceSceneManager.h"
#include "IceGOCEditorInterface.h"


Ogre::String wxEditIceDataMap::SectionToDataMap(wxPropertyGridIterator &it, Ice::DataMap *data)
{
	Ogre::Vector3 vec;
	Ogre::Degree yDeg, pDeg, rDeg; 
	int counter = 0;
	for (; !it.AtEnd(); it++)
	{
		wxPGProperty* p = *it;
		if (p->IsCategory() && p->GetName().find("|") == wxString::npos) return Ogre::String(p->GetName().c_str());
		Ogre::String sType = p->GetName().substr(0, p->GetName().find("|")).c_str();
		Ice::GenericProperty::PropertyTypes prop_type = static_cast<Ice::GenericProperty::PropertyTypes>(Ogre::StringConverter::parseInt(sType));
		Ogre::String key = p->GetName().substr(p->GetName().find("|") + 1, p->GetName().find("--") - p->GetName().find("|") - 1).c_str();
		if (prop_type == Ice::GenericProperty::PropertyTypes::INT)
		{
			if (!p->GetValue().IsNull()) data->AddInt(key, p->GetValue().GetInteger());
			else data->AddInt(key, 0);
		}
		if (prop_type == Ice::GenericProperty::PropertyTypes::FLOAT)
		{
			if (!p->GetValue().IsNull()) data->AddFloat(key, p->GetValue().GetReal());
			else data->AddFloat(key, 0.0f);
		}
		if (prop_type == Ice::GenericProperty::PropertyTypes::BOOL)
		{
			if (!p->GetValue().IsNull()) data->AddBool(key, p->GetValue().GetBool());
			else data->AddBool(key, false);
		}
		if (prop_type == Ice::GenericProperty::PropertyTypes::STRING)
		{
			data->AddOgreString(key, Ogre::String(p->GetValue().GetString().c_str()));
		}
		if (prop_type == Ice::GenericProperty::PropertyTypes::VECTOR3)
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
		if (prop_type == Ice::GenericProperty::PropertyTypes::QUATERNION)
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

void wxEditIceDataMap::AddDataMapSection(Ogre::String name, Ice::DataMap &map, bool expand)
{
	wxPGProperty* csprop = mPropGrid->Append( new wxPropertyCategory(wxT(name.c_str()), wxT(name.c_str())));
	while (map.HasNext())
	{
		Ice::DataMap::Item entry = map.GetNext();
		Ogre::String key = entry.key;
		Ice::GenericProperty::PropertyTypes prop_type = entry.data.getType();
		Ogre::String internname = Ogre::StringConverter::toString(static_cast<int>(prop_type)) + "|" + key + "--" + name;
		if (prop_type == Ice::GenericProperty::PropertyTypes::INT) mPropGrid->AppendIn(csprop, new wxIntProperty(wxT(key.c_str()), wxT(internname.c_str()), entry.data.Get<int>() ));
		if (prop_type == Ice::GenericProperty::PropertyTypes::FLOAT) mPropGrid->AppendIn(csprop, new wxFloatProperty(wxT(key.c_str()), wxT(internname.c_str()), entry.data.Get<float>() ));
		if (prop_type == Ice::GenericProperty::PropertyTypes::BOOL) mPropGrid->AppendIn(csprop, new wxBoolProperty(wxT(key.c_str()), wxT(internname.c_str()), entry.data.Get<bool>() ));
		if (prop_type == Ice::GenericProperty::PropertyTypes::VECTOR3)
		{
			Ogre::Vector3 vec = entry.data.Get<Ogre::Vector3>();
			wxPGProperty* vecprop = mPropGrid->AppendIn(csprop, new wxStringProperty(wxT(key.c_str()), wxT(internname.c_str())));
			mPropGrid->AppendIn(vecprop, new wxFloatProperty(wxT("X"), wxT((internname + "__X").c_str()), vec.x) );
			mPropGrid->AppendIn(vecprop, new wxFloatProperty(wxT("Y"), wxT((internname + "__Y").c_str()), vec.y) );
			mPropGrid->AppendIn(vecprop, new wxFloatProperty(wxT("Z"), wxT((internname + "__Z").c_str()), vec.z) );
			vecprop->SetExpanded(false);
		}
		if (prop_type == Ice::GenericProperty::PropertyTypes::QUATERNION)
		{
			Ogre::Quaternion quat = entry.data.Get<Ogre::Quaternion>();
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
		if (prop_type == Ice::GenericProperty::PropertyTypes::STRING)
		{
			Ogre::String str = entry.data.Get<Ogre::String>();
			if (Ice::DataMap::Enum::isEnum(str))
			{
				Ice::DataMap::Enum enu; enu.fromString(str);
				wxPGChoices choices;
				for (int choiceIter = 0; choiceIter < enu.choices.size(); choiceIter++)
					choices.Add(wxT(enu.choices[choiceIter].c_str()), choiceIter);
				new wxEnumProperty(wxT(key.c_str()), wxT(internname.c_str()), choices);
			}
			else mPropGrid->AppendIn(csprop, new wxStringProperty(wxT(key.c_str()), wxT(internname.c_str()), wxT(str.c_str())) );
		}
	}
	csprop->SetExpanded(expand);
	mPropGrid->Refresh();
}

void wxEditIceSceneParams::OnActivate()
{
	mPropGrid->Clear();
	Ice::DataMap map;
	Ice::SceneManager::Instance().GetParameters(&map);
	AddDataMapSection("Scene params", map);
}

void wxEditIceSceneParams::OnApply()
{
	wxPropertyGridIterator it = mPropGrid->GetIterator(wxPG_ITERATE_ALL);
	it++;
	Ice::DataMap parameters;
	Ogre::String sectionname = SectionToDataMap(it, &parameters);
	Ice::SceneManager::Instance().SetParameters(&parameters);
}