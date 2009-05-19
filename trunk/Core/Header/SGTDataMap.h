
#ifndef __SGTDataMap_H__
#define __SGTDataMap_H__

#include <vector>
#include "SGTIncludes.h"
#include "Ogre.h"
#include "SGTLoadSave.h"
#include "boost/any.hpp"

/*
Klasse zum sicheren und komfortablen Benutzen von std::map<Ogre::String, void*>
*/

class SGTDllExport SGTDataMapEntry 
{
public:
	SGTDataMapEntry();
	~SGTDataMapEntry();
	Ogre::SharedPtr<BYTE> mData;
	Ogre::String mType;
	Ogre::String mKey;
};

class SGTDllExport SGTDataMap : public SGTSaveable
{
private:
	std::vector<SGTDataMapEntry> mData;
	std::vector<SGTDataMapEntry>::iterator mIterator;

public:
	SGTDataMap();
	SGTDataMap(const SGTDataMap &rhs);
	~SGTDataMap();

	bool HasNext();
	SGTDataMapEntry GetNext();
	SGTDataMapEntry* GetNextPtr();

	bool HasKey(Ogre::String keyname);

	void* GetData(Ogre::String keyname);

	int GetInt(Ogre::String keyname);
	bool GetBool(Ogre::String keyname);
	float GetFloat(Ogre::String keyname);
	Ogre::Vector3 GetOgreVec3(Ogre::String keyname);
	Ogre::Quaternion GetOgreQuat(Ogre::String keyname);
	Ogre::String GetOgreString(Ogre::String keyname);


	void AddData(Ogre::String keyname, Ogre::String type, BYTE* data);

	/*
	Template zum speichern von Basistypen.
	*/
	template <class templateType>
		void AddValue(Ogre::String keyname, Ogre::String type, templateType var)
		{
			/*
			AddData(keyname, (void*)(&var));
			Würde die Adresse der übergeben Variable speichern. In der Regel will man aber eine Kopie des Wertes
			speichern und keinen Zeiger auf den Wert selbst. Deswegen wird memcpy benutzt.
			*/
			templateType* data = new templateType;
			*data = var;
			//memcpy(data, &var, sizeof(var));
			AddData(keyname, type, (BYTE*)(data));
		};
	void AddBool(Ogre::String keyname, bool val);
	void AddInt(Ogre::String keyname, int val);
	void AddFloat(Ogre::String keyname, float val);
	void AddOgreVec3(Ogre::String keyname, Ogre::Vector3 vec);
	void AddOgreQuat(Ogre::String keyname, Ogre::Quaternion quat);
	void AddOgreString(Ogre::String keyname, Ogre::String text);

	//Load Save
	void SaveContent(SGTSaveSystem& myManager);
	void LoadContent(SGTLoadSystem& mgr); 
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