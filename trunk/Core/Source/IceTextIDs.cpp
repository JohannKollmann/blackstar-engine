#include "IceIncludes.h"
#include "IceTextIDs.h"

namespace Ice
{
	TextID::TextID(std::pair<int, std::string> *aIDs, int iEnumFirst, int iEnumLast, int iArraySize)
	{
		int iEnumSize=iEnumLast-iEnumFirst;
		IceAssert(iEnumSize==iArraySize);
		for (int i = 0; i < iEnumSize; ++i)
		{
			m_mS2I[aIDs[i].second]=aIDs[i].first;
			m_mI2S[aIDs[i].first]=aIDs[i].second;
		}
		m_iCurrDynamicID=iEnumLast;
	}
	std::string TextID::lookup(unsigned int iID)
	{
		std::map<int, std::string>::const_iterator it=m_mI2S.find(iID);
		if(it!=m_mI2S.end())
			return it->second;
		else
			return std::string("error");
	}
	int TextID::lookup(std::string strID)
	{
		std::map<std::string, int>::const_iterator it=m_mS2I.find(strID);
		if(it!=m_mS2I.end())
			return it->second;
		else
		{
			int iID=m_iCurrDynamicID++;
			m_mS2I[strID]=iID;
			m_mI2S[iID]=strID;
			return iID;
		}
	}
	std::map<std::string, int>::const_iterator
	TextID::iterate()
	{
		return m_mS2I.begin();
	}
}