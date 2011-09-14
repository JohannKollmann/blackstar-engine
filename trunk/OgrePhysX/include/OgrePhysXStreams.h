#pragma once

#include "OgrePhysXClasses.h"
#include <OgreDataStream.h>
#include "PxPhysics.h"
#include "common/PxStream.h" 
#include <streambuf>
#include "OgreString.h"

namespace OgrePhysX
{
	class OgrePhysXClass OgreReadStream : public PxStream
	{
	private:
		Ogre::DataStreamPtr mOgreDataStream;

	public:
		OgreReadStream(Ogre::DataStreamPtr ds);
		~OgreReadStream();

		PxU8            readByte()                              const;
		PxU16           readWord()                              const;
		PxU32           readDword()                             const;
		float           readFloat()                             const;
		double          readDouble()                            const;
		void            readBuffer(void* buffer, PxU32 size)    const;

		PxStream&       storeByte(PxU8 b) { return *this; }
		PxStream&       storeWord(PxU16 w) { return *this; }
		PxStream&       storeDword(PxU32 d) { return *this; }
		PxStream&       storeFloat(PxReal f) { return *this; }
		PxStream&       storeDouble(PxF64 f) { return *this; }
		PxStream&       storeBuffer(const void* buffer, PxU32 size) { return *this; }
	};
}