
#include "SGTLevelMesh.h"
#include "SGTSceneManager.h"

SGTLevelMesh::SGTLevelMesh(Ogre::String mesh)
{
	SGTSceneManager::Instance().BakeStaticMeshShape(mesh);

	NxOgre::NodeRenderableParams nrp;
	nrp.setToDefault();
	nrp.mIdentifier = "LevelMesh";
	nrp.mIdentifierUsage = NxOgre::NodeRenderableParams::IU_Use;
	Ogre::SceneNode *node = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode(nrp.mIdentifier);
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mesh))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mesh + "\" does not exist. Loading dummy Resource...");
		mesh = "DummyMesh.mesh";
	}
	Ogre::Entity *ent = SGTMain::Instance().GetOgreSceneMgr()->createEntity("LevelMesh-entity", mesh);
	ent->setCastShadows(true);
    unsigned short src, dest;
	if (!ent->getMesh()->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest))
		ent->getMesh()->buildTangentVectors(Ogre::VES_TANGENT, src, dest, true, true, true);
	// Second mode cleans mirrored / rotated UVs but requires quality models
	//pMesh->buildTangentVectors(VES_TANGENT, src, dest, true, true);
	node->attachObject(ent);
	node->scale(1.0,1.0,1.0);
	//nrp.mGraphicsModel = mesh;
	//nrp.mGraphicsModelType = NxOgre::NodeRenderableParams::GMU_File;

	NxOgre::ActorParams ap;
	ap.setToDefault();
	ap.mDensity = 0.0f;
	ap.mMass = 0.0f;

	NxOgre::ShapeParams sp;
	sp.setToDefault();
	sp.mDensity = 0.0f;
	sp.mMass = 0.0f;
	sp.mGroup = "Collidable";

	mMeshFileName = mesh;
	NxOgre::TriangleMesh *shape = new NxOgre::TriangleMesh(NxOgre::Resources::ResourceSystem::getSingleton()->getMesh("Data/Media/Meshes/NXS/" + mesh + ".nxs"), sp);
	mBody = (NxOgre::Body*)(SGTMain::Instance().GetNxScene()->createBody<NxOgre::Body>("LevelMesh", shape, Ogre::Vector3(0,0,0), nrp, ap));
}

SGTLevelMesh::~SGTLevelMesh(void)
{
	SGTMain::Instance().GetNxScene()->destroyBody(mBody->getName());
}
