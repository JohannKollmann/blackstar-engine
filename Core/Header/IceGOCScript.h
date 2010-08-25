
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceScriptUser.h"
#include "IceGOCEditorInterface.h"
#include "IceGameObject.h"
#include "IceGOCScriptMakros.h"

namespace Ice
{

	class DllExport GOCScript : public GOComponent, public GOCStaticEditorInterface, public ScriptUser
	{
	public:
		GOCScript() {}
		~GOCScript() {}

		int GetThisID() { IceAssert(mOwnerGO); return mOwnerGO->GetID(); }
		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "Script"; return name; }

		BEGIN_GOCEDITORINTERFACE(GOCScript, "Script")
			PROPERTY_STRING(mScriptFileName, "Script File", ".lua")
		END_GOCEDITORINTERFACE
		void OnSetParameters();

		void SetOwner(GameObject *go);

		std::vector<ScriptParam> RunFunction(Script& caller, std::vector<ScriptParam> &vParams);
		DEFINE_TYPEDGOCLUAMETHOD(GOCScript, RunFunction, "string");

		std::string& TellName() { static std::string name = "Script"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Script"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return ICE_NEW GOCScript; };
		void Save(LoadSave::SaveSystem& mgr) { mgr.SaveAtom("std::string", &mScriptFileName, "ScriptFileName"); }
		void Load(LoadSave::LoadSystem& mgr) { mgr.LoadAtom("std::string", &mScriptFileName); }
	};

	class DllExport GOCScriptMessageCallback : public GOComponent, public MessageListener
	{
	private:
		std::map< Ogre::String, std::vector<ScriptParam> > mObjectMsgCallbacks;

	public:
		GOCScriptMessageCallback();
		~GOCScriptMessageCallback() {}

		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "ScriptCallback"; return name; }

		void ReceiveMessage(Msg &msg);
		void ReceiveObjectMessage(Msg &msg);

		bool HasListener(const Ogre::String &msgType);
		void AddListener(const Ogre::String &msgType, ScriptParam callback);

		std::string& TellName() { static std::string name = "ScriptCallback"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ScriptCallback"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return ICE_NEW GOCScriptMessageCallback; };
		void Save(LoadSave::SaveSystem& mgr) {}
		void Load(LoadSave::LoadSystem& mgr) {}
	};
}