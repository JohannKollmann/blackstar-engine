
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include <string>
#include "IceScriptParam.h"
#include "Ogre.h"

#define UTIL_TEST_PARAMS(errstring, testparams, ...)\
{\
	const int _aiRefParams[]={__VA_ARGS__};\
	errstring=Ice::Utils::TestParameters(testparams, _aiRefParams, sizeof(_aiRefParams)/sizeof(int));\
}

#define UTIL_REPORT_ERROR(errstring)\
{\
	if(errstring.size())\
	{\
		std::vector<Ice::ScriptParam> ret;\
		ret.push_back(Ice::ScriptParam());\
		ret.push_back(Ice::ScriptParam(errstring));\
		return ret;\
	}\
}

namespace Ice
{
	namespace Utils
	{
		template<class T>
		DllExport void DeletePointerVector(std::vector<T*> &vec)
		{
			for (auto i = vec.begin(); i != vec.end(); i++)
			{
				ICE_DELETE *i;
			}
			vec.clear();
		}

		/*
		Sucht rekursiv in einem Verzeichnis nach einer Datei und liefert, wenn erfolgreich, den Pfad zurück.
		*/
		DllExport Ogre::String FindResourcePath(Ogre::String path, Ogre::String filename);
		
		DllExport std::string GetTypeName(int type);
		DllExport std::string GetTypeName(ScriptParam param);
		DllExport std::string GetMultiTypeName(int type);
		DllExport std::string TestParameters(std::vector<ScriptParam> testparams, std::vector<ScriptParam> refparams, bool bAllowMore=false);
		DllExport std::string TestParameters(std::vector<ScriptParam> testparams, std::string refParams, bool bAllowMore=false);
		DllExport std::string TestParameters(std::vector<ScriptParam> testparams, const int* aiRefParams, unsigned int nParams);

		DllExport void LogParameterErrors(const Script& caller, Ogre::String msg, int line = -1);

		DllExport void ScriptParamsToDataMap(const Script& caller, const std::vector<ScriptParam> &params, DataMap* data, int param_start_index = 0);
		
		/**
		 * converts a scriptparam with PARM_TYPE_TABLE to a datamap
		 * the table needs to have string keys and supported values
		 */
		DllExport DataMap TableToDataMap(const Script& caller, const ScriptParam &param);
		/**
		 * converts a datamap to a scriptparam with PARM_TYPE_TABLE
		 */
		DllExport ScriptParam DataMapToTable(const Script& caller, DataMap &datamap);

		DllExport Ogre::Quaternion ZDirToQuat(const Ogre::Vector3 &zDirNormalised, const Ogre::Vector3 &upVector = Ogre::Vector3::UNIT_Y);

		DllExport float ComputeAO(const Ogre::Vector3 &position, const Ogre::Vector3 &normal, int rayCollisionGroups);
	};

};
