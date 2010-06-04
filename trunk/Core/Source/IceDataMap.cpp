
#include "IceDataMap.h"


namespace Ice
{

	GenericProperty::PropertyTypes GenericProperty::getType()
	{
		if (mData.getType() == typeid(bool)) return PropertyTypes::BOOL;
		if (mData.getType() == typeid(float)) return PropertyTypes::FLOAT;
		if (mData.getType() == typeid(int)) return PropertyTypes::INT;
		if (mData.getType() == typeid(Ogre::String)) return PropertyTypes::STRING;
		if (mData.getType() == typeid(Ogre::Vector3)) return PropertyTypes::VECTOR3;
		if (mData.getType() == typeid(Ogre::Quaternion)) return PropertyTypes::QUATERNION;
		
		assert(false);
		return PropertyTypes::INT;
	}
	GenericProperty::PropertyTypes GenericProperty::Get(void *target)
	{
		if (mData.getType() == typeid(bool)) { bool src = Ogre::any_cast<bool>(mData); memcpy(target, &src, sizeof(bool)); return PropertyTypes::BOOL; }
		if (mData.getType() == typeid(float)) { float src = Ogre::any_cast<float>(mData); *((float*)target) = src; return PropertyTypes::FLOAT; }
		if (mData.getType() == typeid(int)) { int src = Ogre::any_cast<int>(mData); *((int*)target) = src; return PropertyTypes::INT; }
		if (mData.getType() == typeid(Ogre::String)) { Ogre::String src = Ogre::any_cast<Ogre::String>(mData); *((Ogre::String*)target) = src; return PropertyTypes::STRING; }
		if (mData.getType() == typeid(Ogre::Vector3)) { Ogre::Vector3 src = Ogre::any_cast<Ogre::Vector3>(mData); *((Ogre::Vector3*)target) = src;  return PropertyTypes::VECTOR3; }
		if (mData.getType() == typeid(Ogre::Quaternion)) { Ogre::Quaternion src = Ogre::any_cast<Ogre::Quaternion>(mData); *((Ogre::Quaternion*)target) = src; return PropertyTypes::QUATERNION; }
		
		assert(false);
		return PropertyTypes::INT;
	}

	void GenericProperty::Set(void *data, const Ogre::String &key, const GenericProperty::PropertyTypes &type)
	{
		switch (type)
		{
		case PropertyTypes::INT:
			Set<int>(*((int*)data), key); break;
		case PropertyTypes::BOOL:
			Set<bool>(*((bool*)data), key); break;
		case PropertyTypes::FLOAT:
			Set<float>(*((float*)data), key); break;
		case PropertyTypes::STRING:
			Set<Ogre::String>(*((Ogre::String*)data), key); break;
		case PropertyTypes::VECTOR3:
			Set<Ogre::Vector3>(*((Ogre::Vector3*)data), key); break;
		case PropertyTypes::QUATERNION:
			Set<Ogre::Quaternion>(*((Ogre::Quaternion*)data), key); break;
		}
	}

	void GenericProperty::Save(LoadSave::SaveSystem& myManager)
	{
		myManager.SaveAtom("Ogre::String", &mKey, "Key");
		int type = getType();
		myManager.SaveAtom("int", &type, "Type");

		int iData; bool bData; float fData; Ogre::String sData; Ogre::Vector3 vData; Ogre::Quaternion qData;

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
		}
	}

	void GenericProperty::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mKey);
		int type = 0;
		mgr.LoadAtom("int", &type);
		int iVal = 0;
		bool bVal = false;
		float fVal = 0.0f;
		Ogre::String sVal;
		Ogre::Vector3 vVal;
		Ogre::Quaternion qVal;
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
		}
	}

	void GenericProperty::GetAsScriptParam(std::vector<ScriptParam> &params)
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

	bool DataMap::HasKey(Ogre::String keyname)
	{
		for (std::vector<GenericProperty>::iterator i = mData.begin(); i != mData.end(); i++)
		{
			if ((*i).mKey == keyname) return true;
		}
		return false;
	}

	int DataMap::GetInt(Ogre::String keyname) const
	{
		return GetValue<int>(keyname, 0);
	}

	bool DataMap::GetBool(Ogre::String keyname) const
	{
		return GetValue<bool>(keyname, false);
	}

	float DataMap::GetFloat(Ogre::String keyname) const
	{
		return GetValue<float>(keyname, 0);
	}

	Ogre::Vector3 DataMap::GetOgreVec3(Ogre::String keyname) const
	{
		return GetValue<Ogre::Vector3>(keyname, Ogre::Vector3());
	}

	Ogre::ColourValue DataMap::GetOgreCol(Ogre::String keyname) const
	{
		Ogre::Vector3 vec = GetValue<Ogre::Vector3>(keyname, Ogre::Vector3());
		return Ogre::ColourValue(vec.x, vec.y, vec.z);
	}

	Ogre::Quaternion DataMap::GetOgreQuat(Ogre::String keyname) const
	{
		return GetValue<Ogre::Quaternion>(keyname, Ogre::Quaternion());
	}

	Ogre::String DataMap::GetOgreString(Ogre::String keyname) const
	{
		return GetValue<Ogre::String>(keyname, "");
	}

	void DataMap::AddBool(Ogre::String keyname, bool val)
	{
		AddValue(keyname, val);
	}


	void DataMap::AddInt(Ogre::String keyname, int val)
	{
		AddValue(keyname, val);
	}

	void DataMap::AddFloat(Ogre::String keyname, float val)
	{
		AddValue(keyname, val);
	}

	void DataMap::AddOgreVec3(Ogre::String keyname, Ogre::Vector3 vec)
	{
		AddValue(keyname, vec);
	}

	void DataMap::AddOgreCol(Ogre::String keyname, Ogre::ColourValue val)
	{
		AddOgreVec3(keyname, Ogre::Vector3(val.r, val.g, val.b));		//Hack: Add ad Vec3
	}

	void DataMap::AddOgreQuat(Ogre::String keyname, Ogre::Quaternion quat)
	{
		AddValue(keyname, quat);
	}

	void DataMap::AddOgreString(Ogre::String keyname, Ogre::String text)
	{
		AddValue<Ogre::String>(keyname, text);
	}

	bool DataMap::HasNext()
	{
		if (mIterator != mData.end()) return true;
		mIterator = mData.begin();
		return false;
	}

	GenericProperty DataMap::GetNext()
	{
		GenericProperty returner = (*mIterator);
		mIterator++;
		return returner;
	}

	GenericProperty* DataMap::GetNextPtr()
	{
		GenericProperty* returner = &(*mIterator);
		mIterator++;
		return returner;
	}

	void DataMap::Save(LoadSave::SaveSystem& myManager)
	{
		myManager.SaveAtom("std::vector<GenericProperty>", &mData, "mDataList");
	}

	void DataMap::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("std::vector<GenericProperty>", &mData);
		mIterator = mData.begin();
	}

};