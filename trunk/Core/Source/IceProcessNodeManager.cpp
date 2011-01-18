
#include "IceProcessNodeManager.h"
#include "IcePlayAnimationProcess.h"
#include "IceProcessNodeQueue.h"
#include "IceTimerProcess.h"
#include "IceUtils.h"
#include "IceScriptSystem.h"

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

	std::shared_ptr<TimerProcess> ProcessNodeManager::CreateTimerProcess(ScriptParam callbackFn, float time)
	{
		std::shared_ptr<TimerProcess> pNode = std::make_shared<TimerProcess>(callbackFn, time);
		pNode->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, TimerProcess>(pNode)));
		return pNode;
	}

	void ProcessNodeManager::RemoveProcessNode(int processID)
	{
		auto i = mProcessNodes.find(processID);
		if (i == mProcessNodes.end()) return;
		i->second->SetActive(false);
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
		auto err = Utils::TestParameters(vParams, "float function");
		if (err == "")
		{
			auto pNode = Instance().CreateTimerProcess(vParams[1], vParams[0].getFloat());
			SCRIPT_RETURNVALUE(pNode->GetProcessID())
		}
		else SCRIPT_RETURNERROR(err)
		SCRIPT_RETURN()
	}

}