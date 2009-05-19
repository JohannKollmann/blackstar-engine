#include <list>
#include <fstream>
#include <string>

enum EMemFileDir
{
	POS_BEGIN,
	POS_END,
	POS_CURR
};

const int BUFSIZE=4096;

class CMemFile
{
public:
	void Write(void* pData, int nBytes);
	void Read(void* pData, int nBytes);
	void Seek(int nBytes, EMemFileDir dir);
	void OpenForWrite();
	int GetDataSize();
	void CloseForWrite(void* pData);
	void OpenForRead(void* pData, int nBytes);
	void CloseForRead();
	int GetPos();

	template <class T> void Read(T* pData){Write(pData, sizeof(T));}
	template <class T> void Write(T* pData){Read(pData, sizeof(T));}
private:
	std::list<void*> m_Data;
	void* m_pData;
	int m_nCurrBytes;
	int m_iCurrPos;
};

const std::string MEMFILENAME=std::string("MEMFILE");

class CHybridOutFile
{
public:
	CHybridOutFile(std::string strFileName);
	CHybridOutFile(void** ppData, int* piSize);//reseves the memory..
	void operator <<(char c);
	void operator <<(std::string str);
	void write(char* pData, int iSize);
	void close();
private:
	bool m_bUsesMem;
	bool m_bValid;
	std::ofstream* m_pOutFile;
	CMemFile* m_pOutMem;
	void** m_ppData;
	int* m_piSize;
};

class CHybridInFile
{
public:
	CHybridInFile(std::string strFileName);
	CHybridInFile(void* pData, int iMemSize);//reseves the memory..
	void operator >>(char& c);
	void read(char* pData, int iSize);
	void seekg(int nBytes, std::ios::seekdir dir);
	void close();
	bool eof();
private:
	bool m_bUsesMem;
	bool m_bValid;
	std::ifstream* m_pInFile;
	CMemFile* m_pInMem;
};