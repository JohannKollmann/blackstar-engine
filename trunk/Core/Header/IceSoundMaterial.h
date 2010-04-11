
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include <map>

namespace Ice
{
	class DllExport SoundMaterialTable
	{
	public:

		typedef Ogre::String SoundMaterialName;
		typedef Ogre::String OgreMaterialName;
		typedef Ogre::String SoundIdentifier;
		typedef unsigned short int NxMaterialID;

		struct DllExport SoundMaterial
		{
			NxMaterialID nxId;
			SoundMaterialName name;
			std::map< SoundMaterialName, SoundIdentifier > relations;
		};

		typedef std::map< SoundMaterialName, SoundMaterial > MaterialTable;
		typedef std::map< OgreMaterialName, NxMaterialID > OgreBindings;

		OgreBindings mOgreBindings;

	private:
		NxMaterialID mCurrentIndex;
		MaterialTable mTable;

		void verifyMaterial(SoundMaterialName name); 

	public:
		SoundMaterialTable() : mCurrentIndex(1) {}
		~SoundMaterialTable() {}

		void AddRelation(SoundMaterialName mat1, SoundMaterialName mat2, SoundIdentifier sound);

		void InitTableFromCfg(Ogre::String cfgFile);
		void InitBindingsFromCfg(Ogre::String cfgFile);

		SoundIdentifier GetSound(SoundMaterialName mat1, SoundMaterialName mat2);
	};
}