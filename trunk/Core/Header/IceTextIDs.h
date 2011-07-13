#pragma once
#include <map>
#include <vector>
#include <string>
#include "IceIncludes.h"

#define TEXTID_BEGIN_DEF(arr_name) std::pair<int, std::string> arr_name[]={
#define TEXTID_DEFINE_CONSTANT(c) std::make_pair(c, #c),
#define TEXTID_END_DEF };

#define TEXTID_DEFS_SIZE(arr_name) ((int)sizeof(arr_name)/sizeof(std::pair<int, std::string>))

namespace Ice
{
	class TextID
	{
	public:
		TextID(){};
		TextID(std::pair<int, std::string> *aIDs, int iEnumSize, int iArraySize);
		std::string lookup(unsigned int iID);
		int lookup(std::string strID);
		std::map<std::string, int>::const_iterator iterate();
	private:
		std::map<std::string, int> m_mS2I;
		std::map<int, std::string> m_mI2S;
	};
}