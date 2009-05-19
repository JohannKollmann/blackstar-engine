//this is the central header for the SGT load and save library

#ifndef __SGTLOADSAVE_INCL
#define __SGTLOADSAVE_INCL

#include <string>
#include <map>
#include <vector>

#include "SGTSaveable.h"
#include "SGTSaveSystem.h"
#include "SGTLoadSystem.h"
#include "SGTAtomHandler.h"
#include "SGTRecordReference.h"

class __declspec( dllexport ) SGTLoadSave
{
public:
	static SGTLoadSave& Instance();
	static void RegisterObject(SGTSaveableRegisterFn registerFn);
	static void RegisterAtom(SGTAtomHandler* pHandler);
	int GetAtomID(std::string strType);//0 if no atom
	int GetObjectID(std::string strType);//0 if no object
	SGTAtomHandler* GetAtomHandler(std::string strAtomName);
	SGTSaveableInstanceFn GetInstanceFunction(std::string strObjectName);
	std::string GetTypeName(int iID);

	std::map<int, int> GetAtomSizes();//first: id; second: bytesize

	SGTSaveSystem* CreateSaveFile(std::string strBinaryFile, std::string strXMLFile);
	SGTLoadSystem* LoadFile(std::string strBinaryFile);
	
	SGTSaveSystem* CreateSaveFile(void* &pData, int& iDataSize, std::string strXMLFile);
	SGTLoadSystem* LoadFile(void* pData, int iDataSize);

	void SetLogFunction(void (*pFn)(std::string));
	
	void PostError(std::string strError);
	void PostLogEntry(std::string strLogEntry);
};

#endif //__SGTLOADSAVE_INCL