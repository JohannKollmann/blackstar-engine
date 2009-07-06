
#ifndef __SGTDataMap_H__
#define __SGTDataMap_H__

#include <vector>
#include "SGTIncludes.h"
#include "Ogre.h"
#include "SGTLoadSave.h"
#include "boost/any.hpp"
#include "OgreAny.h"

/*
Klasse zum sicheren und komfortablen Benutzen von std::map<Ogre::String, void*>
*/

class SGTDllExport SGTGenericProperty : public SGTSaveable
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

	SGTGenericProperty() {};
	template <typename T>
		void Set(T value, Ogre::String type, Ogre::String key) { mData = value; mType = type; mKey = key; };
	~SGTGenericProperty() {};


	void Save(SGTSaveSystem& myManager);
	void Load(SGTLoadSystem& mgr); 
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "SGTGenericProperty"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTGenericProperty* NewInstance() { return new SGTGenericProperty; };
	std::string& TellName() { static std::string name = "SGTGenericProperty"; return name; }
};

class SGTDllExport SGTDataMap : public SGTSaveable
{
private:
	std::vector<SGTGenericProperty> mData;
	std::vector<SGTGenericProperty>::iterator mIterator;

public:
	SGTDataMap();
	SGTDataMap(const SGTDataMap &rhs);
	~SGTDataMap();

	bool HasNext();
	SGTGenericProperty GetNext();
	SGTGenericProperty* GetNextPtr();

	bool HasKey(Ogre::String keyname);

	template <class templateType>
		templateType GetValue(Ogre::String keyname)
		{
			for (std::vector<SGTGenericProperty>::iterator i = mData.begin(); i != mData.end(); i++)
			{
				if ((*i).mKey == keyname) return Ogre::any_cast<templateType>((*i).mData);
			}
			//Invalid key! Let's return some bullshit.
			Ogre::LogManager::getSingleton().logMessage("SGTDataMap::Get Invalid key (" + keyname + ")!");
			return templateType();
		}

	int GetInt(Ogre::String keyname);
	bool GetBool(Ogre::String keyname);
	float GetFloat(Ogre::String keyname);
	Ogre::Vector3 GetOgreVec3(Ogre::String keyname);
	Ogre::Quaternion GetOgreQuat(Ogre::String keyname);
	Ogre::String GetOgreString(Ogre::String keyname);

	/*
	Template zum speichern von Basistypen.
	*/
	template <class templateType>
		void AddValue(Ogre::String keyname, Ogre::String type, templateType var)
		{
			SGTGenericProperty entry;
			entry.Set<templateType>(var, type, keyname);
			mData.push_back(entry);
			mIterator = mData.begin();
		};
	void AddBool(Ogre::String keyname, bool val);
	void AddInt(Ogre::String keyname, int val);
	void AddFloat(Ogre::String keyname, float val);
	void AddOgreVec3(Ogre::String keyname, Ogre::Vector3 vec);
	void AddOgreQuat(Ogre::String keyname, Ogre::Quaternion quat);
	void AddOgreString(Ogre::String keyname, Ogre::String text);

	//Load Save
	void Save(SGTSaveSystem& myManager);
	void Load(SGTLoadSystem& mgr); 
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "SGTDataMap"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTDataMap* NewInstance() { return new SGTDataMap; };
	std::string& TellName() { static std::string name = "SGTDataMap"; return name; };
};

//For Editors
class SGTDllExport ComponentSection : public SGTSaveable
{
private:
	std::string m_strName;
public:
	ComponentSection() { m_strName = "ComponentSection"; }
	Ogre::String mSectionName;
	Ogre::SharedPtr<SGTDataMap> mSectionData;

	void Save(SGTSaveSystem& myManager)
	{
		myManager.SaveAtom("Ogre::String", (void*)&mSectionName, "Key");
		myManager.SaveObject(mSectionData.getPointer(), "Value");	
	}
	void Load(SGTLoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mSectionName);
		mSectionData = Ogre::SharedPtr<SGTDataMap>((SGTDataMap*)mgr.LoadObject());
	};
	static SGTSaveable* NewInstance() { return new ComponentSection; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "ComponentSection"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	std::string& TellName() { return m_strName; };
};

#endif