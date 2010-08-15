
#include "IceDataMap.h"


namespace Ice
{

	GenericProperty::PropertyTypes GenericProperty::getType() const
	{
		if (mData.getType() == typeid(bool)) return PropertyTypes::BOOL;
		if (mData.getType() == typeid(float)) return PropertyTypes::FLOAT;
		if (mData.getType() == typeid(int)) return PropertyTypes::INT;
		if (mData.getType() == typeid(Ogre::String)) return PropertyTypes::STRING;
		if (mData.getType() == typeid(Ogre::Vector3)) return PropertyTypes::VECTOR3;
		if (mData.getType() == typeid(Ogre::Quaternion)) return PropertyTypes::QUATERNION;
		if (mData.getType() == typeid(DataMap::Enum)) return PropertyTypes::ENUM;
		
		IceAssert(false);
		return PropertyTypes::INT;
	}
	GenericProperty::PropertyTypes GenericProperty::Get(void *target) const
	{
		if (mData.getType() == typeid(bool)) { bool src = Ogre::any_cast<bool>(mData); memcpy(target, &src, sizeof(bool)); return PropertyTypes::BOOL; }
		if (mData.getType() == typeid(float)) { float src = Ogre::any_cast<float>(mData); *((float*)target) = src; return PropertyTypes::FLOAT; }
		if (mData.getType() == typeid(int)) { int src = Ogre::any_cast<int>(mData); *((int*)target) = src; return PropertyTypes::INT; }
		if (mData.getType() == typeid(Ogre::String)) { Ogre::String src = Ogre::any_cast<Ogre::String>(mData); *((Ogre::String*)target) = src; return PropertyTypes::STRING; }
		if (mData.getType() == typeid(Ogre::Vector3)) { Ogre::Vector3 src = Ogre::any_cast<Ogre::Vector3>(mData); *((Ogre::Vector3*)target) = src;  return PropertyTypes::VECTOR3; }
		if (mData.getType() == typeid(Ogre::Quaternion)) { Ogre::Quaternion src = Ogre::any_cast<Ogre::Quaternion>(mData); *((Ogre::Quaternion*)target) = src; return PropertyTypes::QUATERNION; }
		if (mData.getType() == typeid(DataMap::Enum)) { DataMap::Enum src = Ogre::any_cast<DataMap::Enum>(mData); *((DataMap::Enum*)target) = src; return PropertyTypes::ENUM; }
		
		IceAssert(false);
		return PropertyTypes::INT;
	}

	void GenericProperty::Set(void *data, const GenericProperty::PropertyTypes &type)
	{
		switch (type)
		{
		case PropertyTypes::INT:
			Set<int>(*((int*)data)); break;
		case PropertyTypes::BOOL:
			Set<bool>(*((bool*)data)); break;
		case PropertyTypes::FLOAT:
			Set<float>(*((float*)data)); break;
		case PropertyTypes::STRING:
			Set<Ogre::String>(*((Ogre::String*)data)); break;
		case PropertyTypes::VECTOR3:
			Set<Ogre::Vector3>(*((Ogre::Vector3*)data)); break;
		case PropertyTypes::QUATERNION:
			Set<Ogre::Quaternion>(*((Ogre::Quaternion*)data)); break;
		case PropertyTypes::ENUM:
			Set<DataMap::Enum>(*((DataMap::Enum*)data)); break;
		}
	}

	void GenericProperty::Save(LoadSave::SaveSystem& myManager)
	{
		int type = getType();
		myManager.SaveAtom("int", &type, "Type");

		int iData; bool bData; float fData; Ogre::String sData; Ogre::Vector3 vData; Ogre::Quaternion qData; DataMap::Enum eData;

		switch (type)
		{
		case PropertyTypes::INT:
			iData = Ogre::any_cast<int>(mData);
			myManager.SaveAtom("int", (void*)&iData, "Data");
			break;
		case PropertyTypes::BOOL:
			bData = Ogre::any_cast<bool>(mData);
			myManager.SaveAtom("bool", (void*)&bData, "Data");
			break;
		case PropertyTypes::FLOAT:
			fData = Ogre::any_cast<float>(mData);
			myManager.SaveAtom("float", (void*)&fData, "Data");
			break;
		case PropertyTypes::STRING:
			sData = Ogre::any_cast<Ogre::String>(mData);
			myManager.SaveAtom("Ogre::String", (void*)&sData, "Data");
			break;
		case PropertyTypes::VECTOR3:
			vData = Ogre::any_cast<Ogre::Vector3>(mData);
			myManager.SaveAtom("Ogre::Vector3", (void*)&vData, "Data");
			break;
		case PropertyTypes::QUATERNION:
			qData = Ogre::any_cast<Ogre::Quaternion>(mData);
			myManager.SaveAtom("Ogre::Quaternion", (void*)&qData, "Data");
			break;
		case PropertyTypes::ENUM:
			eData = Ogre::any_cast<DataMap::Enum>(mData);
			myManager.SaveAtom("Ogre::String", (void*)&eData.toString(), "Data");
			break;
		}
	}

	void GenericProperty::Load(LoadSave::LoadSystem& mgr)
	{
		int type = 0;
		mgr.LoadAtom("int", &type);
		int iVal = 0;
		bool bVal = false;
		float fVal = 0.0f;
		Ogre::String sVal;
		Ogre::Vector3 vVal;
		Ogre::Quaternion qVal;
		DataMap::Enum eData;
		switch (type)
		{
		case PropertyTypes::INT:
			mgr.LoadAtom("int", &iVal);
			mData = iVal;
			break;
		case PropertyTypes::BOOL:
			mgr.LoadAtom("bool", &bVal);
			mData = bVal;
			break;
		case PropertyTypes::FLOAT:
			mgr.LoadAtom("float", &fVal);
			mData = fVal;
			break;
		case PropertyTypes::STRING:
			mgr.LoadAtom("Ogre::String", &sVal);
			mData = sVal;
			break;
		case PropertyTypes::VECTOR3:
			mgr.LoadAtom("Ogre::Vector3", &vVal);
			mData = vVal;
			break;
		case PropertyTypes::QUATERNION:
			mgr.LoadAtom("Ogre::Quaternion", &qVal);
			mData = qVal;
			break;
		case PropertyTypes::ENUM:
			mgr.LoadAtom("Ogre::String", &sVal);
			eData.fromString(sVal);
			mData = eData;
			break;
		}
	}


	void GenericProperty::GetAsScriptParam(std::vector<ScriptParam> &params) const
	{
		int type = getType();
		switch (type)
		{
		case PropertyTypes::INT:
			params.push_back(ScriptParam(Get<int>())); break;
		case PropertyTypes::BOOL:
			params.push_back(ScriptParam(Get<bool>())); break;
		case PropertyTypes::FLOAT:
			params.push_back(ScriptParam(Get<float>())); break;
		case PropertyTypes::STRING:
			params.push_back(ScriptParam(std::string(Get<Ogre::String>().c_str()))); break;
		case PropertyTypes::VECTOR3:
			params.push_back(ScriptParam(Get<Ogre::Vector3>().x));
			params.push_back(ScriptParam(Get<Ogre::Vector3>().y));
			params.push_back(ScriptParam(Get<Ogre::Vector3>().z));
			break;
		case PropertyTypes::QUATERNION:
			params.push_back(ScriptParam(Get<Ogre::Quaternion>().w));
			params.push_back(ScriptParam(Get<Ogre::Quaternion>().x));
			params.push_back(ScriptParam(Get<Ogre::Quaternion>().y));
			params.push_back(ScriptParam(Get<Ogre::Quaternion>().z));
			break;
		case PropertyTypes::ENUM:
			params.push_back(ScriptParam(static_cast<int>(Get<DataMap::Enum>().selection)));
			break;
		}
	}

	DataMap::DataMap()
	{
		mIterator = mData.begin();
	}

	DataMap::DataMap(const DataMap &rhs)
	{
		mData = rhs.mData;
		mIterator = mData.begin();
	}

	DataMap::~DataMap()
	{
		mData.clear();
	}

	bool DataMap::HasKey(const Ogre::String &keyname)
	{
		for (auto i = mData.begin(); i != mData.end(); i++)
		{
			if (i->key == keyname) return true;
		}
		return false;
	}

	int DataMap::GetInt(const Ogre::String &keyname) const
	{
		return GetValue<int>(keyname);
	}

	bool DataMap::GetBool(const Ogre::String &keyname) const
	{
		return GetValue<bool>(keyname);
	}

	float DataMap::GetFloat(const Ogre::String &keyname) const
	{
		return GetValue<float>(keyname);
	}

	Ogre::Vector3 DataMap::GetOgreVec3(const Ogre::String &keyname) const
	{
		return GetValue<Ogre::Vector3>(keyname);
	}

	Ogre::ColourValue DataMap::GetOgreCol(const Ogre::String &keyname) const
	{
		Ogre::Vector3 vec = GetValue<Ogre::Vector3>(keyname);
		return Ogre::ColourValue(vec.x, vec.y, vec.z);
	}

	Ogre::Quaternion DataMap::GetOgreQuat(const Ogre::String &keyname) const
	{
		return GetValue<Ogre::Quaternion>(keyname);
	}

	Ogre::String DataMap::GetOgreString(const Ogre::String &keyname) const
	{
		return GetValue<Ogre::String>(keyname);
	}

	DataMap::Enum DataMap::GetEnum(const Ogre::String &keyname)	const
	{
		return GetValue<Enum>(keyname);
	}

	Ogre::String DataMap::Enum::toString() const
	{
		Ogre::String coded_enum = Ogre::StringConverter::toString(selection) + " ";
		for (std::vector<Ogre::String>::const_iterator i = choices.begin(); i != choices.end(); i++) coded_enum += (*i + " ");
		return coded_enum;
	}
	void DataMap::Enum::fromString(Ogre::String coded_enum)
	{
		selection = Ogre::StringConverter::parseInt(coded_enum.substr(0, 1));
		coded_enum = coded_enum.substr(2, coded_enum.size());
		while (!coded_enum.empty())
		{
			choices.push_back(coded_enum.substr(0, coded_enum.find(" ")));
			coded_enum = coded_enum.substr(coded_enum.find(" ")+1, coded_enum.size());
		}
	}


	void DataMap::AddBool(const Ogre::String &keyname, const bool val)
	{
		AddItem(keyname, val);
	}


	void DataMap::AddInt(const Ogre::String &keyname, const int val)
	{
		AddItem(keyname, val);
	}

	void DataMap::AddFloat(const Ogre::String &keyname, const float val)
	{
		AddItem(keyname, val);
	}

	void DataMap::AddOgreVec3(const Ogre::String &keyname, const Ogre::Vector3 &vec)
	{
		AddItem(keyname, vec);
	}

	void DataMap::AddOgreCol(const Ogre::String &keyname, const Ogre::ColourValue &val)
	{
		AddOgreVec3(keyname, Ogre::Vector3(val.r, val.g, val.b));		//Hack: Add ad Vec3
	}

	void DataMap::AddOgreQuat(const Ogre::String &keyname, const Ogre::Quaternion &quat)
	{
		AddItem(keyname, quat);
	}

	void DataMap::AddOgreString(const Ogre::String &keyname, const Ogre::String &text)
	{
		AddItem<Ogre::String>(keyname, text);
	}

	void DataMap::AddEnum(const Ogre::String &keyname, std::vector<Ogre::String> choices, unsigned int selection)
	{
		IceAssert(choices.size() > selection)

		//hack: code it as Ogre string
		Enum e; e.choices = choices; e.selection = selection;
		AddItem<Enum>(keyname, e);
		//AddOgreString(keyname, e.toString());
	}

	bool DataMap::HasNext()
	{
		if (mIterator != mData.end()) return true;
		mIterator = mData.begin();
		return false;
	}

	DataMap::Item DataMap::GetNext()
	{
		Item returner = *mIterator;
		mIterator++;
		return returner;
	}

	void DataMap::Item::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", &key, "key");
		mgr.SaveObject(&data, "Data");
	}
	void DataMap::Item::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &key);
		data  = *(GenericProperty*)mgr.LoadObject();
	}

	void DataMap::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("std::vector<DataMapItem>", &mData, "Items");
	}

	void DataMap::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("std::vector<DataMapItem>", &mData);
		mIterator = mData.begin();
	}

};