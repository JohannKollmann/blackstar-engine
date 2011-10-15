/*
This source file is part of OgrePhysX.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once

#include "OgrePhysXClasses.h"
#include <OgreDataStream.h>
#include "PxPhysics.h"
#include "PxPhysicsAPI.h"
#include "common/PxStream.h" 
#include <streambuf>
#include "OgreString.h"

namespace OgrePhysX
{
	using namespace physx;

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