#ifndef __STANDARD_ATOMS_INCL
#define __STANDARD_ATOMS_INCL
#include "..\LoadSave\ExternalHeaders\loadsave.h"

#include <list>
#include <sstream>

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

class DoubleHandler : LoadSave::AtomHandler
{
public:
	DoubleHandler(){m_strName="double";}
	std::string& TellName(){return m_strName;}
	int TellByteSize(){return sizeof(double);}
	void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName)
	{
		std::stringstream stream;
		stream<<*((double*)pData);
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

CREATEVECTORHANDLER(float, "float", FloatVectorHandler)

CREATEMAPHANDLER(std::string, "std::string", int, "int", StringIntMapHandler)

CREATELISTHANDLER(float, "float", FloatListHandler);

void
RegisterStandardAtoms()
{
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new IntHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new LongHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new CharHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new StringHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new FloatHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new DoubleHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new FloatVectorHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new StringIntMapHandler());
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new FloatListHandler());
}

#endif