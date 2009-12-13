
#pragma once

#include "SGTIncludes.h"
#include "Ogre.h"
#include <string>
#include "SGTScriptParam.h"
#include "Ogre.h"
#include "NxQuat.h"

namespace SGTUtils
{
	/*
	Sucht rekursiv in einem Verzeichnis nach einer Datei und liefert, wenn erfolgreich, den Pfad zurück.
	*/
	SGTDllExport Ogre::String FindResourcePath(Ogre::String path, Ogre::String filename);

	SGTDllExport std::string GetTypeName(SGTScriptParam param);
	SGTDllExport std::string TestParameters(std::vector<SGTScriptParam> testparams, std::vector<SGTScriptParam> refparams, bool bAllowMore=false);
};
