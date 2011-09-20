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

	class OgrePhysXClass MemoryStream : public PxStream
	{
	private:
		unsigned int			mMaxSize;
		mutable unsigned int	mCurrentPos;
		unsigned char*			mData;

	public:		
		static const int BUFFER_RESIZE_STEP = 4096;
	
		MemoryStream();
		virtual ~MemoryStream();

		void seek(unsigned int pos);

		unsigned char* getData();

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