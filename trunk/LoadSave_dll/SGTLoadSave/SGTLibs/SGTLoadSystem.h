#ifndef __SGTLOADSYSTEM_INCL
#define __SGTLOADSYSTEM_INCL

#include "SGTLoadSave.h"
#include "SGTSaveable.h"

#include "..\LowLevelLibs\LoadSave_dll.h"

class __declspec( dllexport ) SGTLoadSystem
{
public:
	SGTLoadSystem(std::string strFile);
	SGTLoadSystem(void* pData, int iDataSize);
	void CloseFile();
	SGTSaveable* LoadObject();
	void LoadAtom(std::string strType, void* pAtom);
	void ReadAtom(std::string strType, void* pAtom);
	std::vector<int> LoadAtomArray(std::string strType);
	std::vector<int> LoadObjectArray(std::string* pType);//can be NULL
	void LoadArrayAtom(std::string strType, void* pAtom);
	SGTSaveable* LoadArrayObject();

	bool HasObjects();
private:
	int GetAtomSize(int iID);
	std::map<int, SGTSaveable*> m_RecordIDs;
	CLoadManager* m_pLM;
};

#endif //__SGTLOADSYSTEM_INCL