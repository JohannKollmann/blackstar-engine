
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include <map>
#include "IceMessageSystem.h"
#include "PxMaterial.h"

/**
Provides a naming service for PhysX materials and the possibility to map Ogre material names to these named materials.
*/

namespace Ice
{
	class DllExport MaterialTable : public ViewMessageListener
	{
	private:
		std::unordered_map<Ogre::String, PxMaterial*> mMaterialProfilesToPx;
		std::unordered_map<PxMaterial*, Ogre::String> mPxToMaterialProfiles;
		std::unordered_map<Ogre::String, Ogre::String> mOgreMaterialsToProfiles;

	public:
		MaterialTable();
		~MaterialTable() {}

		void InitBindingsFromCfg(Ogre::String cfgFile);
		void SaveBindingsToCfg(Ogre::String cfgFile);

		void SetOgreBinding(const Ogre::String &ogreMat, const Ogre::String &mat);

		Ogre::String GetMaterialNameByOgreMaterial(const Ogre::String &ogreMat);

		void ReceiveMessage(Msg &msg);

		std::vector<Ogre::String> GetMaterialProfiles();
		void AddMaterialProfile(const Ogre::String &name, PxMaterial *pxMat);

		PxMaterial* GetMaterialByName(const Ogre::String &mat);
		Ogre::String GetMaterialNameByPx(PxMaterial *pxMat);

		//Singleton
		static MaterialTable& Instance();
	};
}