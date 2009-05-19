#include "..\LowLevelLibs\CMemFile.h"

void
CMemFile::OpenForWrite()
{
	m_Data.push_back(new char[BUFSIZE]);
	m_nCurrBytes=0;
	m_iCurrPos=0;
	m_pData=NULL;
}

int
CMemFile::GetDataSize()
{
	return m_nCurrBytes;
}

void
CMemFile::CloseForWrite(void *pData)
{
	int i=0;
	while(m_Data.size()>1)
	{
		memcpy(((char*)pData) + i*BUFSIZE, *(m_Data.begin()), BUFSIZE);
		delete *(m_Data.begin());
		m_Data.pop_front();
		i++;
	}
	memcpy(((char*)pData) + i*BUFSIZE, *(m_Data.begin()), m_nCurrBytes%BUFSIZE);
	delete *(m_Data.begin());
	m_Data.pop_front();
}

void
CMemFile::OpenForRead(void *pData, int nBytes)
{
/*	for(int i=0; i<nBytes/BUFSIZE; i++)
	{
		m_Data.push_back(((char*)pData) + i*BUFSIZE);
	}*/
	m_nCurrBytes=nBytes;
	m_iCurrPos=0;
	m_pData=pData;
}

void
CMemFile::CloseForRead()
{/*
	while(m_Data.size()>0)
	{
		m_Data.pop_front();
	}*/

}

void
CMemFile::Seek(int nBytes, EMemFileDir dir)
{
	if(m_pData==NULL)
		return;
	switch(dir)
	{
	case POS_CURR:
		m_iCurrPos+=nBytes;
		break;
	case POS_BEGIN:
		m_iCurrPos=nBytes;
		break;
	case POS_END:
		m_iCurrPos=GetDataSize()-nBytes;
		break;
	}
	if(m_iCurrPos<0)
		m_iCurrPos=0;
	if(m_iCurrPos>=m_nCurrBytes)
		m_iCurrPos=m_nCurrBytes-1;
}

void
CMemFile::Write(void *pData, int nBytes)
{
	int iBytesRemaining=nBytes;
	while((BUFSIZE - (m_iCurrPos%BUFSIZE)) <= iBytesRemaining)
	{
		memcpy(((char*)m_Data.back()) + (m_iCurrPos%BUFSIZE), pData, BUFSIZE - (m_iCurrPos%BUFSIZE));
		m_Data.push_back(new char[BUFSIZE]);
		iBytesRemaining-=BUFSIZE - (m_iCurrPos%BUFSIZE);
		pData=((char*)pData) + BUFSIZE - (m_iCurrPos%BUFSIZE);
		m_iCurrPos=((m_iCurrPos/BUFSIZE)+1)*BUFSIZE;
	}
	memcpy(((char*)m_Data.back()) + (m_iCurrPos%BUFSIZE), pData, iBytesRemaining);
	m_iCurrPos+=iBytesRemaining;
	if(m_iCurrPos>m_nCurrBytes)
		m_nCurrBytes=m_iCurrPos;
}

void
CMemFile::Read(void* pData, int nBytes)
{
	if(m_iCurrPos>=m_nCurrBytes)
		return;
	int iActualBytes=nBytes;
	if(m_iCurrPos+nBytes>=m_nCurrBytes)
		iActualBytes=m_nCurrBytes-m_iCurrPos;
	memcpy(pData, ((char*)m_pData) + m_iCurrPos ,iActualBytes);
	m_iCurrPos += nBytes;
}

int
CMemFile::GetPos()
{
	return m_iCurrPos;
}

//------------------------Hybrid Files------------------------

CHybridOutFile::CHybridOutFile(std::string strFileName)
{
	if(strFileName.size()==0)
		m_bValid=false;
	else
	{
		m_bUsesMem=false;
		m_bValid=true;
		m_pOutFile=new std::ofstream(strFileName.c_str(), std::ios_base::binary);
	}
}

CHybridOutFile::CHybridOutFile(void** ppData, int* piSize)
{
	m_bValid=true;
	m_bUsesMem=true;
	m_pOutMem=new CMemFile();
	m_pOutMem->OpenForWrite();
	m_ppData=ppData;
	m_piSize=piSize;
}

void
CHybridOutFile::close()
{
	if(!m_bValid)
		return;
	if(m_bUsesMem)
	{
		char* pData=new char[m_pOutMem->GetDataSize()];
		*m_piSize=m_pOutMem->GetDataSize();
		m_pOutMem->CloseForWrite(pData);
		*m_ppData=pData;
		delete m_pOutMem;
	}
	else
	{
		m_pOutFile->close();
		delete m_pOutFile;
	}
}

void
CHybridOutFile::write(char *pData, int iSize)
{
	if(!m_bValid)
		return;
	if(m_bUsesMem)
		m_pOutMem->Write(pData, iSize);
	else
		m_pOutFile->write(pData, iSize);
}

void
CHybridOutFile::operator <<(char c)
{
	write(&c, sizeof(char));
}

void
CHybridOutFile::operator <<(std::string str)
{
	write((char*)str.c_str(), str.length()*sizeof(char));
}

CHybridInFile::CHybridInFile(std::string strFileName)
{
	if(strFileName.size()==0)
		m_bValid=false;
	else
	{
		m_bUsesMem=false;
		m_bValid=true;
		m_pInFile=new std::ifstream(strFileName.c_str(), std::ios_base::binary);
	}
}

CHybridInFile::CHybridInFile(void *pData, int iMemSize)
{
	m_bValid=true;
	m_bUsesMem=true;
	m_pInMem=new CMemFile();
	m_pInMem->OpenForRead(pData, iMemSize);
}

void
CHybridInFile::close()
{
	if(!m_bValid)
		return;
	if(m_bUsesMem)
	{
		m_pInMem->CloseForRead();
		delete m_pInMem;
	}
	else
	{
		m_pInFile->close();
		delete m_pInFile;
	}
}

bool
CHybridInFile::eof()
{
	if(!m_bValid)
		return true;
	if(m_bUsesMem)
		return m_pInMem->GetPos()>=(m_pInMem->GetDataSize()-1);
	else
		return m_pInFile->eof();
}

void
CHybridInFile::read(char *pData, int iSize)
{
	if(!m_bValid)
		return;
	if(m_bUsesMem)
		m_pInMem->Read(pData, iSize);
	else
		m_pInFile->read(pData, iSize);
}

void
CHybridInFile::operator >>(char& c)
{
	read(&c, sizeof(char));
}

void
CHybridInFile::seekg(int nBytes, std::ios::seekdir dir)
{
	if(!m_bValid)
		return;
	if(m_bUsesMem)
	{
		switch(dir)
		{
		case std::ios_base::cur:
			m_pInMem->Seek(nBytes, POS_CURR);
			break;
		case std::ios_base::beg:
			m_pInMem->Seek(nBytes, POS_BEGIN);
			break;
		case std::ios_base::end:
			m_pInMem->Seek(nBytes, POS_END);
			break;
		}
	}
	else
		m_pInFile->seekg(nBytes, dir);
}