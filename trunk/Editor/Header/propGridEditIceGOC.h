
#pragma once

#include "EDTIncludes.h"
#include "propGridEditIceEditorInterface.h"
#include "IceGameObject.h"
#include "LoadSave.h"
#include "wx/msw/winundef.h"


class ComponentSection : public LoadSave::Saveable
{
private:
	std::string m_strName;
public:
	ComponentSection() { m_strName = "ComponentSection"; }
	Ogre::String mSectionName;
	Ice::DataMap mSectionData;

	void Save(LoadSave::SaveSystem& myManager)
	{
		myManager.SaveAtom("Ogre::String", (void*)&mSectionName, "Key");
		myManager.SaveObject(&mSectionData, "Value", false, false);	
	}
	void Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mSectionName);
		mSectionData = (*mgr.LoadTypedObject<Ice::DataMap>().get());
		mSectionData.HasNext();
	};
	static LoadSave::Saveable* NewInstance() { return new ComponentSection; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ComponentSection"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	std::string& TellName() { return m_strName; };
};

typedef std::shared_ptr<ComponentSection> ComponentSectionPtr;

/*class ComponentSectionVectorHandler : LoadSave::AtomHandler 
{ 
public: 
	ComponentSectionVectorHandler (){m_strName= std::string("std::vector<") + "ComponentSection" + std::string(">");} 
	std::string& TellName(){return m_strName;} 
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName); 
	void Load(LoadSave::LoadSystem& ls, void* pDest); 
private: 
	std::string m_strName; 
};*/ 
class wxEditGOCSections : public wxEditIceDataMap
{
public:
	wxEditGOCSections() {};
	virtual ~wxEditGOCSections() { };

	virtual void OnLeave();
	virtual void OnActivate();

	void GetGOCSections(std::vector<ComponentSectionPtr> &sections);
	void AddGOCSection(Ogre::String name, Ice::DataMap &map, bool expand = false);
	void RemoveGOCSection(Ogre::String name);

	bool OnDropText(const wxString& text);
};

class wxEditIceGameObject : public wxEditGOCSections
{
private:
	Ice::GameObjectPtr mGameObject;

public:
	wxEditIceGameObject();
	virtual ~wxEditIceGameObject() { };

	void SetObject(Ice::GameObjectPtr object, bool update_ui = true);
	Ice::GameObjectPtr GetGameObject();

	void OnUpdate();
	void OnApply();
	
};

class wxEditGOResource : public wxEditGOCSections
{
private:
	Ogre::String mCurrentEditPath;

public:
	void SetResource(Ogre::String savepath);
	void NewResource(Ogre::String savepath, bool showcomponentbar = true);

	void OnApply();
};
