
#include "SGTScriptedCollisionCallback.h"
#include "SGTMain.h"

SGTScriptedCollisionCallback::SGTScriptedCollisionCallback(void)
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS");
	std::vector<SGTScriptParam> params;
	mScript = SGTScriptSystem::GetInstance().CreateInstance("CollisionCallback.lua", params);
}

SGTScriptedCollisionCallback::~SGTScriptedCollisionCallback(void)
{
	SGTMessageSystem::Instance().QuitNewsgroup(this, "REPARSE_SCRIPTS");
}

void SGTScriptedCollisionCallback::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "REPARSE_SCRIPTS")
	{
		std::vector<SGTScriptParam> params;
		mScript = SGTScriptSystem::GetInstance().CreateInstance(mScript.GetScriptName(), params);
	}
}