#pragma once

#include "LoadSave.h"

namespace LoadSave
{
/**
 * interface class for atom handlers. atoms are objects that are managed externally regarding load/save
 */
class AtomHandler
{
public:
	/**
	 * return the registration name
	 * @return reference to registration name
	 */
	virtual std::string& TellName() = 0;
	/**
	 * tell the size of the data written in bytes
	 * don't overload if the atom will not be directly written using {@link LoadSave::SaveSystem::SaveAtom}
	 * @return size in bytes
	 */
	virtual int TellByteSize(){return 0;}
	/**
	 * load and save atom members in these functions using the managers and provided pointers
	 */
	virtual void Save(SaveSystem& ss, void* pData, std::string strVarName) = 0;
	virtual void Load(LoadSystem& ls, void* pDest) = 0;
};

};