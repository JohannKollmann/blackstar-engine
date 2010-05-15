
#pragma once

/*
Interface for Editors.
*/

#include "IceDataMap.h"
#include "IceIncludes.h"

namespace Ice
{
	class DllExport EditorInterface
	{
	public:
		virtual ~EditorInterface() {}
		virtual void SetParameters(DataMap *parameters) = 0;
		virtual void GetParameters(DataMap *parameters) = 0;
	};

	template<class T>
	class DllExport Cloneable
	{
	public:
		virtual ~Cloneable() {}
		virtual T* New() = 0;
	};

	class DllExport GOCEditorInterface : public EditorInterface, public Cloneable<GOCEditorInterface>
	{
	public:
		virtual ~GOCEditorInterface() {}

		virtual void GetDefaultParameters(DataMap *parameters) = 0;
		virtual Ogre::String GetLabel() = 0;

		virtual GOComponent* GetGOComponent() = 0;
	};

};