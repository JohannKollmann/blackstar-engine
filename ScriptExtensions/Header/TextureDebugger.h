#pragma once

#include "IceIncludes.h"
#include "IceMain.h"
#include "IceMessageSystem.h"
#include "GUISystem.h"
#include "IceScriptSystem.h"

	class __declspec(dllexport) TextureDebugger : public Ice::MessageListener
	{
	private:
		GUISystem::Window mWindow;

		Ogre::String mCurrTextureName;

		TextureDebugger();
		~TextureDebugger();

	public:

		void ShowTexture(Ogre::String textureName);

		void ReceiveMessage(Ice::Msg &msg);

		static std::vector<Ice::ScriptParam> Lua_GetShowTexture(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
		{
			if (vParams.size() == 1 && vParams[0].getType() == Ice::ScriptParam::PARM_TYPE_STRING)
				Instance().ShowTexture(vParams[0].getString());
			return std::vector<Ice::ScriptParam>();
		}

		//Singleton
		static TextureDebugger& Instance();
	};