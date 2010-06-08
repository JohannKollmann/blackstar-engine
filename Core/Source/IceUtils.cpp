
#include "IceUtils.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "IceScript.h"

namespace Ice
{
	namespace Utils
	{
		DeleteListener::~DeleteListener()
		{
			for (auto i = mDeleteListeners.begin(); i != mDeleteListeners.end(); i++)
				(*i)->removeListener(this);
		}
		void DeleteListener::addDeleteListener(DeleteListener *listener)
		{
			listener->_registerDeleteListener(this);
			mDeleteListeners.push_back(listener);
		}
		void DeleteListener::removeListener(DeleteListener *listener)
		{
			for (auto i = mDeleteListeners.begin(); i != mDeleteListeners.end(); i++)
			{
				if (*i == listener)
				{
					mDeleteListeners.erase(i);
					break;
				}
			}
			onDeleteSubject(listener);
		}
	}

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
		switch(param.getType())
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

	void Utils::LogParameterErrors(Script& caller, Ogre::String msg, int line)
	{
		Ogre::String log = "[Script] Error in \"" + caller.GetScriptName() + "\": " + msg;
		if (line >= 0) log = log + " (line " + Ogre::StringConverter::toString(line) + ")";
		Ogre::LogManager::getSingleton().logMessage(log);
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

};