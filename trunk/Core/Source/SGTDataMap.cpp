
#include "SGTDataMap.h"

void SGTGenericProperty::Save(SGTSaveSystem& myManager)
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

void SGTGenericProperty::Load(SGTLoadSystem& mgr)
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

SGTDataMap::SGTDataMap()
{
	mIterator = mData.begin();
}

SGTDataMap::SGTDataMap(const SGTDataMap &rhs)
{
	mData = rhs.mData;
	mIterator = mData.begin();
}

SGTDataMap::~SGTDataMap()
{
	mData.clear();
}

bool SGTDataMap::HasKey(Ogre::String keyname)
{
	for (std::vector<SGTGenericProperty>::iterator i = mData.begin(); i != mData.end(); i++)
	{
		if ((*i).mKey == keyname) return true;
	}
	return false;
}

int SGTDataMap::GetInt(Ogre::String keyname)
{
	return GetValue<int>(keyname);
}

bool SGTDataMap::GetBool(Ogre::String keyname)
{
	return GetValue<bool>(keyname);
}

float SGTDataMap::GetFloat(Ogre::String keyname)
{
	return GetValue<float>(keyname);
}

Ogre::Vector3 SGTDataMap::GetOgreVec3(Ogre::String keyname)
{
	return GetValue<Ogre::Vector3>(keyname);
}

Ogre::Quaternion SGTDataMap::GetOgreQuat(Ogre::String keyname)
{
	return GetValue<Ogre::Quaternion>(keyname);
}

Ogre::String SGTDataMap::GetOgreString(Ogre::String keyname)
{
	return GetValue<Ogre::String>(keyname);
}

void SGTDataMap::AddBool(Ogre::String keyname, bool val)
{
	AddValue(keyname, "bool", val);
}


void SGTDataMap::AddInt(Ogre::String keyname, int val)
{
	AddValue(keyname, "int", val);
}

void SGTDataMap::AddFloat(Ogre::String keyname, float val)
{
	AddValue(keyname, "float", val);
}

void SGTDataMap::AddOgreVec3(Ogre::String keyname, Ogre::Vector3 vec)
{
	AddValue(keyname, "Ogre::Vector3", vec);
}

void SGTDataMap::AddOgreQuat(Ogre::String keyname, Ogre::Quaternion quat)
{
	AddValue(keyname, "Ogre::Quaternion", quat);
}

void SGTDataMap::AddOgreString(Ogre::String keyname, Ogre::String text)
{
	AddValue<Ogre::String>(keyname, "Ogre::String", text);
}

bool SGTDataMap::HasNext()
{
	if (mIterator != mData.end()) return true;
	mIterator = mData.begin();
	return false;
}

SGTGenericProperty SGTDataMap::GetNext()
{
	SGTGenericProperty returner = (*mIterator);
	mIterator++;
	return returner;
}

SGTGenericProperty* SGTDataMap::GetNextPtr()
{
	SGTGenericProperty* returner = &(*mIterator);
	mIterator++;
	return returner;
}

void SGTDataMap::Save(SGTSaveSystem& myManager)
{
	myManager.SaveAtom("std::vector<SGTGenericProperty>", &mData, "mDataList");
}

void SGTDataMap::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("std::vector<SGTGenericProperty>", &mData);
	mIterator = mData.begin();
}