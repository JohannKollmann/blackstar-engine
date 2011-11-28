
#pragma once

#include "IceIncludes.h"
#include "IceScriptSystem.h"
#include "IceGOCPhysics.h"
#include "IceGOCView.h"
#include "IceGOCPlayerInput.h"
#include "IceGOCCameraController.h"
#include "IceGOCAnimatedCharacter.h"
#include "IceGOCWaypoint.h"
#include "IceGOCAI.h"
#include "IceGOCMover.h"
#include "IceGOCScript.h"
#include "IceGOCForceField.h"
#include "IceGOCJoint.h"
#include "IceSceneManager.h"
#include "IceGameObject.h"
#include "IceProcessNodeManager.h"
#include "IceScriptedProcess.h"

namespace Ice
{
	DllExport void InitCoreLuaFunctions()
	{
		//Shared Lua functions

		/**
		Joins a newsgroup.
		Example usage: ReceiveGlobalMessage(GlobalMessageIDs::UPDATE_PER_FRAME, myFunc)
		*/
		ScriptSystem::GetInstance().ShareCFunction("ReceiveGlobalMessage", &ScriptSystem::Lua_JoinNewsgroup);

		/**
		Logs a message to the log.
		Example usage: LogMessage("Everything is okay!")
		*/
		ScriptSystem::GetInstance().ShareCFunction("LogMessage", &SceneManager::Lua_LogMessage);

		/**
		Retrieves the object id of the script instance.
		@pre: Script is an object script.
		Example usage: id = This()
		*/
		ScriptSystem::GetInstance().ShareCFunction("This", &SceneManager::Lua_GetThis);

		ScriptSystem::GetInstance().ShareCFunction("GetObjectIDByName", &SceneManager::Lua_GetObjectByName);

		/**
		Loads a level.
		Example usage: LoadLevel("World.eew")
		*/
		ScriptSystem::GetInstance().ShareCFunction("LoadLevel", &SceneManager::Lua_LoadLevel);
		ScriptSystem::GetInstance().ShareCFunction("SaveLevel", &SceneManager::Lua_SaveLevel);

		/**
		Object get/set methods.
		Example usage: SetPosition(id, 1.0, 2.5, 3.1)
		*/
		ScriptSystem::GetInstance().ShareCFunction("Object_Create", &SceneManager::Lua_CreateGameObject);
		ScriptSystem::GetInstance().ShareCFunction("Object_AddComponent", &GameObject::Lua_AddComponent);
		ScriptSystem::GetInstance().ShareCFunction("Object_SetParent", &GameObject::Lua_SetParent);
		ScriptSystem::GetInstance().ShareCFunction("Object_SetPosition", &GameObject::Lua_SetObjectPosition);
		ScriptSystem::GetInstance().ShareCFunction("Object_SetOrientation", &GameObject::Lua_SetObjectOrientation);
		ScriptSystem::GetInstance().ShareCFunction("Object_SetScale", &GameObject::Lua_SetObjectScale);
		ScriptSystem::GetInstance().ShareCFunction("Object_GetName", &GameObject::Lua_GetObjectName);
		ScriptSystem::GetInstance().ShareCFunction("Object_HasScriptListener", &GameObject::Lua_HasScriptListener);
		ScriptSystem::GetInstance().ShareCFunction("Object_GetReferenced", &GameObject::Lua_GetReferencedObjectByName);
		ScriptSystem::GetInstance().ShareCFunction("Object_IsNpc", &GameObject::Lua_IsNpc);
		ScriptSystem::GetInstance().ShareCFunction("Object_Play3DSound", &GameObject::Lua_Object_Play3DSound);
		ScriptSystem::GetInstance().ShareCFunction("Object_GetDistToObject", &GameObject::Lua_Object_GetDistToObject);

		//Object message sending / receiving
		ScriptSystem::GetInstance().ShareCFunction("Object_ReceiveMessage", &GameObject::Lua_ReceiveObjectMessage);
		ScriptSystem::GetInstance().ShareCFunction("Object_SendMessage", &GameObject::Lua_SendObjectMessage);

		//Scripted properties
		ScriptSystem::GetInstance().ShareCFunction("Object_SetProperty", &GameObject::Lua_Object_SetProperty);
		ScriptSystem::GetInstance().ShareCFunction("Object_GetProperty", &GameObject::Lua_Object_GetProperty);
		ScriptSystem::GetInstance().ShareCFunction("Object_HasProperty", &GameObject::Lua_Object_HasProperty);

		//Tells the npc to go to a certain waypoint.
		ScriptSystem::GetInstance().ShareCFunction("Npc_GetObjectVisibility", &GOCAI::Lua_Npc_GetObjectVisibility);
		ScriptSystem::GetInstance().ShareCFunction("Npc_CreateFollowPathwayProcess", &GOCAI::Lua_Npc_CreateFollowPathwayProcess);
		ScriptSystem::GetInstance().ShareCFunction("Npc_OpenDialog", &GOCAI::Lua_Npc_OpenDialog);

		ScriptSystem::GetInstance().ShareCFunction("GetPlayer", &SceneManager::Lua_GetPlayer);

		ScriptSystem::GetInstance().ShareCFunction("Forcefield_SetActive", &GOCForceField::Lua_Forcefield_Activate);

		ScriptSystem::GetInstance().ShareCFunction("Character_GetGroundMaterial", &GOCCharacterController::Lua_Character_GetGroundMaterial);
		ScriptSystem::GetInstance().ShareCFunction("Character_SetSpeedFactor", &GOCCharacterController::Lua_Character_SetSpeed);

		ScriptSystem::GetInstance().ShareCFunction("Mesh_ReplaceMaterial", &GOCMeshRenderable::Lua_ReplaceMaterial);

		ScriptSystem::GetInstance().ShareCFunction("PFX_SetEmitting", &GOCPfxRenderable::Lua_SetEmitting);

		ScriptSystem::GetInstance().ShareCFunction("Sound3D_StartFade", &GOCSound3D::Lua_StartFade);

		ScriptSystem::GetInstance().ShareCFunction("AnimProcess_Create", &GOCAnimatedCharacter::Lua_AnimProcess_Create);
		ScriptSystem::GetInstance().ShareCFunction("Process_AddDependency", &ProcessNode::Lua_AddDependency);
		ScriptSystem::GetInstance().ShareCFunction("Process_Kill", &ProcessNode::Lua_KillProcess);
		ScriptSystem::GetInstance().ShareCFunction("Process_Activate", &ProcessNode::Lua_Activate);
		ScriptSystem::GetInstance().ShareCFunction("Process_Suspend", &ProcessNode::Lua_Suspend);
		ScriptSystem::GetInstance().ShareCFunction("ProcessQueue_Create", &ProcessNodeQueue::Lua_ProcessQueue_Create);
		ScriptSystem::GetInstance().ShareCFunction("ProcessQueue_EnqueueItem", &ProcessNodeQueue::Lua_ProcessQueue_Enqueue);
		ScriptSystem::GetInstance().ShareCFunction("ProcessQueue_PushItem", &ProcessNodeQueue::Lua_ProcessQueue_PushFront);
		ScriptSystem::GetInstance().ShareCFunction("LoopedProcessQueue_Create", &LoopedProcessNodeQueue::Lua_LoopedProcessQueue_Create);

		ScriptSystem::GetInstance().ShareCFunction("Process_SetEnterCallback", &ScriptedProcess::Lua_SetEnterCallback);
		ScriptSystem::GetInstance().ShareCFunction("Process_SetUpdateCallback", &ScriptedProcess::Lua_SetUpdateCallback);
		ScriptSystem::GetInstance().ShareCFunction("Process_SetLeaveCallback", &ScriptedProcess::Lua_SetLeaveCallback);
		ScriptSystem::GetInstance().ShareCFunction("ScriptedProcess_Create", &ScriptedProcess::Lua_ScriptedProcess_Create);
		ScriptSystem::GetInstance().ShareCFunction("DayCycleProcess_Create", &DayCycleProcess::Lua_DayCycleProcess_Create);

		ScriptSystem::GetInstance().ShareCFunction("TimerProcess_Create", &ProcessNodeManager::Lua_ProcessTimer_Create);

		//Triggers a mover.
		ScriptSystem::GetInstance().ShareCFunction("Mover_Trigger", &GOCMover::Lua_TriggerMover);
		ScriptSystem::GetInstance().ShareCFunction("Mover_Pause", &GOCMover::Lua_PauseMover);
		ScriptSystem::GetInstance().ShareCFunction("Mover_Stop", &GOCMover::Lua_StopMover);
		ScriptSystem::GetInstance().ShareCFunction("Mover_AddKey", &GOCMover::Lua_AddKey);
		ScriptSystem::GetInstance().ShareCFunction("Mover_SetLookAtObject", &GOCMover::Lua_SetLookAtObject);
		ScriptSystem::GetInstance().ShareCFunction("Mover_SetNormalLookAtObject", &GOCMover::Lua_SetNormalLookAtObject);

		//Joint
		ScriptSystem::GetInstance().ShareCFunction("Joint_SetActors", &GOCJoint::Lua_SetActorObjects);

		//Trigger
		ScriptSystem::GetInstance().ShareCFunction("Trigger_SetActive", &GOCTrigger::Lua_Trigger_SetActive);

		//Physical Body
		ScriptSystem::GetInstance().ShareCFunction("Body_GetSpeed", &GOCRigidBody::Lua_Body_GetSpeed);
		ScriptSystem::GetInstance().ShareCFunction("Body_AddImpulse", &GOCRigidBody::Lua_Body_AddImpulse);

		//Time get/set methods
		ScriptSystem::GetInstance().ShareCFunction("GetGameTimeHour", &SceneManager::Lua_GetGameTimeHour);
		ScriptSystem::GetInstance().ShareCFunction("GetGameTimeMinutes", &SceneManager::Lua_GetGameTimeMinutes);
		ScriptSystem::GetInstance().ShareCFunction("SetGameTime", &SceneManager::Lua_SetGameTime);
		ScriptSystem::GetInstance().ShareCFunction("SetGameTimeScale", &SceneManager::Lua_SetGameTimeScale);

		ScriptSystem::GetInstance().ShareCFunction("Play3DSound", &SceneManager::Lua_Play3DSound);
		ScriptSystem::GetInstance().ShareCFunction("CreateMaterialProfile", &SceneManager::Lua_CreateMaterialProfile);

		ScriptSystem::GetInstance().ShareCFunction("GetFocusObject", &SceneManager::Lua_GetFocusObject);

		ScriptSystem::GetInstance().ShareCFunction("Node_SetVisible", &GOCOgreNode::Lua_SetVisible);

		ScriptSystem::GetInstance().ShareCFunction("ConcatToString", &SceneManager::Lua_ConcatToString);

		ScriptSystem::GetInstance().ShareCFunction("GetRandomNumber", &SceneManager::Lua_GetRandomNumber);
	}
}