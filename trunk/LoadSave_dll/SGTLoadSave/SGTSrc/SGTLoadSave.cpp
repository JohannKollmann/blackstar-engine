#include "..\SGTLibs\SGTLoadSave.h"
#include "../LoadSave_Hash.h"

struct SAtom
{
public:
	SGTAtomHandler* pHandler;
	int iByteSize;
	int iID;
};

struct SSaveable
{
public:
	SGTSaveableInstanceFn pFn;
	int iID;
};

std::map<std::string, SSaveable> g_Objects;
std::map<std::string, SAtom> g_Atoms;
std::map<int, std::string> g_Names;
static bool g_bWasInitialized=false;

int g_iCurrID=0;

void (*g_LogFn)(std::string)=NULL;

void
SGTLoadSave::RegisterAtom(SGTAtomHandler *pHandler)
{
	SAtom atomDesc;
	atomDesc.iID=fnFNV((void*)pHandler->TellName().c_str(), pHandler->TellName().length());//++g_iCurrID;
	atomDesc.pHandler=pHandler;
	atomDesc.iByteSize=pHandler->TellByteSize();
	g_Atoms.insert(std::pair<std::string, SAtom>(pHandler->TellName(), atomDesc));
	g_Names.insert(std::pair<int, std::string>(atomDesc.iID, pHandler->TellName()));
}

void
SGTLoadSave::RegisterObject(SGTSaveableRegisterFn registerFn)
{
	std::string strName;
	SGTSaveableInstanceFn instFn;
	registerFn(&strName, &instFn);
	SSaveable objectDesc;
	objectDesc.iID=fnFNV((void*)strName.c_str(), strName.length());//++g_iCurrID;
	objectDesc.pFn=instFn;
	g_Objects.insert(std::pair<std::string, SSaveable>(strName, objectDesc));
	g_Names.insert(std::pair<int, std::string>(objectDesc.iID, strName));
}

int
SGTLoadSave::GetAtomID(std::string strType)
{
	std::map<std::string, SAtom>::const_iterator findIt;
	findIt=g_Atoms.find(strType);
	if(findIt == g_Atoms.end())
		return 0;
	return findIt->second.iID;
}

int
SGTLoadSave::GetObjectID(std::string strType)
{
	std::map<std::string, SSaveable>::const_iterator findIt;
	findIt=g_Objects.find(strType);
	if(findIt == g_Objects.end())
		return 0;
	return findIt->second.iID;
}

SGTAtomHandler*
SGTLoadSave::GetAtomHandler(std::string strAtomName)
{
	std::map<std::string, SAtom>::const_iterator it=g_Atoms.find(strAtomName);
	return it==g_Atoms.end() ? NULL : it->second.pHandler;
}

void ErrorFn(std::string str){SGTLoadSave::Instance().PostError(str);}

SGTLoadSave& SGTLoadSave::Instance()
{
	static SGTLoadSave theOneAndOnly;
	if(!g_bWasInitialized)
	{
		SetErrorFunction(&ErrorFn);
		RegisterAtom((SGTAtomHandler *)new SGTRecordReference());
	}
	g_bWasInitialized=true;
	return theOneAndOnly;
}

SGTSaveSystem*
SGTLoadSave::CreateSaveFile(std::string strBinaryFile, std::string strXMLFile)
{
	SGTSaveSystem* pSS=new SGTSaveSystem(strBinaryFile, strXMLFile);
	return pSS;
}

SGTLoadSystem*
SGTLoadSave::LoadFile(std::string strBinaryFile)
{
	SGTLoadSystem* pLS=new SGTLoadSystem(strBinaryFile);
	return pLS;
}

SGTSaveSystem*
SGTLoadSave::CreateSaveFile(void *&pData, int &iDataSize, std::string strXMLFile)
{
	SGTSaveSystem* pSS=new SGTSaveSystem(pData, iDataSize, strXMLFile);
	return pSS;
}

SGTLoadSystem*
SGTLoadSave::LoadFile(void* pData, int iDataSize)
{
	SGTLoadSystem* pLS=new SGTLoadSystem(pData, iDataSize);
	return pLS;
}

std::map<int, int>
SGTLoadSave::GetAtomSizes()
{
	std::map<int, int> out;
	std::map<std::string, SAtom>::const_iterator it;
	it=g_Atoms.begin();

	do
	{
		if(it->second.iByteSize!=0)//only if it is writeable. the others are just empty shells
			out.insert(std::pair<int, int>(it->second.iID, it->second.iByteSize));
	}
	while(++it!=g_Atoms.end());
	return out;
}

SGTSaveableInstanceFn
SGTLoadSave::GetInstanceFunction(std::string strObjectName)
{
	return g_Objects.find(strObjectName)->second.pFn;
}

std::string
SGTLoadSave::GetTypeName(int iID)
{
	return g_Names.find(iID)->second;
}

void
SGTLoadSave::SetLogFunction(void (*pFn)(std::string))
{
	g_LogFn=pFn;
}

void
SGTLoadSave::PostLogEntry(std::string strLogEntry)
{
	if(g_LogFn!=NULL)
		g_LogFn(strLogEntry);
}

void
SGTLoadSave::PostError(std::string strError)
{
	PostLogEntry(strError);
	exit(-1);
}