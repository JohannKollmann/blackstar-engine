
#pragma once

#include "EDTIncludes.h"
#include "Ogre.h"

class VertexMultitextureWeightBrusher
{
private:
	static unsigned short EnsureHasMultitextureWeightBuffer(Ogre::VertexData *vertexData);

	static void ProcessVertexData(Ogre::VertexData *vertexData, Ogre::Vector3 brushPos, float radius, unsigned int textureLayer); 

public:
	static void EnsureHasMultitextureWeightBuffer(Ogre::MeshPtr Mesh);

	static void SetMultitextureWeight(Ogre::MeshPtr mesh, Ogre::SubMesh *subMesh, Ogre::Vector3 brushPos, float radius, unsigned int textureLayer);
	static void SetMultitextureWeight(Ogre::Entity *entity, Ogre::Ray ray, float radius, unsigned int textureLayer);
};

