
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
	/*
	Sucht rekursiv in einem Verzeichnis nach einer Datei und liefert, wenn erfolgreich, den Pfad zurück.
	*/
	DllExport Ogre::String FindResourcePath(Ogre::String path, Ogre::String filename);

	DllExport std::string GetTypeName(ScriptParam param);
	DllExport std::string TestParameters(std::vector<ScriptParam> testparams, std::vector<ScriptParam> refparams, bool bAllowMore=false);
};

};