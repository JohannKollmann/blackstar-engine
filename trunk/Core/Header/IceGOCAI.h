
#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "IceGOCCharacterController.h"
#include "IceAIState.h"
#include "IceScriptedAIState.h"
#include "IceDataMap.h"
#include "IceScriptSystem.h"
#include "IceGOCScriptMakros.h"
#include "IceScriptUser.h"

namespace Ice
{

	class DllExport GOCAI : public GOCEditorInterface, public CharacterControllerInput, public ScriptUser
	{

	private:
		/*
		Die Idle-Routine des AI Objekts (Tagesablauf), wird immer gelooped.
		*/
		std::vector<DayCycle*> mIdleQueue;

		/*
		Priority Queue von Events, ueberlagert immer Idle-Routine.
		*/
		std::vector<AIState*> mActionQueue;

	public:
		GOCAI(void);
		~GOCAI(void);

		//Scripting
		std::vector<ScriptParam> Npc_AddState(std::vector<ScriptParam> &params);
		std::vector<ScriptParam> Npc_KillActiveState(std::vector<ScriptParam> &params);
		std::vector<ScriptParam> Npc_ClearQueue(std::vector<ScriptParam> &params);
		std::vector<ScriptParam> Npc_AddTA(std::vector<ScriptParam> &params);
		std::vector<ScriptParam> Npc_GotoWP(std::vector<ScriptParam> &params);
		DEFINE_TYPEDGOCLUAMETHOD(GOCAI, Npc_AddState, "string")
		DEFINE_GOCLUAMETHOD(GOCAI, Npc_KillActiveState)
		DEFINE_GOCLUAMETHOD(GOCAI, Npc_ClearQueue)
		DEFINE_TYPEDGOCLUAMETHOD(GOCAI, Npc_AddTA, "string float float")
		DEFINE_TYPEDGOCLUAMETHOD(GOCAI, Npc_GotoWP, "string")

		void AddState(AIState *state);
		void AddDayCycleState(DayCycle *state);
		void ClearActionQueue();
		void ClearIdleQueue();
		void SelectState();
		void LeaveActiveActionState();

		int GetThisID();

		void SetOwner(GameObject *go);

		int GetID();

		void Update(float time);

		void ReceiveObjectMessage(const Msg &msg);

		void OnScriptReload();

		void OnReceiveMessage(Msg &msg);

		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "AI"; return name; }

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