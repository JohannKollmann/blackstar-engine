#pragma once

#include "LoadSave.h"

namespace LoadSave
{

//declare the prototypes
class LoadSystem;
class SaveSystem;
class Saveable;

/**
 * function pointer for the instantiation function
 * @return freshly created saveable object
 */
typedef Saveable* (*SaveableInstanceFn)();
/**
 * function pointer for the registration of a saveable 
 * @param string* pointer to registration name
 * @param SaveableInstanceFn* instantiation function pointer
 */
typedef void (*SaveableRegisterFn)(std::string*, SaveableInstanceFn*);

/**
 * interface class for LoadSave system
 */
class Saveable
{
public: 
	virtual ~Saveable() {}
	/**
	 * load and save class members in these functions using the managers
	 */
	virtual void Save(SaveSystem& myManager) = 0;//the central function
	virtual void Load(LoadSystem& mgr) = 0;
	virtual void SetWeakThis(std::weak_ptr<Saveable> wThis) {}
	/**
	 * return an instance
	 */
	static Saveable* NewInstance();
	/**
	 * registration function for the system
	 */
	static void Register(std::string* pstrName, SaveableInstanceFn* pFn);
	/**
	 * return the registration name
	 */
	virtual std::string& TellName() = 0;
};

/**
 * reference implementation of saveable class
 */
class SaveableDummy : public Saveable
{
private:
	std::string m_strName;
public:
	SaveableDummy() { m_strName = "Saveable"; }
	void Save(SaveSystem& myManager) { };
	void Load(LoadSystem& mgr) { };
	static Saveable* NewInstance() { return new SaveableDummy; };
	static void Register(std::string* pstrName, SaveableInstanceFn* pFn) { *pstrName = "Saveable"; *pFn = (SaveableInstanceFn)&NewInstance; };
	std::string& TellName() { return m_strName; };
};

};