#include "..\SGTLibs\SGTLoadSave.h"
#include "..\SGTLibs\SGTSaveSystem.h"

void DoInit(CSaveManager* pSM)
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
	pSM->DefineAtomSizes(aaiAtomSizes, atomMap.size());
	delete aaiAtomSizes;
}

SGTSaveSystem::SGTSaveSystem(std::string strBinaryFile, std::string strXMLFile)
{
	m_pSM=new CSaveManager(strBinaryFile, strXMLFile);
	DoInit(m_pSM);
}

SGTSaveSystem::SGTSaveSystem(void* & pData, int& iDataSize, std::string strXMLFile)
{
	m_pSM=new CSaveManager(&pData, &iDataSize, strXMLFile);
	DoInit(m_pSM);
}

void
SGTSaveSystem::CloseFiles()
{
	m_pSM->CloseFile();
	delete m_pSM;
}

void StringReplace(std::string &strSource, std::string strToReplace, std::string strReplacement)
{
	int iPos=0;
	while((iPos = strSource.find(strToReplace, iPos)) != std::string::npos)
	{
		strSource.replace(iPos, strToReplace.length(), strReplacement);
		iPos += strReplacement.length();
	}
}

std::string
EscapeXMLName(std::string strOut)
{
	std::string strEscapedOut=strOut;
	StringReplace(strEscapedOut, "<", "&lt;");
	StringReplace(strEscapedOut, ">", "&gt;");
	return strEscapedOut;
}


void
SGTSaveSystem::SaveAtom(std::string strType, void *pData, std::string varName)
{
	//put the atom in an object if it has no size specified
	if(SGTLoadSave::Instance().GetAtomHandler(strType)->TellByteSize()==0)
		m_pSM->AddOpenElement(SGTLoadSave::Instance().GetAtomID(strType), EscapeXMLName(strType), varName, 0, NULL, "", false, NULL, false);
	SGTLoadSave::Instance().GetAtomHandler(strType)->Save(*this, pData, varName);
	if(SGTLoadSave::Instance().GetAtomHandler(strType)->TellByteSize()==0)
		m_pSM->CloseCurrentElement();
}

void
SGTSaveSystem::WriteAtom(std::string strType, void* pData, std::string varName, std::string literalData)
{
	m_pSM->AddOpenElement(SGTLoadSave::Instance().GetAtomID(strType), EscapeXMLName(strType), varName, 0, pData, literalData, false, NULL, false);
	m_pSM->CloseCurrentElement();
}

void
SGTSaveSystem::SaveObject(SGTSaveable *pObj, std::string varName)
{
	if(m_RecordIDs.find(pObj)==m_RecordIDs.end())
	{
		m_RecordIDs.insert(std::pair<SGTSaveable*, int>(pObj, m_pSM->GetRecordID()));
		m_pSM->AddOpenElement(SGTLoadSave::Instance().GetObjectID(pObj->TellName()), EscapeXMLName(pObj->TellName()), varName, 0, NULL, "", false, NULL, false);
		pObj->Save(*this);
		m_pSM->CloseCurrentElement();
	}
	else
	{
		std::map<SGTSaveable*, int>::const_iterator it=m_RecordIDs.find(pObj);
		this->SaveAtom("SGTRecordReference", (void*)&it->second, "");
	}
}

void
SGTSaveSystem::WriteAtomToArray(std::string strType, void *pData, std::string literalData)
{
	m_pSM->AddOpenElement(SGTLoadSave::Instance().GetAtomID(strType), EscapeXMLName(strType), "", 0, pData, literalData, false, NULL, false);
	m_pSM->CloseCurrentElement();
	if(m_pSM->OpenedLastElement())
		m_pSM->CloseCurrentElement();//close the array itself
}

void
SGTSaveSystem::WriteAtomString(std::string strDataType, std::string strArrayType, void *pData, std::string varName, std::string literalData)
{
	m_pSM->AddOpenElement(SGTLoadSave::Instance().GetAtomID(strDataType), EscapeXMLName(strArrayType), varName, 0, pData, literalData, true, NULL, true);
	m_pSM->CloseCurrentElement();
}

void
SGTSaveSystem::OpenObjectArray(std::string strType, std::vector<int> dimSizes, std::string varName)
{
	m_pSM->AddOpenElement(SGTLoadSave::Instance().GetObjectID(strType), EscapeXMLName(strType), varName, 0, NULL, "", true, &dimSizes, false);
	if(dimSizes[0]==0)
		m_pSM->CloseCurrentElement();
}

void
SGTSaveSystem::AddObject(SGTSaveable* pObj)
{
	m_pSM->AddOpenElement(SGTLoadSave::Instance().GetObjectID(pObj->TellName()), EscapeXMLName(pObj->TellName()), "", 0, NULL, "", false, NULL, false);
	SaveObject(pObj, "");
	m_pSM->CloseCurrentElement();
	if(m_pSM->OpenedLastElement())
		m_pSM->CloseCurrentElement();//close the array itself
}

void
SGTSaveSystem::OpenAtomArray(std::string strType, std::vector<int> dimSizes, std::string varName)
{
	m_pSM->AddOpenElement(SGTLoadSave::Instance().GetAtomID(strType), EscapeXMLName(strType), varName, 0, NULL, "", true, &dimSizes, false);
	if(dimSizes[0]==0)
		m_pSM->CloseCurrentElement();
}

void
SGTSaveSystem::AddAtom(std::string strType, void *pData)
{
	SaveAtom(strType, pData, "");
	if(m_pSM->OpenedLastElement())
		m_pSM->CloseCurrentElement();//close the array itself
}