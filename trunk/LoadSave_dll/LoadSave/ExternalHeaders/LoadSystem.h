#pragma once

#include "LoadSave.h"
#include "Saveable.h"

#include "../LowLevelLibs/LoadSave_dll.h"

namespace LoadSave
{

class LS_EXPORT LoadSystem
{
public:
	LoadSystem(std::string strFile);
	LoadSystem(void* pData, int iDataSize);
	void CloseFile();
	template<class T>
		std::shared_ptr<T> LoadTypedObject()
		{
			return std::static_pointer_cast<T, Saveable>(LoadObject());
		}
	std::shared_ptr<Saveable> LoadObject();
	void LoadAtom(std::string strType, void* pAtom);
	void ReadAtom(std::string strType, void* pAtom);
	std::vector<int> LoadAtomArray(std::string strType);
	std::vector<int> LoadObjectArray(std::string* pType);//can be NULL
	void LoadArrayAtom(std::string strType, void* pAtom);
	std::shared_ptr<Saveable>  LoadArrayObject();

	bool HasObjects();
private:
	int GetAtomSize(int iID);
	std::map<int, Saveable*> m_RecordIDs;
	CLoadManager* m_pLM;
};

};
