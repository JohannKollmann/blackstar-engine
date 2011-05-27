#pragma once

#include "Ogre.h"
#include <vector>
#include "EDTIncludes.h"

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

class EntityMaterialInspector
{
private:
	Ogre::Entity *mEntity;
	Ogre::SubEntity *mSubEntity;
	Ogre::Vector3 mHitPos;
	int mHitTriangleIndices[3];

	std::vector<SubMeshInformation*> getSubMeshes(const Ogre::Vector3 &position, const Ogre::Quaternion &orient, const Ogre::Vector3 &scale);
	void inspect(const Ogre::Ray &ray);

public:
	EntityMaterialInspector(Ogre::Entity *entity, Ogre::Ray ray);
	~EntityMaterialInspector(void);

	Ogre::String GetMaterialName();
	Ogre::SubEntity* GetSubEntity() { return mSubEntity; }
	Ogre::Vector3 GetHitPosition() { return mHitPos; }
	int* GetHitTriangleIndices() { return mHitTriangleIndices; }
};
