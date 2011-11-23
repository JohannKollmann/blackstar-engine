
#pragma once

#include "IceIncludes.h"
#include <map>
#include "IceProcessNode.h"
#include "Ice3D.h"
#include "IceScriptedProcess.h"

namespace Ice
{
	class PlayAnimationProcess;
	class ProcessNodeQueue;
	class TimerProcess;
	class OrientationBlendProcess;

	class DllExport ProcessNodeManager
	{

	private:
		int mIDCounter;

		std::unordered_map<int, std::shared_ptr<ProcessNode>> mProcessNodes;

	public:
		ProcessNodeManager() : mIDCounter(0) {}
		virtual ~ProcessNodeManager() {}

		std::shared_ptr<DayCycleProcess> CreateDayCycleProcess(std::vector<ScriptParam> params, int startTimeH, int startTimeM, int endtimeH, int endtimeM);
		std::shared_ptr<ScriptedProcess> CreateScriptedProcess(std::vector<ScriptParam> params);
		std::shared_ptr<PlayAnimationProcess> CreatePlayAnimationProcess(Ogre::AnimationState *state);
		std::shared_ptr<TimerProcess> CreateTimerProcess(float time);
		std::shared_ptr<OrientationBlendProcess> CreateOrientationBlendProcess(Transformable3D *object, Ogre::Quaternion targetOrientation, float duration);
		std::shared_ptr<ProcessNodeQueue> CreateProcessNodeQueue();
		std::shared_ptr<LoopedProcessNodeQueue> CreateLoopedProcessNodeQueue();

		void RemoveProcessNode(int processID);

		std::shared_ptr<ProcessNode> GetProcessNode(int processID);

		static ProcessNodeManager& Instance();

		static std::vector<ScriptParam> Lua_ProcessTimer_Create(Script& caller, std::vector<ScriptParam> vParams);
	};

}
