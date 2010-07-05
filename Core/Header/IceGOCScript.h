
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceScriptUser.h"
#include "IceGOCEditorInterface.h"
#include "IceGameObject.h"

namespace Ice
{
	class GOCScript : public GOComponent, public GOCStaticEditorInterface, public ScriptUser
	{
	public:
		GOCScript() {}
		~GOCScript() {}

		int GetThisID() { IceAssert(mOwnerGO); return mOwnerGO->GetID(); }
		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "Script"; return name; }

		BEGIN_GOCEDITORINTERFACE(GOCScript, "Script")
			PROPERTY_STRING(mScriptFileName, "Script File", ".lua")
		END_GOCEDITORINTERFACE
		void OnSetParameters() { InitScript(mScriptFileName); }

		std::string& TellName() { static std::string name = "Script"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Script"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCScript; };
		void Save(LoadSave::SaveSystem& mgr) { mgr.SaveAtom("std::string", &mScriptFileName, "ScriptFileName"); }
		void Load(LoadSave::LoadSystem& mgr) { mgr.LoadAtom("std::string", &mScriptFileName); }
	};
}