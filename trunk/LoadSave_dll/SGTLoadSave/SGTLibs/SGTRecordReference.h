#ifndef __SGTRECORDREFERENCE_INCL
#define __SGTRECORDREFERENCE_INCL
#include "SGTAtomHandler.h"
#include <sstream>

class SGTRecordReference : SGTAtomHandler
{
public:
	SGTRecordReference(){m_strName="SGTRecordReference";}
	std::string& TellName(){return m_strName;}
	int TellByteSize(){return sizeof(int);}
	void Save(SGTSaveSystem& ss, void* pData, std::string strVarName){	ss.WriteAtom(TellName(), pData, strVarName, Int2Str(*((int*)pData)));}
	void Load(SGTLoadSystem& ls, void* pDest){ls.ReadAtom(TellName(), pDest);}
private:
	std::string m_strName;
};

#endif