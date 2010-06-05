
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
	private:
		bool mInitedRefParams;
	public:
		struct RefParam
		{
			void* target;
			GenericProperty defaultVal;
		};
	protected:
		std::vector<RefParam> mRefParams;
		virtual void _initRefParams() {}
		virtual void OnSetParameters() {}

	public:
		GOCStaticEditorInterface() : mInitedRefParams(false) {}
		virtual ~GOCStaticEditorInterface() {}

		void GetDefaultParameters(DataMap *parameters);
		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
	};

	#define BEGIN_GOCEDITORINTERFACE(className, labelName) public: \
	GOComponent* GetGOComponent() { return this; } \
	GOCEditorInterface* New() { return new className##(); } \
	Ogre::String GetLabel() { return labelName; } \
	protected: \
	void _initRefParams() { mRefParams.clear(); GOCStaticEditorInterface::RefParam rp;

	#define PROPERTY_BOOL(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.defaultVal.Set<bool>(defaultValue, key); mRefParams.push_back(rp);
	#define PROPERTY_INT(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.defaultVal.Set<int>(defaultValue, key); mRefParams.push_back(rp);
	#define PROPERTY_FLOAT(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.defaultVal.Set<float>(defaultValue, key); mRefParams.push_back(rp);
	#define PROPERTY_STRING(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.defaultVal.Set<Ogre::String>(defaultValue, key); mRefParams.push_back(rp);
	#define PROPERTY_VECTOR3(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.defaultVal.Set<Ogre::Vector3>(defaultValue, key); mRefParams.push_back(rp);
	#define PROPERTY_QUATERNION(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.defaultVal.Set<Ogre::Quaternion>(defaultValue, key); mRefParams.push_back(rp);

	#define END_GOCEDITORINTERFACE } \
	public:

};