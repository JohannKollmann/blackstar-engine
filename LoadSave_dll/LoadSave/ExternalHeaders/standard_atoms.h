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
	if(LoadSave::LoadSave::Instance().GetObjectID( strTemplateTypeName )!=0)\
	{/*it is an object*/\
		/*open an object-array*/\
		ss.OpenObjectArray( strTemplateTypeName , dims, "_array");\
		for(unsigned int i=0; i<pVector->size(); i++)\
			ss.AddObject((LoadSave::Saveable*)&((*pVector)[i]));\
	}\
	else\
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
	if(LoadSave::LoadSave::Instance().GetObjectID( strTemplateTypeName )!=0)\
	{/*it is an object*/\
		std::string str;\
		dims=ls.LoadObjectArray(&str);\
		if(str!=strTemplateTypeName)\
			return;\
	}\
	else\
	{/*it must be an atom*/\
		dims=ls.LoadAtomArray(strTemplateTypeName);\
	}\
	for(int i=0; i<dims[0]; i++)\
	{\
		if(LoadSave::LoadSave::Instance().GetObjectID( strTemplateTypeName )!=0)\
		{/*it is an object*/\
			pVector->push_back(*((template_type*)ls.LoadArrayObject()));\
		}\
		else\
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
	if(LoadSave::LoadSave::Instance().GetObjectID( strTemplateTypeName )!=0) \
		ss.OpenObjectArray( strTemplateTypeName , dims, "_list");\
	else \
		ss.OpenAtomArray( strTemplateTypeName , dims, "_list");\
\
	do\
	{\
		if(LoadSave::LoadSave::Instance().GetObjectID( strTemplateTypeName )!=0) \
			ss.AddObject((LoadSave::Saveable*)&(*it));\
		else \
			ss.AddAtom( strTemplateTypeName , (void*)&(*it));\
	}while(++it!=pList->end());\
}\
\
void \
strHandlerName ::Load(LoadSave::LoadSystem& ls, void* pDest)\
{\
	std::vector<int> dims;\
	std::list< template_type >* pList=(std::list< template_type >*)pDest;\
	if(LoadSave::LoadSave::Instance().GetObjectID( strTemplateTypeName )!=0)\
	{/*it is an object*/\
		std::string str;\
		dims=ls.LoadObjectArray(&str);\
		if(str!=strTemplateTypeName)\
			return;\
	}\
	else\
	{/*it must be an atom*/\
		dims=ls.LoadAtomArray(strTemplateTypeName);\
	}\
	for(int i=0; i<dims[0]; i++)\
	{\
		if(LoadSave::LoadSave::Instance().GetObjectID( strTemplateTypeName )!=0)\
		{/*it is an object*/\
			pList->insert(pList->end(), *((template_type*)ls.LoadArrayObject()));\
		}\
		else\
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
	if(LoadSave::LoadSave::Instance().GetObjectID( strKeyTypeName )!=0) \
		ss.OpenObjectArray( strKeyTypeName , dims, "_key");\
	else \
		ss.OpenAtomArray( strKeyTypeName , dims, "_key");\
	do\
	{\
		if(LoadSave::LoadSave::Instance().GetObjectID( strKeyTypeName )!=0) \
			ss.AddObject((LoadSave::Saveable*)&(it->first));\
		else \
			ss.AddAtom( strKeyTypeName , (void*)&(it->first));\
\
	}while(++it!=pMap->end());\
\
	it=pMap->begin();\
\
	if(LoadSave::LoadSave::Instance().GetObjectID( strValueTypeName )!=0) \
		ss.OpenObjectArray( strValueTypeName , dims, "_value");\
	else \
		ss.OpenAtomArray( strValueTypeName , dims, "_value");\
	do\
	{\
		if(LoadSave::LoadSave::Instance().GetObjectID( strValueTypeName )!=0) \
			ss.AddObject((LoadSave::Saveable*)&(it->second));\
		else \
			ss.AddAtom( strValueTypeName , (void*)&(it->second));\
	}while(++it!=pMap->end());\
} \
void \
strHandlerName ::Load(LoadSave::LoadSystem& ls, void* pData)\
{\
	std::vector<int> dims;\
	std::map< key_type, value_type >* pMap=(std::map< key_type, value_type >*)pData;\
\
	std::list<key_type> keylist;\
	if(LoadSave::LoadSave::Instance().GetObjectID( strKeyTypeName )!=0)\
	{/*it is an object*/\
		std::string str;\
		dims=ls.LoadObjectArray(&str);\
		if(str!=strKeyTypeName)\
			return;\
	}\
	else\
	{/*it must be an atom*/\
		dims=ls.LoadAtomArray(strKeyTypeName);\
	}\
	for(int i=0; i<dims[0]; i++)\
	{\
		if(LoadSave::LoadSave::Instance().GetObjectID( strKeyTypeName )!=0)\
		{/*it is an object*/\
			keylist.insert(keylist.end(), *((key_type*)ls.LoadArrayObject()));\
		}\
		else\
		{/*it must be an atom*/\
			key_type atom;\
			ls.LoadArrayAtom(strKeyTypeName, &atom);\
			keylist.insert(keylist.end(), atom);\
		}\
	}\
\
	if(LoadSave::LoadSave::Instance().GetObjectID( strValueTypeName )!=0)\
	{/*it is an object*/\
		std::string str;\
		dims=ls.LoadObjectArray(&str);\
		if(str!=strValueTypeName)\
			return;\
	}\
	else\
	{/*it must be an atom*/\
		dims=ls.LoadAtomArray(strValueTypeName);\
	}\
	std::list<key_type>::const_iterator it=keylist.begin();\
	for(; it!=keylist.end(); it++)\
	{\
		if(LoadSave::LoadSave::Instance().GetObjectID( strValueTypeName )!=0)\
		{/*it is an object*/\
		pMap->insert(std::pair<key_type, value_type>(*it, *((value_type*)ls.LoadArrayObject())));\
		}\
		else\
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

#ifdef _MSC_VER

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

#endif

class SaveableListHandler : LoadSave::AtomHandler
{
public:
	SaveableListHandler (){m_strName= std::string("std::list<Saveable*>");}
	std::string& TellName(){return m_strName;}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName);
	void Load(LoadSave::LoadSystem& ls, void* pDest);
private:
	std::string m_strName;
};

void SaveableListHandler::Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName)
{
	/*test what category the data is*/
	std::vector<int> dims;
	std::list< LoadSave::Saveable* >* pList=(std::list< LoadSave::Saveable* >*)pData;
	dims.push_back(pList->size());

	ss.OpenObjectArray("Saveable" , dims, "_list");

	for (std::list< LoadSave::Saveable* >::const_iterator it=pList->begin(); it != pList->end(); it++)
	{
		ss.AddObject((LoadSave::Saveable*)(*it));
	}
}

void SaveableListHandler::Load(LoadSave::LoadSystem& ls, void* pDest)
{
	std::vector<int> dims;
	std::list< LoadSave::Saveable* >* pList=(std::list< LoadSave::Saveable* >*)pDest;
	std::string str;
	dims=ls.LoadObjectArray(&str);
	if(str!="Saveable")
		return;
	for(int i=0; i<dims[0]; i++)
	{
		pList->push_back(ls.LoadArrayObject());//LoadObject());
		//pList->push_back(*((LoadSave::Saveable**)ls.LoadArrayObject()));
		//pList->insert(pList->end(), *((LoadSave::Saveable**)ls.LoadArrayObject()));
	}
}

class SaveableVectorHandler : LoadSave::AtomHandler
{
public:
	SaveableVectorHandler (){m_strName= std::string("std::vector<Saveable*>");}
	std::string& TellName(){return m_strName;}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName);
	void Load(LoadSave::LoadSystem& ls, void* pDest);
private:
	std::string m_strName;
};

void SaveableVectorHandler::Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName)
{
	/*test what category the data is*/
	std::vector<int> dims;
	std::vector< LoadSave::Saveable* >* pVector=(std::vector< LoadSave::Saveable* >*)pData;
	dims.push_back(pVector->size());

	ss.OpenObjectArray("Saveable" , dims, "_vector");

	for (std::vector< LoadSave::Saveable* >::const_iterator it=pVector->begin(); it != pVector->end(); it++)
	{
		ss.AddObject((LoadSave::Saveable*)(*it));
	}
}

void SaveableVectorHandler::Load(LoadSave::LoadSystem& ls, void* pDest)
{
	std::vector<int> dims;
	std::vector< LoadSave::Saveable* >* pVector=(std::vector< LoadSave::Saveable* >*)pDest;
	std::string str;
	dims=ls.LoadObjectArray(&str);
	if(str!="Saveable")
		return;
	for(int i=0; i<dims[0]; i++)
	{
		pVector->push_back(ls.LoadArrayObject());
	}
}


CREATEVECTORHANDLER(float, "float", FloatVectorHandler)

CREATEMAPHANDLER(std::string, "std::string", int, "int", StringIntMapHandler)

CREATELISTHANDLER(float, "float", FloatListHandler);

CREATEVECTORHANDLER(int, "int", IntVectorHandler)

//CREATEMAPHANDLER(std::string, "std::string", ResidentVariables::LoadSave::SaveableScriptParam, "ResidentVariables::LoadSave::SaveableScriptParam", StringScriptParamMapHandler)

//CREATEMAPHANDLER(std::string, "std::string", ResidentManager::ResidentVariables::ScriptVar, "ResidentManager::ResidentVariables::ScriptVar", ResidentVariablesMapHandler)

#ifdef _MSC_VER

CREATEMAPHANDLER(Ogre::String, "Ogre::String", GenericProperty, "GenericProperty", GenericPropertyMapHandler)

CREATEVECTORHANDLER(DataMap::Item, "DataMapItem", DataMapItemVectorHandler)

CREATEVECTORHANDLER(Ogre::Vector3, "Ogre::Vector3", OgreVec3VectorHandler)

#endif

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
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new SaveableListHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new SaveableVectorHandler());

	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new NullObjectHandler());

#ifdef _MSC_VER

	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new GenericPropertyMapHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new DataMapItemVectorHandler());
	//Ogre
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new OgreVec3Handler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new OgreQuaternionHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new OgreStringHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new IntVectorHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new OgreVec3VectorHandler());
#endif
	//Lua
	//ResidentVariables
	//LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new StringScriptParamMapHandler);
	//LoadSave::RegisterAtom((LoadSave::AtomHandler*)new ResidentVariablesMapHandler());

}

};
