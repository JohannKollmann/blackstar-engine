
#include "IceScriptMessageListener.h"
#include "IceScriptSystem.h"

namespace Ice
{

	ScriptMessageListener::ScriptMessageListener(ScriptParam &callbackFn)
	{
		IceAssert((callbackFn.getType() == ScriptParam::PARM_TYPE_FUNCTION));

		mScriptCallback = callbackFn;
	}


	ScriptMessageListener::~ScriptMessageListener(void)
	{
	}

	void ScriptMessageListener::ReceiveMessage(Msg &msg)
	{
		std::vector<ScriptParam> params;
		while (msg.params.HasNext())
		{
			GenericProperty *prop = msg.params.GetNext().data;
			prop->GetAsScriptParam(params);
		}
		ScriptSystem::RunCallbackFunction(mScriptCallback, params);
	}

}