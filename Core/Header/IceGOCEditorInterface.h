
#pragma once

#include "IceDataMap.h"
#include "IceIncludes.h"
#include "OgreSharedPtr.h"

namespace Ice
{
	/**
	Specifies an interface how objects can be manipulated using a DataMap.
	@remarks This interface is mainly for editors with property grids.
	*/
	class DllExport EditorInterface
	{
	public:
		virtual ~EditorInterface() {}

		virtual void SetParameters(DataMap *parameters) {};
		virtual void GetParameters(DataMap *parameters) {};
	};

	template<class T>
	class DllExport Cloneable
	{
	public:
		virtual ~Cloneable() {}
		virtual T* New() = 0;
	};

	/**
	Editor interface for components.
	@remarks It implements the cloneable interface because the SceneManager uses prototyping for component creation.
	*/
	class DllExport GOCEditorInterface : public EditorInterface, public Cloneable<GOCEditorInterface>
	{
	public:
		virtual ~GOCEditorInterface() {}

		virtual void GetDefaultParameters(DataMap *parameters) {};

		/**
		@return The name of the Component, intended to be displayed in the editor.
		@remarks The label name must be unique.
		*/
		virtual Ogre::String GetLabel() = 0;

		/**
		@return The component assciated with the Editor Interface.
		@remarks In most cases, the implementation "return this;" will be suitable.
		*/
		virtual GOComponent* GetGOComponent() = 0;
	};

	typedef DllExport std::shared_ptr<GOCEditorInterface> GOCEditorInterfacePtr;

	/**
	This class allows components to declare member variables as modifiable using macros.
	*/
	class DllExport GOCStaticEditorInterface : public GOCEditorInterface
	{
	private:
		bool mInitedRefParams;
	public:
		struct RefParam
		{
			void* target;
			GenericProperty defaultVal;
			Ogre::String keyname;
		};
	protected:
		std::vector<RefParam> mRefParams;

		/**
		Setups member variable bindings and default values.
		@remarks
			Do not overload this method manually, use the macros!
		*/
		virtual void _initRefParams() {}

		/**
		Called after member variables were changed using the editor interface.
		*/
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

	#define PROPERTY_BOOL(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.keyname = key; rp.defaultVal.Set<bool>(defaultValue); mRefParams.push_back(rp);
	#define PROPERTY_INT(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.keyname = key; rp.defaultVal.Set<int>(defaultValue); mRefParams.push_back(rp);
	#define PROPERTY_FLOAT(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.keyname = key; rp.defaultVal.Set<float>(defaultValue); mRefParams.push_back(rp);
	#define PROPERTY_STRING(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.keyname = key; rp.defaultVal.Set<Ogre::String>(defaultValue); mRefParams.push_back(rp);
	#define PROPERTY_VECTOR3(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.keyname = key; rp.defaultVal.Set<Ogre::Vector3>(defaultValue); mRefParams.push_back(rp);
	#define PROPERTY_QUATERNION(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.keyname = key; rp.defaultVal.Set<Ogre::Quaternion>(defaultValue); mRefParams.push_back(rp);
	#define PROPERTY_ENUM(memberVariable, key, defaultValue) rp.target = &##memberVariable; rp.keyname = key; rp.defaultVal.Set<Ice::DataMap::Enum>(defaultValue); mRefParams.push_back(rp);

	#define END_GOCEDITORINTERFACE } \
	public:

};