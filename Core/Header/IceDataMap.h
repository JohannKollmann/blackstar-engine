
#pragma once

#include <vector>
#include "IceIncludes.h"
#include "Ogre.h"
#include "LoadSave.h"
#include "boost/any.hpp"
#include "OgreAny.h"
#include "IceScriptParam.h"

namespace Ice
{

/*
Klasse zum sicheren und komfortablen Benutzen von std::map<Ogre::String, void*>
*/

class DllExport GenericProperty : public LoadSave::Saveable
{
private:
	enum PropertyTypes
	{
		INT,
		BOOL,
		FLOAT,
		STRING,
		VECTOR3,
		QUATERNION
	};
public:
	Ogre::Any mData;
	Ogre::String mType;
	Ogre::String mKey;

	GenericProperty() {};
	~GenericProperty() {};

	template <typename T>
		void Set(T value, Ogre::String type, Ogre::String key) { mData = value; mType = type; mKey = key; };

	template <typename T>
		T Get() { return Ogre::any_cast<T>(mData); };

	void GetAsScriptParam(std::vector<ScriptParam> &params);

	void Save(LoadSave::SaveSystem& myManager);
	void Load(LoadSave::LoadSystem& mgr); 
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "GenericProperty"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new GenericProperty; };
	std::string& TellName() { static std::string name = "GenericProperty"; return name; }
};

class DllExport DataMap : public LoadSave::Saveable
{
private:
	std::vector<GenericProperty> mData;
	std::vector<GenericProperty>::iterator mIterator;

public:
	DataMap();
	DataMap(const DataMap &rhs);
	~DataMap();

	bool HasNext();
	GenericProperty GetNext();
	GenericProperty* GetNextPtr();

	bool HasKey(Ogre::String keyname);

	template <class templateType>
	templateType GetValue(Ogre::String keyname) throw(Ogre::Exception)
	{
		for (std::vector<GenericProperty>::iterator i = mData.begin(); i != mData.end(); i++)
		{
			if ((*i).mKey == keyname) return Ogre::any_cast<templateType>((*i).mData);
		}
		//Invalid key! Let's return some bullshit.
		throw Ogre::Exception(Ogre::Exception::ERR_ITEM_NOT_FOUND , "DataMap::Get Invalid key (" + keyname + ")!", "IceDataMap.h, line 71");
	}
	template <class templateType>
	templateType GetValue(Ogre::String keyname, templateType defaultVal)
	{
		templateType val;
		try {
			val = GetValue<templateType>(keyname);
		} catch (Ogre::Exception) { val = defaultVal; }
		return val;
	}

	int GetInt(Ogre::String keyname);
	bool GetBool(Ogre::String keyname);
	float GetFloat(Ogre::String keyname);
	Ogre::Vector3 GetOgreVec3(Ogre::String keyname);
	Ogre::ColourValue GetOgreCol(Ogre::String keyname);
	Ogre::Quaternion GetOgreQuat(Ogre::String keyname);
	Ogre::String GetOgreString(Ogre::String keyname);

	/*
	Template zum speichern von Basistypen.
	*/
	template <class templateType>
		void AddValue(Ogre::String keyname, Ogre::String type, templateType var)
		{
			GenericProperty entry;
			entry.Set<templateType>(var, type, keyname);
			mData.push_back(entry);
			mIterator = mData.begin();
		};
	void AddBool(Ogre::String keyname, bool val);
	void AddInt(Ogre::String keyname, int val);
	void AddFloat(Ogre::String keyname, float val);
	void AddOgreVec3(Ogre::String keyname, Ogre::Vector3 vec);
	void AddOgreCol(Ogre::String keyname, Ogre::ColourValue val);
	void AddOgreQuat(Ogre::String keyname, Ogre::Quaternion quat);
	void AddOgreString(Ogre::String keyname, Ogre::String text);

	//Load Save
	void Save(LoadSave::SaveSystem& myManager);
	void Load(LoadSave::LoadSystem& mgr); 
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "DataMap"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new DataMap; };
	std::string& TellName() { static std::string name = "DataMap"; return name; };
};

//For Editors
class DllExport ComponentSection : public LoadSave::Saveable
{
private:
	std::string m_strName;
public:
	ComponentSection() { m_strName = "ComponentSection"; }
	Ogre::String mSectionName;
	Ogre::SharedPtr<DataMap> mSectionData;

	void Save(LoadSave::SaveSystem& myManager)
	{
		myManager.SaveAtom("Ogre::String", (void*)&mSectionName, "Key");
		myManager.SaveObject(mSectionData.getPointer(), "Value");	
	}
	void Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mSectionName);
		mSectionData = Ogre::SharedPtr<DataMap>((DataMap*)mgr.LoadObject());
	};
	static LoadSave::Saveable* NewInstance() { return new ComponentSection; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ComponentSection"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	std::string& TellName() { return m_strName; };
};

};