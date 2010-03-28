
#pragma once

/*
Interface for Editors.
*/

#include "IceDataMap.h"
#include "IceIncludes.h"

namespace Ice
{
	class EditorInterface
	{
	public:
		virtual ~EditorInterface() {}
		virtual void CreateFromDataMap(DataMap *parameters) = 0;
		virtual void GetParameters(DataMap *parameters) = 0;
		virtual Ogre::String GetLabel() = 0;
		virtual void* GetUserData() { return 0; }
		virtual void InjectUserData(void* data) { }

		static EditorInterface* NewEditorInterfaceInstance();
	};

	class GOCEditorInterface : public EditorInterface
	{
	public:
		virtual ~GOCEditorInterface() {}

		/*
		Attaches the editor interface to a GO and replaces the existing component width the same family, if it exists!
		*/
		virtual void AttachToGO(GameObject *go) = 0;

		static GOCEditorInterface* NewEditorInterfaceInstance();
	};

};