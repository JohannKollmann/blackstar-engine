#pragma once


#include "OgrePhysXClasses.h"
#include "foundation/PxErrorCallback.h"
#include "PxPhysicsAPI.h"
#include "Ogre.h"

namespace OgrePhysX
{

	class OgrePhysXClass LogOutputStream : public PxErrorCallback
	{
		void reportError (PxErrorCode::Enum e, const char* message, const char* file, int line)
		{
			Ogre::String msg = "[OgrePhysX] Error in " + Ogre::String(file) + "line " + Ogre::StringConverter::toString(line) + ": " + Ogre::String(message);
#if	OGREPHYSX_CANMOVESTATICACTORS
			if (msg.find("Static actor moved") == Ogre::String::npos) Ogre::LogManager::getSingleton().logMessage(msg);
#else
			Ogre::LogManager::getSingleton().logMessage(msg);
#endif
		}

	};
}
