
#include "IceSoundMaterial.h"

namespace Ice
{

	void SoundMaterialTable::verifyMaterial(SoundMaterialName name)
	{
		auto i = mTable.find(name);
		if (i == mTable.end())
		{
			SoundMaterial m;
			m.name = name;
			m.nxId = mCurrentIndex++;
			mTable.insert(std::make_pair<SoundMaterialName, SoundMaterial>(name, m));
		}
	}

	void SoundMaterialTable::AddRelation(SoundMaterialName mat1, SoundMaterialName mat2, SoundIdentifier sound)
	{
		verifyMaterial(mat1);
		verifyMaterial(mat2);

		mTable[mat1].relations.insert(std::make_pair<SoundMaterialName, SoundIdentifier>(mat2, sound));
		mTable[mat2].relations.insert(std::make_pair<SoundMaterialName, SoundIdentifier>(mat1, sound));
	}


	void SoundMaterialTable::InitTableFromCfg(Ogre::String cfgFile)
	{
		Ogre::ConfigFile cf;
		cf.loadFromResourceSystem(cfgFile, "General");

		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
				
		while (seci.hasMoreElements())
		{
			Ogre::String secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
			if (secName == "Table")
			{
				for (Ogre::ConfigFile::SettingsMultiMap::iterator i = settings->begin(); i != settings->end(); i++)
				{
					Ogre::String keyName = i->first;
					SoundMaterialName mat1 = keyName.substr(0, keyName.find("|"));
					SoundMaterialName mat2 = keyName.substr(keyName.find("|"));
					AddRelation(mat1, mat2, i->second);
				}
			}
		}
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
					verifyMaterial(mat);
					auto x = mOgreBindings.find(oMat);
					if (x != mOgreBindings.end()) x->second = mTable[mat].nxId;
					else mOgreBindings.insert(std::make_pair<OgreMaterialName, NxMaterialID>(oMat, mTable[mat].nxId));
				}
			}
		}
	}

	SoundMaterialTable::SoundIdentifier SoundMaterialTable::GetSound(SoundMaterialName mat1, SoundMaterialName mat2)
	{
		SoundIdentifier sound;
		auto i = mTable.find(mat1);
		if (i == mTable.end())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::GetSound: Invalid material (" + mat1 + ")");
			return "NotFound.ogg";
		}
		auto x = i->second.relations.find(mat2);
		if (x == i->second.relations.end())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::GetSound: Invalid material (" + mat2 + ")");
			return "NotFound.ogg";
		}
		return x->second;
	}


}