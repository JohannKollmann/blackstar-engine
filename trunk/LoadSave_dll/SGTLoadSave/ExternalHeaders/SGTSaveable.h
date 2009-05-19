#ifndef __SGTSAVEABLE_INCL
#define __SGTSAVEABLE_INCL

#include "SGTLoadSave.h"

//declare the prototypes
class SGTLoadSystem;
class SGTSaveSystem;
class SGTSaveable;

//define readable names for the pointers
typedef SGTSaveable* (*SGTSaveableInstanceFn)();
typedef void (*SGTSaveableRegisterFn)(std::string*, SGTSaveableInstanceFn*);

class SGTSaveable
{
public:
	virtual ~SGTSaveable() {};
	virtual void Save(SGTSaveSystem& myManager) = 0;//the central function
	virtual void Load(SGTLoadSystem& mgr) = 0;
	static SGTSaveable* NewInstance();//overload this one
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn);//and this one too
	virtual std::string& TellName() = 0;//return the name
};

class SGTSaveableDummy : public SGTSaveable
{
private:
	std::string m_strName;
public:
	SGTSaveableDummy() { m_strName = "SGTSaveable"; }
	void Save(SGTSaveSystem& myManager) { };
	void Load(SGTLoadSystem& mgr) { };
	static SGTSaveable* NewInstance() { return new SGTSaveableDummy; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "SGTSaveable"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	std::string& TellName() { return m_strName; };
};


#endif//__SGTSAVEABLE_INCL