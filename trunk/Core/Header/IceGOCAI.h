
#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "IceGOCCharacterController.h"
#include "IceAIProcess.h"
#include "IceDataMap.h"
#include "IceScriptSystem.h"
#include "IceGOCScriptMakros.h"
#include "IceScriptUser.h"
#include "IceProcessOwner.h"
#include "IceProcessNodeQueue.h"
#include "IceSeeSense.h"

namespace Ice
{

	class DayCycleProcess;
	class AIProcess;

	class DllExport GOCAI : public GOCEditorInterface, public CharacterControllerInput, public ProcessOwner, public SeeSense::Callback, public SeeSense::Origin
	{
	private:
		std::shared_ptr<SeeSense> mSeeSense;

	public:
		GOCAI(void);
		~GOCAI(void);

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_NONE; }

		Ogre::Vector3 GetEyePosition();
		Ogre::Quaternion GetEyeOrientation();

		//Scripting
		std::vector<ScriptParam> Npc_GetObjectVisibility(Script& caller, std::vector<ScriptParam> &vParams);	//eye sense
		std::vector<ScriptParam> Npc_CreateFollowPathwayProcess(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Npc_OpenDialog(Script& caller, std::vector<ScriptParam> &vParams);
		DEFINE_TYPEDGOCLUAMETHOD(GOCAI, Npc_GetObjectVisibility, "int")
		DEFINE_TYPEDGOCLUAMETHOD(GOCAI, Npc_CreateFollowPathwayProcess, "int")
		DEFINE_GOCLUAMETHOD(GOCAI, Npc_OpenDialog)

		void SetOwner(std::weak_ptr<GameObject> go);

		int GetID();

		void Update(float time);

		void ReceiveMessage(Msg &msg);

		void OnSeeSomething(const Ogre::Vector3 &eyeSpacePosition, float distance, float viewFactor, int goI); 

		GOComponent::TypeID& GetComponentID() const { static std::string name = "AI"; return name; }

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		GOCEditorInterface* New() { return new GOCAI(); }
		Ogre::String GetLabel() { return "AI"; }
		GOComponent* GetGOComponent() { return this; }

		bool IsStatic() { return false; }

		std::string& TellName() { static std::string name = "AI"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "AI"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCAI; };
		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
	};

};