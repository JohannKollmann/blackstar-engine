#include "SGTLoadSave.h"
#include <list>

#define CREATEMAPHANDLER(key_type, strKeyTypeName, value_type, strValueTypeName, strHandlerName) \
class strHandlerName : SGTAtomHandler\
{\
public:\
	strHandlerName (){m_strName= std::string("std::map<") + strKeyTypeName + std::string(", ") + strValueTypeName + std::string(">");}\
	std::string& TellName(){return m_strName;}\
	void Save(SGTSaveSystem& ss, void* pData, std::string strVarName);\
	void Load(SGTLoadSystem& ls, void* pDest);\
private:\
	std::string m_strName;\
};\
\
void \
strHandlerName ::Save(SGTSaveSystem& ss, void* pData, std::string strVarName)\
{\
	/*test what category the data is*/\
	std::vector<int> dims;\
	std::map< key_type, value_type >* pMap=(std::map< key_type, value_type >*)pData;\
	dims.push_back(pMap->size());\
\
	std::map< key_type, value_type >::const_iterator it=pMap->begin();\
\
	if(SGTLoadSave::Instance().GetObjectID( strKeyTypeName )!=0) \
		ss.OpenObjectArray( strKeyTypeName , dims, "_key");\
	else \
		ss.OpenAtomArray( strKeyTypeName , dims, "_key");\
	do\
	{\
		if(SGTLoadSave::Instance().GetObjectID( strKeyTypeName )!=0) \
			ss.AddObject((SGTSaveable*)&(it->first));\
		else \
			ss.AddAtom( strKeyTypeName , (void*)&(it->first));\
\
	}while(++it!=pMap->end());\
\
	it=pMap->begin();\
\
	if(SGTLoadSave::Instance().GetObjectID( strValueTypeName )!=0) \
		ss.OpenObjectArray( strValueTypeName , dims, "_value");\
	else \
		ss.OpenAtomArray( strValueTypeName , dims, "_value");\
	do\
	{\
		if(SGTLoadSave::Instance().GetObjectID( strValueTypeName )!=0) \
			ss.AddObject((SGTSaveable*)&(it->second));\
		else \
			ss.AddAtom( strValueTypeName , (void*)&(it->second));\
	}while(++it!=pMap->end());\
} \
void \
strHandlerName ::Load(SGTLoadSystem& ls, void* pData)\
{\
	std::vector<int> dims;\
	std::map< key_type, value_type >* pMap=(std::map< key_type, value_type >*)pData;\
\
	std::list<key_type> keylist;\
	if(SGTLoadSave::Instance().GetObjectID( strKeyTypeName )!=0)\
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
		if(SGTLoadSave::Instance().GetObjectID( strKeyTypeName )!=0)\
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
	if(SGTLoadSave::Instance().GetObjectID( strValueTypeName )!=0)\
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
		if(SGTLoadSave::Instance().GetObjectID( strValueTypeName )!=0)\
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

