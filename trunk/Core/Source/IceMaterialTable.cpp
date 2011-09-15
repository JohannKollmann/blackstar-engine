
#include "IceMaterialTable.h"
#include "IceMain.h"
#include "OgrePhysX.h"
#include <fstream>

namespace Ice
{
	MaterialTable::MaterialTable()
	{
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_POST);
	}

	void MaterialTable::InitBindingsFromCfg(Ogre::String cfgFile)
	{
		Clear();

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
					auto pxMat = mMaterialProfilesToPx.find(mat);
					if (pxMat != mMaterialProfilesToPx.end())
					{
						mOgreMaterialsToProfiles.insert(std::make_pair<Ogre::String, Ogre::String>(oMat, mat));
						OgrePhysX::World::getSingleton().registerOgreMaterialName(oMat, pxMat->second);
					}
					else IceWarning("Could not find material: " + mat + ".")
				}
			}
		}
	}

	void MaterialTable::SaveBindingsToCfg(Ogre::String cfgFile)
	{
		std::fstream f;
		f.open(cfgFile.c_str(), std::ios::out | std::ios::trunc);
		f << "[OgreBindings]" << std::endl;
		for (auto i = mOgreMaterialsToProfiles.begin(); i != mOgreMaterialsToProfiles.end(); i++)
		{
			f << i->first << " = " << i->second << std::endl;
		}
		f.close();
	}

	void MaterialTable::SetOgreBinding(const Ogre::String &ogreMat, const Ogre::String &mat)
	{
		auto pxMat = mMaterialProfilesToPx.find(mat);
		if (pxMat != mMaterialProfilesToPx.end())
		{
			mOgreMaterialsToProfiles.insert(std::make_pair<Ogre::String, Ogre::String>(ogreMat, mat));
			OgrePhysX::World::getSingleton().registerOgreMaterialName(ogreMat, pxMat->second);
		}
	}

	void MaterialTable::AddMaterialProfile(const Ogre::String &name, PxMaterial *pxMat)
	{
		if (mMaterialProfilesToPx.find(name) != mMaterialProfilesToPx.end())
		{
			IceWarning("Material " + name + " already exists!");
			return;
		}
		mMaterialProfilesToPx.insert(std::make_pair<Ogre::String, PxMaterial*>(name, pxMat));
		mPxToMaterialProfiles.insert(std::make_pair<PxMaterial*, Ogre::String>(pxMat, name));
	}

	std::vector<Ogre::String> MaterialTable::GetMaterialProfiles()
	{
		std::vector<Ogre::String> mats;
		for (auto i = mMaterialProfilesToPx.begin(); i != mMaterialProfilesToPx.end(); i++)
		{
			mats.push_back(i->first);
		}
		return mats;
	}

	void MaterialTable::Clear()
	{
		mMaterialProfilesToPx.clear();
		mPxToMaterialProfiles.clear();
		mOgreMaterialsToProfiles.clear();
		OgrePhysX::World::getSingleton().getOgreMaterialNames().clear();
	}

	PxMaterial* MaterialTable::GetMaterialByName(const Ogre::String &mat)
	{
		auto pxMat = mMaterialProfilesToPx.find(mat);
		if (pxMat != mMaterialProfilesToPx.end())
			return pxMat->second;
		return &OgrePhysX::World::getSingleton().getDefaultMaterial();
	}

	Ogre::String MaterialTable::GetMaterialNameByPx(PxMaterial *pxMat)
	{
		auto mat = mPxToMaterialProfiles.find(pxMat);
		if (mat != mPxToMaterialProfiles.end())
			return mat->second;
		return "DefaultMaterial";
	}

	Ogre::String MaterialTable::GetMaterialNameByOgreMaterial(const Ogre::String &ogreMat)
	{
		auto mat = mOgreMaterialsToProfiles.find(ogreMat);
		if (mat != mOgreMaterialsToProfiles.end())
			return mat->second;
		return "DefaultMaterial";
	}


	void MaterialTable::ReceiveMessage(Msg &msg)
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

	MaterialTable& MaterialTable::Instance()
	{
		static MaterialTable TheOneAndOnly;
		return TheOneAndOnly;
	};
}