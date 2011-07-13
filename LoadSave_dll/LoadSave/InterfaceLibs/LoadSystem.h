#pragma once

#include "LoadSave.h"
#include "Saveable.h"

#include "../LowLevelLibs/LoadSave_dll.h"

namespace LoadSave
{
/**
 * central class for objects being loaded
 */
class LS_EXPORT LoadSystem
{
public:
	~LoadSystem();
	/**
	 * constructors similar to {@link LoadSave::LoadSave::LoadFile}
	 * you would use the functions of LoadSave class
	 */
	LoadSystem(std::string strFile);
	LoadSystem(void* pData, int iDataSize);
	/**
	 * close files held open by SaveSystem. 
	 * @param strBinaryFile filename of binary save data 
	 */
	void CloseFile();
	template<class T>
		std::shared_ptr<T> LoadTypedObject()
		{
			return std::static_pointer_cast<T, Saveable>(LoadObject());
		}
	/**
	 * close files held open by SaveSystem.
	 * @return loaded object
	 */
	std::shared_ptr<Saveable> LoadObject();
	/**
	 * load an atom 
	 * @param strType registration name of the atom handler
	 * @param pAtom pointer to load data to
	 */
	void LoadAtom(std::string strType, void* pAtom);
	/**
	 * this function is used internally by atoms
	 */
	void ReadAtom(std::string strType, void* pAtom);
	/**
	 * open an array of atoms
	 * @param strType object to save
	 * @return vector with size of each dimension of the array e.g. [1][2][3]
	 */
	std::vector<int> LoadAtomArray(std::string strType);
	/**
	 * open an array of atoms
	 * @param pType pointer to registration name, can be NULL
	 * @return vector with size of each dimension of the array e.g. [1][2][3]
	 * */
	std::vector<int> LoadObjectArray(std::string* pType);
	/**
	 * load an atom from an opened array
	 * @param strType registration name of the atom handler
	 * @param pAtom pointer to load data to
	 */
	void LoadArrayAtom(std::string strType, void* pAtom);
	/**
	 * close files held open by SaveSystem.  
	 * @return loaded object
	 */
	std::shared_ptr<Saveable> LoadArrayObject();

	bool HasObjects();
private:
	int GetAtomSize(int iID);
	std::map<int, std::shared_ptr<Saveable> > m_RecordIDs;
	CLoadManager* m_pLM;
};

};
