#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceMain.h"
#include "Ice3D.h"
#include "IceGOCEditorInterface.h"
#include "IceMessageListener.h"

#include "IceSpline.h"

namespace Ice
{
	class GOCMover;

	class DllExport AnimKey
	{
	protected:
		GOCMover *mMover;
	public:
		AnimKey() : mMover(nullptr) {}
		virtual ~AnimKey() {}

		virtual void SetMover(GOCMover *mover) {};

		GameObject* CreateSuccessor();
	};

	class DllExport GOCAnimKey : public AnimKey, public GOComponent, public GOCEditorInterface
	{
		friend class GOCMover;
	private:
		float mTotalStayTime;
		float mTimeSinceLastKey;
		AnimKey *mPredecessor;
		GOCAnimKey() {}

	public:
		GOCAnimKey(AnimKey *pred);
		~GOCAnimKey(void);

		void SetMover(GOCMover *mover);

		goc_id_family& GetFamilyID() const { static std::string name = "MoverAnimKey"; return name; }
		goc_id_type& GetComponentID() const { static std::string name = "MoverAnimKey"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation) {}

		void* GetUserData();
		void InjectUserData(void* data);

		float GetTimeSinceLastKey() { return mTimeSinceLastKey; }
		float GetTotalStayTime() { return mTotalStayTime; }

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
		friend class GOCAnimKey;

	private:
		std::vector<GameObject*> mAnimKeys;
		std::list<GameObject*> mCurrentTrack;
		Ogre::String mKeyCallbackScript;
		bool mMoving;
		float mfLastPos;
		Spline mSpline;
		Spline mTimeSpline;
		Ogre::ManualObject* mSplineObject;

		void notifyKeyDelete(GOCAnimKey *key);

	public:
		GOCMover(void);
		~GOCMover(void);

		void Trigger();
		void UpdateKeys();

		void ReceiveMessage(Msg &msg);

		goc_id_family& GetFamilyID() const { static std::string name = "Mover"; return name; }
		goc_id_type& GetComponentID() const { static std::string name = "Mover"; return name; }

		void UpdatePosition(Ogre::Vector3 position) {}
		void UpdateOrientation(Ogre::Quaternion orientation) {}

		bool IsMoving() { return mMoving; }

		void InsertKey(GameObject *key, AnimKey *pred);

		void OnAddChild(GameObject *child);

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