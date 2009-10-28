
#include "SGTUtils.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

Ogre::String SGTUtils::FindResourcePath(Ogre::String path, Ogre::String filename)
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

std::string SGTUtils::GetTypeName(SGTScriptParam param)
{
	switch(param.getType())
	{
	case SGTScriptParam::PARM_TYPE_BOOL:
		return std::string("bool");
	case SGTScriptParam::PARM_TYPE_FLOAT:
		return std::string("float");
	case SGTScriptParam::PARM_TYPE_FUNCTION:
		return std::string("function");
	case SGTScriptParam::PARM_TYPE_INT:
		return std::string("int");
	case SGTScriptParam::PARM_TYPE_STRING:
		return std::string("string");
	default:
		return std::string("unknown type");
	}
}
std::string SGTUtils::TestParameters(std::vector<SGTScriptParam> testparams, std::vector<SGTScriptParam> refparams, bool bAllowMore)
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
		strErr=strErr + strNum + std::string("! ");
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