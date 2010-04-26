
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include <map>
#include "IceMessageSystem.h"

namespace Ice
{
	class DllExport SoundMaterialTable : public MessageListener
	{
		typedef unsigned short NxMatID;
	private:
		std::map< Ogre::String, Ogre::String > mOgreBindings;
		std::map< NxMatID, Ogre::String > mNxMatBinds;
		std::map< Ogre::String, NxMatID > mMatNxBinds;

	public:

		std::map< Ogre::String, NxMatID > mOgreNxBinds;

		SoundMaterialTable();
		~SoundMaterialTable() {}

		void InitBindingsFromCfg(Ogre::String cfgFile);
		void SaveBindingsToCfg(Ogre::String cfgFile);

		void Clear();

		void ReceiveMessage(Msg &msg);

		void SetOgreBinding(Ogre::String ogreMat, Ogre::String mat);

		std::vector<Ogre::String> GetMaterialProfiles();
		void AddMaterialProfile(Ogre::String name, NxMatID matID);

		NxMatID GetMaterialID(Ogre::String mat);
		Ogre::String GetMaterialName(NxMatID id);
		Ogre::String GetMaterialName(Ogre::String ogreMat);
	};
}