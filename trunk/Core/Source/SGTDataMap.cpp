
#include "SGTDataMap.h"


/*void* SGTDataMap::EncodeOgreVec3(Ogre::Vector3 vec)
{
	void* data = new BYTE[12];
	memcpy(data, &vec.x, 4);
	memcpy((BYTE*)(data)+4, &vec.y, 4);
	memcpy((BYTE*)(data)+8, &vec.z, 4);
	return data;
}

void* SGTDataMap::EncodeOgreQuat(Ogre::Quaternion quat)
{
	void* data = new BYTE[16];
	memcpy(data, &quat.w, 4);
	memcpy((BYTE*)(data)+4, &quat.x, 4);
	memcpy((BYTE*)(data)+8, &quat.y, 4);
	memcpy((BYTE*)(data)+12, &quat.z, 4);
	return data;
}

void* SGTDataMap::EncodeOgreString(Ogre::String str)
{
	int length = str.length();
	void* data = new BYTE[4 + length];
	memcpy(data, &length, 4);
	memcpy((BYTE*)(data)+4, str.c_str(), length);
	return data;
}


Ogre::Vector3 SGTDataMap::DecodeOgreVec3(void* data)
{
	Ogre::Vector3 returner = Ogre::Vector3(0,0,0);
	if (data == NULL) return returner;
	memcpy(&returner.x, data, 4);
	memcpy(&returner.y, (BYTE*)(data)+4, 4);
	memcpy(&returner.z, (BYTE*)(data)+8, 4);
	return returner;
}

Ogre::Quaternion SGTDataMap::DecodeOgreQuat(void* data)
{
	Ogre::Quaternion returner = Ogre::Quaternion();
	if (data == NULL) return returner;
	memcpy(&returner.w, data, 4);
	memcpy(&returner.x, (BYTE*)(data)+4, 4);
	memcpy(&returner.y, (BYTE*)(data)+8, 4);
	memcpy(&returner.z, (BYTE*)(data)+12, 4);
	return returner;
}

Ogre::String SGTDataMap::DecodeOgreString(void* data)
{
	if (data == NULL) return "";
	int length;
	memcpy(&length, data, 4);
	char* blubb = new char[length+1];
	memcpy(blubb, (BYTE*)(data)+4, length);
	blubb[length] = '\0';
	Ogre::String returner = blubb;
	delete blubb;
	return returner;
}*/

SGTDataMapEntry::SGTDataMapEntry()
{
	mData.setNull();
}

SGTDataMapEntry::~SGTDataMapEntry()
{
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
	/*for (std::vector<SGTDataMapEntry>::iterator i = mData.begin(); i != mData.end(); i++)
	{
		Ogre::LogManager::getSingleton().logMessage(" Deleting " + (*i).mKey + " " + (*i).mType);
		if ((*i).mData != NULL)
		{
			delete ((*i).mData);
			(*i).mData = NULL;
		}
	}
	Ogre::LogManager::getSingleton().logMessage("~SGTDataMap 2");*/
	mData.clear();
}

bool SGTDataMap::HasKey(Ogre::String keyname)
{
	for (std::vector<SGTDataMapEntry>::iterator i = mData.begin(); i != mData.end(); i++)
	{
		if ((*i).mKey == keyname) return true;
	}
	return false;
}

void* SGTDataMap::GetData(Ogre::String keyname)
{
	for (std::vector<SGTDataMapEntry>::iterator i = mData.begin(); i != mData.end(); i++)
	{
		if ((*i).mKey == keyname) return (*i).mData.getPointer();
	}
	//Invalid key! Let's return some bullshit.
	Ogre::LogManager::getSingleton().logMessage("SGTDataMap::Get Invalid key (" + keyname + ")!");
	return NULL;
}

int SGTDataMap::GetInt(Ogre::String keyname)
{
	void* data = GetData(keyname);
	if (data == NULL) return 0;
	return *((int*)(data));
}

bool SGTDataMap::GetBool(Ogre::String keyname)
{
	void* data = GetData(keyname);
	if (data == NULL) return false;
	return *((bool*)(data));
}

float SGTDataMap::GetFloat(Ogre::String keyname)
{
	void* data = GetData(keyname);
	if (data == NULL) return 0.0f;
	return *((float*)(data));
}

Ogre::Vector3 SGTDataMap::GetOgreVec3(Ogre::String keyname)
{
	void* data = GetData(keyname);
	if (data == NULL) return Ogre::Vector3(0,0,0);
	return *((Ogre::Vector3*)(data));
}

Ogre::Quaternion SGTDataMap::GetOgreQuat(Ogre::String keyname)
{
	void* data = GetData(keyname);
	if (data == NULL) return Ogre::Quaternion();
	return *((Ogre::Quaternion*)(data));
}

Ogre::String SGTDataMap::GetOgreString(Ogre::String keyname)
{
	char* data = (char*)GetData(keyname);
	if (data == NULL) return "";
	return Ogre::String(data);
	//return *((Ogre::String*)(data));
}


void SGTDataMap::AddData(Ogre::String keyname, Ogre::String type, BYTE* data)
{
	if (!HasKey(keyname))
	{
		SGTDataMapEntry entry;
		entry.mData = Ogre::SharedPtr<BYTE>(data);
		entry.mType = type;
		entry.mKey = keyname;
		mData.push_back(entry);
		mIterator = mData.begin();
	}
	else
	{
#if _DEBUG
		DebugBreak();
#endif
		Ogre::LogManager::getSingleton().logMessage("SGTDataMap::Add Key (" + keyname + ") already exists. " + Ogre::StringConverter::toString(mData.size()));
	}
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
	char *data = new char[text.size()+1];
	memcpy(data, text.c_str(), text.size()+1);
	AddData(keyname, "Ogre::String", (BYTE*)data);
}

bool SGTDataMap::HasNext()
{
	if (mIterator != mData.end()) return true;
	mIterator = mData.begin();
	return false;
}

SGTDataMapEntry SGTDataMap::GetNext()
{
	SGTDataMapEntry returner = (*mIterator);
	mIterator++;
	return returner;
}

SGTDataMapEntry* SGTDataMap::GetNextPtr()
{
	SGTDataMapEntry* returner = &(*mIterator);
	mIterator++;
	return returner;
}

void SGTDataMap::SaveContent(SGTSaveSystem& myManager)
{
	while (HasNext())
	{
		SGTDataMapEntry current = GetNext();
		if (current.mType == "Ogre::String")
		{
			char *c_str = (char*)current.mData.getPointer();
			Ogre::String str(c_str);
			myManager.SaveAtom(current.mType, &str, current.mKey);
		}
		else myManager.SaveAtom(current.mType, current.mData.getPointer(), current.mKey);
	}
}

void SGTDataMap::LoadContent(SGTLoadSystem& mgr)
{
	while (HasNext())
	{
		SGTDataMapEntry *current = GetNextPtr();

		Ogre::String key = current->mKey;
		Ogre::String type = current->mType;
		if (type == "int")
		{
			int val = 0;
			mgr.LoadAtom(type, &val);
			current->mData = Ogre::SharedPtr<BYTE>((BYTE*)&val);
			continue;
		}
		else if (type == "float")
		{
			float val = 0.0f;
			mgr.LoadAtom(type, &val);
			current->mData = Ogre::SharedPtr<BYTE>((BYTE*)&val);
			continue;
		}
		else if (type == "bool")
		{
			bool val = false;
			mgr.LoadAtom(type, &val);
			current->mData = Ogre::SharedPtr<BYTE>((BYTE*)&val);
			continue;
		}
		else if (type == "Ogre::String")
		{
			Ogre::String val = "";
			mgr.LoadAtom(type, &val);
			current->mData = Ogre::SharedPtr<BYTE>((BYTE*)&val);
			continue;
		}
		else if (type == "Ogre::Vector3")
		{
			Ogre::Vector3 val = Ogre::Vector3(0,0,0);
			mgr.LoadAtom(type, &val);
			current->mData = Ogre::SharedPtr<BYTE>((BYTE*)&val);
			continue;
		}
		else if (type == "Ogre::Quaternion")
		{
			Ogre::Quaternion val = Ogre::Quaternion();
			mgr.LoadAtom(type, &val);
			current->mData = Ogre::SharedPtr<BYTE>((BYTE*)&val);
			continue;
		}
	}
}

void SGTDataMap::Save(SGTSaveSystem& myManager)
{
	int size = mData.size();
	myManager.SaveAtom("int", (void*)(&size), "mMapSize");
	while (HasNext())
	{
		SGTDataMapEntry current = GetNext();
		myManager.SaveAtom("Ogre::String", (void*)(&current.mKey), "mKeyName");
		myManager.SaveAtom("Ogre::String", (void*)(&current.mType), "mType");
		if (current.mType == "Ogre::String")
		{
			char *c_str = (char*)current.mData.getPointer();
			Ogre::String str(c_str);
			myManager.SaveAtom(current.mType, &str, "mData");
		}
		else myManager.SaveAtom(current.mType, current.mData.getPointer(), "mData");
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