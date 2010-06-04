
#pragma once

#include "IceIncludes.h"
#include "IceScriptable.h"
#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"

namespace Ice
{

	class GOCScriptable : public GOComponent, public GOCStaticEditorInterface
	{
	protected:
		Ogre::String mScriptFileName;

	public:
		GOCScriptable(void);
		~GOCScriptable(void);

		virtual goc_id_type& GetComponentID() const { static std::string name = "Scriptable"; return name; }
		goc_id_type& GetFamilyID() const { static std::string name = "Scriptable"; return name; }

		GOComponent* GetGOComponent() { return this; }
		GOCEditorInterface* New() { return new GOCScriptable(); }

		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Scriptable"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCScriptable; };

		BEGIN_EDITORINTERFACE
			PROPERTY(mScriptFileName, "ScriptFile", std::string("Test"))
		END_EDITORINTERFACE
		/*void _initRefParams() { GOCStaticEditorInterface::RefParam rp;
		rp.target = &mScriptFileName; rp.defaultVal.Set(std::string(), "ScriptFile");
		}*/

		Ogre::String GetLabel() { return "Scriptable"; }
	};

}
	