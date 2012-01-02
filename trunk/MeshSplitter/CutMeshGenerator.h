/*
 * CutMeshGenerator.h
 *
 *  Created on: Apr 30, 2011
 *      Author: _1nsane
 */

#ifndef CUTMESHGENERATOR_H_
#define CUTMESHGENERATOR_H_

#include <Ogre.h>

class CutMeshGenerator {
public:
	/*
	 * generates cut mesh in xy-plane with  [0:1] extents and [0:1] UVs
	 * transformation are then applied
	 */
	static void Generate(Ogre::MeshPtr meshptr, float** aafHeightmap, int iWidth, int iHeight,
			Ogre::Matrix4 mTransform, Ogre::Matrix3 mUVTransform);
	/*
	 * generates cut mesh in xy-plane with  [0:1] extents and [0:1] UVs
	 * transformation are then applied
	 */
	static void CreateRandomCutMesh(Ogre::MeshPtr inMesh, Ogre::MeshPtr outMesh, float fRoughness, float fResolution, Ogre::String strMatName);
	/*
	 * transforms given mesh with given transforms
	 */
	//static void TransformMesh(Ogre::MeshPtr mesh, Ogre::Matrix4 mTransform, Ogre::Matrix3 mUVTransform);
};

#endif /* CUTMESHGENERATOR_H_ */
