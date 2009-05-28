#pragma once

#include "SGTIncludes.h"
#include "SGTMain.h"
#include "tinyxml.h"

class SGTDotSceneLoader
{
private:
	Ogre::Quaternion LoadRotation(TiXmlElement* objectElement);
	void LoadXYZ(TiXmlElement* objectElement, Ogre::Vector3& xyz);
	float GetRealAttribute(TiXmlElement* xmlElement, const char* name, float defaultValue = 0.0f);
	Ogre::String GetStringAttribute(TiXmlElement* xmlElement, const char* name);
	void LoadNode(TiXmlElement* objectElement);
	void LoadNodes(TiXmlElement* Element);

public:
	SGTDotSceneLoader(void);
	~SGTDotSceneLoader(void);

	void ImportScene(Ogre::String file);

	//Singleton
	static SGTDotSceneLoader& Instance();
};
