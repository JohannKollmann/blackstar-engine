
#pragma once

#include "EDTIncludes.h"
#include "Ogre.h"

class AmbientOcclusionGenerator
{
private:
	void processVertexData(Ogre::VertexData *targetData, Ogre::VertexData *inputData, int rayCollisionGroups, Ogre::Node *baseNode = nullptr);
	void computeAO(Ogre::Vector3 position, Ogre::Vector3 normal, int rayCollisionGroups, Ogre::ColourValue &target);

public:
	AmbientOcclusionGenerator(void);
	~AmbientOcclusionGenerator(void);

	static AmbientOcclusionGenerator& Instance();

	void bakeAmbientOcclusion(Ogre::MeshPtr mesh, Ogre::String outputFile);

	/**
	Bakes the ambient occlusion for an existing entity.
	*/
	void bakeAmbientOcclusion(Ogre::Entity *ent, Ogre::String outputFile);
};

