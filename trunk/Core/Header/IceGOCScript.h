
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceScriptUser.h"
#include "IceGOCEditorInterface.h"
#include "IceGameObject.h"
#include "IceGOCScriptMakros.h"

namespace Ice
{
	/**
	Provides a map of scriptable object properties.
	*/
	class DllExport GOCScriptedProperties : public GOComponent, public GOCEditorInterface
	{
	protected:
		std::unordered_map<Ogre::String, ScriptParam> mScriptProperties;

		DataMap mTempMap;	//HACK while save system can't turn off record references for child objects

	public:
		GOCScriptedProperties() {}
		~GOCScriptedProperties() {}

		GOComponent::TypeID& GetComponentID() const { static std::string name = "ScriptedProperties"; return name; }

		void SetProperty(const Ogre::String &propertyName, const ScriptParam &prop);
		void GetProperty(const Ogre::String &propertyName, ScriptParam &prop);
		bool HasProperty(const Ogre::String &propertyName);

		//Editor interface
		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Scripted Properties"; }
		GOComponent* GetGOComponent() { return this; }
		GOCEditorInterface* New() { return new GOCScriptedProperties(); }

		//Loadsave
		std::string& TellName() { static std::string name = "ScriptedProperties"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ScriptedProperties"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCScriptedProperties(); };
		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
	};

	class DllExport GOCScript : public GOComponent, public GOCEditorInterface
	{
	private:
		Ogre::String mScriptFileNames;

		class ScriptItem : public ScriptUser
		{
		private:
			GOCScript *mGOCScript;
		public:
			ScriptItem(GOCScript *gocs, Ogre::String scriptName) : mGOCScript(gocs) { InitScript(scriptName); }
			int GetThisID() { return mGOCScript->GetThisID(); }
		};

		std::vector<std::shared_ptr<ScriptItem>> mScripts;

	public:
		GOCScript() {}
		~GOCScript() {}

		GOComponent::TypeID& GetComponentID() const { static std::string name = "Script"; return name; }

		void Create();

		int GetThisID() { GameObjectPtr owner = mOwnerGO.lock(); IceAssert(owner.get()); return owner->GetID(); }

		void NotifyPostInit() { Create(); }

		//Editor interface
		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Script"; }
		GOComponent* GetGOComponent() { return this; }
		GOCEditorInterface* New() { return new GOCScript(); }

		//Loadsave
		std::string& TellName() { static std::string name = "Script"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Script"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCScript(); };
		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
	};

	class DllExport GOCScriptMessageCallback : public GOComponent
	{
	private:
		std::map< MsgTypeID, std::vector<ScriptParam> > mObjectMsgCallbacks;

	public:
		GOCScriptMessageCallback();
		~GOCScriptMessageCallback() {}

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_ALL; }

		GOComponent::TypeID& GetComponentID() const { static std::string name = "ScriptCallback"; return name; }

		void ReceiveMessage(Msg &msg);

		bool HasListener(MsgTypeID msgType);
		void AddListener(MsgTypeID msgType, ScriptParam callback);

		std::string& TellName() { static std::string name = "ScriptCallback"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ScriptCallback"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCScriptMessageCallback; };
		void Save(LoadSave::SaveSystem& mgr) {}
		void Load(LoadSave::LoadSystem& mgr) {}
	};
}