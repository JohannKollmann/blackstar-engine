
#pragma once

#include <vector>
#include "IceIncludes.h"
#include "Ogre.h"
#include "LoadSave.h"
#include "boost/any.hpp"
#include "OgreAny.h"
#include "IceScriptParam.h"

namespace Ice
{

	/**
	A piece of any "basic" data. Supports int, bool, float, string, Vector3 and Quaternion
	*/
	class DllExport GenericProperty : public LoadSave::Saveable
	{
	private:
		Ogre::Any mData;

	public:
		GenericProperty() {};
		~GenericProperty() {};

		enum PropertyTypes
		{
			INT = 0,
			BOOL = 1,
			FLOAT = 2,
			STRING = 3,
			VECTOR3 = 4,
			QUATERNION = 5,
			ENUM = 6
		};

		///Retrieves the data type.
		PropertyTypes getType() const;

		///Sets the value of type T.
		template <typename T>
			void Set(const T &value) { mData = value;};
		
		///Sets the value using a void pointer and a type identifier.
		void Set(void *data, const PropertyTypes &type);

		///Converts from a ScriptParam.
		void Set(const ScriptParam &scriptParam);

		///Retrieves the saved data of type T.
		template <typename T>
			T Get() const { return Ogre::any_cast<T>(mData); }
		
		///Writes the saved data to a target buffer.
		PropertyTypes Get(void *target) const;

		///Converts the saved data to a ScriptParam.
		void GetAsScriptParam(std::vector<ScriptParam> &params) const;

		//Load Save methods
		void Save(LoadSave::SaveSystem& myManager);
		void Load(LoadSave::LoadSystem& mgr); 
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "GenericProperty"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GenericProperty; };
		std::string& TellName() { static std::string name = "GenericProperty"; return name; }
	};

	/**
	This class wraps a map<String, GenericProperty>
	*/
	class DllExport DataMap : public LoadSave::Saveable
	{
	public:
		class DllExport Item : public LoadSave::Saveable
		{
		public:
			Ogre::String key;
			GenericProperty data;

			//Load Save methods
			void Save(LoadSave::SaveSystem& myManager);
			void Load(LoadSave::LoadSystem& mgr); 
			static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "DataMapItem"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
			static LoadSave::Saveable* NewInstance() { return new Item; };
			std::string& TellName() { static std::string name = "DataMapItem"; return name; };
		};

	private:
		std::vector<Item> mData;
		std::vector<Item>::iterator mIterator;

	public:
		DataMap();
		DataMap(const DataMap &rhs);
		~DataMap();

		//Iterator methods
		bool HasNext();
		Item GetNext();
		
		//Inits the datamap from a types key-value string ("vector3 position = x y z; float weight = 8;")
		void ParseString(Ogre::String &keyValues);

		///Retrieves whether there is an item with key keyname.
		bool HasKey(const Ogre::String &keyname);

		///Retrieves the data of the item with the key keyname.
		template <class templateType>
		templateType GetValue(const Ogre::String &keyname)	const
		{
			for (auto i = mData.begin(); i != mData.end(); i++)
				if (i->key == keyname) return i->data.Get<templateType>();
			IceAssert(false)
			return templateType();
		}

		///Retrieves the data of the item with the key keyname, retungs defaultVal if it does not exist.
		template <class templateType>
		templateType GetValue(const Ogre::String &keyname, templateType defaultVal)	const
		{
			for (auto i = mData.begin(); i != mData.end(); i++)
				if (i->key == keyname) return i->data.Get<templateType>();
			return defaultVal;
		}

		//Typed get methods
		int GetInt(const Ogre::String &keyname)						const;
		bool GetBool(const Ogre::String &keyname)					const;
		float GetFloat(const Ogre::String &keyname)					const;
		Ogre::Vector3 GetOgreVec3(const Ogre::String &keyname)		const;
		Ogre::ColourValue GetOgreCol(const Ogre::String &keyname)	const;
		Ogre::Quaternion GetOgreQuat(const Ogre::String &keyname)	const;
		Ogre::String GetOgreString(const Ogre::String &keyname)		const;

		class DllExport Enum
		{
		public:
			Ogre::String toString() const;
			void fromString(Ogre::String coded_enum);
			std::vector<Ogre::String> choices;
			unsigned int selection;

			friend DllExport std::ostream& operator<<(std::ostream& o, const Enum &e)
			{
				o << e.toString();
				return o;
			}
		};
		DataMap::Enum GetEnum(const Ogre::String &keyname)			const;

		///Inserts a new item into the DataMap.
		template <class templateType>
		void AddItem(const Ogre::String &keyname, const templateType &var)
		{
			Item item;
			item.key = keyname;
			item.data.Set<templateType>(var);
			mData.push_back(item);
			mIterator = mData.begin();
		};
		///Inserts a new item into the DataMap, the data is passed using a GenericProperty.
		void AddItem(const Ogre::String &keyname, const GenericProperty &prop)
		{
			Item item;
			item.key = keyname;
			item.data = prop;
			mData.push_back(item);
			mIterator = mData.begin();
		}

		///Inserts a new item into the DataMap, the data is passed using a void buffer.
		void AddItem(const Ogre::String &keyname, void *data, const GenericProperty::PropertyTypes &type)
		{
			Item item;
			item.key = keyname;
			item.data.Set(data, type);
			mData.push_back(item);
			mIterator = mData.begin();
		};

		//Typed set methods
		void AddBool(const Ogre::String &keyname, const bool val);
		void AddInt(const Ogre::String &keyname, const int val);
		void AddFloat(const Ogre::String &keyname, const float val);
		void AddOgreVec3(const Ogre::String &keyname, const Ogre::Vector3 &vec);
		void AddOgreCol(const Ogre::String &keyname, const Ogre::ColourValue &val);
		void AddOgreQuat(const Ogre::String &keyname, const Ogre::Quaternion &quat);
		void AddOgreString(const Ogre::String &keyname, const Ogre::String &text);
		void AddEnum(const Ogre::String &keyname, std::vector<Ogre::String> choices, unsigned int selection = 0);

		void AddScriptParam(const Ogre::String &keyname, const ScriptParam &param);

		//Load Save methods
		void Save(LoadSave::SaveSystem& myManager);
		void Load(LoadSave::LoadSystem& mgr); 
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "DataMap"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new DataMap; };
		std::string& TellName() { static std::string name = "DataMap"; return name; };
	};

};