#pragma once

#include "OgrePhysXClasses.h"
#include <OgreDataStream.h>
#include "NxPhysics.h"
#include <NxStream.h> 
#include <streambuf>
#include "OgreString.h"

namespace OgrePhysX
{
	class OgrePhysXClass OgreReadStream : public NxStream
	{
	private:
		Ogre::DataStreamPtr mOgreDataStream;

	public:
		OgreReadStream(Ogre::DataStreamPtr ds);
		~OgreReadStream();

		NxU8            readByte()                              const;
		NxU16           readWord()                              const;
		NxU32           readDword()                             const;
		float           readFloat()                             const;
		double          readDouble()                            const;
		void            readBuffer(void* buffer, NxU32 size)    const;

		NxStream&       storeByte(NxU8 b) { return *this; }
		NxStream&       storeWord(NxU16 w) { return *this; }
		NxStream&       storeDword(NxU32 d) { return *this; }
		NxStream&       storeFloat(NxReal f) { return *this; }
		NxStream&       storeDouble(NxF64 f) { return *this; }
		NxStream&       storeBuffer(const void* buffer, NxU32 size) { return *this; }
	};
}