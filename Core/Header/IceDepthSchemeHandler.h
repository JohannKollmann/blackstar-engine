
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"

namespace Ice
{
	class DepthSchemeHandler : public Ogre::MaterialManager::Listener
	{
	public:
		Ogre::Technique* handleSchemeNotFound(unsigned short schemeIndex, 
			const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
			const Ogre::Renderable* rend)
		{
			Ogre::Technique *ret = new Ogre::Technique(originalMaterial);
			ret->setSchemeName("Depth");
			Ogre::Pass *pass = ret->createPass();
			pass->setVertexProgram("DepthOnly_vp");
			pass->setFragmentProgram("DepthNoRefraction_fp");
			return ret;
		}
	};
}