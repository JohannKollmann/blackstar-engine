#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceMain.h"
#include "Ice3D.h"
#include "IceGOCEditorInterface.h"
#include "IceMessageListener.h"
#include "IceGOCView.h"
#include "IceSpline.h"

namespace Ice
{
	class GOCMover;

	class DllExport AnimKey
	{
	protected:
		GOCMover *mMover;
		float mTimeToNextKey;

	public:
		AnimKey() : mMover(nullptr), mTimeToNextKey(1) {}
		virtual ~AnimKey() {}

		virtual void SetMover(GOCMover *mover) {};

		float GetTimeToNextKey() { return mTimeToNextKey; }

		GameObject* CreateSuccessor();
	};

	class DllExport GOCAnimKey : public AnimKey, public GOCEditorVisualised, public GOCEditorInterface
	{
		friend class GOCMover;
	private:
		AnimKey *mPredecessor;

	public:
		GOCAnimKey() {}
		GOCAnimKey(AnimKey *pred);
		~GOCAnimKey(void);

		void SetMover(GOCMover *mover);

		goc_id_type& GetComponentID() const { static std::string name = "MoverAnimKey"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation) {}

		Ogre::String GetEditorVisualMeshName() { return "Editor_AnimKey.mesh"; }

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Anim Key"; }
		GOCEditorInterface* New() { return new GOCAnimKey(); }
		GOComponent* GetGOComponent() { return this; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "MoverAnimKey"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCAnimKey; }
	};

	class DllExport GOCMover : public AnimKey, public GOComponentEditable, public MessageListener
	{
		friend class GOCAnimKey;

	private:
		std::vector<GameObject*> mAnimKeys;
		Ogre::String mKeyCallbackScript;
		bool mMoving;
		bool mIsClosed;
		float mfLastPos;
		Spline mSpline;
		Spline mTimeSpline;
		Ogre::ManualObject* mSplineObject;

		void notifyKeyDelete(GOCAnimKey *key);

	public:
		GOCMover(void);
		~GOCMover(void);

		void Init();

		void Trigger();
		void UpdateKeys();

		void ReceiveMessage(Msg &msg);

		goc_id_type& GetComponentID() const { static std::string name = "Mover"; return name; }

		void UpdatePosition(Ogre::Vector3 position) {}
		void UpdateOrientation(Ogre::Quaternion orientation) {}

		Ogre::String GetEditorVisualMeshName() { return "Editor_AnimKey.mesh"; }

		bool IsMoving() { return mMoving; }

		void InsertKey(GameObject *key, AnimKey *pred);

		void OnAddChild(GameObject *child);

		void SetKeyIgnoreParent(bool ignore);

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Mover"; }
		GOCEditorInterface* New() { return new GOCMover(); }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Mover"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCMover; }
	};

}