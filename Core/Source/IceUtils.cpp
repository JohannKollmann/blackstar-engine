
#include "IceUtils.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "IceScript.h"
#include "IceDataMap.h"
#include "IceMain.h"
#include "OgrePhysX.h"
#include "IceCollisionCallback.h"

namespace Ice
{
	Ogre::String Utils::FindResourcePath(Ogre::String path, Ogre::String filename)
	{
		boost::filesystem::path bpath(path.c_str());
		for (boost::filesystem::directory_iterator i(bpath); i != boost::filesystem::directory_iterator(); i++)
		{
			Ogre::String file = Ogre::String((*i).path().leaf().c_str());
			if (file == filename) return (*i).path().file_string();
			if (boost::filesystem::is_directory((*i)))
			{
				Ogre::String csearch = FindResourcePath((*i).path().directory_string().c_str(), filename);
				if (csearch != "") return csearch;
			}
		}
		return "";
	}

	std::string Utils::GetTypeName(ScriptParam param)
	{
		return GetTypeName(param.getType());
	}
	
	std::string Utils::GetTypeName(int type)
	{
		switch(type)
		{
		case ScriptParam::PARM_TYPE_BOOL:
			return std::string("bool");
		case ScriptParam::PARM_TYPE_FLOAT:
			return std::string("float");
		case ScriptParam::PARM_TYPE_FUNCTION:
			return std::string("function");
		case ScriptParam::PARM_TYPE_INT:
			return std::string("int");
		case ScriptParam::PARM_TYPE_STRING:
			return std::string("string");
		case ScriptParam::PARM_TYPE_ANY:
			return std::string("any");
		case ScriptParam::PARM_TYPE_MORE:
			return std::string("more");
		default:
			return std::string("unknown type");
		}
	}
	
	
	std::string Utils::TestParameters(std::vector<ScriptParam> testparams, std::vector<ScriptParam> refparams, bool bAllowMore)
	{
		std::string strOut("");
		//test the number of parameters
		if(testparams.size()<refparams.size() || (testparams.size()>refparams.size() && !bAllowMore))
		{
			std::stringstream sstr;
			std::string strNum;
			std::string strErr=std::string("expecting ") + (bAllowMore ? std::string("at least ") : std::string(""));
			sstr<<refparams.size();
			sstr>>strNum;
			sstr.clear();
			strErr=strErr + strNum + std::string(" , not ");
			sstr<<testparams.size();
			sstr>>strNum;
			strErr=strErr + strNum + std::string(" parameters! ");
			strOut=strOut + strErr;
		}
		//test every single parameter
		for(unsigned int iParam=0; iParam<((refparams.size()<testparams.size()) ? refparams.size() : testparams.size()) ; iParam++)
		{
			if(refparams[iParam].getType()!=testparams[iParam].getType())
			{
				std::stringstream sstr;
				std::string strErr;
				sstr<<iParam;
				sstr>>strErr;
				strErr=std::string("expecting parameter ") + strErr + std::string(" to be of type ") + GetTypeName(refparams[iParam]) + std::string(", not ") + GetTypeName(testparams[iParam]) + std::string("! ");
				strOut+=strErr;
			}
		}
		return strOut;
	}

	std::string Utils::TestParameters(std::vector<ScriptParam> testparams, std::string sRefParams, bool bAllowMore)
	{
		std::vector<ScriptParam> vRefParams;
		int i=0; std::string s; double d=0; bool b = false;  
		std::string currParam = "";
		unsigned int index = 0;
		sRefParams += " ";	//Append blank
		while (index < sRefParams.length())
		{
			if (sRefParams[index] == ' ')
			{
				if (currParam == "int") vRefParams.push_back(ScriptParam(d));
				else if (currParam == "bool") vRefParams.push_back(ScriptParam(b));
				else if (currParam == "string") vRefParams.push_back(ScriptParam(s));
				else if (currParam == "float") vRefParams.push_back(ScriptParam(d));
				else if (currParam == "double") vRefParams.push_back(ScriptParam(d));
				else if (currParam == "function") vRefParams.push_back(ScriptParam(ScriptParam::PARM_TYPE_FUNCTION));

				currParam = "";
			}
			else currParam.push_back(sRefParams[index]);

			index++;
		}

		return TestParameters(testparams, vRefParams, bAllowMore);
	}

	std::string Utils::GetMultiTypeName(int type)
	{
		std::string strOut("");
		if(type&ScriptParam::PARM_TYPE_MORE)
			strOut+=std::string("zero or more ");
		int nTypes=0;
		if((type&ScriptParam::PARM_TYPE_ANY)==ScriptParam::PARM_TYPE_ANY)
			strOut+=std::string("<all types>");
		else
		{
			for(int j=1; j<ScriptParam::PARM_TYPE_MORE; j<<=1)
			{
				if(!(type&j))
					continue;
				if(nTypes++)
					strOut+=std::string(", ");
				strOut+=GetTypeName(type&j);
			}
		}
		if(nTypes>1)
			strOut=std::string("{") + strOut + std::string("}");
		return strOut;
	}

	
	std::string
	Utils::TestParameters(std::vector<ScriptParam> testparams, const int* aiRefParams, unsigned int nParams)
	{
		std::string strOut("");
		//int nMin=min(nParams, testparams.size());
		int variadicType;
		bool bVariadic=false;
		if(aiRefParams[nParams-1]&ScriptParam::PARM_TYPE_MORE)
		{
			bVariadic=true;
			variadicType=aiRefParams[nParams-1]&(~ScriptParam::PARM_TYPE_MORE);
		}
		bool bErr=false;
	
		//check if too few arguments were given
		if(testparams.size()<nParams-(bVariadic?1:0))
		{
			std::stringstream sstr;
			std::string strNum;
			std::string strErr=std::string("expecting ") +
					(bVariadic ? std::string("at least ") : std::string(""));
			sstr<<nParams-(bVariadic?1:0);
			sstr>>strNum;
			sstr.clear();
			strErr=strErr + strNum + std::string(" , not ");
			sstr<<testparams.size();
			sstr>>strNum;
			strErr=strErr + strNum + std::string(" parameters! ");
			strOut=strOut + strErr;
		}
		//iterate given parms
		for(unsigned int iParam=0; iParam<testparams.size(); iParam++)
		{
			int reftype = 0;
			if(iParam>=nParams)
			{
				if(bVariadic)
					reftype=variadicType;
				else
				{
					bErr=true;
					//error, too many arguments
					std::stringstream sstr;
					std::string strNum;
					std::string strErr=std::string("expecting ");
					sstr<<nParams;
					sstr>>strNum;
					sstr.clear();
					strErr=strErr + strNum + std::string(" , not ");
					sstr<<testparams.size();
					sstr>>strNum;
					strErr=strErr + strNum + std::string(" parameters! ");
					strOut=strOut + strErr;
				}
			}
			else
				reftype=aiRefParams[iParam];

			if(!(reftype&testparams[iParam].getType()))
			{
				bErr=true;
				std::stringstream sstr;
				std::string strErr;
				sstr<<iParam;
				sstr>>strErr;
				strErr=std::string("expecting parameter ") + strErr + std::string(" to be of type ") +
						GetMultiTypeName(reftype) + std::string(", not ") +
						GetTypeName(testparams[iParam]) + std::string("! ");
				strOut+=strErr;
			
			}
		}
		if(bErr)
		{
			strOut+=std::string("Usage: (");
			for(unsigned int i=0; i<nParams; i++)
			{
				if(i)
					strOut+=std::string(", ");
				strOut+=GetMultiTypeName(aiRefParams[i]);
			}
			strOut+=std::string(")");
		}
		return strOut;
	}

	void Utils::LogParameterErrors(const Script& caller, Ogre::String msg, int line)
	{
		Ogre::String log = "[Script] Error in \"" + caller.GetScriptName() + "\": " + msg;
		if (line >= 0) log = log + " (line " + Ogre::StringConverter::toString(line) + ")";
		Log::Instance().LogMessage(log);
	}

	void Utils::ScriptParamsToDataMap(const Script& caller, const std::vector<ScriptParam> &params, DataMap* data, int param_start_index)
	{
		IceAssert(data);
		for (unsigned int i = param_start_index; i < params.size(); i++)
		{
			if (params[i].getType() != ScriptParam::PARM_TYPE_STRING)
			{
				LogParameterErrors(caller, Ogre::StringConverter::toString(param_start_index + i) + ". is not a keyname (must be of type string).");
				return;
			}
			Ogre::String param = params[i].getString();
			int from = 0;
			int to = param.find(" ");
			Ogre::String key = param.substr(from, to);
			from = to + 1;
			to = param.find(" ", from);
			Ogre::String type = param.substr(from, to);
			from = to + 1;
			to = param.find(" ", from);
			Ogre::String value = param.substr(from, to);

			if (key == "int") data->AddInt(key, Ogre::StringConverter::parseInt(value));
			else if (key == "float") data->AddFloat(key, Ogre::StringConverter::parseReal(value));
			else if (key == "bool") data->AddBool(key, Ogre::StringConverter::parseBool(value));
			else if (key == "string") data->AddOgreString(key, value);
			else if (key == "Vec3") data->AddOgreVec3(key, Ogre::StringConverter::parseVector3(value));
			else if (key == "Quat") data->AddOgreQuat(key, Ogre::StringConverter::parseQuaternion(value));
			else if (key == "Col") data->AddOgreCol(key, Ogre::StringConverter::parseColourValue(value));
		}
	}

	DataMap
	Utils::TableToDataMap(const Script& caller, const ScriptParam &param)
	{
		if(param.getType()!=ScriptParam::PARM_TYPE_TABLE)
		{
			IceWarning("conversion to datamap requires a table as input!");
			return DataMap();
		}
		std::map<ScriptParam, ScriptParam> mTable=param.getTable();
		DataMap datamap;
		GenericProperty prop;
		for(std::map<ScriptParam, ScriptParam>::const_iterator it=mTable.begin(); it!=mTable.end(); it++)
		{
			//check if the key is a string
			if(it->first.getType()!=ScriptParam::PARM_TYPE_STRING)
			{
				IceWarning("table keys have to be strings!");
				return DataMap();
			}
			//convert value to generic property
			prop.Set(it->second);
			datamap.AddItem(it->first.getString(), prop);
		}
		return datamap;
	}
	
	void Utils::DataMapToTable(const Script& caller, DataMap &datamap, std::map<ScriptParam, ScriptParam> &table)
	{
		datamap.First();
		while(datamap.HasNext())
		{
			DataMap::Item item=datamap.GetNext();
			item.Data->GetAsScriptParam(table[ScriptParam(item.Key)]);
		}
	}

	Ogre::Quaternion Utils::ZDirToQuat(const Ogre::Vector3 &zDirNormalised, const Ogre::Vector3 &upVector)
	{
		Ogre::Vector3 zAxis = zDirNormalised;

		Ogre::Vector3 xAxis = Ogre::Vector3::UNIT_X;
		if (zAxis.dotProduct(upVector) != 0)
			xAxis = upVector.crossProduct(zAxis);
		xAxis.normalise();

		Ogre::Vector3 yAxis = zAxis.crossProduct(xAxis);
		return Ogre::Quaternion (xAxis, yAxis, zAxis);
	}

	float Utils::ComputeAO(const Ogre::Vector3 &position, const Ogre::Vector3 &normal, int rayCollisionGroups)
	{
		const float c = 4;
		float fNumSamples = 0;
		float sum = 0;

		PxSceneQueryFilterData filterData;
		filterData.data.word0 = rayCollisionGroups;
		filterData.flags = PxSceneQueryFilterFlag::eDYNAMIC|PxSceneQueryFilterFlag::eSTATIC;

		for(float h=Ogre::Math::PI/4/c; h<Ogre::Math::PI/2; h+=Ogre::Math::PI/2/c)
		{
			for(float v=0; v<2*Ogre::Math::PI; v+=Ogre::Math::PI/2/(c*Ogre::Math::Cos(h)))
			{
				Ogre::Vector3 castVector = Ogre::Vector3(Ogre::Math::Cos(h)*Ogre::Math::Sin(v),Ogre::Math::Sin(h),Ogre::Math::Cos(h)*Ogre::Math::Cos(v));
				castVector.normalise();
				Ogre::Quaternion q = Ogre::Vector3::UNIT_Y.getRotationTo(normal);
				castVector = q * castVector;
				Ogre::Ray ray(position + (castVector * 0.001f), castVector);
				float test = Ogre::Vector3::UNIT_Y.dotProduct(castVector);	
				float weight = Ogre::Math::Abs(normal.dotProduct(castVector));
				if (test < 0)	
				{
					fNumSamples += 0.5f*weight;	//light is coming from below	
					continue;
				}
				if (test >= 0 && !Ice::Main::Instance().GetPhysXScene()->raycastAny(position + (castVector * 0.001f), castVector, 20, filterData))
					sum += 1*weight;
				fNumSamples += 1*weight;
			}
		}
		if (fNumSamples < 1) fNumSamples = 1;
		float avg = sum/fNumSamples;
		if (avg > 1) avg = 1;

		return avg;
	}


};
