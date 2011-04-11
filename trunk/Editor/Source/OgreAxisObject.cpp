//
//
//
//	Filename : AxisObject.cpp

#include "Ogre.h"
#include "OgreMaterial.h"
#include "OgreAxisObject.h"
#include "IceMain.h"
#include "IceSceneManager.h"
#include "IceGameObject.h"
#include "IceGOCView.h"

using namespace Ogre;

void AxisComponent::addBox(ManualObject* obj, Vector3 dim, Vector3 pos, ColourValue color, short boxMask)
{
	if(!obj)
		return;

	obj->begin("Axis", Ogre::RenderOperation::OT_TRIANGLE_LIST); 

	dim/=2;

	Ogre::Real l = dim.x;
	Ogre::Real h = dim.y;
	Ogre::Real w = dim.z;
	
	obj->position(Ogre::Vector3(-l, h, w) + pos);
	obj->colour(color);
	obj->position(Ogre::Vector3(-l, -h, w) + pos);
	obj->colour(color);
	obj->position(Ogre::Vector3(l, -h, w) + pos);
	obj->colour(color);
	obj->position(Ogre::Vector3(l, h, w) + pos);

	obj->position(Ogre::Vector3(-l, h, -w) + pos);
	obj->colour(color);
	obj->position(Ogre::Vector3(-l, -h, -w) + pos); 
	obj->colour(color);
	obj->position(Ogre::Vector3(l, -h, -w) + pos);
	obj->colour(color);
	obj->position(Ogre::Vector3(l, h, -w) + pos);

	// front back
	if(boxMask & BOX_FRONT)
		obj->quad(0, 1, 2, 3);
	if(boxMask & BOX_BACK)
		obj->quad(7, 6, 5, 4);

	// top bottom
	if(boxMask & BOX_TOP)
		obj->quad(0, 3, 7, 4);
	if(boxMask & BOX_BOT)
		obj->quad(2, 1, 5, 6);

	// end caps
	if(boxMask & BOX_RIGHT)
		obj->quad(1, 0, 4, 5);
	if(boxMask & BOX_LEFT)
		obj->quad(3, 2, 6, 7);

	obj->end(); 
}

void AxisComponent::addMaterial(const Ogre::String &mat, Ogre::ColourValue &clr, Ogre::SceneBlendType sbt)
{
	static int init=false;
	if(init)
		return;
	else
		init=true;

	Ogre::MaterialPtr matptr = Ogre::MaterialManager::getSingleton().create(mat, "General"); 
	matptr->setReceiveShadows(false); 
	matptr->getTechnique(0)->setLightingEnabled(true);
	matptr->getTechnique(0)->getPass(0)->setDiffuse(clr); 
	matptr->getTechnique(0)->getPass(0)->setAmbient(clr); 
	matptr->getTechnique(0)->getPass(0)->setSelfIllumination(clr); 
	matptr->getTechnique(0)->getPass(0)->setSceneBlending(sbt);
	matptr->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	matptr->getTechnique(0)->getPass(0)->setVertexColourTracking(Ogre::TVC_DIFFUSE);
}

Ogre::ManualObject* AxisComponent::createAxis(const Ogre::String &name, Ogre::Vector3 scale)
{
	addMaterial("Axis", Ogre::ColourValue(1,1,1,.75), Ogre::SBT_TRANSPARENT_ALPHA);

	mAxisObject	= Ice::Main::Instance().GetOgreSceneMgr()->createManualObject(name); 

	Ogre::Real Xlen=scale.x/2 + 0.4f;
	Ogre::Real Ylen=scale.y/2 + 0.4f;
	Ogre::Real Zlen=scale.z/2 + 0.4f;
	Ogre::Real Xscl=0.095f;
	Ogre::Real Yscl=0.095f;
	Ogre::Real Zscl=0.095f;
	Ogre::Real Xloc=Xlen/2+Xscl/2;
	Ogre::Real Yloc=Ylen/2+Yscl/2;
	Ogre::Real Zloc=Zlen/2+Zscl/2;
	Ogre::Real fade=0.8f;
	Ogre::Real solid=0.8f;
	
	addBox(mAxisObject, Vector3(Xlen, Xscl, Xscl), Vector3(Xloc,0,0), ColourValue(0, 0, solid, solid), (BOX_ALL & ~BOX_RIGHT)); 
	addBox(mAxisObject, Vector3(Xlen, Xscl, Xscl), Vector3(-Xloc,0,0), ColourValue(0, 0, fade, fade), (BOX_ALL & ~BOX_LEFT)); 

	addBox(mAxisObject, Vector3(Yscl, Ylen, Yscl), Vector3(0,Yloc,0), ColourValue(0, solid, 0, solid), (BOX_ALL & ~BOX_BOT)); 
	addBox(mAxisObject, Vector3(Yscl, Ylen, Yscl), Vector3(0,-Yloc,0), ColourValue(0, fade, 0, fade), (BOX_ALL & ~BOX_TOP)); 

	addBox(mAxisObject, Vector3(Zscl, Zscl, Zlen), Vector3(0,0,Zloc), ColourValue(solid, 0, 0, solid), (BOX_ALL & ~BOX_BACK)); 
	addBox(mAxisObject, Vector3(Zscl, Zscl, Zlen), Vector3(0,0,-Zloc), ColourValue(fade, 0, 0, fade), (BOX_ALL & ~BOX_FRONT)); 

	mAxisObject->setCastShadows(false);
	mAxisObject->setVisibilityFlags( Ice::VisibilityFlags::V_EDITOROBJECT);

	//axis->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY - 1);

	return mAxisObject;
}

AxisComponent::AxisComponent()
{
	mAxisObject = nullptr;
}
AxisComponent::~AxisComponent()
{
	if (mAxisObject)
	{
		Ice::Main::Instance().GetOgreSceneMgr()->destroyManualObject(mAxisObject);
	}
}

void AxisComponent::NotifyOwnerGO()
{
	Ice::GameObjectPtr owner = mOwnerGO.lock();
	if (!owner.get()) return;
	if (mAxisObject)
	{
		Ice::Main::Instance().GetOgreSceneMgr()->destroyManualObject(mAxisObject);
	}
	Ogre::Vector3 vAxisDimensions = Ogre::Vector3(1,1,1);
	Ice::GOCMeshRenderable *gocmesh = owner->GetComponent<Ice::GOCMeshRenderable>();
	if (gocmesh != 0)
	{
		vAxisDimensions = gocmesh->GetEntity()->getBoundingBox().getSize() * gocmesh->GetNode()->_getDerivedScale();
	}
	createAxis("AxisObject_" + Ogre::StringConverter::toString(Ice::SceneManager::Instance().RequestID()), vAxisDimensions);
	GetNode()->attachObject(mAxisObject);
	GetNode()->setPosition(owner->GetGlobalPosition());
	GetNode()->setOrientation(owner->GetGlobalOrientation());
}