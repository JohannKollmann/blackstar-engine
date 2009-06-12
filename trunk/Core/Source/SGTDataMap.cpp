
#include "SGTDataMap.h"


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
	int size = mData.size();
	myManager.SaveAtom("int", (void*)(&size), "mMapSize");
	while (HasNext())
	{
		SGTGenericProperty current = GetNext();
		myManager.SaveAtom("Ogre::String", (void*)(&current.mKey), "mKeyName");
		myManager.SaveAtom("Ogre::String", (void*)(&current.mType), "mType");
		if (current.mType == "int")
		{
			int data = Ogre::any_cast<int>(current.mData);
			myManager.SaveAtom(current.mType, (void*)&data, "mType");
			continue;
		}
		else if (current.mType == "float")
		{
			float data = Ogre::any_cast<float>(current.mData);
			myManager.SaveAtom(current.mType, (void*)&data, "mType");
			continue;
		}
		else if (current.mType == "bool")
		{
			bool data = Ogre::any_cast<bool>(current.mData);
			myManager.SaveAtom(current.mType, (void*)&data, "mType");
			continue;
		}
		else if (current.mType == "Ogre::Vector3")
		{
			Ogre::Vector3 data = Ogre::any_cast<Ogre::Vector3>(current.mData);
			myManager.SaveAtom(current.mType, (void*)&data, "mType");
			continue;
		}
		else if (current.mType == "Ogre::Quaternion")
		{
			Ogre::Quaternion data = Ogre::any_cast<Ogre::Quaternion>(current.mData);
			myManager.SaveAtom(current.mType, (void*)&data, "mType");
			continue;
		}
		else if (current.mType == "Ogre::String")
		{
			Ogre::String data = Ogre::any_cast<Ogre::String>(current.mData);
			myManager.SaveAtom(current.mType, (void*)&data, "mType");
			continue;
		}
	}
}

void SGTDataMap::Load(SGTLoadSystem& mgr)
{
	int size = 0;
	mgr.LoadAtom("int", &size);
	for (int i = 0; i < size; i++)
	{
		Ogre::String key = "";
		Ogre::String type = "";
		mgr.LoadAtom("Ogre::String", &key);
		mgr.LoadAtom("Ogre::String", &type);
		if (type == "int")
		{
			int val = 0;
			mgr.LoadAtom(type, &val);
			AddInt(key, val);
			continue;
		}
		else if (type == "float")
		{
			float val = 0.0f;
			mgr.LoadAtom(type, &val);
			AddFloat(key, val);
			continue;
		}
		else if (type == "bool")
		{
			bool val = false;
			mgr.LoadAtom(type, &val);
			AddBool(key, val);
			continue;
		}
		else if (type == "Ogre::Vector3")
		{
			Ogre::Vector3 val = Ogre::Vector3(0,0,0);
			mgr.LoadAtom(type, &val);
			AddOgreVec3(key, val);
			continue;
		}
		else if (type == "Ogre::Quaternion")
		{
			Ogre::Quaternion val = Ogre::Quaternion();
			mgr.LoadAtom(type, &val);
			AddOgreQuat(key, val);
			continue;
		}
		else if (type == "Ogre::String")
		{
			Ogre::String val = "";
			mgr.LoadAtom(type, &val);
			AddOgreString(key, val);
			continue;
		}
	}
}