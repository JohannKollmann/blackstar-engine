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
/**
 * main class of LoadSave-System
 * - manages creation of writers and parsers for save files
 * - allows registration of classes as saveable objects 
 */
class LS_EXPORT LoadSave
{
public:
	/**
	 * get singleton
	 * @return singleton instance
	 */
	static LoadSave& Instance();
	/**
	 * register an object that implements {@link LoadSave::Saveable} interface
	 * @param registerFn function to deliver name and instantiation function for saveable
	 */
	static void RegisterObject(SaveableRegisterFn registerFn);
	/**
	 * register an {@link LoadSave::AtomHandler} to handle primitive data types or foreign objects
	 * @param pHandler instance of AtomHandler Object
	 */
	static void RegisterAtom(AtomHandler* pHandler);
	/**
	 * get ID for registration name of an atom
	 * @param strType registration name
	 * @return ID
	 */
	int GetAtomID(std::string strType);//0 if no atom
	/**
	 * get ID for registration name of an object
	 * @param strType registration name
	 * @return ID
	 */
	int GetObjectID(std::string strType);//0 if no object
	/**
	 * get AtomHandler for registration name of an atom
	 * @param strAtomName registration name
	 * @return AtomHandler pointer
	 */
	AtomHandler* GetAtomHandler(std::string strAtomName);
	/**
	 * get instantiating function for Saveable
	 * @param strObjectName registration name
	 * @return instantiating function
	 */
	SaveableInstanceFn GetInstanceFunction(std::string strObjectName);
	/**
	 * get name for ID
	 * @param iID ID of Object/Atom
	 * @return registration name
	 */
	std::string GetTypeName(int iID);
	/**
	 * get map of atom IDs and sizes
	 * @param iID ID of Object/Atom
	 * @return registration name
	 */
	std::map<int, int> GetAtomSizes();//first: id; second: bytesize
	/**
	 * create a save file in binary format. xml output is for debug purposes only and can't be read
	 * @param strBinaryFile filename of binary save data, file will be overwritten or created
	 * @param strXMLFile filename of xml debug data, file will be overwritten or created.
	 * xml dabug output can be debugged in a visual manner by using the XSLT viewer in a web browser 
	 * @return {@link LoadSave::SaveSystem} object to access the savefile
	 */
	SaveSystem* CreateSaveFile(std::string strBinaryFile, std::string strXMLFile);
	/**
	 * read a save file in binary format
	 * @param strBinaryFile filename of binary save data 
	 * @return {@link LoadSave::LoadSystem} object to access the savefile
	 */
	LoadSystem* LoadFile(std::string strBinaryFile);
	
	/**
	 * similar to the overloaded functions, using memory instead of files
	 */
	SaveSystem* CreateSaveFile(void* &pData, int& iDataSize, std::string strXMLFile);
	LoadSystem* LoadFile(void* pData, int iDataSize);
	/**
	 * set log function for the loadsave library
	 * @param pFn function pointer to a log function
	 */
	void SetLogFunction(void (*pFn)(std::string));
	
	/**
	 * post error or log entries
	 * @param string message to log
	 */
	void PostError(std::string strError);
	void PostLogEntry(std::string strLogEntry);
};

};
