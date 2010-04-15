#pragma once


#include "OgrePhysXClasses.h"
#include "NxUserOutputStream.h"
#include "Ogre.h"

namespace OgrePhysX
{

	class OgrePhysXClass LogOutputStream : public NxUserOutputStream
	{
        void reportError (NxErrorCode code, const char *message, const char* file, int line)
        {
			Ogre::String msg = "[OgrePhysX] Error in " + Ogre::String(file) + "line " + Ogre::StringConverter::toString(line) + ": " + Ogre::String(message);
#if	OGREPHYSX_CANMOVESTATICACTORS
			if (msg.find("Static actor moved") == Ogre::String::npos) Ogre::LogManager::getSingleton().logMessage(msg);
#else
			Ogre::LogManager::getSingleton().logMessage(msg);
#endif
        }
                
        NxAssertResponse reportAssertViolation (const char *message, const char *file,int line)
        {
            //this should not get hit by
            // a properly debugged SDK!
            assert(0);
            return NX_AR_CONTINUE;
        }
                
        void print (const char *message)
        {
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] " + Ogre::String(message));
        }

	};
}
