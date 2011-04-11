#pragma once

#include "LoadSave.h"

#include <list>
#include <sstream>

//#include "ResidentVariables.h"

//the stl atoms

//this is not a regular hack, it's MADNESS !1!one1eleven!!
//who would use macros to define CLASSES? wtf? :D

 #define CREATEVECTORHANDLER(template_type, strTemplateTypeName, strHandlerName) \
class strHandlerName : LoadSave::AtomHandler\
{\
public:\
	strHandlerName (){m_strName= std::string("std::vector<") + strTemplateTypeName + std::string(">");}\
	std::string& TellName(){return m_strName;}\
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName);\
	void Load(LoadSave::LoadSystem& ls, void* pDest);\
private:\
	std::string m_strName;\
};\
\
void \
strHandlerName ::Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName)\
{\
	/*test what category the data is*/\
	std::vector<int> dims;\
	std::vector< template_type >* pVector=(std::vector< template_type >*)pData;\
	dims.push_back(pVector->size());\
\
	{/*it must be an atom*/\
		/*open an atom-array*/\
		ss.OpenAtomArray( strTemplateTypeName , dims, "_array");\
		for(unsigned int i=0; i<pVector->size(); i++)\
			ss.AddAtom( strTemplateTypeName , &((*pVector)[i]));\
	}\
} \
\
void \
strHandlerName ::Load(LoadSave::LoadSystem& ls, void* pDest)\
{\
	std::vector<int> dims;\
	std::vector< template_type >* pVector=(std::vector< template_type >*)pDest;\
	{/*it must be an atom*/\
		dims=ls.LoadAtomArray(strTemplateTypeName);\
	}\
	for(int i=0; i<dims[0]; i++)\
	{\
		{/*it must be an atom*/\
			template_type atom;\
			ls.LoadArrayAtom(strTemplateTypeName, &atom);\
			pVector->push_back(atom);\
		}\
	}\
}

#define CREATELISTHANDLER(template_type, strTemplateTypeName, strHandlerName) \
class strHandlerName : LoadSave::AtomHandler\
{\
public:\
	strHandlerName (){m_strName= std::string("std::list<") + strTemplateTypeName + std::string(">");}\
	std::string& TellName(){return m_strName;}\
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName);\
	void Load(LoadSave::LoadSystem& ls, void* pDest);\
private:\
	std::string m_strName;\
};\
\
void \
strHandlerName ::Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName)\
{\
	/*test what category the data is*/\
	std::vector<int> dims;\
	std::list< template_type >* pList=(std::list< template_type >*)pData;\
	dims.push_back(pList->size());\
\
	std::list< template_type >::const_iterator it=pList->begin();\
\
	ss.OpenAtomArray( strTemplateTypeName , dims, "_list");\
\
	for(;it!=pList->end(); it++)\
	{\
		ss.AddAtom( strTemplateTypeName , (void*)&(*it));\
	}\
}\
\
void \
strHandlerName ::Load(LoadSave::LoadSystem& ls, void* pDest)\
{\
	std::vector<int> dims;\
	std::list< template_type >* pList=(std::list< template_type >*)pDest;\
	{/*it must be an atom*/\
		dims=ls.LoadAtomArray(strTemplateTypeName);\
	}\
	for(int i=0; i<dims[0]; i++)\
	{\
		{/*it must be an atom*/\
			template_type atom;\
			ls.LoadArrayAtom(strTemplateTypeName, &atom);\
			pList->insert(pList->end(), atom);\
		}\
	}\
}

#define CREATEMAPHANDLER(key_type, strKeyTypeName, value_type, strValueTypeName, strHandlerName) \
class strHandlerName : LoadSave::AtomHandler\
{\
public:\
	strHandlerName (){m_strName= std::string("std::map<") + strKeyTypeName + std::string(", ") + strValueTypeName + std::string(">");}\
	std::string& TellName(){return m_strName;}\
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName);\
	void Load(LoadSave::LoadSystem& ls, void* pDest);\
private:\
	std::string m_strName;\
};\
\
void \
strHandlerName ::Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName)\
{\
	/*test what category the data is*/\
	std::vector<int> dims;\
	std::map< key_type, value_type >* pMap=(std::map< key_type, value_type >*)pData;\
	dims.push_back(pMap->size());\
\
	std::map< key_type, value_type >::const_iterator it=pMap->begin();\
\
	ss.OpenAtomArray( strKeyTypeName , dims, "_key");\
	for(;it!=pMap->end();it++)\
	{\
		ss.AddAtom( strKeyTypeName , (void*)&(it->first));\
	}\
\
	it=pMap->begin();\
\
	ss.OpenAtomArray( strValueTypeName , dims, "_value");\
	for(;it!=pMap->end();it++)\
	{\
		ss.AddAtom( strValueTypeName , (void*)&(it->second));\
	}\
} \
void \
strHandlerName ::Load(LoadSave::LoadSystem& ls, void* pData)\
{\
	std::vector<int> dims;\
	std::map< key_type, value_type >* pMap=(std::map< key_type, value_type >*)pData;\
\
	std::list<key_type> keylist;\
	{/*it must be an atom*/\
		dims=ls.LoadAtomArray(strKeyTypeName);\
	}\
	for(int i=0; i<dims[0]; i++)\
	{\
		{/*it must be an atom*/\
			key_type atom;\
			ls.LoadArrayAtom(strKeyTypeName, &atom);\
			keylist.insert(keylist.end(), atom);\
		}\
	}\
\
	{/*it must be an atom*/\
		dims=ls.LoadAtomArray(strValueTypeName);\
	}\
	std::list<key_type>::const_iterator it=keylist.begin();\
	for(; it!=keylist.end(); it++)\
	{\
		{/*it must be an atom*/\
			value_type atom;\
			ls.LoadArrayAtom(strValueTypeName, &atom);\
			pMap->insert(std::pair<key_type, value_type>(*it, atom));\
		}\
	}\
}

namespace Ice
{

class NullObjectHandler : LoadSave::AtomHandler
{
public:
	NullObjectHandler(){m_strName="NullObject";}
	std::string& TellName(){return m_strName;}
	int TellByteSize(){return 0;}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName){}
	void Load(LoadSave::LoadSystem& ls, void* pDest){}
private:
	std::string m_strName;
};

class BoolHandler : LoadSave::AtomHandler
{
public:
	BoolHandler(){m_strName="bool";}
	std::string& TellName(){return m_strName;}
	int TellByteSize(){return sizeof(bool);}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName){ss.WriteAtom(TellName(), pData, strVarName, std::string((*(bool*)(pData) == true) ? "true" : "false"));}
	void Load(LoadSave::LoadSystem& ls, void* pDest){ls.ReadAtom(m_strName, pDest);}
private:
	std::string m_strName;
};


std::string Int2Str(int i)
{
	std::stringstream stream;
	stream<<i;
	std::string str;
	stream>>str;
	return str;
}

class IntHandler : LoadSave::AtomHandler
{
public:
	IntHandler(){m_strName="int";}
	std::string& TellName(){return m_strName;}
	int TellByteSize(){return sizeof(int);}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName){ss.WriteAtom(TellName(), pData, strVarName, Int2Str(*((int*)pData)));}
	void Load(LoadSave::LoadSystem& ls, void* pDest){ls.ReadAtom(m_strName, pDest);}
private:
	std::string m_strName;
};

class CharHandler : LoadSave::AtomHandler
{
public:
	CharHandler(){m_strName="char";}
	std::string& TellName(){return m_strName;}
	int TellByteSize(){return sizeof(char);}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName){std::string s; s+=*((char*)pData);ss.WriteAtom(TellName(), pData, strVarName, s);}
	void Load(LoadSave::LoadSystem& ls, void* pDest){ls.ReadAtom(m_strName, pDest);}
private:
	std::string m_strName;
};

class StringHandler : LoadSave::AtomHandler
{
public:
	StringHandler(){m_strName="std::string";}
	std::string& TellName(){return m_strName;}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName){std::string* pStr=(std::string*)pData; ss.WriteAtomString("char", TellName(), (void*)pStr->c_str(), strVarName, *pStr);}
	void Load(LoadSave::LoadSystem& ls, void* pDest);
private:
	std::string m_strName;
};

void
StringHandler::Load(LoadSave::LoadSystem &ls, void *pDest)
{
	std::vector<int> vSize=ls.LoadAtomArray("char");
	std::string* pStr=(std::string*)pDest;
	pStr->clear();
	for(int i=0; i<vSize[0]; i++)
	{
		char ch=0;
		ls.LoadArrayAtom("char", &ch);
		pStr->push_back(ch);
	}
}
class FloatHandler : LoadSave::AtomHandler
{
public:
	FloatHandler(){m_strName="float";}
	std::string& TellName(){return m_strName;}
	int TellByteSize(){return sizeof(float);}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName)
	{
		std::stringstream stream;
		stream<<*((float*)pData);
		std::string str;
		stream>>str;
		ss.WriteAtom(TellName(), pData, strVarName, str);
	}
	void Load(LoadSave::LoadSystem& ls, void* pDest)
	{
		ls.ReadAtom(m_strName, pDest);
	}
private:
	std::string m_strName;
};


class LongHandler : LoadSave::AtomHandler
{
public:
	LongHandler(){m_strName="long";}
	std::string& TellName(){return m_strName;}
	int TellByteSize(){return sizeof(long);}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName){ss.WriteAtom(TellName(), pData, strVarName, Int2Str(*((int*)pData)));}
	void Load(LoadSave::LoadSystem& ls, void* pDest){ls.ReadAtom(m_strName, pDest);}
private:
	std::string m_strName;
};

const int BIT_ONE_SET=0x80000000;
#define NEGATIVE_ZERO *((float*)&BIT_ONE_SET)

class OgreVec3Handler : LoadSave::AtomHandler
{
public:
	OgreVec3Handler(){m_strName="Ogre::Vector3";}
	std::string& TellName(){return m_strName;}
	//int TellByteSize(){return 16;}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName)
	{
		Ogre::Vector3* temp = (Ogre::Vector3*)(pData);
		/*float* pRealData = new float[4];
		pRealData[0] = temp->x;
		pRealData[1] = temp->y;
		pRealData[2] = temp->z;
		pRealData[3] = 0;*/
		float pRData[4];
		pRData[0] = temp->x;
		pRData[1] = temp->y;
		pRData[2] = temp->z;
		pRData[3] = 0;
		for (int i = 0; i < 3; i++)
		{
			if (pRData[i] == 0) pRData[i] = -0.0f;
		}
		/*char cData[13];
		memcpy(&cData, &pRData, 12);
		cData[12] = 0;*/
		Ogre::String ostr=Ogre::StringConverter::toString(*temp);
		std::string str=std::string(ostr.c_str());
		ss.WriteAtomString("float", TellName(), &pRData, strVarName, str);
		//delete pRealData;
	}
	void Load(LoadSave::LoadSystem& ls, void* pDest)
	{
		ls.LoadAtomArray("float");
		Ogre::Vector3* pVec3=(Ogre::Vector3*)pDest;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		ls.LoadArrayAtom("float", &x);
		ls.LoadArrayAtom("float", &y);
		ls.LoadArrayAtom("float", &z);
		if (x == -0.0f) x = 0.0f;
		if (y == -0.0f) y = 0.0f;
		if (z == -0.0f) z = 0.0f;
		pVec3->x = x;
		pVec3->y = y;
		pVec3->z = z;
	}
private:
	std::string m_strName;
};

class OgreQuaternionHandler : LoadSave::AtomHandler
{
public:
	OgreQuaternionHandler(){m_strName="Ogre::Quaternion";}
	std::string& TellName(){return m_strName;}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName)
	{
		Ogre::Quaternion* temp = (Ogre::Quaternion*)(pData);
		float* pRealData = new float[5];
		pRealData[0] = temp->x;
		pRealData[1] = temp->y;
		pRealData[2] = temp->z;
		pRealData[3] = temp->w;
		pRealData[4] = 0;
		for (int i = 0; i < 4; i++)
		{
			if (pRealData[i] == 0) pRealData[i] = -0.0f;
		}
		Ogre::String ostr=Ogre::StringConverter::toString(*temp);
		std::string str=std::string(ostr.c_str());
		ss.WriteAtomString("float", TellName(), pRealData, strVarName, str);
		delete pRealData;
	}
	void Load(LoadSave::LoadSystem& ls, void* pDest)
	{
		ls.LoadAtomArray("float");
		Ogre::Quaternion* pVec3=(Ogre::Quaternion*)pDest;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float w = 0.0f;
		ls.LoadArrayAtom("float", &x);
		ls.LoadArrayAtom("float", &y);
		ls.LoadArrayAtom("float", &z);
		ls.LoadArrayAtom("float", &w);
		if (x == -0.0f) x = 0.0f;
		if (y == -0.0f) y = 0.0f;
		if (z == -0.0f) z = 0.0f;
		if (w == -0.0f) w = 0.0f;
		pVec3->x = x;
		pVec3->y = y;
		pVec3->z = z;
		pVec3->w = w;
	}
private:
	std::string m_strName;
};


class OgreStringHandler : LoadSave::AtomHandler
{
public:
	OgreStringHandler(){m_strName="Ogre::String";}
	std::string& TellName(){return m_strName;}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName){Ogre::String* pStr=(Ogre::String*)pData; ss.WriteAtomString("char", TellName(), (void*)pStr->c_str(), strVarName, *pStr);}
	void Load(LoadSave::LoadSystem& ls, void* pDest);
private:
	std::string m_strName;
};
void OgreStringHandler::Load(LoadSave::LoadSystem &ls, void *pDest)
{
	std::vector<int> vSize=ls.LoadAtomArray("char");
	Ogre::String* pStr=(Ogre::String*)pDest;
	pStr->clear();
	for(int i=0; i<vSize[0]; i++)
	{
		char ch=0;
		ls.LoadArrayAtom("char", &ch);
		pStr->push_back(ch);
	}
}

CREATEVECTORHANDLER(float, "float", FloatVectorHandler)

CREATEMAPHANDLER(std::string, "std::string", int, "int", StringIntMapHandler)

CREATELISTHANDLER(float, "float", FloatListHandler)

CREATELISTHANDLER(int, "int", IntListHandler)

CREATEVECTORHANDLER(int, "int", IntVectorHandler)

//CREATEMAPHANDLER(std::string, "std::string", ResidentVariables::LoadSave::SaveableScriptParam, "ResidentVariables::LoadSave::SaveableScriptParam", StringScriptParamMapHandler)

//CREATEMAPHANDLER(std::string, "std::string", ResidentManager::ResidentVariables::ScriptVar, "ResidentManager::ResidentVariables::ScriptVar", ResidentVariablesMapHandler)

CREATEVECTORHANDLER(Ogre::Vector3, "Ogre::Vector3", OgreVec3VectorHandler)


void
RegisterStandardAtoms()
{
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new BoolHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new IntHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new LongHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new CharHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new StringHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new FloatHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new FloatVectorHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new StringIntMapHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new FloatListHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new IntListHandler());

	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new NullObjectHandler());

	//Ogre
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new OgreVec3Handler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new OgreQuaternionHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new OgreStringHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new IntVectorHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new OgreVec3VectorHandler());

	//Lua
	//ResidentVariables
	//LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new StringScriptParamMapHandler);
	//LoadSave::RegisterAtom((LoadSave::AtomHandler*)new ResidentVariablesMapHandler());

}

};
