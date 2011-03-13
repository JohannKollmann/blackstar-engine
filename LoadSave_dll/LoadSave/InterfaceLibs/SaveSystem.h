#pragma once

#include "LoadSave.h"
#include "Saveable.h"

#include "../LowLevelLibs/LoadSave_dll.h"

namespace LoadSave
{

	class LS_EXPORT SaveSystem
	{
	public:
		SaveSystem(std::string strBinaryFile, std::string strXMLFile);
		SaveSystem(void* &pData, int& iDataSize, std::string strXMLFile);
		void CloseFiles();
		//this one is normally used to save an atom, will work with complex types
		void SaveAtom(std::string strType, void* pData, std::string varName);
		//these are only internally used to write the atom to disk in fact, directly writes the plain data
		void WriteAtom(std::string strType, void* pData, std::string varName, std::string literalData);
		void WriteAtomString(std::string strDataType, std::string strArrayType, void *pData, std::string varName, std::string literalData);
		void WriteAtomToArray(std::string strType, void* pData, std::string literalData);
		//will save obejcts with the same pointer as reference to that object and also deliver the same pointers when loading them
		void SaveObject(Saveable* pObj, std::string varName, bool allowNull = false, bool useRecordReferences = true);
		//array functions
		void OpenAtomArray(std::string strType, std::vector<int> dimSizes, std::string varName);
		void AddAtom(std::string strType, void* pData);
		void AddAtomA(std::string strType, void* pData){this->AddAtom(strType, pData);}
		//void AddAtomW(std::wstring strType, void* pData){USES_CONVERSION; this->AddAtom(W2A(strType.c_str()), pData);}
		void OpenObjectArray(std::string strType, std::vector<int> dimSizes, std::string varName);
		void AddObject(Saveable* pObj);

		void SetUseRecordReferences(bool use);
	private:
		CSaveManager* m_pSM;
		std::map<Saveable*, int> m_RecordIDs;

		bool mUseRecordReferences;

	};

};
