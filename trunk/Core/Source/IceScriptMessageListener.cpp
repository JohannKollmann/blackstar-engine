
#include "IceScriptMessageListener.h"
#include "IceScriptSystem.h"

namespace Ice
{

	ScriptMessageListener::ScriptMessageListener(ScriptParam &callbackFn)
	{
		assert(callbackFn.getType() == ScriptParam::PARM_TYPE_FUNCTION);

		mScriptCallback = callbackFn;
	}


	ScriptMessageListener::~ScriptMessageListener(void)
	{
	}

	void ScriptMessageListener::ReceiveMessage(Msg &msg)
	{
		std::vector<ScriptParam> params;
		while (msg.mData.HasNext())
		{
			GenericProperty prop = msg.mData.GetNext();
			prop.GetAsScriptParam(params);
		}
		ScriptSystem::RunCallbackFunction(mScriptCallback, params);
	}

}