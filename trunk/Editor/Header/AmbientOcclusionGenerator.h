
#pragma once

#include "EDTIncludes.h"
#include "Ogre.h"

class AmbientOcclusionGenerator
{
private:
	void processVertexData(Ogre::VertexData *targetData, Ogre::VertexData *inputData);
	void computeAO(Ogre::Vector3 position, Ogre::Vector3 normal, Ogre::ColourValue &target);

public:
	AmbientOcclusionGenerator(void);
	~AmbientOcclusionGenerator(void);

	static AmbientOcclusionGenerator& Instance();

	void bakeAmbientOcclusion(Ogre::MeshPtr mesh, Ogre::String outputFile);
};

