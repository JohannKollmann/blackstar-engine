
#include "IceProcessNodeManager.h"
#include "IcePlayAnimationProcess.h"
#include "IceOrientationBlendProcess.h"
#include "IceProcessNodeQueue.h"
#include "IceTimerProcess.h"
#include "IceUtils.h"
#include "IceScriptSystem.h"
#include "IceFollowPathwayProcess.h"

namespace Ice
{

	std::shared_ptr<ProcessNode> ProcessNodeManager::GetProcessNode(int processID)
	{
		auto i = mProcessNodes.find(processID);
		if (i == mProcessNodes.end())
		{
			IceWarning("Process ID not found")
			return std::shared_ptr<ProcessNode>(nullptr);
		}
		return i->second;
	}

	std::shared_ptr<DayCycleProcess> ProcessNodeManager::CreateDayCycleProcess(std::vector<ScriptParam> params, int startTimeH, int startTimeM, int endtimeH, int endtimeM)
	{
		std::shared_ptr<DayCycleProcess> pProcess = std::make_shared<DayCycleProcess>(params, startTimeH, startTimeM, endtimeH, endtimeM);
		pProcess->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, DayCycleProcess>(pProcess)));
		return pProcess;
	}
	std::shared_ptr<ScriptedProcess> ProcessNodeManager::CreateScriptedProcess(std::vector<ScriptParam> params)
	{
		std::shared_ptr<ScriptedProcess> pProcess = std::make_shared<ScriptedProcess>(params);
		pProcess->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, ScriptedProcess>(pProcess)));
		return pProcess;
	}

	std::shared_ptr<LoopedProcessNodeQueue> ProcessNodeManager::CreateLoopedProcessNodeQueue()
	{
		std::shared_ptr<LoopedProcessNodeQueue> pNode = std::make_shared<LoopedProcessNodeQueue>();
		pNode->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, LoopedProcessNodeQueue>(pNode)));
		return pNode;
	}

	std::shared_ptr<ProcessNodeQueue> ProcessNodeManager::CreateProcessNodeQueue()
	{
		std::shared_ptr<ProcessNodeQueue> pNode = std::make_shared<ProcessNodeQueue>();
		pNode->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, ProcessNodeQueue>(pNode)));
		return pNode;
	}

	std::shared_ptr<PlayAnimationProcess> ProcessNodeManager::CreatePlayAnimationProcess(Ogre::AnimationState *state)
	{
		std::shared_ptr<PlayAnimationProcess> pNode = std::make_shared<PlayAnimationProcess>(state);
		pNode->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, PlayAnimationProcess>(pNode)));
		return pNode;
	}

	std::shared_ptr<TimerProcess> ProcessNodeManager::CreateTimerProcess(float time)
	{
		std::shared_ptr<TimerProcess> pNode = std::make_shared<TimerProcess>(time);
		pNode->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, TimerProcess>(pNode)));
		return pNode;
	}

	std::shared_ptr<OrientationBlendProcess> ProcessNodeManager::CreateOrientationBlendProcess(Transformable3D *object, Ogre::Quaternion targetOrientation, float duration)
	{
		std::shared_ptr<OrientationBlendProcess> pNode = std::make_shared<OrientationBlendProcess>(object, targetOrientation, duration);
		pNode->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, OrientationBlendProcess>(pNode)));
		return pNode;
	}

	std::shared_ptr<FollowPathwayProcess> ProcessNodeManager::CreateFollowPathwayProcess(std::shared_ptr<GOCAI> ai, const Ogre::Vector3 &targetPosition, float radius)
	{
		std::shared_ptr<FollowPathwayProcess> pNode = std::make_shared<FollowPathwayProcess>(ai, targetPosition, radius);
		pNode->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, FollowPathwayProcess>(pNode)));
		return pNode;
	}

	void ProcessNodeManager::RemoveProcessNode(int processID)
	{
		auto i = mProcessNodes.find(processID);
		if (i == mProcessNodes.end()) return;
		i->second->_setActive(false);
		mProcessNodes.erase(i);
	}

	//Singleton
	ProcessNodeManager& ProcessNodeManager::Instance()
	{
		static ProcessNodeManager TheOneAndOnly;
		return TheOneAndOnly;
	}

	std::vector<ScriptParam> ProcessNodeManager::Lua_ProcessTimer_Create(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "float", true);
		if (err == "")
		{
			auto pNode = Instance().CreateTimerProcess(vParams[0].getFloat());
			pNode->SetScriptCallback(vParams[1]);
			SCRIPT_RETURNVALUE(pNode->GetProcessID())
		}
		else SCRIPT_RETURNERROR(err)
	}

}