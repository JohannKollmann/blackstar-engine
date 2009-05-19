#ifndef __SGTSAVESYSTEM_INCL
#define __SGTSAVESYSTEM_INCL

#include "SGTLoadSave.h"
#include "SGTSaveable.h"

#include "..\LowLevelLibs\LoadSave_dll.h"



class __declspec( dllexport ) SGTSaveSystem
{
public:
	SGTSaveSystem(std::string strBinaryFile, std::string strXMLFile);
	SGTSaveSystem(void* &pData, int& iDataSize, std::string strXMLFile);
	void CloseFiles();
	//this one is normally used to save an atom, will work with complex types
	void SaveAtom(std::string strType, void* pData, std::string varName);
	//these are only internally used to write the atom to disk in fact, directly writes the plain data
	void WriteAtom(std::string strType, void* pData, std::string varName, std::string literalData);
	void WriteAtomString(std::string strDataType, std::string strArrayType, void *pData, std::string varName, std::string literalData);
	void WriteAtomToArray(std::string strType, void* pData, std::string literalData);
	//will save obejcts with the same pointer as reference to that object and also deliver the same pointers when loading them
	void SaveObject(SGTSaveable* pObj, std::string varName);
	//array functions
	void OpenAtomArray(std::string strType, std::vector<int> dimSizes, std::string varName);
	void AddAtom(std::string strType, void* pData);
	void AddAtomA(std::string strType, void* pData){this->AddAtom(strType, pData);}
	void OpenObjectArray(std::string strType, std::vector<int> dimSizes, std::string varName);
	void AddObject(SGTSaveable* pObj);
private:
	CSaveManager* m_pSM;
	std::map<SGTSaveable*, int> m_RecordIDs;

};

#endif //__SGTSAVESYSTEM_INCL