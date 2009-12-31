
#include "SGTLevelMesh.h"
#include "SGTSceneManager.h"
#include "SGTMain.h"
#include "SGTGOCPhysics.h"

SGTLevelMesh::SGTLevelMesh(Ogre::String mesh)
{
	mNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("LevelMesh");
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mesh))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mesh + "\" does not exist. Loading dummy Resource...");
		mesh = "DummyMesh.mesh";
	}
	mEntity = SGTMain::Instance().GetOgreSceneMgr()->createEntity("LevelMesh-entity", mesh);
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

	mActor = SGTMain::Instance().GetPhysXScene()->createActor(OgrePhysX::CookedMeshShape(mesh + ".nxs").group(SGTCollisionGroups::LEVELMESH));
}

SGTLevelMesh::~SGTLevelMesh(void)
{
	SGTMain::Instance().GetPhysXScene()->destroyActor(mActor);
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
}
