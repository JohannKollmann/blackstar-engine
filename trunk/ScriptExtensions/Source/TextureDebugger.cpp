
#include "TextureDebugger.h"


	TextureDebugger::TextureDebugger()
	{
		Ice::ScriptSystem::GetInstance().ShareCFunction("ShowTexture", &Lua_GetShowTexture);
	}


	TextureDebugger::~TextureDebugger()
	{
	}

	void TextureDebugger::ShowTexture(Ogre::String textureName)
	{
		mCurrTextureName = textureName;

		Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(textureName);
		if (texture.isNull())
		{
			IceWarning("Texture " + textureName + " does not exist");
			//return;
		}

		texture->getBuffer()->getRenderTarget()->writeContentsToFile("RTT.bmp");

		Ogre::String matName = textureName + "_mat";

		if (!Ogre::MaterialManager::getSingleton().getByName(matName).isNull())
			Ogre::MaterialManager::getSingleton().remove(matName);

		if (mCurrTextureName != "") GUISystem::GetInstance().DeleteWindow(mWindow.GetHandle());

		Ogre::MaterialPtr baseMat = Ogre::MaterialManager::getSingleton().getByName("gui/runtime");
		if (baseMat.isNull())
		{
			IceWarning("Base gui/runtime gui/runtime does not exist");
			return;
		}
		Ogre::MaterialPtr mat = baseMat->clone(matName);

		mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(textureName);

		mat->compile(true);

		mWindow = GUISystem::GetInstance().MakeWindow(0.25f, 0.25f, 0.5f, 0.5f);
		mWindow.Bake();
		mWindow.SetMaterial(matName);

	}

	void TextureDebugger::ReceiveMessage(Ice::Msg &msg)
	{
		//if (mCurrTextureName != "") ShowTexture(mCurrTextureName);
	}

	TextureDebugger& TextureDebugger::Instance()
	{
		static TextureDebugger TheOneAndOnly;
		return TheOneAndOnly;
	};
