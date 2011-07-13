#pragma once

#include "LoadSave.h"
#include "Saveable.h"

#include "../LowLevelLibs/LoadSave_dll.h"

namespace LoadSave
{
	/**
	 * central class for objects being saved
	 */
	class LS_EXPORT SaveSystem
	{
	public:
		/**
		 * constructors similar to {@link LoadSave::LoadSave::CreateSaveFile}
		 * you would use the functions of LoadSave class
		 */
		SaveSystem(std::string strBinaryFile, std::string strXMLFile);
		SaveSystem(void* &pData, int& iDataSize, std::string strXMLFile);
		/**
		 * close files held open by SaveSystem. 
		 * @param strBinaryFile filename of binary save data 
		 */
		void CloseFiles();
		/**
		 * save an atom 
		 * @param strType registration name of the atom handler
		 * @param pData pointer to the atom
		 * @param varName name of the variable, used for debug xml
		 */
		void SaveAtom(std::string strType, void* pData, std::string varName);
		/**
		 * these are only internally used to write the atom to disk. in fact, directly writes the plain data
		 */
		void WriteAtom(std::string strType, void* pData, std::string varName, std::string literalData);
		void WriteAtomString(std::string strDataType, std::string strArrayType, void *pData, std::string varName, std::string literalData);
		void WriteAtomToArray(std::string strType, void* pData, std::string literalData);
		/**
		 * save objects with the same pointer as reference to that object and also deliver the same pointers when loading them
		 * @param pObj object to save
		 * @param varName name of the variable, used for debug xml
		 * @param allowNull allow NULL as an argument
		 * @param useRecordReferences use record references, i.e. if the same pointer is used for saving, only one pointer will be restored when loading
		 */
		void SaveObject(Saveable* pObj, std::string varName, bool allowNull = false, bool useRecordReferences = true);
		/**
		 * begin an array of atoms
		 * @param strType object to save
		 * @param dimSizes size of each dimension of the array e.g. [1][2][3]
		 * @param varName name of the variable, used for debug xml
		 */
		void OpenAtomArray(std::string strType, std::vector<int> dimSizes, std::string varName);
		/**
		 * Add an atom to an opened array. the array will be closed when the declared size has been reached.
		 * @param strType registration name of the atom handler
		 * @param pData pointer to the atom
		 */
		void AddAtom(std::string strType, void* pData);
		//library bug
		void AddAtomA(std::string strType, void* pData){this->AddAtom(strType, pData);}
		//void AddAtomW(std::wstring strType, void* pData){USES_CONVERSION; this->AddAtom(W2A(strType.c_str()), pData);}
		/**
		 * begin an array of objects
		 * @param strType object to save
		 * @param dimSizes size of each dimension of the array e.g. [1][2][3]
		 * @param varName name of the variable, used for debug xml
		 */
		void OpenObjectArray(std::string strType, std::vector<int> dimSizes, std::string varName);
		/**
		 * Add an onject to an opened array. the array will be closed when the declared size has been reached.
		 * @param pObj Object to add
		 */
		void AddObject(Saveable* pObj);
		/**
		 * switch record references on or off
		 * @param use use or not
		 */
		void SetUseRecordReferences(bool use);
	private:
		CSaveManager* m_pSM;
		std::map<Saveable*, int> m_RecordIDs;

		bool mUseRecordReferences;
	};
};
