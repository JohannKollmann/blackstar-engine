#pragma once

#include "LoadSave.h"

namespace LoadSave
{

class AtomHandler
{
public:
	virtual std::string& TellName() = 0;
	virtual int TellByteSize(){return 0;}//don't overload if the atom will not be written

	virtual void Save(SaveSystem& ss, void* pData, std::string strVarName) = 0;
	virtual void Load(LoadSystem& ls, void* pDest) = 0;
};

};