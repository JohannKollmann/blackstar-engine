
#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "Ogre.h"
#include "OgreOggSound.h"
#include "IceGOCEditorInterface.h"
#include "IceGOCOgreNode.h"
#include "IceGOCScriptMakros.h"

namespace Ice
{

	class DllExport GOCEditorVisualised : public GOCOgreNodeUser
	{
	protected:
		Ogre::Entity *mEditorVisual;

	protected:
		virtual Ogre::String GetEditorVisualMeshName() = 0;

	public:
		GOCEditorVisualised() : mEditorVisual(nullptr) {}
		virtual ~GOCEditorVisualised();
		virtual void ShowEditorVisual(bool show);
	};

	class DllExport GOCMeshRenderable : public GOCOgreNodeUserEditable
	{
	private:
		Ogre::Entity *mEntity;
		void Create(Ogre::String meshname, bool shadowcaster);
		void _clear();

	public:
		GOCMeshRenderable() : mEntity(nullptr) {}
		GOCMeshRenderable(Ogre::String meshname, bool shadowcaster);
		virtual ~GOCMeshRenderable();

		void NotifyOwnerGO();

		Ogre::Entity* GetEntity() { return mEntity; }

		virtual void SetParameters(DataMap *parameters);
		virtual void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Mesh"; }
		Ogre::String& GetComponentID() const { static Ogre::String name = "GOCMeshRenderable"; return name; };

		virtual void Save(LoadSave::SaveSystem& mgr);
		virtual void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "GOCMeshRenderable"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance();
		GOCEditorInterface* New() { return new GOCMeshRenderable(); }


		std::vector<ScriptParam> SetVisible(Script &caller, std::vector<ScriptParam> &params) { mEntity->setVisible(params[0].getBool());  return std::vector<ScriptParam>(); };
		DEFINE_TYPEDGOCLUAMETHOD(GOCMeshRenderable, SetVisible, "bool")
	};

	class DllExport GOCBillboard : public GOCOgreNodeUser, public GOCStaticEditorInterface
	{
	private:
		Ogre::BillboardSet *mBillboardSet;
		Ogre::MaterialPtr mMaterial;
		Ogre::String mMaterialName;
		Ogre::Vector3 mDimensions;
		float mRadius;
		float mDensity;
		Ogre::String mTextureName;
		void _create();
		void _clear();

	public:
		GOCBillboard() : mBillboardSet(nullptr) {}
		GOCBillboard(Ogre::String materialName) : mMaterialName(materialName) { _create(); }
		virtual ~GOCBillboard() { _clear(); }

		void NotifyOwnerGO();

		Ogre::BillboardSet* GetBillboardSet() { return mBillboardSet; }

		BEGIN_GOCEDITORINTERFACE(GOCBillboard, "Billboard")
			PROPERTY_STRING(mMaterialName, "Material", "")
			PROPERTY_STRING(mTextureName, "Texture", "")
			PROPERTY_VECTOR3(mDimensions, "Dimensions", Ogre::Vector3(1,1,1))
			PROPERTY_FLOAT(mRadius, "Radius", 1.0)
			PROPERTY_FLOAT(mDensity, "Density", 1.0)
		END_GOCEDITORINTERFACE
		void OnSetParameters() { _create(); }

		Ogre::String& GetComponentID() const { static Ogre::String name = "GOCBillboard"; return name; };

		virtual void Save(LoadSave::SaveSystem& mgr);
		virtual void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "GOCBillboard"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance();
	};

	class DllExport GOCPfxRenderable : public GOCEditorVisualised, public GOCEditorInterface
	{
	private:
		Ogre::ParticleSystem *mParticleSystem;
		Ogre::String mParticleResource;
		void Create(Ogre::String pfxresname);
		void _clear();

	public:
		GOCPfxRenderable() : mParticleSystem(nullptr) {}
		GOCPfxRenderable(Ogre::String pfxresname);
		~GOCPfxRenderable();

		Ogre::String& GetComponentID() const { static Ogre::String name = "ParticleSystem"; return name; };

		Ogre::String GetEditorVisualMeshName() { return "Editor_Particle.mesh"; }

		void NotifyOwnerGO();

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ParticleSystem"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCPfxRenderable; };
		GOCEditorInterface* New() { return new GOCPfxRenderable(); }
		Ogre::String GetLabel() { return "PFX"; }
		GOComponent* GetGOComponent() { return this; }
	};


	class DllExport GOCSound3D : public GOCOgreNodeUser, public GOCEditorInterface
	{
	private:
		OgreOggSound::OgreOggISound *mSound;
		Ogre::String mAudioFile;
		float mReferenceDistance;
		float mMaxDistance;
		bool mStreamed;
		bool mLooped;
		bool mPrebuffered;
		void Create(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBuffered);
		void _clear();

	public:
		GOCSound3D() : mSound(nullptr) {}
		GOCSound3D(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBuffered);
		~GOCSound3D();

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);

		void NotifyOwnerGO();

		Ogre::String& GetComponentID() const { static Ogre::String name = "GOCSound3D"; return name; };

		Ogre::String GetEditorVisualMeshName() { return "Editor_Sound.mesh"; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "GOCSound3D"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCSound3D; };
		GOCEditorInterface* New() { return new GOCSound3D(); }
		Ogre::String GetLabel() { return "Sound3D"; }
		GOComponent* GetGOComponent() { return this; }
	};


	class DllExport GOCLocalLightRenderable : public GOCOgreNodeUser, public GOCEditorInterface
	{
	private:
		Ogre::Light *mLight;
		Ogre::ColourValue mOriginalColour;
		Ogre::String mEditorMeshName;
		float mFlickerSpeed;
		float mFlickerIntensity;
		float mFlickerOffset;
		void Create(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity);
		void _clear();

	public:
		GOCLocalLightRenderable() : mLight(nullptr) {}
		GOCLocalLightRenderable(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity);
		~GOCLocalLightRenderable();

		void NotifyOwnerGO();

		Ogre::String& GetComponentID() const { static Ogre::String name = "LocalLight"; return name; };

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);

		Ogre::String GetEditorVisualMeshName() { return mEditorMeshName; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "LocalLight"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCLocalLightRenderable; };
		GOCEditorInterface* New() { return new GOCLocalLightRenderable(); }
		GOComponent* GetGOComponent() { return this; }
		Ogre::String GetLabel() { return "Light"; }
	};

};