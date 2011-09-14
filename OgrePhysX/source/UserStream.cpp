// This code contains NVIDIA Confidential Information and is disclosed to you 
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and 
// any modifications thereto. Any use, reproduction, disclosure, or 
// distribution of this software and related documentation without an express 
// license agreement from NVIDIA Corporation is strictly prohibited.
// 
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2011 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "UserStream.h"
#include "PxAssert.h"
#include "internal/include/PsFile.h"

using namespace physx::pubfnd3;
using namespace PxToolkit;

UserStream::UserStream(const char* filename, bool load) : fp(NULL)
{
	physx::shdfnd3::fopen_s(&fp, filename, load ? "rb" : "wb");
}

UserStream::~UserStream()
{
	if(fp)	fclose(fp);
}

// Loading API
PxU8 UserStream::readByte() const
{
	PxU8 b;
	size_t r = fread(&b, sizeof(PxU8), 1, fp);
	PX_ASSERT(r);
	return b;
}

PxU16 UserStream::readWord() const
{
	PxU16 w;
	size_t r = fread(&w, sizeof(PxU16), 1, fp);
	PX_ASSERT(r);
	return w;
}

PxU32 UserStream::readDword() const
	{
	PxU32 d;
	size_t r = fread(&d, sizeof(PxU32), 1, fp);
	PX_ASSERT(r);
	return d;
	}

float UserStream::readFloat() const
{
	PxReal f;
	size_t r = fread(&f, sizeof(PxReal), 1, fp);
	PX_ASSERT(r);
	return f;
}

double UserStream::readDouble() const
{
	PxF64 f;
	size_t r = fread(&f, sizeof(PxF64), 1, fp);
	PX_ASSERT(r);
	return f;
}

void UserStream::readBuffer(void* buffer, PxU32 size)	const
{
	size_t w = fread(buffer, size, 1, fp);
	PX_ASSERT(w);
}

// Saving API
PxStream& UserStream::storeByte(PxU8 b)
{
	size_t w = fwrite(&b, sizeof(PxU8), 1, fp);
	PX_ASSERT(w);
	return *this;
}

PxStream& UserStream::storeWord(PxU16 w)
	{
	size_t ww = fwrite(&w, sizeof(PxU16), 1, fp);
	PX_ASSERT(ww);
	return *this;
	}

PxStream& UserStream::storeDword(PxU32 d)
{
	size_t w = fwrite(&d, sizeof(PxU32), 1, fp);
	PX_ASSERT(w);
	return *this;
}

PxStream& UserStream::storeFloat(PxReal f)
{
	size_t w = fwrite(&f, sizeof(PxReal), 1, fp);
	PX_ASSERT(w);
	return *this;
}

PxStream& UserStream::storeDouble(PxF64 f)
{
	size_t w = fwrite(&f, sizeof(PxF64), 1, fp);
	PX_ASSERT(w);
	return *this;
}

PxStream& UserStream::storeBuffer(const void* buffer, PxU32 size)
{
	size_t w = fwrite(buffer, size, 1, fp);
	PX_ASSERT(w);
	return *this;
}




MemoryWriteBuffer::MemoryWriteBuffer() : currentSize(0), maxSize(0), data(NULL)
{
}

MemoryWriteBuffer::~MemoryWriteBuffer()
{
	delete [] data;
}

void MemoryWriteBuffer::clear()
{
	currentSize = 0;
}

void MemoryWriteBuffer::readBuffer(void*, PxU32) const
{
	PX_ASSERT(0); 
}

PxStream& MemoryWriteBuffer::storeByte(PxU8 b)
{
	storeBuffer(&b, sizeof(PxU8));
	return *this;
}

PxStream& MemoryWriteBuffer::storeWord(PxU16 w)
{
	storeBuffer(&w, sizeof(PxU16));
	return *this;
}

PxStream& MemoryWriteBuffer::storeDword(PxU32 d)
{
	storeBuffer(&d, sizeof(PxU32));
	return *this;
}

PxStream& MemoryWriteBuffer::storeFloat(PxReal f)
{
	storeBuffer(&f, sizeof(PxReal));
	return *this;
}

PxStream& MemoryWriteBuffer::storeDouble(PxF64 f)
{
	storeBuffer(&f, sizeof(PxF64));
	return *this;
}

PxStream& MemoryWriteBuffer::storeBuffer(const void* buffer, PxU32 size)
{
	PxU32 expectedSize = currentSize + size;
	if(expectedSize > maxSize)
	{
		maxSize = expectedSize + 4096;

		PxU8* newData = new PxU8[maxSize];
		PX_ASSERT(newData!=NULL);

		if(data)
		{
			memcpy(newData, data, currentSize);
			delete[] data;
		}
		data = newData;
	}
	memcpy(data+currentSize, buffer, size);
	currentSize += size;
	return *this;
}


MemoryReadBuffer::MemoryReadBuffer(const PxU8* data) : buffer(data)
{
}

MemoryReadBuffer::~MemoryReadBuffer()
{
	// We don't own the data => no delete
}

PxU8 MemoryReadBuffer::readByte() const
{
	PxU8 b;
	memcpy(&b, buffer, sizeof(PxU8));
	buffer += sizeof(PxU8);
	return b;
}

PxU16 MemoryReadBuffer::readWord() const
{
	PxU16 w;
	memcpy(&w, buffer, sizeof(PxU16));
	buffer += sizeof(PxU16);
	return w;
}

PxU32 MemoryReadBuffer::readDword() const
{
	PxU32 d;
	memcpy(&d, buffer, sizeof(PxU32));
	buffer += sizeof(PxU32);
	return d;
}

float MemoryReadBuffer::readFloat() const
{
	float f;
	memcpy(&f, buffer, sizeof(float));
	buffer += sizeof(float);
	return f;
}

double MemoryReadBuffer::readDouble() const
{
	double f;
	memcpy(&f, buffer, sizeof(double));
	buffer += sizeof(double);
	return f;
}

void MemoryReadBuffer::readBuffer(void* dest, PxU32 size) const
{
	memcpy(dest, buffer, size);
	buffer += size;
}

PxStream& MemoryReadBuffer::storeBuffer(const void*, PxU32)
{
	PX_ASSERT(0); 
	return *this;
}


PxU8 MemoryWriteBuffer::readByte() const		{ PX_ASSERT(0);	return 0;		}
PxU16 MemoryWriteBuffer::readWord() const		{ PX_ASSERT(0);	return 0;		}
PxU32 MemoryWriteBuffer::readDword() const		{ PX_ASSERT(0);	return 0;		}
float MemoryWriteBuffer::readFloat() const		{ PX_ASSERT(0);	return 0.0f;	}
double MemoryWriteBuffer::readDouble() const	{ PX_ASSERT(0);	return 0.0;		}

PxStream& MemoryReadBuffer::storeByte(PxU8)		{ PX_ASSERT(0);	return *this;	}
PxStream& MemoryReadBuffer::storeWord(PxU16)	{ PX_ASSERT(0);	return *this;	}
PxStream& MemoryReadBuffer::storeDword(PxU32)	{ PX_ASSERT(0);	return *this;	}
PxStream& MemoryReadBuffer::storeFloat(PxReal)	{ PX_ASSERT(0);	return *this;	}
PxStream& MemoryReadBuffer::storeDouble(PxF64)	{ PX_ASSERT(0);	return *this;	}

