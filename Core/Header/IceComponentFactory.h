
#pragma once

#include "IceIncludes.h"
#include "IceGOCEditorInterface.h"

namespace Ice
{
	class DllExport ComponentFactory
	{
	public:
		struct DllExport GOCDefaultParams
		{
			DataMap params;
			Ogre::String exclusiveFamily;
		};

	protected:
		std::map<Ogre::String, GOCEditorInterfacePtr> mGOCPrototypes;		
		std::map<Ogre::String, std::map<Ogre::String, GOCDefaultParams > > mGOCDefaultParameters;

		ComponentFactory();
		~ComponentFactory();

	public:

		std::map<Ogre::String, std::map<Ogre::String, GOCDefaultParams > >::iterator GetDefaultParametersIteratorBegin() { return mGOCDefaultParameters.begin(); }
		std::map<Ogre::String, std::map<Ogre::String, GOCDefaultParams > >::iterator GetDefaultParametersIteratorEnd() { return mGOCDefaultParameters.end(); }

		void RegisterComponentDefaultParams(const Ogre::String &type, DataMap &params, const Ogre::String &editorRow, const Ogre::String &exclusiveFamily = "");

		void RegisterGOCPrototype(GOCEditorInterfacePtr prototype);
		void RegisterGOCPrototype(GOCEditorInterfacePtr prototype, const Ogre::String &editorRow, const Ogre::String &exclusiveFamily = "");

		///Retrieves the prototype object of a given type.
		GOCEditorInterface* GetGOCPrototype(const Ogre::String &type);

		///Creates a new goc object by cloning the matching prototype object.
		GOCEditorInterface* CreateGOC(const Ogre::String &type);

		void Shutdown()
		{
			mGOCPrototypes.clear();
			mGOCDefaultParameters.clear();
		}

		//Singleton
		static ComponentFactory& Instance();
	};
}