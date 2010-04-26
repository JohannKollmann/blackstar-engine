#pragma once

#include "AtomHandler.h"
#include <sstream>

namespace LoadSave
{

class RecordReference : AtomHandler
{
public:
	RecordReference(){m_strName="RecordReference";}
	std::string& TellName(){return m_strName;}
	int TellByteSize(){return sizeof(int);}
	void Save(SaveSystem& ss, void* pData, std::string strVarName){	ss.WriteAtom(TellName(), pData, strVarName, Int2Str(*((int*)pData)));}
	void Load(LoadSystem& ls, void* pDest){ls.ReadAtom(TellName(), pDest);}
private:
	std::string m_strName;
};

};