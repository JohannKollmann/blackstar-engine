#pragma once

#include "LoadSave.h"

namespace LoadSave
{

//declare the prototypes
class LoadSystem;
class SaveSystem;
class Saveable;

//define readable names for the pointers
typedef Saveable* (*SaveableInstanceFn)();
typedef void (*SaveableRegisterFn)(std::string*, SaveableInstanceFn*);

class Saveable
{
public: 
	//virtual ~Saveable() {};
	virtual void Save(SaveSystem& myManager) = 0;//the central function
	virtual void Load(LoadSystem& mgr) = 0;
	static Saveable* NewInstance();//overload this one
	static void Register(std::string* pstrName, SaveableInstanceFn* pFn);//and this one too
	virtual std::string& TellName() = 0;//return the name
};

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