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

#ifndef PX_TOOLKIT_STREAM_H
#define PX_TOOLKIT_STREAM_H

#include "common/PxStream.h"
#include <stdio.h>
#include "OgrePhysXClasses.h"

	namespace PxToolkit
	{

	class OgrePhysXClass UserStream : public PxStream
	{
		public:
									UserStream(const char* filename, bool load);
		virtual						~UserStream();

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

					FILE*			fp;
	};

	class OgrePhysXClass MemoryWriteBuffer : public PxStream
	{
		public:
									MemoryWriteBuffer();
		virtual						~MemoryWriteBuffer();
					void			clear();

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

					PxU32			currentSize;
					PxU32			maxSize;
					PxU8*			data;
	};

	class OgrePhysXClass MemoryReadBuffer : public PxStream
	{
		public:
									MemoryReadBuffer(const PxU8* data);
		virtual						~MemoryReadBuffer();

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

		mutable		const PxU8*		buffer;
	};

}

#endif
