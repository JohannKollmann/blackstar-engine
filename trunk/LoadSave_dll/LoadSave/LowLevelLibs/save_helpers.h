#include "LoadSave_dll.h"

namespace LoadSave
{

void WriteHeaderINT(CHybridOutFile *pOut, unsigned int uiINT)
{
	pOut->write((char*)&uiINT, sizeof(unsigned int));
}

void WriteHeader(CHybridOutFile *pOut, unsigned int uiID, unsigned int uiModifier, unsigned int uiRecordID)
{
	WriteHeaderINT(pOut, uiID);
	WriteHeaderINT(pOut, uiModifier);
	WriteHeaderINT(pOut, uiRecordID);
}
void WriteArrayDimensions(CHybridOutFile *pOut, std::vector<int> dims)
{
	WriteHeaderINT(pOut, dims.size());
	for(unsigned int i=0; i<dims.size(); i++)
		WriteHeaderINT(pOut, dims[i]);
}

static void WriteAtomArrayEntry(std::stack<CSaveFileLevel>* levels, CHybridOutFile* pBinaryOut, CHybridOutFile* pXMLOut,
								int* piRecordID, int iDataSize, char* pData, std::string strLiteralData)
{
	//go to the least level of the array
	CSaveFileLevel currLevel=levels->top();
	if(currLevel.iArrayLevel!=currLevel.levelSizes.size()-1)
		for(unsigned int iLevel=currLevel.iArrayLevel+1; iLevel<currLevel.levelSizes.size(); iLevel++)
		{
			CSaveFileLevel newLevel=currLevel;
			newLevel.iArrayLevel=iLevel;
			newLevel.iCurrLevelPos=0;
			levels->push(newLevel);
			if(iLevel%2==0)
			{//create a new 2D-array for every sublevel in a 2D-array
				(*pXMLOut)<<"<var>";

				(*pXMLOut)<<"<type_modifier>array</type_modifier>";
				std::string strType=currLevel.strArrayType;
				for(unsigned int i=iLevel; i<(currLevel.levelSizes[currLevel.levelSizes.size()-1]==1 ? currLevel.levelSizes.size()-1 : currLevel.levelSizes.size())-1; i++)
				{
					strType+="*";
				}
				(*pXMLOut)<<std::string("<type>") + strType + std::string("</type>");
				(*pXMLOut)<<std::string("<record>") + Int2Str((*piRecordID)++) + std::string("</record>");
				//this code generates the horizontal indices
				if(currLevel.levelSizes[iLevel+1]>4)
				{
					(*pXMLOut)<<"<arr_column_indices>";
					for(int i=0; i<currLevel.levelSizes[iLevel+1]; i++)
					{
						(*pXMLOut)<<std::string("<value>") + Int2Str(i) + std::string("</value>");
					}
					(*pXMLOut)<<"</arr_column_indices>";
				}
			}
			else
			{
				(*pXMLOut)<<"<arr_line>";
				//this generates the vertical indices
				if(currLevel.levelSizes[iLevel-1]>4)
				{
					//look at what position the higher level is right now
					CSaveFileLevel myLevel=levels->top();
					levels->pop();
					(*pXMLOut)<<std::string("<index>") + Int2Str(levels->top().iCurrLevelPos) + std::string("</index>");
					levels->push(myLevel);
				}
			}
		}
	currLevel=levels->top();

	//this is the part where the action takes place

	(*pXMLOut)<<std::string("<value>") + strLiteralData + std::string("</value>");
	pBinaryOut->write(pData, iDataSize);

	//increment the least level
	currLevel=levels->top();
	levels->pop();
	currLevel.iCurrLevelPos++;
	currLevel.bArrayEntryClosed=true;//data has been written
	levels->push(currLevel);

	//increase the counter of the level on top the current if boundary was exceeded
	bool bArrayEnd=false;
	while(levels->top().levelSizes[levels->top().iArrayLevel]<=levels->top().iCurrLevelPos)
	{//test if the levels have reached their maximum
		if(levels->top().iArrayLevel==0)
		{
			bArrayEnd=true;
			break;
		}
		if(levels->top().iArrayLevel%2==0)
		{//create a new 2D-array for every sublevel in a 2D-array
			(*pXMLOut)<<"</var>";
		}
		else
		{
			(*pXMLOut)<<"</arr_line>";
		}
		levels->pop();
		
		currLevel=levels->top();
		levels->pop();
		currLevel.iCurrLevelPos++;
		levels->push(currLevel);
	}
}

static void WriteObjectArrayEntry(std::stack<CSaveFileLevel>* levels, CHybridOutFile* pBinaryOut, CHybridOutFile* pXMLOut,
								  int* piRecordID, int iID, int iTypeModifier)
{//the xml part is just a copy of the routine for atoms, except the write is missing
	//go to the least level of the array
	CSaveFileLevel currLevel=levels->top();
	if(currLevel.iArrayLevel!=currLevel.levelSizes.size()-1)
		for(unsigned int iLevel=currLevel.iArrayLevel+1; iLevel<currLevel.levelSizes.size(); iLevel++)
		{
			CSaveFileLevel newLevel=currLevel;
			newLevel.iArrayLevel=iLevel;
			newLevel.iCurrLevelPos=0;
			levels->push(newLevel);
			if(iLevel%2==0)
			{//create a new 2D-array for every sublevel in a 2D-array
				(*pXMLOut)<<"<var>";

				(*pXMLOut)<<"<type_modifier>array</type_modifier>";
				std::string strType=currLevel.strArrayType;
				for(unsigned int i=iLevel; i<(currLevel.levelSizes[currLevel.levelSizes.size()-1]==1 ? currLevel.levelSizes.size()-1 : currLevel.levelSizes.size())-1; i++)
				{
					strType+="*";
				}
				(*pXMLOut)<<std::string("<type>") + strType + std::string("</type>");
				(*pXMLOut)<<std::string("<record>") + Int2Str((*piRecordID)++) + std::string("</record>");
				//this code generates the horizontal indices
				if(currLevel.levelSizes[iLevel+1]>4)
				{
					(*pXMLOut)<<"<arr_column_indices>";
					for(int i=0; i<currLevel.levelSizes[iLevel+1]; i++)
					{
						(*pXMLOut)<<std::string("<value>") + Int2Str(i) + std::string("</value>");
					}
					(*pXMLOut)<<"</arr_column_indices>";
				}
			}
			else
			{
				(*pXMLOut)<<"<arr_line>";
				//this generates the vertical indices
				if(currLevel.levelSizes[iLevel-1]>4)
				{
					//look at what position the higher level is right now
					CSaveFileLevel myLevel=levels->top();
					levels->pop();
					(*pXMLOut)<<std::string("<index>") + Int2Str(levels->top().iCurrLevelPos) + std::string("</index>");
					levels->push(myLevel);
				}
			}
		}
	currLevel=levels->top();

	levels->pop();
	currLevel.iCurrLevelPos++;
	currLevel.bArrayEntryClosed=false;

	levels->push(currLevel);
}

static void WriteObject(std::stack<CSaveFileLevel>* levels, CHybridOutFile* pBinaryOut, CHybridOutFile* pXMLOut,
						int* piRecordID, int iID, int iTypeModifier, std::string strLiteralType, std::string strVarName)
{
	(*pBinaryOut)<<CHUNK_START;
	WriteHeader(pBinaryOut, iID, iTypeModifier, *piRecordID);


	if(!levels->empty())
	{
		(*pXMLOut)<<"<var>";
		(*pXMLOut)<<"<type>object</type>";
		(*pXMLOut)<<std::string("<name>") + strVarName + std::string("</name>");
	}
	(*pXMLOut)<<"<object>";

	(*pXMLOut)<<std::string("<id>") + Int2Str(iID) + std::string("</id>");
	(*pXMLOut)<<std::string("<name>") + strLiteralType + std::string("</name>");
	(*pXMLOut)<<std::string("<record>") + Int2Str((*piRecordID)++) + std::string("</record>");

	CSaveFileLevel newLevel;
	newLevel.level_desc=NO_ATOM;
	levels->push(newLevel);
	
}

static void WriteAtom(std::stack<CSaveFileLevel>* levels, CHybridOutFile* pBinaryOut, CHybridOutFile* pXMLOut,
					  int iID, int iTypeModifier, int iDataSize, char* pData, std::string strLiteralData,
					  std::string strLiteralType, std::string strVarName)
{
	(*pBinaryOut)<<CHUNK_START;
	WriteHeader(pBinaryOut, iID, iTypeModifier, -1);
	pBinaryOut->write(pData, iDataSize);

	(*pXMLOut)<<"<var>";

	(*pXMLOut)<<std::string("<type>") + strLiteralType + std::string("</type>");
	(*pXMLOut)<<std::string("<name>") + strVarName + std::string("</name>");
	(*pXMLOut)<<std::string("<value>") + strLiteralData + std::string("</value>");

	CSaveFileLevel newLevel;
	newLevel.level_desc=ATOM;
	levels->push(newLevel);
}

static void WriteArray(std::stack<CSaveFileLevel>* levels, CHybridOutFile* pBinaryOut, CHybridOutFile* pXMLOut,
					   int* piRecordID, int iID, bool bAtomArray, int iTypeModifier, std::string strLiteralType, std::string strVarName,
					   std::vector<int> arrayLevels)
{
	(*pBinaryOut)<<CHUNK_START;
	WriteHeader(pBinaryOut, iID, iTypeModifier|0x80000000, -1);
	WriteArrayDimensions(pBinaryOut, arrayLevels);
	

	(*pXMLOut)<<"<var>";

	(*pXMLOut)<<"<type_modifier>array</type_modifier>";
	(*pXMLOut)<<std::string("<name>") + strVarName + std::string("</name>");
	std::string strType=strLiteralType;
	for(unsigned int i=0; i<arrayLevels.size()-1; i++)
	{
		strType+="*";
	}
	(*pXMLOut)<<std::string("<type>") + strType + std::string("</type>");
	(*pXMLOut)<<std::string("<record>") + Int2Str((*piRecordID)++) + std::string("</record>");

	CSaveFileLevel newLevel;
	newLevel.level_desc=bAtomArray ? ATOM_ARRAY : NO_ATOM_ARRAY;
	newLevel.iArrayLevel=0;
	newLevel.iCurrLevelPos=0;
	if(arrayLevels.size()%2==1)
		arrayLevels.push_back(1);
	newLevel.levelSizes=arrayLevels;
	newLevel.strArrayType=strLiteralType;
	newLevel.bArrayEntryClosed=true;

	levels->push(newLevel);
}

static void WriteAtomString(std::stack<CSaveFileLevel>* levels, CHybridOutFile* pBinaryOut, CHybridOutFile* pXMLOut,
							int iID, int iTypeModifier, std::string strLiteralType, std::string strVarName, int iDataSize,
							int nAtoms, char* pData, std::string strLiteralData)
{
	(*pBinaryOut)<<CHUNK_START;
	WriteHeader(pBinaryOut, iID, iTypeModifier|0x80000000, -1);
	WriteHeaderINT(pBinaryOut, 1);
	WriteHeaderINT(pBinaryOut, nAtoms);
	pBinaryOut->write(pData, iDataSize*nAtoms);

	(*pXMLOut)<<"<var>";

	(*pXMLOut)<<std::string("<type>") + strLiteralType + std::string("</type>");
	(*pXMLOut)<<std::string("<name>") + strVarName + std::string("</name>");
	(*pXMLOut)<<std::string("<value>") + strLiteralData + std::string("</value>");

	CSaveFileLevel newLevel;
	newLevel.level_desc=ATOM;//hack for XML
	levels->push(newLevel);
}

static void WriteElementClose(std::stack<CSaveFileLevel>* levels, CHybridOutFile* pBinaryOut, CHybridOutFile* pXMLOut)
{
	switch(levels->top().level_desc)
	{
	case NO_ATOM:
		(*pXMLOut)<<"</object>";
		if(levels->size()>1)
			(*pXMLOut)<<"</var>";
		(*pBinaryOut)<<CHUNK_END;
		break;
	case ATOM:
		(*pXMLOut)<<"</var>";
		(*pBinaryOut)<<CHUNK_END;
		break;
	case NO_ATOM_ARRAY:
	{
		//increase the counter of the level on top the current if boundary was exceeded
		if(levels->top().iCurrLevelPos!=0)
		{
			while(levels->top().levelSizes[levels->top().iArrayLevel]<=levels->top().iCurrLevelPos)
			{//test if the levels have reached their maximum
				if(levels->top().iArrayLevel==0)
				{
					break;
				}
				if(levels->top().iArrayLevel%2==0)
				{//create a new 2D-array for every sublevel in a 2D-array
					(*pXMLOut)<<"</var>";
				}
				else
				{
					(*pXMLOut)<<"</arr_line>";
				}
				levels->pop();
			
				CSaveFileLevel currLevel=levels->top();
				levels->pop();
				currLevel.iCurrLevelPos++;
				levels->push(currLevel);
			}
		}
	}
	case ATOM_ARRAY:
		if(levels->top().iArrayLevel==0 && levels->top().iCurrLevelPos==0)
		{
			(*pXMLOut)<<"</var>";
			(*pBinaryOut)<<CHUNK_END;
			break;
		}
		else
		{
			if(levels->top().iArrayLevel==0 && levels->top().iCurrLevelPos>=levels->top().levelSizes[0])
			{
				CSaveFileLevel myLevel=levels->top();
				levels->pop();
				myLevel.iCurrLevelPos=0;
				levels->push(myLevel);
			}
			levels->top().bArrayEntryClosed=true;
			return;
		}
	}
	levels->pop();
}

};