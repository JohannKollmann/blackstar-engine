
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceScriptUser.h"
#include "IceGOCEditorInterface.h"
#include "IceGameObject.h"
#include "IceGOCScriptMakros.h"

namespace Ice
{

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

		std::map<Ogre::String, ScriptParam> mScriptProperties;
		DataMap mTempMap;//HACK while save system can't turn off record references for child objects

	public:
		GOCScript() {}
		~GOCScript() {}

		void Create();

		int GetThisID() { GameObjectPtr owner = mOwnerGO.lock(); IceAssert(owner.get()); return owner->GetID(); }
		GOComponent::TypeID& GetComponentID() const { static std::string name = "Script"; return name; }

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Script"; }
		GOComponent* GetGOComponent() { return this; }
		GOCScript* New() { return new GOCScript(); }

		void SetOwner(std::weak_ptr<GameObject> go);

		void NotifyPostInit() { Create(); }

		std::vector<ScriptParam> Script_SetProperty(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Script_GetProperty(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Script_HasProperty(Script& caller, std::vector<ScriptParam> &vParams);
		DEFINE_TYPEDGOCLUAMETHOD(GOCScript, Script_SetProperty, "string")
		DEFINE_TYPEDGOCLUAMETHOD(GOCScript, Script_GetProperty, "string")
		DEFINE_TYPEDGOCLUAMETHOD(GOCScript, Script_HasProperty, "string")

		std::string& TellName() { static std::string name = "Script"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Script"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCScript; };
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