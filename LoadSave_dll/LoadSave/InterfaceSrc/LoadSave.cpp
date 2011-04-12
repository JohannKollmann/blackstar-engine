#include "../InterfaceLibs/LoadSave.h"
#include "../LoadSave_Hash.h"

namespace LoadSave
{

struct SAtom
{
public:
	AtomHandler* pHandler;
	int iByteSize;
	int iID;
};

struct SSaveable
{
public:
	SaveableInstanceFn pFn;
	int iID;
};

std::map<std::string, SSaveable> g_Objects;
std::map<std::string, SAtom> g_Atoms;
std::map<int, std::string> g_Names;
static bool g_bWasInitialized=false;

int g_iCurrID=0;

void (*g_LogFn)(std::string)=NULL;

LoadSystem::~LoadSystem()
{
	m_RecordIDs.clear();
}

void
LoadSave::RegisterAtom(AtomHandler *pHandler)
{
	SAtom atomDesc;
	atomDesc.iID=fnFNV((void*)pHandler->TellName().c_str(), pHandler->TellName().length());//++g_iCurrID;
	atomDesc.pHandler=pHandler;
	atomDesc.iByteSize=pHandler->TellByteSize();
	g_Atoms.insert(std::pair<std::string, SAtom>(pHandler->TellName(), atomDesc));
	g_Names.insert(std::pair<int, std::string>(atomDesc.iID, pHandler->TellName()));
}

void
LoadSave::RegisterObject(SaveableRegisterFn registerFn)
{
	std::string strName;
	SaveableInstanceFn instFn;
	registerFn(&strName, &instFn);
	SSaveable objectDesc;
	objectDesc.iID=fnFNV((void*)strName.c_str(), strName.length());//++g_iCurrID;
	objectDesc.pFn=instFn;
	g_Objects.insert(std::pair<std::string, SSaveable>(strName, objectDesc));
	g_Names.insert(std::pair<int, std::string>(objectDesc.iID, strName));
}

int
LoadSave::GetAtomID(std::string strType)
{
	std::map<std::string, SAtom>::const_iterator findIt;
	findIt=g_Atoms.find(strType);
	if(findIt == g_Atoms.end())
		return 0;
	return findIt->second.iID;
}

int
LoadSave::GetObjectID(std::string strType)
{
	std::map<std::string, SSaveable>::const_iterator findIt;
	findIt=g_Objects.find(strType);
	if(findIt == g_Objects.end())
		return 0;
	return findIt->second.iID;
}

AtomHandler*
LoadSave::GetAtomHandler(std::string strAtomName)
{
	std::map<std::string, SAtom>::const_iterator it=g_Atoms.find(strAtomName);
	return it==g_Atoms.end() ? NULL : it->second.pHandler;
}

void ErrorFn(std::string str){LoadSave::Instance().PostError(str);}

LoadSave& LoadSave::Instance()
{
	static LoadSave theOneAndOnly;
	if(!g_bWasInitialized)
	{
		SetErrorFunction(&ErrorFn);
		RegisterAtom((AtomHandler *)new RecordReference());
	}
	g_bWasInitialized=true;
	return theOneAndOnly;
}

SaveSystem*
LoadSave::CreateSaveFile(std::string strBinaryFile, std::string strXMLFile)
{
	SaveSystem* pSS=new SaveSystem(strBinaryFile, strXMLFile);
	return pSS;
}

LoadSystem*
LoadSave::LoadFile(std::string strBinaryFile)
{
	LoadSystem* pLS=new LoadSystem(strBinaryFile);
	return pLS;
}

SaveSystem*
LoadSave::CreateSaveFile(void *&pData, int &iDataSize, std::string strXMLFile)
{
	SaveSystem* pSS=new SaveSystem(pData, iDataSize, strXMLFile);
	return pSS;
}

LoadSystem*
LoadSave::LoadFile(void* pData, int iDataSize)
{
	LoadSystem* pLS=new LoadSystem(pData, iDataSize);
	return pLS;
}

std::map<int, int>
LoadSave::GetAtomSizes()
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

SaveableInstanceFn
LoadSave::GetInstanceFunction(std::string strObjectName)
{
	return g_Objects.find(strObjectName)->second.pFn;
}

std::string
LoadSave::GetTypeName(int iID)
{
	return g_Names.find(iID)->second;
}

void
LoadSave::SetLogFunction(void (*pFn)(std::string))
{
	g_LogFn=pFn;
}

void
LoadSave::PostLogEntry(std::string strLogEntry)
{
	if(g_LogFn!=NULL)
		g_LogFn(strLogEntry);
}

void
LoadSave::PostError(std::string strError)
{
	PostLogEntry(std::string("[Error] LoadSave: ") + strError);
	//exit(-1);
}

};
