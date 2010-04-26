
#pragma once

#include "Ogre.h"
#include <string>
#include "EDTIncludes.h"

using namespace Ogre;

void UnloadResource(Ogre::ResourceManager* resMgr, const std::string& resourceName)
{
	Ogre::ResourcePtr rPtr = resMgr->getByName(resourceName);
	if (rPtr.isNull())
	{
		StringUtil::StrStreamType msg;
		msg << "Resource no longer exists: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
		return;
	}

	rPtr->unload();
	if (rPtr->isLoaded())
	{
		StringUtil::StrStreamType msg;
		msg << "Resource was not unloaded: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
	}
	else
	{
		StringUtil::StrStreamType msg;
		msg << "Resource was unloaded: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
	}
	
	resMgr->remove(resourceName);
	rPtr = resMgr->getByName(resourceName);
	if (rPtr.isNull())
	{
		StringUtil::StrStreamType msg;
		msg << "Resource was removed: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
	}
	else
	{
		StringUtil::StrStreamType msg;
		msg << "Resource was not removed: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
	}

	if (resMgr->resourceExists(resourceName))
	{
		StringUtil::StrStreamType msg;
		msg << "Resource still exists: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
	}
	else
	{
		StringUtil::StrStreamType msg;
		msg << "Resource no longer exists: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
	}
}

void LoadResource(Ogre::ResourceManager* resMgr, const std::string& resourceName, const std::string& resourceGroup)
{
	Ogre::ResourcePtr rPtr = resMgr->load(resourceName, resourceGroup);
	if (rPtr.isNull())
	{
		StringUtil::StrStreamType msg;
		msg << "Resource no longer exists: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
		return;
	}

	rPtr->reload();
	if (rPtr->isLoaded())
	{
		StringUtil::StrStreamType msg;
		msg << "Resource was reloaded: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
	}
	else
	{
		StringUtil::StrStreamType msg;
		msg << "Resource was not reloaded: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
	}

	if (resMgr->resourceExists(resourceName))
	{
		StringUtil::StrStreamType msg;
		msg << "Resource still exists: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
	}
	else
	{
		StringUtil::StrStreamType msg;
		msg << "Resource no longer exists: " << resourceName;
		//Ogre::LogManager::getSingleton().logMessage(msg.str());
	}
}

void UnloadMaterials(const std::string& filename)
{
	if (filename.empty())
	{
		//Ogre::LogManager::getSingleton().logMessage("Filename is empty.");
		return;
	}

	Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename);
	if(!stream.isNull())
	{
		try
		{
			while(!stream->eof())
			{
				std::string line = stream->getLine();
				StringUtil::trim(line);

				///
				/// UNLOAD MATERIALS
				///
				if (StringUtil::startsWith(line, "material"))
				{
					Ogre::StringVector vec = Ogre::StringUtil::split(line," \t:");
					bool skipFirst = true;
					for (Ogre::StringVector::iterator it = vec.begin(); it != vec.end(); ++it)
					{
						if (skipFirst)
						{
							skipFirst = false;
							continue;
						}
						std::string match = (*it);
						StringUtil::trim(match);
						if (!match.empty())
						{
							UnloadResource(Ogre::MaterialManager::getSingletonPtr(), match);
							break;
						}
					}
				}
			}
		}
		catch (Ogre::Exception &e)
		{
			StringUtil::StrStreamType msg;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			Ogre::LogManager::getSingleton().logMessage(msg.str());
		}
	}
	stream->close();
}

void UnloadVertexPrograms(const std::string& filename)
{
	if (filename.empty())
	{
		//Ogre::LogManager::getSingleton().logMessage("Material filename is empty.");
		return;
	}

	Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename);
	if(!stream.isNull())
	{
		try
		{
			while(!stream->eof())
			{
				std::string line = stream->getLine();
				StringUtil::trim(line);

				///
				/// UNLOAD VERTEX PROGRAMS
				///
				if (StringUtil::startsWith(line, "vertex_program") && !StringUtil::startsWith(line, "vertex_program_ref"))
				{
					Ogre::StringVector vec = StringUtil::split(line," \t");
					bool skipFirst = true;
					for (Ogre::StringVector::iterator it = vec.begin(); it < vec.end(); ++it)
					{
						if (skipFirst)
						{
							skipFirst = false;
							continue;
						}
						std::string match = (*it);
						StringUtil::trim(match);
						if (!match.empty())
						{
							UnloadResource(Ogre::HighLevelGpuProgramManager::getSingletonPtr(), match);
							break;
						}
					}
				}
			}
		}
		catch (Ogre::Exception &e)
		{
			StringUtil::StrStreamType msg;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			Ogre::LogManager::getSingleton().logMessage(msg.str());
		}
	}
	stream->close();
}

void UnloadFragmentPrograms(const std::string& filename)
{
	if (filename.empty())
	{
		//Ogre::LogManager::getSingleton().logMessage("Material filename is empty.");
		return;
	}

	Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename);
	if(!stream.isNull())
	{
		try
		{
			while(!stream->eof())
			{
				std::string line = stream->getLine();
				StringUtil::trim(line);

				///
				/// UNLOAD FRAGMENT PROGRAMS
				///
				if (StringUtil::startsWith(line, "fragment_program") && !StringUtil::startsWith(line, "fragment_program_ref"))
				{
					Ogre::StringVector vec = StringUtil::split(line," \t");
					bool skipFirst = true;
					for (Ogre::StringVector::iterator it = vec.begin(); it < vec.end(); ++it)
					{
						if (skipFirst)
						{
							skipFirst = false;
							continue;
						}
						std::string match = (*it);
						StringUtil::trim(match);
						if (!match.empty())
						{
							UnloadResource(Ogre::HighLevelGpuProgramManager::getSingletonPtr(), match);
							break;
						}
					}
				}
			}
		}
		catch (Ogre::Exception &e)
		{
			StringUtil::StrStreamType msg;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			Ogre::LogManager::getSingleton().logMessage(msg.str());
		}
	}
	stream->close();
}

void ReloadMaterial(const std::string& materialName, const std::string& groupName, const std::string& filename, bool parseMaterialScript)
{
	if (materialName.empty())
	{
		//Ogre::LogManager::getSingleton().logMessage("Material name is empty.");
		return;
	}

	if (groupName.empty())
	{
		//Ogre::LogManager::getSingleton().logMessage("Group name is empty.");
		return;
	}

	if (filename.empty())
	{
		//Ogre::LogManager::getSingleton().logMessage("Filename is empty.");
		return;
	}

	UnloadMaterials(filename);
	UnloadVertexPrograms(filename);
	UnloadFragmentPrograms(filename);
	
	if (parseMaterialScript)
	{
		Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename);
		if(!stream.isNull())
		{
			try
			{
				Ogre::ScriptCompilerManager::getSingleton().parseScript(stream, groupName);
				Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(materialName);
				if (!materialPtr.isNull())
				{
					materialPtr->compile();
					materialPtr->load();
				}
			}
			catch (Ogre::Exception &e)
			{
				StringUtil::StrStreamType msg;
				msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
				Ogre::LogManager::getSingleton().logMessage(msg.str());
			}
		}
		stream->close();
		
		///
		/// RELOAD MATERIAL SCRIPT CONTENTS
		///
		stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename);
		if(!stream.isNull())
		{
			try
			{
				///
				/// RELOAD ALL MATERIAL CONTENTS IN FILE
				///
				while(!stream->eof())
				{
					std::string line = stream->getLine();
					StringUtil::trim(line);
					///
					/// RELOAD MATERIALS
					///
					if (StringUtil::startsWith(line, "material"))
					{
						Ogre::StringVector vec = StringUtil::split(line," \t:");
						bool skipFirst = true;
						for (Ogre::StringVector::iterator it = vec.begin(); it < vec.end(); ++it)
						{
							if (skipFirst)
							{
								skipFirst = false;
								continue;
							}
							std::string match = (*it);
							StringUtil::trim(match);
							if (!match.empty())
							{
								LoadResource(Ogre::MaterialManager::getSingletonPtr(), match, groupName);
								break;
							}
						}
					}
					///
					/// RELOAD VERTEX PROGRAMS
					///
					if (StringUtil::startsWith(line, "vertex_program") && !StringUtil::startsWith(line, "vertex_program_ref"))
					{
						Ogre::StringVector vec = StringUtil::split(line," \t");
						bool skipFirst = true;
						for (Ogre::StringVector::iterator it = vec.begin(); it < vec.end(); ++it)
						{
							if (skipFirst)
							{
								skipFirst = false;
								continue;
							}
							std::string match = (*it);
							StringUtil::trim(match);
							if (!match.empty())
							{
								LoadResource(Ogre::HighLevelGpuProgramManager::getSingletonPtr(), match, groupName);
								break;
							}
						}
					}
					///
					/// RELOAD FRAGMENT PROGRAMS
					///
					if (StringUtil::startsWith(line, "fragment_program") && !StringUtil::startsWith(line, "fragment_program_ref"))
					{
						Ogre::StringVector vec = StringUtil::split(line," \t");
						bool skipFirst = true;
						for (Ogre::StringVector::iterator it = vec.begin(); it < vec.end(); ++it)
						{
							if (skipFirst)
							{
								skipFirst = false;
								continue;
							}
							std::string match = (*it);
							StringUtil::trim(match);
							if (!match.empty())
							{
								LoadResource(Ogre::HighLevelGpuProgramManager::getSingletonPtr(), match, groupName);
								break;
							}
						}
					}
				}
			}
			catch (Ogre::Exception &e)
			{
				StringUtil::StrStreamType msg;
				msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
				Ogre::LogManager::getSingleton().logMessage(msg.str());
			}
		}
		stream->close();

		try
		{
			// Do a render test if it fails, leave materials unloaded
			Ogre::Root::getSingleton().renderOneFrame();
			return;
		}
		catch (Ogre::Exception &e)
		{
			UnloadVertexPrograms(filename);

			StringUtil::StrStreamType msg;
			msg << "Render test failed. Unloading vertex programs." << std::endl;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			Ogre::LogManager::getSingleton().logMessage(msg.str());
		}

		try
		{
			// Do a render test if it fails, leave materials unloaded
			Ogre::Root::getSingleton().renderOneFrame();
		}
		catch (Ogre::Exception &e)
		{
			// Don't load the script this time
			ReloadMaterial(materialName, groupName, filename, false);

			StringUtil::StrStreamType msg;
			msg << "Render test failed. Unloading materials." << std::endl;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			Ogre::LogManager::getSingleton().logMessage(msg.str());
		}
	}
}
