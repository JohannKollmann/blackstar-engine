
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include <string>
#include "IceScriptParam.h"
#include "Ogre.h"

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

		DllExport std::string GetTypeName(ScriptParam param);
		DllExport std::string TestParameters(std::vector<ScriptParam> testparams, std::vector<ScriptParam> refparams, bool bAllowMore=false);
		DllExport std::string TestParameters(std::vector<ScriptParam> testparams, std::string refParams, bool bAllowMore=false);

		DllExport void LogParameterErrors(const Script& caller, Ogre::String msg, int line = -1);

		DllExport void ScriptParamsToDataMap(const Script& caller, const std::vector<ScriptParam> &params, DataMap* data, int param_start_index = 0);

		DllExport Ogre::Quaternion ZDirToQuat(const Ogre::Vector3 &zDirNormalised, const Ogre::Vector3 &upVector = Ogre::Vector3::UNIT_Y);

	};

};