
#include "IceDataMap.h"


namespace Ice
{

void GenericProperty::Save(LoadSave::SaveSystem& myManager)
{
	myManager.SaveAtom("Ogre::String", &mKey, "Key");
	int iType = 0;

	if (mType == "int")
	{
		iType = PropertyTypes::INT;
		myManager.SaveAtom("int", &iType, "Type");
		int data = Ogre::any_cast<int>(mData);
		myManager.SaveAtom(mType, (void*)&data, "Data");
	}
	else if (mType == "float")
	{
		iType = PropertyTypes::FLOAT;
		myManager.SaveAtom("int", &iType, "Type");
		float data = Ogre::any_cast<float>(mData);
		myManager.SaveAtom(mType, (void*)&data, "Data");
	}
	else if (mType == "bool")
	{
		iType = PropertyTypes::BOOL;
		myManager.SaveAtom("int", &iType, "Type");
		bool data = Ogre::any_cast<bool>(mData);
		myManager.SaveAtom(mType, (void*)&data, "Data");
	}
	else if (mType == "Ogre::Vector3")
	{
		iType = PropertyTypes::VECTOR3;
		myManager.SaveAtom("int", &iType, "Type");
		Ogre::Vector3 data = Ogre::any_cast<Ogre::Vector3>(mData);
		myManager.SaveAtom(mType, (void*)&data, "Data");
	}
	else if (mType == "Ogre::Quaternion")
	{
		iType = PropertyTypes::QUATERNION;
		myManager.SaveAtom("int", &iType, "Type");
		Ogre::Quaternion data = Ogre::any_cast<Ogre::Quaternion>(mData);
		myManager.SaveAtom(mType, (void*)&data, "Data");
	}
	else if (mType == "Ogre::String")
	{
		iType = PropertyTypes::STRING;
		myManager.SaveAtom("int", &iType, "Type");
		Ogre::String data = Ogre::any_cast<Ogre::String>(mData);
		myManager.SaveAtom(mType, (void*)&data, "Data");
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
		mType = "int";
		mgr.LoadAtom(mType, &iVal);
		mData = iVal;
		break;
	case PropertyTypes::BOOL:
		mType = "bool";
		mgr.LoadAtom(mType, &bVal);
		mData = bVal;
		break;
	case PropertyTypes::FLOAT:
		mType = "float";
		mgr.LoadAtom(mType, &fVal);
		mData = fVal;
		break;
	case PropertyTypes::STRING:
		mType = "Ogre::String";
		mgr.LoadAtom(mType, &sVal);
		mData = sVal;
		break;
	case PropertyTypes::VECTOR3:
		mType = "Ogre::Vector3";
		mgr.LoadAtom(mType, &vVal);
		mData = vVal;
		break;
	case PropertyTypes::QUATERNION:
		mType = "Ogre::Quaternion";
		mgr.LoadAtom(mType, &qVal);
		mData = qVal;
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

int DataMap::GetInt(Ogre::String keyname)
{
	return GetValue<int>(keyname);
}

bool DataMap::GetBool(Ogre::String keyname)
{
	return GetValue<bool>(keyname);
}

float DataMap::GetFloat(Ogre::String keyname)
{
	return GetValue<float>(keyname);
}

Ogre::Vector3 DataMap::GetOgreVec3(Ogre::String keyname)
{
	return GetValue<Ogre::Vector3>(keyname);
}

Ogre::Quaternion DataMap::GetOgreQuat(Ogre::String keyname)
{
	return GetValue<Ogre::Quaternion>(keyname);
}

Ogre::String DataMap::GetOgreString(Ogre::String keyname)
{
	return GetValue<Ogre::String>(keyname);
}

void DataMap::AddBool(Ogre::String keyname, bool val)
{
	AddValue(keyname, "bool", val);
}


void DataMap::AddInt(Ogre::String keyname, int val)
{
	AddValue(keyname, "int", val);
}

void DataMap::AddFloat(Ogre::String keyname, float val)
{
	AddValue(keyname, "float", val);
}

void DataMap::AddOgreVec3(Ogre::String keyname, Ogre::Vector3 vec)
{
	AddValue(keyname, "Ogre::Vector3", vec);
}

void DataMap::AddOgreQuat(Ogre::String keyname, Ogre::Quaternion quat)
{
	AddValue(keyname, "Ogre::Quaternion", quat);
}

void DataMap::AddOgreString(Ogre::String keyname, Ogre::String text)
{
	AddValue<Ogre::String>(keyname, "Ogre::String", text);
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