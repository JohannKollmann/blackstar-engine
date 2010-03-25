#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceMain.h"
#include "Ice3D.h"
#include "IceGOCEditorInterface.h"
#include "IceMessageListener.h"

namespace Ice
{
	class GOCMover;

	class DllExport AnimKey
	{
	protected:
		bool mIsLastKey;
		GOCMover *mMover;

	public:
		AnimKey() : mIsLastKey(true) {}
		virtual ~AnimKey() {}

		void SetMover(GOCMover *mover);

		GameObject* CreateSuccessor();

		bool IsLastKey() { return mIsLastKey; }
	};

	class DllExport GOCAnimKey : public AnimKey, public GOComponent, public GOCEditorInterface
	{
		friend class GOCMover;
	private:
		float mTimeSinceLastKey;

	public:
		GOCAnimKey(void);
		~GOCAnimKey(void);

		goc_id_family& GetFamilyID() const { static std::string name = "MoverAnimKey"; return name; }
		goc_id_type& GetComponentID() const { static std::string name = "MoverAnimKey"; return name; }

		void UpdatePosition(Ogre::Vector3 position) {}
		void UpdateOrientation(Ogre::Quaternion orientation) {}

		void CreateFromDataMap(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		static void GetDefaultParameters(DataMap *parameters);
		void AttachToGO(GameObject *go);
		Ogre::String GetLabel() { return "Anim Key"; }
		static GOCEditorInterface* NewEditorInterfaceInstance() { return new GOCAnimKey(); }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "MoverAnimKey"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCAnimKey; }
	};

	class DllExport GOCMover : public AnimKey, public GOComponent, public GOCEditorInterface, public MessageListener
	{
	private:
		std::vector<GameObject*> mAnimKeys;
		std::list<GameObject*> mCurrentTrack;
		Ogre::String mKeyCallbackScript;
		bool mMoving;

	public:
		GOCMover(void);
		~GOCMover(void);

		GameObject* CreateAnimKey();

		void Trigger();

		void ReceiveMessage(Msg &msg);

		goc_id_family& GetFamilyID() const { static std::string name = "Mover"; return name; }
		goc_id_type& GetComponentID() const { static std::string name = "Mover"; return name; }

		void UpdatePosition(Ogre::Vector3 position) {}
		void UpdateOrientation(Ogre::Quaternion orientation) {}

		bool IsMoving() { return mMoving; }

		void SetKeyIgnoreParent(bool ignore);

		void CreateFromDataMap(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		static void GetDefaultParameters(DataMap *parameters);
		void AttachToGO(GameObject *go);
		Ogre::String GetLabel() { return "Mover"; }
		static GOCEditorInterface* NewEditorInterfaceInstance() { return new GOCMover(); }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Mover"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCMover; }
	};

}