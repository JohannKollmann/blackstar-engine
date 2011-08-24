#include "IceDataMap.h"

namespace Ice
{

	GenericProperty::PropertyTypes GenericProperty::getType() const
	{
		if (mData.getType() == typeid(bool)) return PropertyTypes::BOOL;
		if (mData.getType() == typeid(float)) return PropertyTypes::FLOAT;
		if (mData.getType() == typeid(int)) return PropertyTypes::INT;
		if (mData.getType() == typeid(Ogre::String)) return PropertyTypes::STRING;
		if (mData.getType() == typeid(Ogre::Vector3)) return PropertyTypes::VECTOR3;
		if (mData.getType() == typeid(Ogre::Quaternion)) return PropertyTypes::QUATERNION;
		if (mData.getType() == typeid(DataMap::Enum)) return PropertyTypes::ENUM;
		
		IceAssert(false);
		return PropertyTypes::INT;
	}
	GenericProperty::PropertyTypes GenericProperty::Get(void *target) const
	{
		if (mData.getType() == typeid(bool)) { bool src = Ogre::any_cast<bool>(mData); memcpy(target, &src, sizeof(bool)); return PropertyTypes::BOOL; }
		if (mData.getType() == typeid(float)) { float src = Ogre::any_cast<float>(mData); *((float*)target) = src; return PropertyTypes::FLOAT; }
		if (mData.getType() == typeid(int)) { int src = Ogre::any_cast<int>(mData); *((int*)target) = src; return PropertyTypes::INT; }
		if (mData.getType() == typeid(Ogre::String)) { Ogre::String src = Ogre::any_cast<Ogre::String>(mData); *((Ogre::String*)target) = src; return PropertyTypes::STRING; }
		if (mData.getType() == typeid(Ogre::Vector3)) { Ogre::Vector3 src = Ogre::any_cast<Ogre::Vector3>(mData); *((Ogre::Vector3*)target) = src;  return PropertyTypes::VECTOR3; }
		if (mData.getType() == typeid(Ogre::Quaternion)) { Ogre::Quaternion src = Ogre::any_cast<Ogre::Quaternion>(mData); *((Ogre::Quaternion*)target) = src; return PropertyTypes::QUATERNION; }
		if (mData.getType() == typeid(DataMap::Enum)) { DataMap::Enum src = Ogre::any_cast<DataMap::Enum>(mData); *((DataMap::Enum*)target) = src; return PropertyTypes::ENUM; }
		
		IceAssert(false);
		return PropertyTypes::INT;
	}

	void GenericProperty::Set(void *data, const GenericProperty::PropertyTypes &type)
	{
		switch (type)
		{
		case PropertyTypes::INT:
			Set<int>(*((int*)data)); break;
		case PropertyTypes::BOOL:
			Set<bool>(*((bool*)data)); break;
		case PropertyTypes::FLOAT:
			Set<float>(*((float*)data)); break;
		case PropertyTypes::STRING:
			Set<Ogre::String>(*((Ogre::String*)data)); break;
		case PropertyTypes::VECTOR3:
			Set<Ogre::Vector3>(*((Ogre::Vector3*)data)); break;
		case PropertyTypes::QUATERNION:
			Set<Ogre::Quaternion>(*((Ogre::Quaternion*)data)); break;
		case PropertyTypes::ENUM:
			Set<DataMap::Enum>(*((DataMap::Enum*)data)); break;
		}
	}

	void GenericProperty::Set(const ScriptParam &scriptParam)
	{
		switch(scriptParam.getType())
		{
			case ScriptParam::PARM_TYPE_BOOL:
				mData = scriptParam.getBool();
				break;
			case ScriptParam::PARM_TYPE_FLOAT:
				mData = static_cast<float>(scriptParam.getFloat());
				break;
			case ScriptParam::PARM_TYPE_INT:
				mData = scriptParam.getInt();
				break;
			case ScriptParam::PARM_TYPE_STRING:
				mData = scriptParam.getString();
				break;
			//more tricky params: vectors
			case ScriptParam::PARM_TYPE_TABLE:
			{
				std::map<ScriptParam, ScriptParam> mTable=scriptParam.getTable();
				std::map<ScriptParam, ScriptParam>::const_iterator it;
				float aCoords[4];
				std::string aChars[4]={"x", "y", "z", "w"};
				
				int i=0;
				for(; i<4; i++)
				{
					bool bFound=false;
					if((it=mTable.find(ScriptParam(aChars[i])))!=mTable.end())
						if(it->second.getType()==ScriptParam::PARM_TYPE_FLOAT)
						{
							aCoords[i]=it->second.getFloat();
							bFound=true;
						}
					if(!bFound)
						break;
				}
				bool bSetData=true;
				switch(i)
				{
					case 3:
						Set<Ogre::Vector3>(Ogre::Vector3(aCoords[0],aCoords[1],aCoords[2]));
						break;
					case 4:
						Set<Ogre::Quaternion>(Ogre::Quaternion(aCoords[3],aCoords[0],aCoords[1],aCoords[2]));
						break;
					default:
						bSetData=false;
				}
				if(bSetData)
					break;
				//else fall through
			}
			default:
				IceWarning("Could not convert ScriptParam to GenericProperty!");
		}
	}

	void GenericProperty::Save(LoadSave::SaveSystem& myManager)
	{
		int type = getType();
		myManager.SaveAtom("int", &type, "Type");

		int iData; bool bData; float fData; Ogre::String sData; Ogre::Vector3 vData; Ogre::Quaternion qData; DataMap::Enum eData;

		switch (type)
		{
		case PropertyTypes::INT:
			iData = Ogre::any_cast<int>(mData);
			myManager.SaveAtom("int", (void*)&iData, "Data");
			break;
		case PropertyTypes::BOOL:
			bData = Ogre::any_cast<bool>(mData);
			myManager.SaveAtom("bool", (void*)&bData, "Data");
			break;
		case PropertyTypes::FLOAT:
			fData = Ogre::any_cast<float>(mData);
			myManager.SaveAtom("float", (void*)&fData, "Data");
			break;
		case PropertyTypes::STRING:
			sData = Ogre::any_cast<Ogre::String>(mData);
			myManager.SaveAtom("Ogre::String", (void*)&sData, "Data");
			break;
		case PropertyTypes::VECTOR3:
			vData = Ogre::any_cast<Ogre::Vector3>(mData);
			myManager.SaveAtom("Ogre::Vector3", (void*)&vData, "Data");
			break;
		case PropertyTypes::QUATERNION:
			qData = Ogre::any_cast<Ogre::Quaternion>(mData);
			myManager.SaveAtom("Ogre::Quaternion", (void*)&qData, "Data");
			break;
		case PropertyTypes::ENUM:
			eData = Ogre::any_cast<DataMap::Enum>(mData);
			myManager.SaveAtom("Ogre::String", (void*)&eData.toString(), "Data");
			break;
		}
	}

	void GenericProperty::Load(LoadSave::LoadSystem& mgr)
	{
		int type = 0;
		mgr.LoadAtom("int", &type);
		int iVal = 0;
		bool bVal = false;
		float fVal = 0.0f;
		Ogre::String sVal;
		Ogre::Vector3 vVal;
		Ogre::Quaternion qVal;
		DataMap::Enum eData;
		switch (type)
		{
		case PropertyTypes::INT:
			mgr.LoadAtom("int", &iVal);
			mData = iVal;
			break;
		case PropertyTypes::BOOL:
			mgr.LoadAtom("bool", &bVal);
			mData = bVal;
			break;
		case PropertyTypes::FLOAT:
			mgr.LoadAtom("float", &fVal);
			mData = fVal;
			break;
		case PropertyTypes::STRING:
			mgr.LoadAtom("Ogre::String", &sVal);
			mData = sVal;
			break;
		case PropertyTypes::VECTOR3:
			mgr.LoadAtom("Ogre::Vector3", &vVal);
			mData = vVal;
			break;
		case PropertyTypes::QUATERNION:
			mgr.LoadAtom("Ogre::Quaternion", &qVal);
			mData = qVal;
			break;
		case PropertyTypes::ENUM:
			mgr.LoadAtom("Ogre::String", &sVal);
			eData.fromString(sVal);
			mData = eData;
			break;
		}
	}


	ScriptParam GenericProperty::GetAsScriptParam()
	{
		int type = getType();
		std::map<ScriptParam, ScriptParam> mTable;
		switch (type)
		{
		case PropertyTypes::INT:
			return ScriptParam(Get<int>());
		case PropertyTypes::BOOL:
			return ScriptParam(Get<bool>());
		case PropertyTypes::FLOAT:
			return ScriptParam(Get<float>());
		case PropertyTypes::STRING:
			return ScriptParam(std::string(Get<Ogre::String>().c_str()));
		case PropertyTypes::VECTOR3:
			mTable[ScriptParam(std::string("x"))]=ScriptParam(Get<Ogre::Vector3>().x);
			mTable[ScriptParam(std::string("y"))]=ScriptParam(Get<Ogre::Vector3>().y);
			mTable[ScriptParam(std::string("z"))]=ScriptParam(Get<Ogre::Vector3>().z);
			return ScriptParam(mTable);
		case PropertyTypes::QUATERNION:
			mTable[ScriptParam(std::string("w"))]=ScriptParam(Get<Ogre::Quaternion>().w);
			mTable[ScriptParam(std::string("x"))]=ScriptParam(Get<Ogre::Quaternion>().x);
			mTable[ScriptParam(std::string("y"))]=ScriptParam(Get<Ogre::Quaternion>().y);
			mTable[ScriptParam(std::string("z"))]=ScriptParam(Get<Ogre::Quaternion>().z);
			return ScriptParam(mTable);
		case PropertyTypes::ENUM:
			return ScriptParam(static_cast<int>(Get<DataMap::Enum>().selection));
			break;
		}
	}

	DataMap::DataMap()
	{
		mIterator = mItems.begin();
	}

	DataMap::DataMap(const DataMap &rhs)
	{
		mItems = std::vector< std::shared_ptr<Item> >(rhs.mItems);
		mIterator = mItems.begin();
	}

	DataMap::~DataMap()
	{
		mItems.clear();
	}

	void DataMap::ParseString(Ogre::String &keyValues)
	{
		Ogre::String type;
		Ogre::String key;
		Ogre::String value;
		int readingState = 0;
		for (unsigned int i = 0; i < keyValues.length(); i++)
		{
			if (readingState == 0)
			{
				if (keyValues[i] == ' ' && type != "")
				{
					readingState = 1;
				}
				else if (keyValues[i] != ' ') type = type + keyValues[i];
			}
			else if (readingState == 1)
			{
				if (keyValues[i] == '=')
				{
					if (key == "")
					{
						IceWarning("Key must not be empty!")
						return;
					}
					readingState = 2;
				}
				else key = key + keyValues[i];
			}
			else if (readingState == 2)
			{
				if (keyValues[i] == ';') 
				{
					Ogre::StringUtil::trim(type);
					Ogre::StringUtil::trim(key);
					Ogre::StringUtil::trim(value);
					if (type == "int") AddInt(key, Ogre::StringConverter::parseInt(value));
					else if (type == "float") AddFloat(key, Ogre::StringConverter::parseReal(value));
					else if (type == "bool") AddBool(key, Ogre::StringConverter::parseBool(value));
					else if (type == "string") AddOgreString(key, value);
					else if (type == "vector3") AddOgreVec3(key, Ogre::StringConverter::parseVector3(value));
					else if (type == "quat") AddOgreQuat(key, Ogre::StringConverter::parseQuaternion(value));
					else if (type == "enum") AddEnum(key, std::vector<std::string>(), Ogre::StringConverter::parseInt(value));
					else IceWarning("Unknown data type: " + type)
					readingState = 0;
					type = "";
					key = "";
					value = "";
				}
				else value = value + keyValues[i];
			}
		}
	}

	bool DataMap::HasKey(const Ogre::String &keyname)
	{
		for (auto i = mItems.begin(); i != mItems.end(); i++)
		{
			if ((*i)->Key == keyname) return true;
		}
		return false;
	}

	int DataMap::GetInt(const Ogre::String &keyname, int defaultVal) const
	{
		return GetValue<int>(keyname, defaultVal);
	}

	bool DataMap::GetBool(const Ogre::String &keyname, bool defaultVal) const
	{
		return GetValue<bool>(keyname, defaultVal);
	}

	float DataMap::GetFloat(const Ogre::String &keyname, float defaultVal) const
	{
		return GetValue<float>(keyname, defaultVal);
	}

	Ogre::Vector3 DataMap::GetOgreVec3(const Ogre::String &keyname, Ogre::Vector3 defaultVal) const
	{
		return GetValue<Ogre::Vector3>(keyname, defaultVal);
	}

	Ogre::ColourValue DataMap::GetOgreCol(const Ogre::String &keyname, Ogre::ColourValue defaultVal) const
	{
		Ogre::Vector3 vec = GetValue<Ogre::Vector3>(keyname, Ogre::Vector3(defaultVal.r, defaultVal.g, defaultVal.b));
		return Ogre::ColourValue(vec.x, vec.y, vec.z);
	}

	Ogre::Quaternion DataMap::GetOgreQuat(const Ogre::String &keyname, Ogre::Quaternion defaultVal) const
	{
		return GetValue<Ogre::Quaternion>(keyname, defaultVal);
	}

	Ogre::String DataMap::GetOgreString(const Ogre::String &keyname, Ogre::String defaultVal) const
	{
		return GetValue<Ogre::String>(keyname, defaultVal);
	}

	DataMap::Enum DataMap::GetEnum(const Ogre::String &keyname, DataMap::Enum defaultVal)	const
	{
		return GetValue<Enum>(keyname, defaultVal);
	}

	Ogre::String DataMap::Enum::toString() const
	{
		Ogre::String coded_enum = Ogre::StringConverter::toString(selection) + " ";
		for (std::vector<Ogre::String>::const_iterator i = choices.begin(); i != choices.end(); i++) coded_enum += (*i + " ");
		return coded_enum;
	}
	void DataMap::Enum::fromString(Ogre::String coded_enum)
	{
		selection = Ogre::StringConverter::parseInt(coded_enum.substr(0, 1));
		coded_enum = coded_enum.substr(2, coded_enum.size());
		while (!coded_enum.empty())
		{
			choices.push_back(coded_enum.substr(0, coded_enum.find(" ")));
			coded_enum = coded_enum.substr(coded_enum.find(" ")+1, coded_enum.size());
		}
	}


	void DataMap::AddBool(const Ogre::String &keyname, const bool val)
	{
		AddItem(keyname, val);
	}


	void DataMap::AddInt(const Ogre::String &keyname, const int val)
	{
		AddItem(keyname, val);
	}

	void DataMap::AddFloat(const Ogre::String &keyname, const float val)
	{
		AddItem(keyname, val);
	}

	void DataMap::AddOgreVec3(const Ogre::String &keyname, const Ogre::Vector3 &vec)
	{
		AddItem(keyname, vec);
	}

	void DataMap::AddOgreCol(const Ogre::String &keyname, const Ogre::ColourValue &val)
	{
		AddOgreVec3(keyname, Ogre::Vector3(val.r, val.g, val.b));		//Hack: Add ad Vec3
	}

	void DataMap::AddOgreQuat(const Ogre::String &keyname, const Ogre::Quaternion &quat)
	{
		AddItem(keyname, quat);
	}

	void DataMap::AddOgreString(const Ogre::String &keyname, const Ogre::String &text)
	{
		AddItem<Ogre::String>(keyname, text);
	}

	void DataMap::AddEnum(const Ogre::String &keyname, std::vector<Ogre::String> choices, unsigned int selection)
	{
		//hack: code it as Ogre string
		Enum e; e.choices = choices; e.selection = selection;
		AddItem<Enum>(keyname, e);
		//AddOgreString(keyname, e.toString());
	}

	void DataMap::AddScriptParam(const Ogre::String &keyname, const ScriptParam &param)
	{
		GenericProperty gp;
		gp.Set(param);
		AddItem(keyname, gp);
	}

	bool DataMap::HasNext()
	{
		if (mIterator != mItems.end()) return true;
		mIterator = mItems.begin();
		return false;
	}

	DataMap::Item DataMap::GetNext()
	{
		Item returner = *(*mIterator).get();
		mIterator++;
		return returner;
	}

	DataMap::Item::Item()
	{
		Data = std::make_shared<GenericProperty>();
	}

	void DataMap::Item::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", &Key, "key");
		mgr.SaveObject(Data.get(), "Data");
	}
	void DataMap::Item::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &Key);
		Data = mgr.LoadTypedObject<GenericProperty>();
	}

	void DataMap::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("vector<DataMapItemPtr>", &mItems, "Items");
	}

	void DataMap::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("vector<DataMapItemPtr>", &mItems);
		mIterator = mItems.begin();
	}

};
