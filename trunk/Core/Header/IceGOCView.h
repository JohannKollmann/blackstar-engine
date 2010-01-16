
#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "Ogre.h"
#include "OgreOggSound.h"
#include "IceGOCEditorInterface.h"

namespace Ice
{

class GOCViewComponent : public LoadSave::Saveable
{
public:
	virtual ~GOCViewComponent() {};
	virtual Ogre::MovableObject* GetEditorVisual() = 0;
	virtual void AttachToNode(Ogre::SceneNode *node) = 0;
	virtual void ShowEditorVisual(bool show) = 0;
	virtual Ogre::String GetTypeName() = 0;
};

class DllExport GOCViewComponentEDT : public GOCEditorInterface, public GOCViewComponent
{
public:
	void AttachToGO(GameObject *go);
};

class DllExport GOCNodeRenderable : public GOComponent
{
protected:
	Ogre::SceneNode *mNode;

public:
	GOCNodeRenderable(void) {}
	virtual ~GOCNodeRenderable(void) {}
	virtual void UpdatePosition(Ogre::Vector3 position);
	virtual void UpdateOrientation(Ogre::Quaternion orientation);

	virtual goc_id_family& GetFamilyID() const { static std::string name = "View"; return name; }

	Ogre::SceneNode *GetNode() { return mNode; }
};

class DllExport GOCViewContainer : public GOCNodeRenderable
{
private:
	std::list<GOCViewComponent*> mItems;

public:
	GOCViewContainer(void);
	~GOCViewContainer(void);

	GOComponent::goc_id_type& GetComponentID() const { static std::string name = "ViewContainer"; return name; }

	void AddItem(GOCViewComponent *item);
	GOCViewComponent* GetItem(Ogre::String type);
	void RemoveItem(Ogre::String type);
	void SetOwner(GameObject *go);
	std::list<GOCViewComponent*>::iterator GetItemIterator() { return mItems.begin(); }
	std::list<GOCViewComponent*>::iterator GetItemIteratorEnd() { return mItems.end(); }

	void UpdateScale(Ogre::Vector3 scale);

	void ShowEditorVisual(bool show);

	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);
	std::string& TellName() { static std::string name = "ViewContainer"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ViewContainer"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; }
	static LoadSave::Saveable* NewInstance() { return new GOCViewContainer; }
};


//Some Basic View components

class DllExport MeshRenderable : public GOCViewComponentEDT
{
private:
	Ogre::Entity *mEntity;
	void Create(Ogre::String meshname, bool shadowcaster);

public:
	MeshRenderable() {}
	MeshRenderable(Ogre::String meshname, bool shadowcaster);
	~MeshRenderable();

	virtual Ogre::MovableObject* GetEditorVisual() { return mEntity; }
	virtual void AttachToNode(Ogre::SceneNode *node);
	virtual void ShowEditorVisual(bool show) {};

	virtual void CreateFromDataMap(DataMap *parameters);
	virtual void GetParameters(DataMap *parameters);
	static void GetDefaultParameters(DataMap *parameters);
	Ogre::String GetLabel() { return "Mesh"; }

	virtual void Save(LoadSave::SaveSystem& mgr);
	virtual void Load(LoadSave::LoadSystem& mgr);
	virtual std::string& TellName() { static std::string name = "MeshRenderable"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "MeshRenderable"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance();
	static GOCEditorInterface* NewEditorInterfaceInstance() { return new MeshRenderable(); }

	virtual std::string GetTypeName() { return "MeshRenderable"; }
};

class DllExport MeshDebugRenderable : public GOCNodeRenderable
{
private:
	Ogre::Entity *mEntity;

public:
	MeshDebugRenderable() {}
	MeshDebugRenderable(Ogre::String meshname);
	~MeshDebugRenderable();
	goc_id_type& GetComponentID() const { static std::string name = "MeshDebugRenderable"; return name; } 
	goc_id_family& GetFamilyID() const { static std::string name = "MeshDebugRenderable"; return name; } 
	void SetOwner(GameObject *go);

	void Save(LoadSave::SaveSystem& mgr) {};
	void Load(LoadSave::LoadSystem& mgr) {};
	virtual std::string& TellName() { static std::string name = "MeshDebugRenderable"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "MeshDebugRenderable"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new MeshDebugRenderable; };
};

class DllExport PfxRenderable : public GOCViewComponentEDT
{
private:
	Ogre::ParticleSystem *mParticleSystem;
	Ogre::Entity *mEditorVisual;
	Ogre::String mParticleResource;
	void Create(Ogre::String pfxresname);

public:
	PfxRenderable() {}
	PfxRenderable(Ogre::String pfxresname);
	~PfxRenderable();

	Ogre::MovableObject* GetEditorVisual() { return mEditorVisual; }
	void AttachToNode(Ogre::SceneNode *node);

	void CreateFromDataMap(DataMap *parameters);
	void GetParameters(DataMap *parameters);
	static void GetDefaultParameters(DataMap *parameters);

	void ShowEditorVisual(bool show);

	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);
	std::string& TellName() { static std::string name = "ParticleSystem"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ParticleSystem"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new PfxRenderable; };
	static GOCEditorInterface* NewEditorInterfaceInstance() { return new PfxRenderable(); }
	Ogre::String GetLabel() { return "PFX"; }

	std::string GetTypeName() { return "ParticleSystem"; }
};


class DllExport Sound3D : public GOCViewComponentEDT
{
private:
	OgreOggSound::OgreOggISound *mSound;
	Ogre::Entity *mEditorVisual;
	Ogre::String mAudioFile;
	float mReferenceDistance;
	float mMaxDistance;
	bool mStreamed;
	bool mLooped;
	bool mPrebuffered;
	void Create(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBufferede);

public:
	Sound3D() {}
	Sound3D(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBuffered);
	~Sound3D();

	Ogre::MovableObject* GetEditorVisual() { return mEditorVisual; }
	void AttachToNode(Ogre::SceneNode *node);

	void CreateFromDataMap(DataMap *parameters);
	void GetParameters(DataMap *parameters);
	static void GetDefaultParameters(DataMap *parameters);

	void ShowEditorVisual(bool show);

	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);
	std::string& TellName() { static std::string name = "Sound3D"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Sound3D"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new Sound3D; };
	static GOCEditorInterface* NewEditorInterfaceInstance() { return new Sound3D(); }
	Ogre::String GetLabel() { return "Sound3D"; }

	std::string GetTypeName() { return "Sound3D"; }
};


class DllExport LocalLightRenderable : public GOCViewComponentEDT
{
private:
	Ogre::Light *mLight;
	Ogre::Entity *mEditorVisual;
	Ogre::ColourValue mOriginalColour;
	float mFlickerSpeed;
	float mFlickerIntensity;
	float mFlickerOffset;
	void Create(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity);

public:
	LocalLightRenderable() {}
	LocalLightRenderable(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity);
	~LocalLightRenderable();

	Ogre::MovableObject* GetEditorVisual() { return mEditorVisual; }
	void AttachToNode(Ogre::SceneNode *node);

	void CreateFromDataMap(DataMap *parameters);
	void GetParameters(DataMap *parameters);
	static void GetDefaultParameters(DataMap *parameters);

	void ShowEditorVisual(bool show);

	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);
	std::string& TellName() { static std::string name = "LocalLight"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "LocalLight"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new LocalLightRenderable; };
	static GOCEditorInterface* NewEditorInterfaceInstance() { return new LocalLightRenderable(); }

	Ogre::String GetLabel() { return "Light"; }

	std::string GetTypeName() { return "LocalLight"; }
};

};