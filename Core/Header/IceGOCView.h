
#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "Ogre.h"
#include "OgreOggSound.h"
#include "IceGOCEditorInterface.h"
#include "IceGOCOgreNode.h"

namespace Ice
{

	class DllExport GOCEditorVisualised : public GOCOgreNodeUser
	{
	private:
		Ogre::Entity *mEditorVisual;

	protected:
		virtual Ogre::String GetEditorVisualMeshName() = 0;

	public:
		GOCEditorVisualised() : mEditorVisual(nullptr) {}
		virtual ~GOCEditorVisualised();
		void ShowEditorVisual(bool show);
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

		Ogre::String& GetComponentID() const { static Ogre::String name = "GOCSound3D"; return name; };

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