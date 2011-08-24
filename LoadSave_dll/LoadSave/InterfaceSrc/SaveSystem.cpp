#include "../InterfaceLibs/LoadSave.h"
#include "../InterfaceLibs/SaveSystem.h"

namespace LoadSave
{

void DoInit(CSaveManager* pSM)
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
	pSM->DefineAtomSizes(aaiAtomSizes, atomMap.size());
	delete[] aaiAtomSizes;
}

SaveSystem::SaveSystem(std::string strBinaryFile, std::string strXMLFile)
{
	m_pSM=new CSaveManager(strBinaryFile, strXMLFile);
	mUseRecordReferences = true;
	DoInit(m_pSM);
}

SaveSystem::SaveSystem(void* & pData, int& iDataSize, std::string strXMLFile)
{
	m_pSM=new CSaveManager(&pData, &iDataSize, strXMLFile);
	mUseRecordReferences = true;
	DoInit(m_pSM);
}

void
SaveSystem::CloseFiles()
{
	m_pSM->CloseFile();
	delete m_pSM;
}

void StringReplace(std::string &strSource, std::string strToReplace, std::string strReplacement)
{
	int iPos=0;
	while((iPos = strSource.find(strToReplace, iPos)) != (int)std::string::npos)
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
SaveSystem::SaveAtom(std::string strType, void *pData, std::string varName)
{
	//put the atom in an object if it has no size specified
	if(LoadSave::Instance().GetAtomHandler(strType)->TellByteSize()==0)
		m_pSM->AddOpenElement(LoadSave::Instance().GetAtomID(strType), EscapeXMLName(strType), varName, 0, NULL, "", false, NULL, false);
	LoadSave::Instance().GetAtomHandler(strType)->Save(*this, pData, varName);
	if(LoadSave::Instance().GetAtomHandler(strType)->TellByteSize()==0)
		m_pSM->CloseCurrentElement();
}

void
SaveSystem::WriteAtom(std::string strType, void* pData, std::string varName, std::string literalData)
{
	m_pSM->AddOpenElement(LoadSave::Instance().GetAtomID(strType), EscapeXMLName(strType), varName, 0, pData, literalData, false, NULL, false);
	m_pSM->CloseCurrentElement();
}

void SaveSystem::SetUseRecordReferences(bool use)
{
	mUseRecordReferences = use;
}

void
SaveSystem::SaveObject(Saveable *pObj, std::string varName, bool allowNull, bool useRecordReferences)
{
	if (!pObj)
	{
		if (!allowNull) LoadSave::Instance().PostError("SaveSystem::SaveObject " + varName + " is null!");

		this->SaveAtom("NullObject", nullptr, varName);
		return;
	}
	if(!useRecordReferences || !mUseRecordReferences || m_RecordIDs.find(pObj)==m_RecordIDs.end())
	{
		if (useRecordReferences) m_RecordIDs.insert(std::pair<Saveable*, int>(pObj, m_pSM->GetRecordID()));
		m_pSM->AddOpenElement(LoadSave::Instance().GetObjectID(pObj->TellName()), EscapeXMLName(pObj->TellName()), varName, 0, NULL, "", false, NULL, false);
		pObj->Save(*this);
		m_pSM->CloseCurrentElement();
	}
	else
	{
		std::map<Saveable*, int>::const_iterator it=m_RecordIDs.find(pObj);
		this->SaveAtom("RecordReference", (void*)&it->second, "");
	}
}

void
SaveSystem::WriteAtomToArray(std::string strType, void *pData, std::string literalData)
{
	m_pSM->AddOpenElement(LoadSave::Instance().GetAtomID(strType), EscapeXMLName(strType), "", 0, pData, literalData, false, NULL, false);
	m_pSM->CloseCurrentElement();
	if(m_pSM->OpenedLastElement())
		m_pSM->CloseCurrentElement();//close the array itself
}

void
SaveSystem::WriteAtomString(std::string strDataType, std::string strArrayType, void *pData, std::string varName, std::string literalData)
{
	m_pSM->AddOpenElement(LoadSave::Instance().GetAtomID(strDataType), EscapeXMLName(strArrayType), varName, 0, pData, literalData, true, NULL, true);
	m_pSM->CloseCurrentElement();
}

void
SaveSystem::OpenObjectArray(std::string strType, std::vector<int> dimSizes, std::string varName)
{
	m_pSM->AddOpenElement(LoadSave::Instance().GetObjectID(strType), EscapeXMLName(strType), varName, 0, NULL, "", true, &dimSizes, false);
	if(dimSizes[0]==0)
		m_pSM->CloseCurrentElement();
}

void
SaveSystem::AddObject(Saveable* pObj)
{
	m_pSM->AddOpenElement(LoadSave::Instance().GetObjectID(pObj->TellName()), EscapeXMLName(pObj->TellName()), "", 0, NULL, "", false, NULL, false);
	SaveObject(pObj, "");
	m_pSM->CloseCurrentElement();
	if(m_pSM->OpenedLastElement())
		m_pSM->CloseCurrentElement();//close the array itself
}

void
SaveSystem::OpenAtomArray(std::string strType, std::vector<int> dimSizes, std::string varName)
{
	m_pSM->AddOpenElement(LoadSave::Instance().GetAtomID(strType), EscapeXMLName(strType), varName, 0, NULL, "", true, &dimSizes, false);
	if(dimSizes[0]==0)
		m_pSM->CloseCurrentElement();
}

void
SaveSystem::AddAtom(std::string strType, void *pData)
{
	SaveAtom(strType, pData, "");
	if(m_pSM->OpenedLastElement())
		m_pSM->CloseCurrentElement();//close the array itself
}

};
