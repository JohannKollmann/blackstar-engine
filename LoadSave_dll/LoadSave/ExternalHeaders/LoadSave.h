//this is the central header for the  load and save library

#ifdef _MSC_VER
	#define LS_EXPORT __declspec( dllexport )
#else
	#define LS_EXPORT
	#define nullptr 0
#endif

#pragma once

#include <string>
#include <map>
#include <vector>

#include "Saveable.h"
#include "SaveSystem.h"
#include "LoadSystem.h"
#include "AtomHandler.h"
#include "RecordReference.h"

namespace LoadSave
{

class LS_EXPORT LoadSave
{
public:
	static LoadSave& Instance();
	static void RegisterObject(SaveableRegisterFn registerFn);
	static void RegisterAtom(AtomHandler* pHandler);
	int GetAtomID(std::string strType);//0 if no atom
	int GetObjectID(std::string strType);//0 if no object
	AtomHandler* GetAtomHandler(std::string strAtomName);
	SaveableInstanceFn GetInstanceFunction(std::string strObjectName);
	std::string GetTypeName(int iID);

	std::map<int, int> GetAtomSizes();//first: id; second: bytesize

	SaveSystem* CreateSaveFile(std::string strBinaryFile, std::string strXMLFile);
	LoadSystem* LoadFile(std::string strBinaryFile);
	
	SaveSystem* CreateSaveFile(void* &pData, int& iDataSize, std::string strXMLFile);
	LoadSystem* LoadFile(void* pData, int iDataSize);

	void SetLogFunction(void (*pFn)(std::string));
	
	void PostError(std::string strError);
	void PostLogEntry(std::string strLogEntry);
};

};
