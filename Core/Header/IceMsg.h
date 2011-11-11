
#pragma once

#include "IceIncludes.h"
#include "IceDataMap.h"

namespace Ice
{
	typedef int MsgTypeID;

	class DllExport Msg
	{
	public:

		Msg() : rawData(nullptr) {}
		Msg(MsgTypeID type) : rawData(nullptr), typeID(type) {}

		virtual ~Msg() { }

		DataMap params;
		MsgTypeID typeID;
		void *rawData;
	};
};