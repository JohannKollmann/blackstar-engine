
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include <map>
#include "IceMessageSystem.h"

namespace Ice
{
	class DllExport SoundMaterialTable : public ViewMessageListener
	{
	public:
		typedef unsigned short MaterialIndex;

	private:
		std::map< MaterialIndex, Ogre::String > mPxToMaterialProfiles;
		std::map< Ogre::String, MaterialIndex > mMaterialProfilesToPx;

	public:

		SoundMaterialTable();
		~SoundMaterialTable() {}

		void InitBindingsFromCfg(Ogre::String cfgFile);
		void SaveBindingsToCfg(Ogre::String cfgFile);

		void Clear();

		void ReceiveMessage(Msg &msg);

		void SetOgreBinding(Ogre::String ogreMat, Ogre::String mat);

		std::vector<Ogre::String> GetMaterialProfiles();
		void AddMaterialProfile(const Ogre::String &name, MaterialIndex matID);

		MaterialIndex GetMaterialID(const Ogre::String &mat);
		Ogre::String GetMaterialName(MaterialIndex id);
	};
}