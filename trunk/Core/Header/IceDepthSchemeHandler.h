
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"

namespace Ice
{
	class DepthSchemeHandler : public Ogre::MaterialManager::Listener
	{
	private:
		Ogre::Technique *mDepthTechnique;
		Ogre::MaterialPtr mRefMaterial;

	public:
		DepthSchemeHandler()
		{
			mDepthTechnique = nullptr;
			mRefMaterial = Ogre::MaterialManager::getSingleton().getByName("DepthBase");
			if (!mRefMaterial.isNull())
			{
				mDepthTechnique = mRefMaterial->getTechnique(0);
			}
		}
		Ogre::Technique* handleSchemeNotFound(unsigned short schemeIndex, 
			const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
			const Ogre::Renderable* rend)
		{
			return mDepthTechnique;
		}
	};
}