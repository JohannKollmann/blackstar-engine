
#pragma once

#include "IceIncludes.h"
#include "IceScriptSystem.h"
#include "IceMessageSystem.h"
#include "Ogre.h"

namespace Ice
{

	class ScriptMessageListener : public MessageListener
	{
	private:
		ScriptParam mScriptCallback;

	public:
		ScriptMessageListener(ScriptParam &callbackFn);
		~ScriptMessageListener(void);

		void ReceiveMessage(Msg &msg);

	};

}
