
#include "TextureDebugger.h"


	TextureDebugger::TextureDebugger()
	{
		Ice::MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
		//ShowTexture("Ogre/ShadowTexture0");
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

		Ogre::TextureUnitState *tus = mat->getTechnique(0)->getPass(0)->createTextureUnitState();
		tus->setContentType(Ogre::TextureUnitState::ContentType::CONTENT_SHADOW);

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
