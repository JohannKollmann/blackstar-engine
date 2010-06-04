
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

	class DllExport GOCStaticEditorInterface : public GOCEditorInterface
	{
	public:
		struct RefParam
		{
			void* target;
			GenericProperty defaultVal;
		};
	protected:
		std::vector<RefParam> mRefParams;
		virtual void _initRefParams() = 0;

	public:
		virtual ~GOCStaticEditorInterface() {}

		void GetDefaultParameters(DataMap *parameters)
		{
			for (auto i = mRefParams.begin(); i != mRefParams.end(); i++)
			{
				parameters->AddProperty(i->defaultVal);
			}
		}
		void SetParameters(DataMap *parameters)
		{
			while (parameters->HasNext())
			{
				GenericProperty gp = parameters->GetNext();
				for (auto i = mRefParams.begin(); i != mRefParams.end(); i++)
				{
					if (i->defaultVal.mKey == gp.mKey)
					{
						assert(i->defaultVal.getType() == gp.getType());
						gp.Get(i->target);
						break;
					}
				}
			}
		}
		void GetParameters(DataMap *parameters)
		{
			for (auto i = mRefParams.begin(); i != mRefParams.end(); i++)
			{
				GenericProperty::PropertyTypes type = i->defaultVal.getType();
				parameters->AddValue(i->defaultVal.mKey, i->target, i->defaultVal.getType());
			}
		}
	};

	#define BEGIN_EDITORINTERFACE void _initRefParams() { GOCStaticEditorInterface::RefParam rp;
	#define PROPERTY(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.defaultVal.Set(defaultValue, key);
	#define END_EDITORINTERFACE }

};