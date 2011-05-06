
#include "IceSoundMaterialTable.h"
#include "IceMain.h"
#include "OgrePhysX.h"
#include "NxMaterialDesc.h"
#include <fstream>

namespace Ice
{
	SoundMaterialTable::SoundMaterialTable()
	{
		MessageSystem::JoinNewsgroup(this, GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
		MessageSystem::JoinNewsgroup(this, GlobalMessageIDs::REPARSE_SCRIPTS_POST);
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
		for (auto i = mOgreBindings.begin(); i != mOgreBindings.end(); i++)
		{
			f << i->first << " = " << i->second << std::endl;
		}
		f.close();
	}

	void SoundMaterialTable::SetOgreBinding(Ogre::String ogreMat, Ogre::String mat)
	{
		auto test = mMatNxBinds.find(mat);
		if (test == mMatNxBinds.end())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::SetOgreBinding: Material " + mat + " does not exist!");
			return;
		}
		auto x = mOgreBindings.find(ogreMat);
		if (x != mOgreBindings.end()) x->second = mat;
		else mOgreBindings.insert(std::make_pair<Ogre::String, Ogre::String>(ogreMat, mat));

		NxMatID id = test->second;
		auto y = mOgreNxBinds.find(ogreMat);
		if (y != mOgreNxBinds.end()) y->second = id;
		else mOgreNxBinds.insert(std::make_pair<Ogre::String, NxMatID>(ogreMat, id));
	}

	void SoundMaterialTable::AddMaterialProfile(Ogre::String name, NxMatID matID)
	{
		auto x = mMatNxBinds.find(name);
		if (x != mMatNxBinds.end())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::AddMaterialProfile: Material " + name + " already exists!");
			return;
		}
		mMatNxBinds.insert(std::make_pair<Ogre::String, NxMatID>(name, matID));
		mNxMatBinds.insert(std::make_pair<NxMatID, Ogre::String>(matID, name));
	}

	std::vector<Ogre::String> SoundMaterialTable::GetMaterialProfiles()
	{
		std::vector<Ogre::String> mats;
		for (auto i = mMatNxBinds.begin(); i != mMatNxBinds.end(); i++)
		{
			mats.push_back(i->first);
		}
		return mats;
	}

	Ogre::String SoundMaterialTable::GetMaterialName( Ogre::String ogreMat )
	{
		auto i = mOgreBindings.find(ogreMat);
		if (i == mOgreBindings.end())
		{
			//Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::GetMaterialNameByOgreMaterial: Invalid material (" + ogreMat + ")");
			return "DefaultMaterial";
		}
		return i->second;
	}

	Ogre::String SoundMaterialTable::GetMaterialName( NxMatID id )
	{
		auto i = mNxMatBinds.find(id);
		if (i == mNxMatBinds.end())
		{
			//Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::GetMaterialNameByOgreMaterial: Invalid material id (" + Ogre::StringConverter::toString(id) + ")");
			return "DefaultMaterial";
		}
		return i->second;
	}

	SoundMaterialTable::NxMatID SoundMaterialTable::GetMaterialID( Ogre::String mat )
	{
		auto i = mMatNxBinds.find(mat);
		if (i == mMatNxBinds.end())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::GetMaterialNameByOgreMaterial: Invalid material (" + mat + ")");
			return 0;
		}
		return i->second;
	}

	void SoundMaterialTable::Clear()
	{
		for (int i = mNxMatBinds.size(); i > 0; i--)
		{
			unsigned int index = i;
			Main::Instance().GetPhysXScene()->getNxScene()->releaseMaterial(
				*Main::Instance().GetPhysXScene()->getNxScene()->getMaterialFromIndex(index));
		}
		mOgreBindings.clear();
		mNxMatBinds.clear();
		mMatNxBinds.clear();
		mOgreNxBinds.clear();

	}

	void SoundMaterialTable::ReceiveMessage(Msg &msg)
	{
		if (msg.type == GlobalMessageIDs::REPARSE_SCRIPTS_PRE)
		{
			Clear();
		}
		else if (msg.type == GlobalMessageIDs::REPARSE_SCRIPTS_POST)
		{
			InitBindingsFromCfg("OgreMaterialSoundBindings.cfg");
		}
	}


}