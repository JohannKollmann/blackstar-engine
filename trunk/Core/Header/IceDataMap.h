
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
public:
	enum PropertyTypes
	{
		INT = 0,
		BOOL = 1,
		FLOAT = 2,
		STRING = 3,
		VECTOR3 = 4,
		QUATERNION = 5
	};
	PropertyTypes getType();

	Ogre::Any mData;
	Ogre::String mKey;

	GenericProperty() {};
	~GenericProperty() {};

	template <typename T>
		void Set(const T &value, const Ogre::String &key) { mData = value; mKey = key; };

	void Set(void *data, const Ogre::String &key, const PropertyTypes &types);

	template <typename T>
		T Get() { return Ogre::any_cast<T>(mData); }

	PropertyTypes Get(void *target);

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
	templateType GetValue(Ogre::String keyname) const
	{
		for (auto i = mData.begin(); i != mData.end(); i++)
		{
			if ((*i).mKey == keyname) return Ogre::any_cast<templateType>((*i).mData);
		}
		//Invalid key! return the default val
		OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND , "DataMap::Get Invalid key (" + keyname + ")!", "IceDataMap.h, line 71");
		return templateType();
	}
	template <class templateType>
	templateType GetValue(Ogre::String keyname, templateType defaultVal) const
	{
		templateType val;
		try {
			val = GetValue<templateType>(keyname);
		} catch (Ogre::Exception) { val = defaultVal; }
		return val;
	}

	int GetInt(Ogre::String keyname)					const;
	bool GetBool(Ogre::String keyname)					const;
	float GetFloat(Ogre::String keyname)				const;
	Ogre::Vector3 GetOgreVec3(Ogre::String keyname)		const;
	Ogre::ColourValue GetOgreCol(Ogre::String keyname)	const;
	Ogre::Quaternion GetOgreQuat(Ogre::String keyname)	const;
	Ogre::String GetOgreString(Ogre::String keyname)	const;

	void AddProperty(GenericProperty &prop)
	{
		mData.push_back(prop);
		mIterator = mData.begin();
	}

	void AddValue(Ogre::String keyname, void *data, const GenericProperty::PropertyTypes &type)
	{
		GenericProperty entry;
		entry.Set(data, keyname, type);
		AddProperty(entry);
	};

	/*
	Template zum speichern von Basistypen.
	*/
	template <class templateType>
		void AddValue(Ogre::String keyname, templateType var)
		{
			GenericProperty entry;
			entry.Set<templateType>(var, keyname);
			AddProperty(entry);
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

};