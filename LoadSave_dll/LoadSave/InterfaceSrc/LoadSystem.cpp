#include "..\InterfaceLibs\LoadSave.h"
#include "..\InterfaceLibs\LoadSystem.h"

namespace LoadSave
{

void
DoInit(CLoadManager *pLM)
{
	std::map<int, int> atomMap=LoadSave::Instance().GetAtomSizes();//first: id; second: bytesize
	int (*aaiAtomSizes)[2]=new int[atomMap.size()][2];
	std::map<int, int>::const_iterator it;
	it=atomMap.begin();

	unsigned int iAtom=0;
	do
	{
		aaiAtomSizes[iAtom][0]=it->first;
		aaiAtomSizes[iAtom][1]=it->second;
		++it;
	}
	while(++iAtom<atomMap.size());
	pLM->DefineAtomSizes(aaiAtomSizes, atomMap.size());
}

LoadSystem::LoadSystem(std::string strFile)
{
	m_pLM=new CLoadManager(strFile);
	DoInit(m_pLM);
//	m_pLM->Bin2Text();
}

LoadSystem::LoadSystem(void* pData, int iDataSize)
{
	m_pLM=new CLoadManager(pData, iDataSize);
	DoInit(m_pLM);
	//	m_pLM->Bin2Text();
}


void
LoadSystem::CloseFile()
{
	m_pLM->CloseFile();
}

bool
LoadSystem::HasObjects()
{
	return m_pLM->HasChunks();
}

void
LoadSystem::ReadAtom(std::string strType, void *pAtom)
{
	int iID;
	int iTypeModifier;
	int iRecordID;
	std::vector<int> arrDims;
	std::vector<int> arrPos;
	int iAtomSize;

	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrDims.size()==0)
		m_pLM->EnterChunk();

	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);

	if(LoadSave::Instance().GetAtomID(strType)!=iID)
	{
		LoadSave::Instance().PostError("atoms do not match!");
		return;
	}
	m_pLM->ReadAtom(pAtom);
	
	if(arrDims.size()==0)
		m_pLM->ExitChunk();
}

void
LoadSystem::LoadAtom(std::string strType, void *pAtom)
{
	if(LoadSave::Instance().GetAtomHandler(strType)->TellByteSize()==0)
		if(!m_pLM->EnterChunk())
			return;//jump over the dummy object surrounding the atom
	AtomHandler* pHandler=LoadSave::Instance().GetAtomHandler(strType);
	pHandler->Load(*this, pAtom);
	if(LoadSave::Instance().GetAtomHandler(strType)->TellByteSize()==0)
		m_pLM->ExitChunk();//and jump out
}

Saveable*
LoadSystem::LoadObject()
{

	if(!m_pLM->EnterChunk())
		return (Saveable*)0;
	int iID;
	int iTypeModifier;
	int iRecordID;
	std::vector<int> arrDims;
	std::vector<int> arrPos;
	int iAtomSize;

	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrDims.size()!=0 || arrPos.size()!=0)
	{
		LoadSave::Instance().PostError("tried to load an array as flat type!");
		m_pLM->ExitChunk();
	}
	//test if a reference was saved instead of a real object
	if(iID == LoadSave::Instance().GetAtomID("RecordReference"))
	{
		int iRefRecordID;
		m_pLM->ReadAtom(&iRefRecordID);// a bit hacky but didn't work another way, cuz we already entered the atom
		m_pLM->ExitChunk();
		std::map<int, Saveable*>::const_iterator it=m_RecordIDs.find(iRefRecordID);
		return it->second;
	}
	std::string strType=LoadSave::Instance().GetTypeName(iID);
	Saveable* pObj=LoadSave::Instance().GetInstanceFunction(strType)();

	m_RecordIDs.insert(std::pair<int, Saveable*>(iRecordID, pObj));
	pObj->Load(*this);
	
	m_pLM->ExitChunk();

	return pObj;
}

void
LoadSystem::LoadArrayAtom(std::string strType, void* pAtom)
{
	int iID;
	int iTypeModifier;
	int iRecordID;
	std::vector<int> arrDims;
	std::vector<int> arrPos;
	int iAtomSize;
	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrPos[0]<arrDims[0])
		LoadAtom(strType, pAtom);
	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrPos.size())//in certain cases LoadAtom closes the array. so we don't have to lcose it here
		if(arrPos[0]==arrDims[0])//if the array ended
			m_pLM->ExitChunk();
}

std::vector<int>
LoadSystem::LoadAtomArray(std::string strType)
{
	if(!m_pLM->EnterChunk())
		return std::vector<int>();
	int iID;
	int iTypeModifier;
	int iRecordID;
	std::vector<int> arrDims;
	std::vector<int> arrPos;
	int iAtomSize;

	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrDims.size()==0 || arrPos.size()==0)
	{
		LoadSave::Instance().PostError("tried to load a flat type as an array!");
		m_pLM->ExitChunk();
		return std::vector<int>();
	}
	if(iID!=LoadSave::Instance().GetAtomID(strType))
	{
		LoadSave::Instance().PostError("tried to load with wrong array type!");
		m_pLM->ExitChunk();
		return std::vector<int>();
	}
	if(arrDims[0]==0)//if the array is empty
		m_pLM->ExitChunk();
	return arrDims;
}

Saveable*
LoadSystem::LoadArrayObject()
{
	int iID;
	int iTypeModifier;
	int iRecordID;
	std::vector<int> arrDims;
	std::vector<int> arrPos;
	int iAtomSize;
	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	Saveable* pObj;
	if(arrPos[0]<arrDims[0])
		pObj=LoadObject();
	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrPos[0]==arrDims[0])//if the array ended
		m_pLM->ExitChunk();
	return pObj;
}

std::vector<int>
LoadSystem::LoadObjectArray(std::string* pstrType)
{
	if(!m_pLM->EnterChunk())
		return std::vector<int>();
	int iID;
	int iTypeModifier;
	int iRecordID;
	std::vector<int> arrDims;
	std::vector<int> arrPos;
	int iAtomSize;

	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrDims.size()==0 || arrPos.size()==0)
	{
		LoadSave::Instance().PostError("tried to load a flat type as an array!");
		m_pLM->ExitChunk();
		return std::vector<int>();
	}
	
	if(pstrType!=NULL)
		*pstrType=LoadSave::Instance().GetTypeName(iID);
	if(arrDims[0]==0)//if the array is empty
		m_pLM->ExitChunk();

	return arrDims;
}

};