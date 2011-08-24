
#include "IceScriptMessageListener.h"
#include "IceScriptSystem.h"
#include "IceUtils.h"

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
		/*std::vector<ScriptParam> params;
		while (msg.params.HasNext())
		{
			msg.params.GetNext().Data->GetAsScriptParam(params);
		}*/
		ScriptParam parm=Utils::DataMapToTable(Script(), msg.params);
		ScriptSystem::RunCallbackFunction(mScriptCallback, std::vector<ScriptParam>(1, parm));
	}

}