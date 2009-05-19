#include "..\LowLevelLibs\LoadSave_dll.h"
#include "..\LowLevelLibs\LoadSave_dll_intl.h"

int FileSize(std::ifstream in)
{
	in.seekg(0, std::ios::end);
	int iSize = in.tellg();
	in.seekg(0);
	return iSize;
}

void
IncreaseArrayPosition(std::vector<int>& arrPos, std::vector<int> arrSize)
{
	int iLevel=arrSize.size();
	arrPos[--iLevel]++;
	while(arrPos[iLevel]>=arrSize[iLevel] && iLevel!=0)
	{
		arrPos[iLevel]=0;
		iLevel--;
		arrPos[iLevel]++;
	}
}

CLoadManager::CLoadManager(std::string strFileName)
{
	m_pIn= new CHybridInFile(strFileName.c_str());
	m_TreeLevels=new std::stack<CLoadFileLevel>();
}

CLoadManager::CLoadManager(void* pData, int iDataSize)
{
	m_pIn= new CHybridInFile(pData, iDataSize);
	m_TreeLevels=new std::stack<CLoadFileLevel>();
}

void
CLoadManager::CloseFile()
{
	m_pIn->close();
}

void
CLoadManager::DefineAtomSizes(int aaiIDsDataLen[][2], int nAtoms)
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

bool
CLoadManager::HasChunks()
{
	if(m_pIn->eof())
		return false;
	return true;
}

void ReadHeaderINT(CHybridInFile *pIn, unsigned int* puiINT)
{
	pIn->read((char*)puiINT, sizeof(unsigned int));
}

void ReadHeaderINT(CHybridInFile *pIn, int* piINT)
{
	pIn->read((char*)piINT, sizeof(unsigned int));
}

void
CLoadManager::EnterChunk()
{
	if(m_TreeLevels->size())
		if(GetAtomSize(m_aaiAtomSizes, m_TreeLevels->top().iID))
			return;//in an atom array there are no chunks
	char cByte;
	(*m_pIn)>>cByte;
	if(cByte!=CHUNK_START)
	{
		m_pIn->seekg(-1, std::ios::cur);
		PutErrorMessage("tried to enter chunk, but is no chunk!");
		return;
	}
	//read the header...
	CLoadFileLevel level;
	level.level_desc=CHUNK;
	m_TreeLevels->push(level);//push the chunk-tag
	//but now read the real data
	ReadHeaderINT(m_pIn, &level.iID);
	ReadHeaderINT(m_pIn, &level.iTypeMod);
	bool bIsArray=level.iTypeMod&0x80000000 ? true : false;
	level.iTypeMod&=0x7fffffff;//eliminate the top bit
	ReadHeaderINT(m_pIn, &level.iRecordID);//might be -1
	if(bIsArray)
	{
		unsigned int uiDims;
		ReadHeaderINT(m_pIn, &uiDims);
		std::vector<int> arrayDims;
		std::vector<int> arrayPos;
		for(unsigned int i=0; i<uiDims; i++)
		{
			unsigned int uiDimSize;
			ReadHeaderINT(m_pIn, &uiDimSize);
			arrayDims.push_back(uiDimSize);
			arrayPos.push_back(0);
		}
		level.levelSizes=arrayDims;
		level.levelPos=arrayPos;

		if(GetAtomSize(m_aaiAtomSizes, level.iID))
			level.level_desc=ATOM_ARRAY;
		else
			level.level_desc=NO_ATOM_ARRAY;
	}
	else
	{
		if(GetAtomSize(m_aaiAtomSizes, level.iID))
			level.level_desc=ATOM;
		else
			level.level_desc=NO_ATOM;
	}
	m_TreeLevels->push(level);
}

void
CLoadManager::ExitChunk()
{
	char cByte;
	(*m_pIn)>>cByte;
	if(cByte!=CHUNK_END)
	{
		m_pIn->seekg(-1, std::ios::cur);
		PutErrorMessage("tried to exit chunk, but had not ended yet!");
		return;
	}
	switch(m_TreeLevels->top().level_desc)
	{
	case ATOM:
		break;//just do nothing
	case NO_ATOM:
		break;//just do nothing
	case ATOM_ARRAY:
		break;
	case NO_ATOM_ARRAY:
		if(m_TreeLevels->top().levelPos[0]<m_TreeLevels->top().levelSizes[0])
		{
			PutErrorMessage("tried to leave an array unfinished");
			return;
		}
		break;
	case CHUNK:
		PutErrorMessage("seems as if you exited an empty chunk");
	}
	m_TreeLevels->pop();
	if(m_TreeLevels->top().level_desc==CHUNK)
		m_TreeLevels->pop();
	else
	{
		PutErrorMessage("left a chunk with no beginning!");
		return;
	}
	//test if the layer over the just released object is an array
	if(m_TreeLevels->size())
		if(m_TreeLevels->top().level_desc==NO_ATOM_ARRAY)
		{
			//increase the array position.
			IncreaseArrayPosition(m_TreeLevels->top().levelPos, m_TreeLevels->top().levelSizes);
		}
}

void
CLoadManager::ReadChunkInfo(int *piID, int *piTypeModifier, int *piRecordID, std::vector<int> *pArrayDims, std::vector<int> *pArrayPos, int *piAtomSize)
{
	pArrayDims->clear();
	pArrayPos->clear();
	bool bIsObject=true;
	switch(m_TreeLevels->top().level_desc)
	{
	case ATOM_ARRAY:
		bIsObject=false;
	case NO_ATOM_ARRAY:
		*pArrayDims=m_TreeLevels->top().levelSizes;
		*pArrayPos=m_TreeLevels->top().levelPos;
	case ATOM:
		bIsObject=false;
	case NO_ATOM:
		*piID=m_TreeLevels->top().iID;
		*piTypeModifier=m_TreeLevels->top().iTypeMod;
		*piRecordID=m_TreeLevels->top().iRecordID;
		*piAtomSize=GetAtomSize(m_aaiAtomSizes, *piID);
		break;
	default:
		*piID=-1;
		*piTypeModifier=-1;
		*piRecordID=-1;
	}
}

void
CLoadManager::ReadAtom(void *pData)
{
	switch(m_TreeLevels->top().level_desc)
	{
	case ATOM_ARRAY:
		if(m_TreeLevels->top().levelPos[0]>=m_TreeLevels->top().levelSizes[0])
		{
			PutErrorMessage("tried to read one element too many from an atom array");
			return;
		}
		IncreaseArrayPosition(m_TreeLevels->top().levelPos, m_TreeLevels->top().levelSizes);
	case ATOM:
		m_pIn->read((char*)pData, GetAtomSize(m_aaiAtomSizes, m_TreeLevels->top().iID));
		break;
	default:
		PutErrorMessage("tried to read data but is no atom or atom array");
		return;
	}
}

void
CLoadManager::Bin2Text()
{
	int iLevel=0;
	char ch;
	while(!m_pIn->eof())
	{
		(*m_pIn)>>ch;
		if(ch==CHUNK_START)
		{
			for(int i=0; i<iLevel; i++)
				PutErrorMessage(" ");
			PutErrorMessage("<");
			iLevel++;
		}
		else
		{
			PutErrorMessage(std::string("data: ") + Int2Str(ch) + std::string("('") + std::string(&ch, 1) + std::string("') ") + std::string("is not expected here") );
			m_pIn->seekg(-1, std::ios_base::cur);
			return;
		}
		int iID;
		int iType;
		int iRecord;
		m_pIn->read((char*)&iID, sizeof(int));
		m_pIn->read((char*)&iType, sizeof(int));
		m_pIn->read((char*)&iRecord, sizeof(int));
		if(ch==CHUNK_START)
			PutErrorMessage(std::string("id=") + Int2Str(iID) + std::string("; ty=") + std::string(iType&0x80000000 ? "array" : "plain") + std::string("; rec=") + Int2Str(iRecord) + std::string(">\n"));
		if(GetAtomSize(m_aaiAtomSizes, iID) && !(iType&0x80000000))
		{
			char* pData=new char[GetAtomSize(m_aaiAtomSizes, iID)];
			m_pIn->read(pData, GetAtomSize(m_aaiAtomSizes, iID));
			for(int i=0; i<iLevel; i++)
				PutErrorMessage(" ");
			for(int i=0; i<GetAtomSize(m_aaiAtomSizes, iID); i++)
				PutErrorMessage(Int2Str(pData[i]) + std::string("('") + std::string(&(pData[i]), 1) + std::string("'); "));
			PutErrorMessage("\n");
			delete pData;
		}
		if(iType&0x80000000)
		{
			int iSize;
			int iLayers;
			m_pIn->read((char*)&iLayers, sizeof(int));
			int nAtoms=1;
			for(int i=0; i<iLayers; i++)
			{
				for(int j=0; j<iLevel; j++)
					PutErrorMessage(" ");
				m_pIn->read((char*)&iSize, sizeof(int));
				PutErrorMessage(std::string("dim") + Int2Str(i) + std::string(": ") + Int2Str(iSize) + std::string("\n"));
				nAtoms*=iSize;
			}
			if(GetAtomSize(m_aaiAtomSizes, iID))
			{
				for(int i=0; i<nAtoms; i++)
				{//code copy
					char* pData=new char[GetAtomSize(m_aaiAtomSizes, iID)];
					m_pIn->read(pData, GetAtomSize(m_aaiAtomSizes, iID));
					for(int i=0; i<iLevel; i++)
						PutErrorMessage(" ");
					for(int i=0; i<GetAtomSize(m_aaiAtomSizes, iID); i++)
						PutErrorMessage(Int2Str(pData[i]) + std::string("('") + std::string(&(pData[i]), 1) + std::string("'); "));
					PutErrorMessage("\n");
					delete pData;
				}
			}
		}
		(*m_pIn)>>ch;
		while(ch==CHUNK_END)
		{
			iLevel--;
			for(int i=0; i<iLevel; i++)
				PutErrorMessage(" ");
			PutErrorMessage(std::string("</>\n"));
			(*m_pIn)>>ch;
			if(m_pIn->eof())
				break;
		}
		m_pIn->seekg(-1, std::ios_base::cur);
	}
}
