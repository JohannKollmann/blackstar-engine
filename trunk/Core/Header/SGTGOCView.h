
#pragma once

#include "SGTGOComponent.h"
#include "SGTIncludes.h"
#include "Ogre.h"
#include "OgreOggSound.h"
#include "SGTGOCEditorInterface.h"

class SGTGOCViewComponent : public SGTSaveable
{
public:
	virtual ~SGTGOCViewComponent() {};
	virtual Ogre::MovableObject* GetEditorVisual() = 0;
	virtual void AttachToNode(Ogre::SceneNode *node) = 0;
	virtual void ShowEditorVisual(bool show) = 0;
	virtual Ogre::String GetTypeName() = 0;
};

class SGTDllExport SGTGOCNodeRenderable : public SGTGOComponent
{
protected:
	Ogre::SceneNode *mNode;

public:
	SGTGOCNodeRenderable(void) {}
	virtual ~SGTGOCNodeRenderable(void) {}
	virtual void UpdatePosition(Ogre::Vector3 position);
	virtual void UpdateOrientation(Ogre::Quaternion orientation);

	Ogre::SceneNode *GetNode() { return mNode; }
};

class SGTDllExport SGTGOCViewContainer : public SGTGOCNodeRenderable
{
private:
	std::list<SGTGOCViewComponent*> mItems;

public:
	SGTGOCViewContainer(void);
	~SGTGOCViewContainer(void);

	goc_id_family& GetFamilyID() const { static std::string name = "GOCView"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "GOCViewContainer"; return name; }

	void AddItem(SGTGOCViewComponent *item);
	SGTGOCViewComponent* GetItem(Ogre::String type);
	void RemoveItem(Ogre::String type);
	void SetOwner(SGTGameObject *go);
	std::list<SGTGOCViewComponent*>::iterator GetItemIterator() { return mItems.begin(); }
	std::list<SGTGOCViewComponent*>::iterator GetItemIteratorEnd() { return mItems.end(); }

	void UpdateScale(Ogre::Vector3 scale);

	void ShowEditorVisual(bool show);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "GOCViewContainer"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "GOCViewContainer"; *pFn = (SGTSaveableInstanceFn)&NewInstance; }
	static SGTSaveable* NewInstance() { return new SGTGOCViewContainer; }
};


//Some Basic View components

class SGTDllExport SGTMeshRenderable : public SGTGOCEditorInterface, public SGTGOCViewComponent
{
private:
	Ogre::Entity *mEntity;
	void Create(Ogre::String meshname, bool shadowcaster);

public:
	SGTMeshRenderable() {}
	SGTMeshRenderable(Ogre::String meshname, bool shadowcaster);
	~SGTMeshRenderable();

	virtual Ogre::MovableObject* GetEditorVisual() { return mEntity; }
	virtual void AttachToNode(Ogre::SceneNode *node);
	virtual void ShowEditorVisual(bool show) {};

	virtual void CreateFromDataMap(SGTDataMap *parameters);
	virtual void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);
	virtual bool IsViewComponent() { return true; }

	virtual void Save(SGTSaveSystem& mgr);
	virtual void Load(SGTLoadSystem& mgr);
	virtual std::string& TellName() { static std::string name = "MeshRenderable"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "MeshRenderable"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance();
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTMeshRenderable(); }

	virtual std::string GetTypeName() { return "MeshRenderable"; }
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

	void Save(SGTSaveSystem& mgr) {};
	void Load(SGTLoadSystem& mgr) {};
	virtual std::string& TellName() { static std::string name = "MeshDebugRenderable"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "MeshDebugRenderable"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTMeshDebugRenderable; };
};

class SGTDllExport SGTPfxRenderable : public SGTGOCEditorInterface, public SGTGOCViewComponent
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
	bool IsViewComponent() { return true; }

	void ShowEditorVisual(bool show);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "ParticleSystem"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "ParticleSystem"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTPfxRenderable; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTPfxRenderable(); }

	std::string GetTypeName() { return "ParticleSystem"; }
};


class SGTDllExport SGTSound3D : public SGTGOCEditorInterface, public SGTGOCViewComponent
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
	bool IsViewComponent() { return true; }

	void ShowEditorVisual(bool show);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "Sound3D"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "Sound3D"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTSound3D; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTSound3D(); }

	std::string GetTypeName() { return "Sound3D"; }
};


class SGTDllExport SGTLocalLightRenderable : public SGTGOCEditorInterface, public SGTGOCViewComponent
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
	bool IsViewComponent() { return true; }

	void ShowEditorVisual(bool show);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "LocalLight"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "LocalLight"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTLocalLightRenderable; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTLocalLightRenderable(); }

	std::string GetTypeName() { return "LocalLight"; }
};