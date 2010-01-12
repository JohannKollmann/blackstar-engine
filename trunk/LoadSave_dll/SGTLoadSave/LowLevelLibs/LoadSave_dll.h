#ifndef __LOADSAVE_INCL
#define __LOADSAVE_INCL

#include <string>
#include <fstream>
#include <stack>
#include <vector>
#include <sstream>

#include "CMemFile.h"

//define some magic numbers für the start and end of the chunks
//(they're variable length, if not atoms)
//need the char-conversion, else it outputs ascii-eqivalents for the numbers...
#define CHUNK_START ((char)0x01)
#define CHUNK_END ((char)0x02)

void SetErrorFunction(void (*pErrFn)(std::string));
void SetLogFunction(void (*pErrFn)(std::string));

void PutErrorMessage(std::string strErrMsg);
void PutLogMessage(std::string strLogMessage);

static std::string Int2Str(int i)
{
	std::stringstream stream;
	stream<<i;
	std::string str;
	stream>>str;
	return str;
}

class CSaveFileLevel;
class CLoadFileLevel;

class CSaveManager
{
public:
	CSaveManager(std::string strFileName, std::string strXMLFileName);
	CSaveManager(void** ppData, int* piDataSize, std::string strXMLFileName);
	~CSaveManager();
	void DefineAtomSizes(int aaiIDsDataLen[][2], int nAtoms);//for defined atoms, you can later set datalen to 0
	//the main function of this module: adds the element to the streams
	//it actually writes data only if the element was an atom.
	//else it will just write management overhead to encapsule the atoms
	//when adding arrays, only further calls will actually fill the array with data.
	//the first call just sets everything up (this does NOT apply to zero-terminated atom arrays)
	void AddOpenElement(int iID, std::string strLiteralType,//id may never be negative!
						std::string strVarName,
						int iTypeModifier,
						void *pData, std::string strLiteralData,
						bool bIsArray,//the xml writer will make as many 2D-arrays as possible
						std::vector<int>* pDimensionSizes,
						bool bZeroTermAtomArray);//intended for strings. otherwise a string would be displayed as an array in xml
	void CloseCurrentElement();
	bool OpenedLastElement();//return whether the current array has ended (will sadly also return true if the first element has not yet been written)
	int GetRecordID();
	void CloseFile();
private:
	CHybridOutFile *m_pBinaryOut,
				   *m_pXMLOut;
	int** m_aaiAtomSizes;
	std::stack<CSaveFileLevel>* m_TreeLevels;
	bool m_bUsesXML;
	int m_iRecordID;
	bool bWritingAtomArray;
	int m_nAtoms;
};

class CLoadManager
{
public:
	CLoadManager(std::string strFileName);
	CLoadManager(void* pData, int iDataSize);
	void DefineAtomSizes(int aaiIDsDataLen[][2], int nAtoms);//for defined atoms, you can later set datalen to 0
	void ReadChunkInfo(int* piID, int* piTypeModifier, int* piRecordID, std::vector<int>* pArrayDims, std::vector<int>* pArrayPos, int* piAtomSize);
	bool EnterChunk();
	bool HasChunks();
	bool ExitChunk();
	void ReadAtom(void* pData);

	void Bin2Text();//atom sizes have to be initialized
	
	void CloseFile();
private:
	CHybridInFile *m_pIn;
	int** m_aaiAtomSizes;
	std::stack<CLoadFileLevel>* m_TreeLevels;
	int m_nAtoms;
};

#endif //__LOADSAVE_INCL