#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"
#include "IceGameObject.h"
#include "IceMain.h"
#include "IceSceneManager.h"
#include "Rtt.h"
#include "IceGOCOgreNode.h"
#include "Caelum.h"
#include "InternalUtilities.h"

class __declspec(dllexport) GOCSimpleWater : public Ice::GOCOgreNodeUser, public Ice::GOCStaticEditorInterface
{
private:
	Ogre::String mWaterMesh;
	Ogre::String mRefMaterial;
	Ogre::Vector3 mUpVector;
	WaterPlane mWaterPlane;
	Ogre::Entity *mEntity;
	float mWidth;
	float mHeight;
	bool mReflection;
	int mReflectionRttRes;
	//float mRadius;
	Ice::DataMap::Enum mBodyTypeEnum;

	void _clear()
	{
		mWaterPlane.unloadResources();
		if (mEntity)
		{
			Ice::Main::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
			mEntity = nullptr;
		}
	}
	void _create()
	{
		_clear();

		Ogre::String id = Ice::SceneManager::Instance().RequestIDStr();
		Ogre::Plane waterPlane(mUpVector, 0);
		Ogre::Vector3 upTexVector(0,0,1);
		if (mUpVector.directionEquals(upTexVector, Ogre::Radian(Ogre::Degree(10)))) upTexVector = Ogre::Vector3::UNIT_Y;
		upTexVector = waterPlane.projectVector(upTexVector);

		if (mBodyTypeEnum.selection == 0)		//Plane
		{
			mWaterMesh = "WaterSurface_" + id;
			Ogre::MeshManager::getSingleton().createPlane(mWaterMesh, "General", waterPlane, mWidth, mHeight, 1, 1, true, 1, 1, 1, upTexVector);
		}
		else if (mBodyTypeEnum.selection == 2)	//Dome
		{
			mWaterMesh = "WaterSurface_" + id;
			Caelum::InternalUtilities::generateSphericDome (mWaterMesh, 32, Caelum::InternalUtilities::DT_SKY_DOME);
		}

		//create Entity
		if (!Ogre::MeshManager::getSingleton().resourceExists(mWaterMesh) && !Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mWaterMesh))
		{
			Ice::Log::Instance().LogMessage("Error: Resource \"" + mWaterMesh + "\" does not exist. Loading dummy Resource...");
			mWaterMesh = "DummyMesh.mesh";
		}
		mEntity = Ice::Main::Instance().GetOgreSceneMgr()->createEntity(id, mWaterMesh);
		mEntity->setCastShadows(false);

		mWaterPlane.create(mEntity, Ice::Main::Instance().GetCamera(), mRefMaterial, mReflection, mReflectionRttRes);
		mWaterPlane.setPlane(waterPlane);

		NotifyOwnerGO();

	}

public:

	static void InstallPlugin()
	{
		LoadSave::LoadSave::Instance().RegisterObject(&GOCSimpleWater::Register);
		Ice::SceneManager::Instance().RegisterGOCPrototype("E", std::make_shared<GOCSimpleWater>());
		//Ogre::Root::getSingleton().createRenderQueueInvocationSequence("RefractionSurfacesOnly")->add(Ogre::RENDER_QUEUE_7, "");
	}

	GOCSimpleWater(void) : mEntity(nullptr)
	{
		mBodyTypeEnum.choices.push_back("Rectangle");
		mBodyTypeEnum.choices.push_back("Mesh");
		mBodyTypeEnum.choices.push_back("Dome");
		mBodyTypeEnum.selection = 0;
	}
	~GOCSimpleWater(void)
	{
		_clear();
	}

	GOComponent::TypeID& GetComponentID() const { static std::string name = "SimpleWater"; return name; }

	void UpdatePosition(Ogre::Vector3 position)
	{
		Ice::GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		mWaterPlane.setPlane(Ogre::Plane(owner->GetGlobalOrientation() * mUpVector, owner->GetGlobalPosition()));
	}
	void UpdateOrientation(Ogre::Quaternion orientation)
	{
		Ice::GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		mWaterPlane.setPlane(Ogre::Plane(owner->GetGlobalOrientation() * mUpVector, owner->GetGlobalPosition()));
	}
	void UpdateScale(Ogre::Vector3 scale)
	{
	}

	void NotifyOwnerGO()
	{
		Ice::GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		if (mEntity)
		{
			mEntity->setUserAny(Ogre::Any(owner.get()));
			GetNode()->attachObject(mEntity);
		}
		UpdatePosition(owner->GetGlobalPosition());
	}

	void ShowEditorVisual(bool show)
	{
	}

	//Editor interface
	BEGIN_GOCEDITORINTERFACE(GOCSimpleWater, "Simple Water")
		PROPERTY_ENUM(mBodyTypeEnum, "Body Type", mBodyTypeEnum)
		PROPERTY_FLOAT(mWidth, "Width", 1.0f);
		PROPERTY_FLOAT(mHeight, "Height", 1.0f);
		PROPERTY_STRING(mRefMaterial, "Material", "WaterReflectionRefraction")
		PROPERTY_STRING(mWaterMesh, "Water mesh", ".mesh")
		PROPERTY_BOOL(mReflection, "Reflection", true)
		PROPERTY_INT(mReflectionRttRes, "ReflectionRttResolution", 512);
		PROPERTY_VECTOR3(mUpVector, "Up Vector", Ogre::Vector3(0,1,0));
	END_GOCEDITORINTERFACE

	void OnSetParameters()
	{
		_create();
	}

	void Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("int", &mBodyTypeEnum.selection, "Body Type");
		mgr.SaveAtom("float", &mWidth, "Width");
		mgr.SaveAtom("float", &mHeight, "Height");
		mgr.SaveAtom("Ogre::String", &mWaterMesh, "Water Mesh");
		mgr.SaveAtom("Ogre::Vector3", &mUpVector, "Up Vector");
	}
	void Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("int", &mBodyTypeEnum.selection);
		mgr.LoadAtom("float", &mWidth);
		mgr.LoadAtom("float", &mHeight);
		mgr.LoadAtom("Ogre::String", &mWaterMesh);
		mgr.LoadAtom("Ogre::Vector3", &mUpVector);
		_create();
	}
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "SimpleWater"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new GOCSimpleWater; };
};

