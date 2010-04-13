
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

		std::map< OgreMaterialName, NxMaterialID > mOgreNxBindings;

	private:
		std::map< SoundMaterialName, SoundMaterial > mTable;
		std::map<NxMaterialID, SoundMaterialName> mNxBinds;
		std::map< OgreMaterialName, SoundMaterialName > mOgreBindings;

		void verifyMaterial(SoundMaterialName name); 

	public:
		SoundMaterialTable() {}
		~SoundMaterialTable() {}

		void AddRelation(SoundMaterialName mat1, SoundMaterialName mat2, SoundIdentifier sound);

		void InitTableFromCfg(Ogre::String cfgFile);
		void InitBindingsFromCfg(Ogre::String cfgFile);

		NxMaterialID GetMaterialID(SoundMaterialName mat);
		NxMaterialID GetMaterialIDByOgreMaterial(Ogre::String mat);

		SoundMaterialName GetMaterialNameByOgreMaterial(Ogre::String mat);
		SoundMaterialName GetMaterialNameByNxMaterial(NxMaterialID id);

		SoundIdentifier GetSound(SoundMaterialName mat1, SoundMaterialName mat2);
		SoundIdentifier GetSound(NxMaterialID mat1, NxMaterialID mat2);
	};
}