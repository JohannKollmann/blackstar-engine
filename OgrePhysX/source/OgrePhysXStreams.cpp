
#include "OgrePhysXStreams.h"

namespace OgrePhysX
{
	OgreReadStream::OgreReadStream(Ogre::DataStreamPtr ds)
	{
		mOgreDataStream = ds;
	}
	OgreReadStream::~OgreReadStream() {}

	NxU8 OgreReadStream::readByte() const
	{
		NxU8 byte = 0;
		mOgreDataStream->read(&byte, sizeof(NxU8));
		return byte;
	}
	NxU16 OgreReadStream::readWord() const
	{
		NxU16 word = 0;
		mOgreDataStream->read(&word, sizeof(NxU16));
		return word;
	}
	NxU32 OgreReadStream::readDword() const
	{
		NxU32 dword = 0;
		mOgreDataStream->read(&dword, sizeof(NxU32));
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
	void OgreReadStream::readBuffer(void* buffer, NxU32 size) const
	{
		mOgreDataStream->read(buffer, size);
	}
}