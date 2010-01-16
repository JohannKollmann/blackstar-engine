#pragma once

#include "IceIncludes.h"
#include "IceMain.h"
#include "tinyxml.h"
#include "EDTIncludes.h"

class DotSceneLoader
{
private:
	Ogre::Quaternion LoadRotation(TiXmlElement* objectElement);
	void LoadXYZ(TiXmlElement* objectElement, Ogre::Vector3& xyz);
	float GetRealAttribute(TiXmlElement* xmlElement, const char* name, float defaultValue = 0.0f);
	Ogre::String GetStringAttribute(TiXmlElement* xmlElement, const char* name);
	void LoadNode(TiXmlElement* objectElement);
	void LoadNodes(TiXmlElement* Element);

public:
	DotSceneLoader(void);
	~DotSceneLoader(void);

	void ImportScene(Ogre::String file);

	//Singleton
	static DotSceneLoader& Instance();
};
