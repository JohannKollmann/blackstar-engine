#pragma once

#include "IceIncludes.h"
#include "IceMain.h"
#include "IceMessageSystem.h"
#include "GUISystem.h"

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

		//Singleton
		static TextureDebugger& Instance();
	};