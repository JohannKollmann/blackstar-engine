#ifndef __SGTATOMHANDLER_INCL
#define __SGTATOMHANDLER_INCL

#include "SGTLoadSave.h"

class SGTAtomHandler
{
public:
	virtual std::string& TellName() = 0;
	virtual int TellByteSize(){return 0;}//don't overload if the atom will not be written

	virtual void Save(SGTSaveSystem& ss, void* pData, std::string strVarName) = 0;
	virtual void Load(SGTLoadSystem& ls, void* pDest) = 0;
};

#endif