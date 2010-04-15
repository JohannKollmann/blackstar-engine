
#include "IceSoundMaterial.h"
#include "IceMain.h"
#include "OgrePhysX.h"
#include "NxMaterialDesc.h"

namespace Ice
{

	void SoundMaterialTable::verifyMaterial(SoundMaterialName name)
	{
		auto i = mTable.find(name);
		if (i == mTable.end())
		{
			SoundMaterial m;
			m.name = name;

			NxMaterialDesc	physXMat;
			physXMat.restitution		= 0.1f;
			physXMat.staticFriction	= 0.5f;
			physXMat.dynamicFriction	= 0.5f;
			m.nxId = Main::Instance().GetPhysXScene()->getNxScene()->createMaterial(physXMat)->getMaterialIndex();
			mTable.insert(std::make_pair<SoundMaterialName, SoundMaterial>(name, m));
			mNxBinds.insert(std::make_pair<NxMaterialID, SoundMaterialName>(m.nxId, name));
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
					SoundMaterialName mat2 = keyName.substr(keyName.find("|")+1);
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
					auto x = mOgreNxBindings.find(oMat);
					if (x != mOgreNxBindings.end()) x->second = mTable[mat].nxId;
					else mOgreNxBindings.insert(std::make_pair<OgreMaterialName, NxMaterialID>(oMat, mTable[mat].nxId));

					auto y = mOgreBindings.find(oMat);
					if (y != mOgreBindings.end()) y->second=mat;
					else mOgreBindings.insert(std::make_pair<OgreMaterialName, SoundMaterialName>(oMat, mat));
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

	SoundMaterialTable::SoundIdentifier SoundMaterialTable::GetSound(NxMaterialID mat1, NxMaterialID mat2)
	{
		auto i = mNxBinds.find(mat1);
		auto i2 = mNxBinds.find(mat2);
		if (i == mNxBinds.end() || i2 == mNxBinds.end())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::GetSound: Invalid Nx material ID");
			return "NotFound.ogg";
		}
		return GetSound(i->second, i2->second);
	}

	SoundMaterialTable::NxMaterialID SoundMaterialTable::GetMaterialID( SoundMaterialName mat )
	{
		auto i = mTable.find(mat);
		if (i == mTable.end())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::GetMaterialName: Invalid material (" + mat + ")");
			return 0;
		}
		return i->second.nxId;
	}

	SoundMaterialTable::NxMaterialID SoundMaterialTable::GetMaterialIDByOgreMaterial( Ogre::String mat )
	{
		auto i = mOgreNxBindings.find(mat);
		if (i == mOgreNxBindings.end())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::GetMaterialIDByOgreMaterial: Invalid material (" + mat + ")");
			return 0;
		}
		return i->second;
	}

	SoundMaterialTable::SoundMaterialName SoundMaterialTable::GetMaterialNameByOgreMaterial( Ogre::String mat )
	{
		auto i = mOgreBindings.find(mat);
		if (i == mOgreBindings.end())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::GetMaterialNameByOgreMaterial: Invalid material (" + mat + ")");
			return "DefaultMaterial";
		}
		return i->second;
	}

	SoundMaterialTable::SoundMaterialName SoundMaterialTable::GetMaterialNameByNxMaterial(NxMaterialID id)
	{
		auto i = mNxBinds.find(id);
		if (i == mNxBinds.end())
		{
			//Ogre::LogManager::getSingleton().logMessage("Error in SoundMaterialTable::GetMaterialNameByNxMaterial: Invalid id (" + Ogre::StringConverter::toString(id) + ")");
			return "DefaultMaterial";
		}
		return i->second;
	}


}