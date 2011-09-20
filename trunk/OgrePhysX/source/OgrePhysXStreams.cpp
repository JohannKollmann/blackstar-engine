
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


	MemoryStream::MemoryStream() : mMaxSize(0), mCurrentPos(0), mData(nullptr)
	{
	}

	MemoryStream::~MemoryStream()
	{
		delete[] mData;
	}

	unsigned char* MemoryStream::getData()
	{
		return mData;
	}

	PxStream& MemoryStream::storeByte(PxU8 b)
	{
		storeBuffer(&b, sizeof(PxU8));
		return *this;
	}

	PxStream& MemoryStream::storeWord(PxU16 w)
	{
		storeBuffer(&w, sizeof(PxU16));
		return *this;
	}

	PxStream& MemoryStream::storeDword(PxU32 d)
	{
		storeBuffer(&d, sizeof(PxU32));
		return *this;
	}

	PxStream& MemoryStream::storeFloat(PxReal f)
	{
		storeBuffer(&f, sizeof(PxReal));
		return *this;
	}

	PxStream& MemoryStream::storeDouble(PxF64 f)
	{
		storeBuffer(&f, sizeof(PxF64));
		return *this;
	}

	PxStream& MemoryStream::storeBuffer(const void* buffer, PxU32 size)
	{
		unsigned int newPos = mCurrentPos + size;
		if(newPos > mMaxSize)
		{	//resize
			mMaxSize = newPos + BUFFER_RESIZE_STEP;

			unsigned char* newData = new unsigned char[mMaxSize];
			PX_ASSERT(newData != nullptr);

			if(mData)
			{
				memcpy(newData, mData, mCurrentPos);
				delete[] mData;
			}
			mData = newData;
		}

		memcpy(mData + mCurrentPos, buffer, size);
		mCurrentPos += size;
		return *this;
	}

	PxU8 MemoryStream::readByte() const
	{
		PxU8 b;
		memcpy(&b, mData + mCurrentPos, sizeof(PxU8));
		mCurrentPos += sizeof(PxU8);
		return b;
	}

	PxU16 MemoryStream::readWord() const
	{
		PxU16 w;
		memcpy(&w, mData + mCurrentPos, sizeof(PxU16));
		mCurrentPos += sizeof(PxU16);
		return w;
	}

	PxU32 MemoryStream::readDword() const
	{
		PxU32 d;
		memcpy(&d, mData + mCurrentPos, sizeof(PxU32));
		mCurrentPos += sizeof(PxU32);
		return d;
	}

	float MemoryStream::readFloat() const
	{
		float f;
		memcpy(&f, mData + mCurrentPos, sizeof(float));
		mCurrentPos += sizeof(float);
		return f;
	}

	double MemoryStream::readDouble() const
	{
		double f;
		memcpy(&f, mData + mCurrentPos, sizeof(double));
		mCurrentPos += sizeof(double);
		return f;
	}

	void MemoryStream::readBuffer(void* dest, PxU32 size) const
	{
		memcpy(dest, mData + mCurrentPos, size);
		mCurrentPos += size;
	}

	void MemoryStream::seek(PxU32 pos)
	{
		mCurrentPos = 0;
	}
}