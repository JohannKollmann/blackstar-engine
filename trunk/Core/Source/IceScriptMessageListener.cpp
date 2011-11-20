
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
		std::map<ScriptParam, ScriptParam> table;
		Utils::DataMapToTable(Script(), msg.params, table);
		ScriptParam parm(table);
		ScriptSystem::RunCallbackFunction(mScriptCallback, std::vector<ScriptParam>(1, parm));
	}

}