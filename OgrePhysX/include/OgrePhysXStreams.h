#pragma once

#include "OgrePhysXClasses.h"
#include <OgreDataStream.h>
#include "PxPhysics.h"
#include "common/PxStream.h" 
#include <streambuf>
#include "OgreString.h"

namespace OgrePhysX
{
	class OgrePhysXClass OgrePxStream : public PxStream
	{
	private:
		Ogre::DataStreamPtr mOgreDataStream;

	public:
		OgrePxStream(Ogre::DataStreamPtr ds);
		~OgrePxStream();

		PxU8            readByte()                              const;
		PxU16           readWord()                              const;
		PxU32           readDword()                             const;
		float           readFloat()                             const;
		double          readDouble()                            const;
		void            readBuffer(void* buffer, PxU32 size)    const;

		PxStream&       storeByte(PxU8 b);
		PxStream&       storeWord(PxU16 w);
		PxStream&       storeDword(PxU32 d);
		PxStream&       storeFloat(PxReal f);
		PxStream&       storeDouble(PxF64 f);
		PxStream&       storeBuffer(const void* buffer, PxU32 size);

		Ogre::DataStreamPtr getOgreDataStream();
	};

	class OgrePhysXClass MemoryWriteStream : public PxStream
	{
	private:
		PxU32			mCurrentSize;
		PxU32			mMaxSize;

	public:
		PxU8*			data;

		static const int BUFFER_RESIZE_STEP = 4096;
	
		MemoryWriteStream();
		virtual ~MemoryWriteStream();

		virtual		PxU8			readByte()								const;
		virtual		PxU16			readWord()								const;
		virtual		PxU32			readDword()								const;
		virtual		float			readFloat()								const;
		virtual		double			readDouble()							const;
		virtual		void			readBuffer(void* buffer, PxU32 size)	const;

		virtual		PxStream&		storeByte(PxU8 b);
		virtual		PxStream&		storeWord(PxU16 w);
		virtual		PxStream&		storeDword(PxU32 d);
		virtual		PxStream&		storeFloat(PxReal f);
		virtual		PxStream&		storeDouble(PxF64 f);
		virtual		PxStream&		storeBuffer(const void* buffer, PxU32 size);
	};

	class OgrePhysXClass MemoryReadStream : public PxStream
	{
	public:

		mutable	const PxU8*	buffer;

		MemoryReadStream(const PxU8* data);
		virtual						~MemoryReadStream();

		virtual		PxU8			readByte()								const;
		virtual		PxU16			readWord()								const;
		virtual		PxU32			readDword()								const;
		virtual		float			readFloat()								const;
		virtual		double			readDouble()							const;
		virtual		void			readBuffer(void* buffer, PxU32 size)	const;

		virtual		PxStream&		storeByte(PxU8 b);
		virtual		PxStream&		storeWord(PxU16 w);
		virtual		PxStream&		storeDword(PxU32 d);
		virtual		PxStream&		storeFloat(PxReal f);
		virtual		PxStream&		storeDouble(PxF64 f);
		virtual		PxStream&		storeBuffer(const void* buffer, PxU32 size);
	};
}