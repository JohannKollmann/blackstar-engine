#include "..\SGTLibs\SGTLoadSave.h"
#include "..\SGTLibs\SGTLoadSystem.h"

void
DoInit(CLoadManager *pLM)
{
	std::map<int, int> atomMap=SGTLoadSave::Instance().GetAtomSizes();//first: id; second: bytesize
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

SGTLoadSystem::SGTLoadSystem(std::string strFile)
{
	m_pLM=new CLoadManager(strFile);
	DoInit(m_pLM);
//	m_pLM->Bin2Text();
}

SGTLoadSystem::SGTLoadSystem(void* pData, int iDataSize)
{
	m_pLM=new CLoadManager(pData, iDataSize);
	DoInit(m_pLM);
	//	m_pLM->Bin2Text();
}


void
SGTLoadSystem::CloseFile()
{
	m_pLM->CloseFile();
}

bool
SGTLoadSystem::HasObjects()
{
	return m_pLM->HasChunks();
}

void
SGTLoadSystem::ReadAtom(std::string strType, void *pAtom)
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

	if(SGTLoadSave::Instance().GetAtomID(strType)!=iID)
		SGTLoadSave::Instance().PostError("atoms do not match!");
	m_pLM->ReadAtom(pAtom);
	
	if(arrDims.size()==0)
		m_pLM->ExitChunk();
}

void
SGTLoadSystem::LoadAtom(std::string strType, void *pAtom)
{
	if(SGTLoadSave::Instance().GetAtomHandler(strType)->TellByteSize()==0)
		m_pLM->EnterChunk();//jump over the dummy object surrounding the atom
	SGTAtomHandler* pHandler=SGTLoadSave::Instance().GetAtomHandler(strType);
	pHandler->Load(*this, pAtom);
	if(SGTLoadSave::Instance().GetAtomHandler(strType)->TellByteSize()==0)
		m_pLM->ExitChunk();//and jump out
}

SGTSaveable*
SGTLoadSystem::LoadObject()
{

	m_pLM->EnterChunk();
	int iID;
	int iTypeModifier;
	int iRecordID;
	std::vector<int> arrDims;
	std::vector<int> arrPos;
	int iAtomSize;

	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrDims.size()!=0 || arrPos.size()!=0)
		SGTLoadSave::Instance().PostError("tried to load an array as flat type!");
	//test if a reference was saved instead of a real object
	if(iID == SGTLoadSave::Instance().GetAtomID("SGTRecordReference"))
	{
		int iRefRecordID;
		m_pLM->ReadAtom(&iRefRecordID);// a bit hacky but didn't work another way, cuz we already entered the atom
		m_pLM->ExitChunk();
		std::map<int, SGTSaveable*>::const_iterator it=m_RecordIDs.find(iRefRecordID);
		return it->second;
	}
	std::string strType=SGTLoadSave::Instance().GetTypeName(iID);
	SGTSaveable* pObj=SGTLoadSave::Instance().GetInstanceFunction(strType)();

	m_RecordIDs.insert(std::pair<int, SGTSaveable*>(iRecordID, pObj));
	pObj->Load(*this);
	
	m_pLM->ExitChunk();

	return pObj;
}

void
SGTLoadSystem::LoadArrayAtom(std::string strType, void* pAtom)
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
SGTLoadSystem::LoadAtomArray(std::string strType)
{
	m_pLM->EnterChunk();
	int iID;
	int iTypeModifier;
	int iRecordID;
	std::vector<int> arrDims;
	std::vector<int> arrPos;
	int iAtomSize;

	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrDims.size()==0 || arrPos.size()==0)
		SGTLoadSave::Instance().PostError("tried to load a flat type as an array!");
	if(iID!=SGTLoadSave::Instance().GetAtomID(strType))
		SGTLoadSave::Instance().PostError("tried to load with wrong array type!");
	if(arrDims[0]==0)//if the array is empty
		m_pLM->ExitChunk();
	return arrDims;
}

SGTSaveable*
SGTLoadSystem::LoadArrayObject()
{
	int iID;
	int iTypeModifier;
	int iRecordID;
	std::vector<int> arrDims;
	std::vector<int> arrPos;
	int iAtomSize;
	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	SGTSaveable* pObj;
	if(arrPos[0]<arrDims[0])
		pObj=LoadObject();
	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrPos[0]==arrDims[0])//if the array ended
		m_pLM->ExitChunk();
	return pObj;
}

std::vector<int>
SGTLoadSystem::LoadObjectArray(std::string* pstrType)
{
	m_pLM->EnterChunk();
	int iID;
	int iTypeModifier;
	int iRecordID;
	std::vector<int> arrDims;
	std::vector<int> arrPos;
	int iAtomSize;

	m_pLM->ReadChunkInfo(&iID, &iTypeModifier, &iRecordID, &arrDims, &arrPos, &iAtomSize);
	if(arrDims.size()==0 || arrPos.size()==0)
		SGTLoadSave::Instance().PostError("tried to load a flat type as an array!");
	
	if(pstrType!=NULL)
		*pstrType=SGTLoadSave::Instance().GetTypeName(iID);
	if(arrDims[0]==0)//if the array is empty
		m_pLM->ExitChunk();

	return arrDims;
}