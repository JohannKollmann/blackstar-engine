
#pragma once

#include "LoadSave.h"
#include "IceIncludes.h"
#include "OgreString.h"

namespace Ice
{

	class LoadSaveable : public LoadSave::Saveable
	{
	protected:
		struct LoadSaveAtom
		{
			std::string type;
			void *data;
			std::string description;
		};
		void AddLoadSaveAtom(Ogre::String type, void *data, Ogre::String description);
		virtual void SaveObjects(LoadSave::SaveSystem& mgr) {}
		virtual void LoadObjects(LoadSave::LoadSystem& mgr) {}
		virtual void OnLoaded() {}

	private:
		std::vector<LoadSaveAtom> mLoadSaveProperties;

	public:
		LoadSaveable(void);
		~LoadSaveable(void);

		virtual void Save(LoadSave::SaveSystem& mgr);
		virtual void Load(LoadSave::LoadSystem& mgr);
	};
}

