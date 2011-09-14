
#include "IceSoundMaterialTable.h"
#include "IceMain.h"
#include "OgrePhysX.h"
#include <fstream>

namespace Ice
{
	SoundMaterialTable::SoundMaterialTable()
	{
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_POST);
	}

	void SoundMaterialTable::InitBindingsFromCfg(Ogre::String cfgFile)
	{
		Ogre::ConfigFile cf;
		cf.loadFromResourceSystem(cfgFile, "General");

		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
				
		while (seci.hasMoreElements())
		{
			Ogre::String secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
			if (secName == "OgreBindings")
			{
				for (Ogre::ConfigFile::SettingsMultiMap::iterator i = settings->begin(); i != settings->end(); i++)
				{
					Ogre::String oMat = i->first;
					Ogre::String mat = i->second;
					SetOgreBinding(oMat, mat);
				}
			}
		}
	}

	void SoundMaterialTable::SaveBindingsToCfg(Ogre::String cfgFile)
	{
		std::fstream f;
		f.open(cfgFile.c_str(), std::ios::out | std::ios::trunc);
		f << "[OgreBindings]" << std::endl;
		auto matBindings = OgrePhysX::World::getSingleton().getOgreMaterialBindings();
		for (auto i = matBindings.begin(); i != matBindings.end(); i++)
		{
			f << i->first << " = " << i->second << std::endl;
		}
		f.close();
	}

	void SoundMaterialTable::SetOgreBinding(Ogre::String ogreMat, Ogre::String mat)
	{
		auto test = mMaterialProfilesToPx.find(mat);
		if (test == mMaterialProfilesToPx.end())
		{
			Log::Instance().LogMessage("Error in SoundMaterialTable::SetOgreBinding: Material " + mat + " does not exist!");
			return;
		}
		OgrePhysX::World::getSingleton().bindOgreMaterial(ogreMat, test->second);
	}

	void SoundMaterialTable::AddMaterialProfile(const Ogre::String &name, MaterialIndex matID)
	{
		if (mMaterialProfilesToPx.find(name) != mMaterialProfilesToPx.end())
		{
			Log::Instance().LogMessage("Error in SoundMaterialTable::AddMaterialProfile: Material " + name + " already exists!");
			return;
		}
		mMaterialProfilesToPx.insert(std::make_pair<Ogre::String, MaterialIndex>(name, matID));
		mPxToMaterialProfiles.insert(std::make_pair<MaterialIndex, Ogre::String>(matID, name));
	}

	std::vector<Ogre::String> SoundMaterialTable::GetMaterialProfiles()
	{
		std::vector<Ogre::String> mats;
		for (auto i = mMaterialProfilesToPx.begin(); i != mMaterialProfilesToPx.end(); i++)
		{
			mats.push_back(i->first);
		}
		return mats;
	}

	Ogre::String SoundMaterialTable::GetMaterialName(MaterialIndex matID)
	{
		auto i = mPxToMaterialProfiles.find(matID);
		if (i == mPxToMaterialProfiles.end())
		{
			return "DefaultMaterial";
		}
		return i->second;
	}

	SoundMaterialTable::MaterialIndex SoundMaterialTable::GetMaterialID(const Ogre::String &mat)
	{
		auto i = mMaterialProfilesToPx.find(mat);
		if (i == mMaterialProfilesToPx.end())
		{
			Log::Instance().LogMessage("Error in SoundMaterialTable::GetMaterialNameByOgreMaterial: Invalid material (" + mat + ")");
			return 0;
		}
		return i->second;
	}

	void SoundMaterialTable::Clear()
	{
		mMaterialProfilesToPx.clear();
		mPxToMaterialProfiles.clear();

	}

	void SoundMaterialTable::ReceiveMessage(Msg &msg)
	{
		if (msg.typeID == GlobalMessageIDs::REPARSE_SCRIPTS_PRE)
		{
			Clear();
		}
		else if (msg.typeID == GlobalMessageIDs::REPARSE_SCRIPTS_POST)
		{
			InitBindingsFromCfg("OgreMaterialSoundBindings.cfg");
		}
	}


}