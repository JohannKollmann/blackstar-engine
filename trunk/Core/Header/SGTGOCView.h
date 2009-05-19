
#pragma once

#include "SGTGOComponent.h"
#include "SGTIncludes.h"
#include "Ogre.h"
#include "OgreOggSound.h"
#include "SGTGOCEditorInterface.h"

class SGTDllExport SGTGOCViewComponent : public SGTSaveable
{
public:
	SGTGOCViewComponent(void) {};
	virtual ~SGTGOCViewComponent(void) {};

	virtual Ogre::MovableObject* GetEditorVisual() = 0;
	virtual void AttachToNode(Ogre::SceneNode *node) = 0;
	virtual Ogre::String GetEditorMeshName() { return "None"; }
	virtual void ShowEditorVisual(bool show) {};
};

class SGTDllExport SGTGOCViewComponentEDT : public SGTGOCEditorInterface, public SGTGOCViewComponent
{
public:
	virtual ~SGTGOCViewComponentEDT(void) {};
	bool IsViewComponent() { return true; }
};


class SGTDllExport SGTGOCNodeRenderable : public SGTGOComponent
{
protected:
	Ogre::SceneNode *mNode;

public:
	SGTGOCNodeRenderable(void) {}
	virtual ~SGTGOCNodeRenderable(void) {}
	virtual void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg);
	virtual void UpdatePosition(Ogre::Vector3 position);
	virtual void UpdateOrientation(Ogre::Quaternion orientation);

	Ogre::SceneNode *GetNode() { return mNode; }
};

class SGTDllExport SGTGOCViewContainer : public SGTGOCNodeRenderable
{
protected:
	std::list<SGTGOCViewComponent*> mItems;

public:
	SGTGOCViewContainer(void);
	~SGTGOCViewContainer(void);

	goc_id_family& GetFamilyID() const { static std::string name = "GOCView"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "GOCViewContainer"; return name; }

	void AddItem(SGTGOCViewComponent *item);
	SGTGOCViewComponent* GetItem(Ogre::String type);
	void RemoveItem(SGTGOCViewComponent *item);
	void SetOwner(SGTGameObject *go);
	std::list<SGTGOCViewComponent*>::iterator GetItemIterator() { return mItems.begin(); }
	std::list<SGTGOCViewComponent*>::iterator GetItemIteratorEnd() { return mItems.end(); }

	void UpdateScale(Ogre::Vector3 scale);

	void ShowEditorVisual(bool show);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "GOCView"; *pFn = (SGTSaveableInstanceFn)&NewInstance; }
	static SGTGOCViewContainer* NewInstance() { return new SGTGOCViewContainer; }
};


//Some Basic View components

class SGTDllExport SGTMeshRenderable : public SGTGOCViewComponentEDT
{
private:
	Ogre::Entity *mEntity;
	void Create(Ogre::String meshname, bool shadowcaster);

public:
	SGTMeshRenderable() {}
	SGTMeshRenderable(Ogre::String meshname, bool shadowcaster);
	~SGTMeshRenderable();

	Ogre::MovableObject* GetEditorVisual() { return mEntity; }
	void AttachToNode(Ogre::SceneNode *node);

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	virtual std::string& TellName() { static std::string name = "MeshRenderable"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "MeshRenderable"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTMeshRenderable* NewInstance() { return new SGTMeshRenderable; };
};

class SGTDllExport SGTMeshDebugRenderable : public SGTGOCNodeRenderable
{
private:
	Ogre::Entity *mEntity;

public:
	SGTMeshDebugRenderable() {}
	SGTMeshDebugRenderable(Ogre::String meshname);
	~SGTMeshDebugRenderable();
	goc_id_type& GetComponentID() const { static std::string name = "MeshDebugRenderable"; return name; } 
	goc_id_family& GetFamilyID() const { static std::string name = "MeshDebugRenderable"; return name; } 
	void SetOwner(SGTGameObject *go);
};

class SGTDllExport SGTPfxRenderable : public SGTGOCViewComponentEDT
{
private:
	Ogre::ParticleSystem *mParticleSystem;
	Ogre::Entity *mEditorVisual;
	Ogre::String mParticleResource;
	void Create(Ogre::String pfxresname);

public:
	SGTPfxRenderable() {}
	SGTPfxRenderable(Ogre::String pfxresname);
	~SGTPfxRenderable();

	Ogre::MovableObject* GetEditorVisual() { return mEditorVisual; }
	void AttachToNode(Ogre::SceneNode *node);

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);

	void ShowEditorVisual(bool show);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "ParticleSystem"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "ParticleSystem"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTPfxRenderable* NewInstance() { return new SGTPfxRenderable; };
};


class SGTDllExport SGTSound3D : public SGTGOCViewComponentEDT
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
	SGTSound3D() {}
	SGTSound3D(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBuffered);
	~SGTSound3D();

	Ogre::MovableObject* GetEditorVisual() { return mEditorVisual; }
	void AttachToNode(Ogre::SceneNode *node);

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);

	void ShowEditorVisual(bool show);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "Sound3D"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "Sound3D"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSound3D* NewInstance() { return new SGTSound3D; };
};


class SGTDllExport SGTLocalLightRenderable : public SGTGOCViewComponentEDT
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
	SGTLocalLightRenderable() {}
	SGTLocalLightRenderable(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity);
	~SGTLocalLightRenderable();

	Ogre::MovableObject* GetEditorVisual() { return mEditorVisual; }
	void AttachToNode(Ogre::SceneNode *node);

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);

	void ShowEditorVisual(bool show);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "LocalLight"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "LocalLight"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTLocalLightRenderable* NewInstance() { return new SGTLocalLightRenderable; };
};