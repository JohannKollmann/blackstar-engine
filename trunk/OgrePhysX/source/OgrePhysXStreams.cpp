
#include "OgrePhysXStreams.h"
#include "PxAssert.h"
#include "internal/include/PsFile.h"

namespace OgrePhysX
{
	OgrePxStream::OgrePxStream(Ogre::DataStreamPtr ds)
	{
		mOgreDataStream = ds;
	}
	OgrePxStream::~OgrePxStream() {}

	PxU8 OgrePxStream::readByte() const
	{
		PxU8 byte = 0;
		mOgreDataStream->read(&byte, sizeof(PxU8));
		return byte;
	}
	PxU16 OgrePxStream::readWord() const
	{
		PxU16 word = 0;
		mOgreDataStream->read(&word, sizeof(PxU16));
		return word;
	}
	PxU32 OgrePxStream::readDword() const
	{
		PxU32 dword = 0;
		mOgreDataStream->read(&dword, sizeof(PxU32));
		return dword;
	}
	float OgrePxStream::readFloat() const
	{
		float f = 0;
		mOgreDataStream->read(&f, sizeof(float));
		return f;
	}
	double OgrePxStream::readDouble() const
	{
		double d = 0;
		mOgreDataStream->read(&d, sizeof(double));
		return d;
	}
	void OgrePxStream::readBuffer(void* buffer, PxU32 size) const
	{
		mOgreDataStream->read(buffer, size);
	}

	PxStream& OgrePxStream::storeByte(PxU8 b)
	{
		mOgreDataStream->write(&b, sizeof(PxU8));
		return *this;
	}
	PxStream& OgrePxStream::storeWord(PxU16 w)
	{
		mOgreDataStream->write(&w, sizeof(PxU16));
		return *this;
	}
	PxStream& OgrePxStream::storeDword(PxU32 d)
	{
		mOgreDataStream->write(&d, sizeof(PxU32));
		return *this;
	}
	PxStream& OgrePxStream::storeFloat(PxReal f)
	{
		mOgreDataStream->write(&f, sizeof(PxReal));
		return *this;
	}
	PxStream& OgrePxStream::storeDouble(PxF64 f)
	{
		mOgreDataStream->write(&f, sizeof(PxU64));
		return *this;
	}
	PxStream& OgrePxStream::storeBuffer(const void* buffer, PxU32 size)
	{
		mOgreDataStream->write(buffer, size);
		return *this;
	}

	Ogre::DataStreamPtr OgrePxStream::getOgreDataStream()
	{
		return mOgreDataStream;
	}


	MemoryWriteStream::MemoryWriteStream() : mCurrentSize(0), mMaxSize(0), data(nullptr)
	{
	}

	MemoryWriteStream::~MemoryWriteStream()
	{
		delete[] data;
	}

	void MemoryWriteStream::readBuffer(void*, PxU32) const
	{
		PX_ASSERT(0); 
	}

	PxStream& MemoryWriteStream::storeByte(PxU8 b)
	{
		storeBuffer(&b, sizeof(PxU8));
		return *this;
	}

	PxStream& MemoryWriteStream::storeWord(PxU16 w)
	{
		storeBuffer(&w, sizeof(PxU16));
		return *this;
	}

	PxStream& MemoryWriteStream::storeDword(PxU32 d)
	{
		storeBuffer(&d, sizeof(PxU32));
		return *this;
	}

	PxStream& MemoryWriteStream::storeFloat(PxReal f)
	{
		storeBuffer(&f, sizeof(PxReal));
		return *this;
	}

	PxStream& MemoryWriteStream::storeDouble(PxF64 f)
	{
		storeBuffer(&f, sizeof(PxF64));
		return *this;
	}

	PxStream& MemoryWriteStream::storeBuffer(const void* buffer, PxU32 size)
	{
		unsigned int newSize = mCurrentSize + size;
		if(newSize > mMaxSize)
		{	//resize
			mMaxSize = newSize + BUFFER_RESIZE_STEP;

			PxU8* newData = new PxU8[mMaxSize];
			PX_ASSERT(newData != nullptr);

			if(data)
			{
				memcpy(newData, data, mCurrentSize);
				delete[] data;
			}
			data = newData;
		}

		memcpy(data + mCurrentSize, buffer, size);
		mCurrentSize += size;
		return *this;
	}


	MemoryReadStream::MemoryReadStream(const PxU8* data) : buffer(data)
	{
	}

	MemoryReadStream::~MemoryReadStream() {}

	PxU8 MemoryReadStream::readByte() const
	{
		PxU8 b;
		memcpy(&b, buffer, sizeof(PxU8));
		buffer += sizeof(PxU8);
		return b;
	}

	PxU16 MemoryReadStream::readWord() const
	{
		PxU16 w;
		memcpy(&w, buffer, sizeof(PxU16));
		buffer += sizeof(PxU16);
		return w;
	}

	PxU32 MemoryReadStream::readDword() const
	{
		PxU32 d;
		memcpy(&d, buffer, sizeof(PxU32));
		buffer += sizeof(PxU32);
		return d;
	}

	float MemoryReadStream::readFloat() const
	{
		float f;
		memcpy(&f, buffer, sizeof(float));
		buffer += sizeof(float);
		return f;
	}

	double MemoryReadStream::readDouble() const
	{
		double f;
		memcpy(&f, buffer, sizeof(double));
		buffer += sizeof(double);
		return f;
	}

	void MemoryReadStream::readBuffer(void* dest, PxU32 size) const
	{
		memcpy(dest, buffer, size);
		buffer += size;
	}

	PxStream& MemoryReadStream::storeBuffer(const void*, PxU32)
	{
		PX_ASSERT(0); 
		return *this;
	}


	PxU8 MemoryWriteStream::readByte() const		{ PX_ASSERT(0);	return 0;		}
	PxU16 MemoryWriteStream::readWord() const		{ PX_ASSERT(0);	return 0;		}
	PxU32 MemoryWriteStream::readDword() const		{ PX_ASSERT(0);	return 0;		}
	float MemoryWriteStream::readFloat() const		{ PX_ASSERT(0);	return 0.0f;	}
	double MemoryWriteStream::readDouble() const	{ PX_ASSERT(0);	return 0.0;		}

	PxStream& MemoryReadStream::storeByte(PxU8)		{ PX_ASSERT(0);	return *this;	}
	PxStream& MemoryReadStream::storeWord(PxU16)	{ PX_ASSERT(0);	return *this;	}
	PxStream& MemoryReadStream::storeDword(PxU32)	{ PX_ASSERT(0);	return *this;	}
	PxStream& MemoryReadStream::storeFloat(PxReal)	{ PX_ASSERT(0);	return *this;	}
	PxStream& MemoryReadStream::storeDouble(PxF64)	{ PX_ASSERT(0);	return *this;	}
}