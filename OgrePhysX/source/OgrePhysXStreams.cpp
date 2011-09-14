
#include "OgrePhysXStreams.h"

namespace OgrePhysX
{
	OgreReadStream::OgreReadStream(Ogre::DataStreamPtr ds)
	{
		mOgreDataStream = ds;
	}
	OgreReadStream::~OgreReadStream() {}

	PxU8 OgreReadStream::readByte() const
	{
		PxU8 byte = 0;
		mOgreDataStream->read(&byte, sizeof(PxU8));
		return byte;
	}
	PxU16 OgreReadStream::readWord() const
	{
		PxU16 word = 0;
		mOgreDataStream->read(&word, sizeof(PxU16));
		return word;
	}
	PxU32 OgreReadStream::readDword() const
	{
		PxU32 dword = 0;
		mOgreDataStream->read(&dword, sizeof(PxU32));
		return dword;
	}
	float OgreReadStream::readFloat() const
	{
		float f = 0;
		mOgreDataStream->read(&f, sizeof(float));
		return f;
	}
	double OgreReadStream::readDouble() const
	{
		double d = 0;
		mOgreDataStream->read(&d, sizeof(double));
		return d;
	}
	void OgreReadStream::readBuffer(void* buffer, PxU32 size) const
	{
		mOgreDataStream->read(buffer, size);
	}
}