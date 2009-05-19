#include "..\LowLevelLibs\LoadSave_dll.h"
#include "..\LowLevelLibs\LoadSave_dll_intl.h"
#include "..\LowLevelLibs\save_helpers.h"
#include <memory.h>

void (*g_pErrFn)(std::string)=NULL;
void (*g_pLogFn)(std::string)=NULL;

void SetErrorFunction(void (*pErrFn)(std::string))
{
	g_pErrFn=pErrFn;
}

void PutErrorMessage(std::string strErrMsg)
{
#if _DEBUG
	__debugbreak();
#endif
	if(g_pErrFn!=NULL)
		g_pErrFn(strErrMsg);
}

void SetLogFunction(void (*pLogFn)(std::string))
{
	g_pLogFn=pLogFn;
}

void
PutLogMessage(std::string strLogMessage)
{
	if(g_pLogFn!=NULL)
		g_pLogFn(strLogMessage);
}

CSaveManager::CSaveManager(void **ppData, int *piDataSize, std::string strXMLFileName)
{
	m_pBinaryOut=new CHybridOutFile(ppData, piDataSize);
	m_pXMLOut=new CHybridOutFile(strXMLFileName.c_str());
	(*m_pXMLOut)<<std::string("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n") +
				  std::string("<?xml-stylesheet type=\"text/xsl\" href=\"treetransform.xsl\"?>\n") +
				  std::string("<save>") +
				  std::string("<save_name>") +
				  std::string("memsave") +
				  std::string("</save_name>");
	m_TreeLevels=new std::stack<CSaveFileLevel>();
	m_iRecordID=0;
}

CSaveManager::CSaveManager(std::string strFileName, std::string strXMLFileName)
{//copy of the other constructor
	m_pBinaryOut=new CHybridOutFile(strFileName.c_str());
	m_pXMLOut=new CHybridOutFile(strXMLFileName.c_str());
	(*m_pXMLOut)<<std::string("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n") +
				  std::string("<?xml-stylesheet type=\"text/xsl\" href=\"treetransform.xsl\"?>\n") +
				  std::string("<save>") +
				  std::string("<save_name>") +
				  (strFileName.find_last_of('\\')!=-1 ? strFileName.substr(strFileName.find_last_of('\\')) : strFileName) +
				  std::string("</save_name>");

	m_bUsesXML=false;
	m_TreeLevels=new std::stack<CSaveFileLevel>();
	m_iRecordID=0;
}

void CSaveManager::CloseFile()
{
	m_pBinaryOut->close();
	(*m_pXMLOut)<<"</save>";
	m_pXMLOut->close();
}

void CSaveManager::DefineAtomSizes(int aaiIDsDataLen[][2], int nAtoms)
{
	m_nAtoms=nAtoms;
	m_aaiAtomSizes=new int*[nAtoms+1];
	for(int i=0; i<m_nAtoms; i++)
	{
		m_aaiAtomSizes[i]=new int[2];
		m_aaiAtomSizes[i][0]=aaiIDsDataLen[i][0];
		m_aaiAtomSizes[i][1]=aaiIDsDataLen[i][1];
	}
	m_aaiAtomSizes[nAtoms]=NULL;
}


void CSaveManager::AddOpenElement(int iID, std::string strLiteralType,//id may never be negative!
								  std::string strVarName,
								  int iTypeModifier,
								  void *pData, std::string strLiteralData,
								  bool bIsArray,//the xml writer will make as many 2D-arrays as possible
								  std::vector<int>* pDimensionSizes,
								  bool bZeroTermAtomArray)//intended for strings. otherwise a string would be displayed as an array in xml
{
	CSaveFileLevel currLevel;
	currLevel.level_desc=ATOM;
	if(!m_TreeLevels->empty())
		currLevel=m_TreeLevels->top();

	if(currLevel.level_desc==ATOM_ARRAY)
	{
		if(m_TreeLevels->top().bArrayEntryClosed)
			WriteAtomArrayEntry(m_TreeLevels, m_pBinaryOut, m_pXMLOut, &m_iRecordID, GetAtomSize(m_aaiAtomSizes, iID), (char*)pData, strLiteralData);
		return;
	}

	if(currLevel.level_desc==NO_ATOM_ARRAY && !bZeroTermAtomArray)
	{
		if(m_TreeLevels->top().bArrayEntryClosed)
			WriteObjectArrayEntry(m_TreeLevels, m_pBinaryOut, m_pXMLOut, &m_iRecordID, iID, iTypeModifier);
		else
		{
			if(GetAtomSize(m_aaiAtomSizes, iID))//hack for the higher level: record refs have a size (they're atoms)
			{
				//now here's where you can see all the things that did not fit into concept.
				//this binch of XML sets up the wrapping object
				(*m_pXMLOut)<<std::string("<var><type>object</type><object><id>") + Int2Str(iID) + std::string("</id>") +
							  std::string("<name>") + strLiteralType + std::string("</name>") +
							  std::string("<record>") + Int2Str(m_iRecordID) + std::string("</record>");
				//now proceed with our atom (it can contain anything, even though inside this hack
				WriteAtom(m_TreeLevels, m_pBinaryOut, m_pXMLOut, iID, iTypeModifier, GetAtomSize(m_aaiAtomSizes, iID), (char*)pData, strLiteralData, strLiteralType, strVarName);
				//and now close the wrapper.. but as you can see we do not close the <object> but the <var>
				//that is because the closing tag for our data will be written by the next call to CloseCurrentElement
				//and so we first have to cloase our atom, then the object, and the close for the <var> surrounding the
				//object is the close intended for the inner <var>
				(*m_pXMLOut)<<std::string("</var></object>");
			}
			else
				WriteObject(m_TreeLevels, m_pBinaryOut, m_pXMLOut, &m_iRecordID, iID, iTypeModifier, strLiteralType, strVarName);
		}
		m_iRecordID++;
		return;
	}

	if(bIsArray && bZeroTermAtomArray)
	{
		int iLength=0, iAtomSize=GetAtomSize(m_aaiAtomSizes, iID);
		char* aDummy=new char[iAtomSize];
		memset(aDummy, 0, iAtomSize);
		while(memcmp(((char*)pData)+(iLength*iAtomSize), aDummy, iAtomSize))
			iLength++;
		WriteAtomString(m_TreeLevels, m_pBinaryOut, m_pXMLOut, iID, iTypeModifier, strLiteralType, strVarName, iAtomSize, iLength, (char*)pData, strLiteralData);
		return;
	}

	if(bIsArray)
		WriteArray(m_TreeLevels, m_pBinaryOut, m_pXMLOut, &m_iRecordID, iID, GetAtomSize(m_aaiAtomSizes, iID) ? true : false, iTypeModifier, strLiteralType, strVarName, *pDimensionSizes);
	else
	{
		if(GetAtomSize(m_aaiAtomSizes, iID))
			WriteAtom(m_TreeLevels, m_pBinaryOut, m_pXMLOut, iID, iTypeModifier, GetAtomSize(m_aaiAtomSizes, iID), (char*)pData, strLiteralData, strLiteralType, strVarName);
		else
		{
			WriteObject(m_TreeLevels, m_pBinaryOut, m_pXMLOut, &m_iRecordID, iID, iTypeModifier, strLiteralType, strVarName);
			m_iRecordID++;
		}
	}
	
}

int CSaveManager::GetRecordID(){return m_iRecordID;}

void CSaveManager::CloseCurrentElement()
{
	WriteElementClose(m_TreeLevels, m_pBinaryOut, m_pXMLOut);
}

bool CSaveManager::OpenedLastElement()
{//tests the same thing as the elementclose() function
	if(m_TreeLevels->top().iArrayLevel==0 && m_TreeLevels->top().iCurrLevelPos==0)
		return true;//this condition is true if the next array entry will be out of bounds (i.e. this one was the last)
	return false;
}

CSaveManager::~CSaveManager()
{
	delete m_pBinaryOut;
	delete m_pXMLOut;
	for(int i=0; i<m_nAtoms; i++)
		delete m_aaiAtomSizes[i];
	delete m_aaiAtomSizes;
}