
#include "IceLevelMesh.h"
#include "IceSceneManager.h"
#include "IceMain.h"
#include "IceGOCPhysics.h"

namespace Ice
{

LevelMesh::LevelMesh(Ogre::String mesh)
{
	mNode = Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("LevelMesh");
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mesh))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mesh + "\" does not exist. Loading dummy Resource...");
		mesh = "DummyMesh.mesh";
	}
	mEntity = Main::Instance().GetOgreSceneMgr()->createEntity("LevelMesh-entity", mesh);
	mEntity->setCastShadows(true);
    unsigned short src, dest;
	if (!mEntity->getMesh()->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest))
		mEntity->getMesh()->buildTangentVectors(Ogre::VES_TANGENT, src, dest, true, true, true);
	// Second mode cleans mirrored / rotated UVs but requires quality models
	//pMesh->buildTangentVectors(VES_TANGENT, src, dest, true, true);

	mNode->attachObject(mEntity);
	mNode->scale(1.0,1.0,1.0);

	if (!OgrePhysX::Cooker::getSingleton().hasNxMesh(mesh + ".nxs"))
		OgrePhysX::Cooker::getSingleton().cookNxMeshToFile(mEntity->getMesh(), "Data\\Media\\Meshes\\NXS\\" + mesh + ".nxs");

	mActor = Main::Instance().GetPhysXScene()->createActor(OgrePhysX::CookedMeshShape(mesh + ".nxs").group(CollisionGroups::LEVELMESH));
}

LevelMesh::~LevelMesh(void)
{
	Main::Instance().GetPhysXScene()->destroyActor(mActor);
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
	Main::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
}

};