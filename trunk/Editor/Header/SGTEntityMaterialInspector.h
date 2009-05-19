#pragma once

#include "Ogre.h"
#include <vector>

class SubMeshInformation
{
public:
	SubMeshInformation()
	{
		mVertex_count = 0;
		mIndex_count = 0;
		mSubMesh = NULL;
		mVertices = NULL;
		mIndices = NULL;
	};
	~SubMeshInformation()
	{
		if (mVertices)
		{
			delete[] mVertices;
			mVertices = NULL;
		}
		if (mIndices)
		{
			delete[] mIndices;
			mIndices = NULL;
		}
	};
	Ogre::SubMesh *mSubMesh;
	unsigned short mIndex;
	size_t mVertex_count;
	size_t mIndex_count;
	Ogre::Vector3* mVertices;
	unsigned long* mIndices;
};

class SGTEntityMaterialInspector
{


private:
	Ogre::Entity *mEntity;

	std::vector<SubMeshInformation*> GetSubMeshes(const Ogre::Vector3 &position, const Ogre::Quaternion &orient, const Ogre::Vector3 &scale);

public:
	SGTEntityMaterialInspector(Ogre::Entity *entity);
	Ogre::SubEntity* GetSubEntity(Ogre::Ray ray);
	Ogre::String GetMaterialName(Ogre::Ray ray);

	~SGTEntityMaterialInspector(void);
};
