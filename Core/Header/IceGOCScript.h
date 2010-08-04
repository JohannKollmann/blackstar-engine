
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
	private:
		std::map<Ogre::String, ScriptParam> mObjectMsgCallbacks;

	public:
		GOCScript();
		~GOCScript() {}

		int GetThisID() { IceAssert(mOwnerGO); return mOwnerGO->GetID(); }
		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "Script"; return name; }

		BEGIN_GOCEDITORINTERFACE(GOCScript, "Script")
			PROPERTY_STRING(mScriptFileName, "Script File", ".lua")
		END_GOCEDITORINTERFACE
		void OnSetParameters() { InitScript(mScriptFileName); }

		static std::vector<ScriptParam> Lua_ListenToObjectEvent(Script &caller, std::vector<ScriptParam> params);

		void ReceiveObjectMessage(Msg &msg);
		void OnReceiveMessage(Msg &msg);

		std::string& TellName() { static std::string name = "Script"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Script"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCScript; };
		void Save(LoadSave::SaveSystem& mgr) { mgr.SaveAtom("std::string", &mScriptFileName, "ScriptFileName"); }
		void Load(LoadSave::LoadSystem& mgr) { mgr.LoadAtom("std::string", &mScriptFileName); }
	};
}