/*
 * idconst.cpp
 *
 *  Created on: Jul 11, 2011
 *      Author: insi
 */
#include "IceTextIDs.h"

namespace Ice
{
	TextID::TextID(std::pair<int, std::string> *aIDs, int iEnumSize, int iArraySize)
	{
		assert(iEnumSize==iArraySize);
		for (int i = 0; i < iEnumSize; ++i)
		{
			m_mS2I[aIDs[i].second]=aIDs[i].first;
			m_mI2S[aIDs[i].first]=aIDs[i].second;
		}
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
			return -1;
	}
	std::map<std::string, int>::const_iterator
	TextID::iterate()
	{
		return m_mS2I.begin();
	}
}
/*
int main()
{
	int nPublishedConstants=(int)sizeof(aMyConstants)/sizeof(std::pair<int, std::string>);
	printf("%d, %d\n", CONST_SIZE, nPublishedConstants);
	//check if all constants have been published
	assert(CONST_SIZE==nPublishedConstants);
	CIDLookup l(aMyConstants, CONST_SIZE);
	int i=l.lookup("CONST_B");
	std::string str=l.lookup(CONST_C);
	return 0;
}
*/