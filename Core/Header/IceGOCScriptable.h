
#pragma once

#include "IceIncludes.h"
#include "IceScriptable.h"
#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"

namespace Ice
{

	class DllExport GOCScriptable : public GOComponent, public GOCStaticEditorInterface
	{
	protected:
		Ogre::String mScriptFileName;

	public:
		GOCScriptable(void);
		~GOCScriptable(void);

		virtual goc_id_type& GetComponentID() const { static std::string name = "Scriptable"; return name; }
		goc_id_type& GetFamilyID() const { static std::string name = "Scriptable"; return name; }

		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Scriptable"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCScriptable; };

		BEGIN_GOCEDITORINTERFACE(GOCScriptable, "Scriptable")
			PROPERTY_STRING(mScriptFileName, "ScriptFile", "Test")
		END_GOCEDITORINTERFACE
	};

}
	