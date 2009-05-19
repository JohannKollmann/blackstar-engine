#include "..\..\..\Dependencies\corona\include\corona.h"
#include <sstream>
#include <iostream>
#include <fstream>

const char* szHelp="USAGE\n\
-----\n\
\n\
TextTexture [text_width_in_units] [char_widths_file] [text_file] [output_png]\n\
\n\
the output file is in the png format. the appropriate suffix will be set automatically.\n\
char widths file syntax:\n\
DELIMITER is [^0-9], \r\n treated as \n.\n\
the font texture consists of 16x16 congruent squares.\n\
[UNITS_PER_FONT_TEXTURE_TILE]\n\
DELIMITER [CHAR0_BEGIN_UNIT] DELIMITER [CHAR0_END_UNIT]\n\
DELIMITER [CHAR1_BEGIN_UNIT] DELIMITER [CHAR1_END_UNIT]\n\
...\n\
DELIMITER [CHAR255_BEGIN_UNIT] DELIMITER [CHAR255_END_UNIT]\n\
\n\
text has an escape character ('$'). after it the color for the following text is set in hex as argb.\n\
outputting the escape character itself is possible by doubling it (\"$$\").\n";

struct SCharInfo
{
	char cLetter;
	unsigned short wColor;
	int iLine;
	int iOffset;
	int iWidth;
};

int
HexCharToNumber(char c)
{
	int r=-1;
	if(c>='a' && c<='f')
		r=10+c-'a';
	if(c>='0' && c<='9')
		r=c-'0';
	return r;
}

void main(int argc, char** argv)
{
	if(argc!=5)
	{
		std::cout<<szHelp;
		return;
	}
	std::ifstream widthsFile(argv[2], std::ios_base::ate);
	std::stringstream sstr;
	sstr<<argv[1];
	int nWidth=0;
	sstr>>nWidth;

	if(nWidth<1 || !widthsFile.good())
	{
		std::cout<<szHelp;
		return;
	}

	//parse the file
	//first check file size
	int iFileSize=widthsFile.tellg();
	widthsFile.seekg(0, std::ios_base::beg);
	//reserve some mem and load it
	char* pcFile=new char[iFileSize+1];
	int iChar;
	for(iChar=iFileSize>>1; iChar<iFileSize+1; iChar++)
		pcFile[iChar]='\0';//set half the array to 0, since \r\n is parsed as \n
	widthsFile.read(pcFile, iFileSize);
	widthsFile.close();
	//set up an array for the char widths
	int aaiCharWidths[2][256];
	int iCurrWidth;
	for(iCurrWidth=0; iCurrWidth<256; iCurrWidth++)
	{
		aaiCharWidths[0][iCurrWidth]=0;
		aaiCharWidths[1][iCurrWidth]=0;
	}
	iCurrWidth=0;
	//read units per tile
	sstr.clear();
	iChar=0;
	while('0'<=pcFile[iChar] && pcFile[iChar]<='9')
	{
		sstr<<pcFile[iChar];
		iChar++;
	}
	int nUnitsPerTile=0;
	sstr>>nUnitsPerTile;
	if(nUnitsPerTile==0)
	{
		std::cout<<"parsing \"units per tile\" failed\n";
		return;
	}
	do
	{
		iChar++;//jump over the delimiter
		if(iChar>iFileSize-1)
			break;

		sstr.clear();
		//read start
		int iStartChar=iChar;
		while('0'<=pcFile[iChar] && pcFile[iChar]<='9')
		{
			sstr<<pcFile[iChar];
			iChar++;
		}
		sstr>>aaiCharWidths[0][iCurrWidth];
		if(iStartChar==iChar || aaiCharWidths[0][iCurrWidth]<0 || aaiCharWidths[0][iCurrWidth]>nUnitsPerTile)
			break;
		iChar++;//jump over the delimiter
		if(iChar>iFileSize-1)
			break;

		sstr.clear();
		//read end
		iStartChar=iChar;
		while('0'<=pcFile[iChar] && pcFile[iChar]<='9')
		{
			sstr<<pcFile[iChar];
			iChar++;
		}
		sstr>>aaiCharWidths[1][iCurrWidth];
		if(iStartChar==iChar || aaiCharWidths[1][iCurrWidth]<0 || aaiCharWidths[1][iCurrWidth]>nUnitsPerTile)
			break;
		//increase position
		iCurrWidth++;
	}while(iChar<=iFileSize && iCurrWidth<256);

	if(iCurrWidth!=256)
	{
		std::cout<<"error parsing char widths file! (char# ";
		std::cout<<iChar;
		std::cout<<", parsed ";
		std::cout<<iCurrWidth;
		std::cout<<" of 256 char widths)\n";
		return;
	}
	delete pcFile;
	//read the text file
	std::ifstream textFile(argv[3], std::ios_base::ate);
	if(!textFile.good())
	{
		std::cout<<szHelp;
		return;
	}
	iFileSize=textFile.tellg();
	textFile.seekg(0, std::ios_base::beg);
	pcFile=new char[iFileSize+1];
	for(iChar=iFileSize>>1; iChar<iFileSize+1; iChar++)
		pcFile[iChar]='\0';//set half the array to 0, since \r\n is parsed as \n
	textFile.read(pcFile, iFileSize);
	textFile.close();
	for(iChar=iFileSize-1; iChar>0 && pcFile[iChar]=='\0'; iChar--);
	iFileSize=iChar+1;
	
	//parse the text
	//set up all information we need to draw the texture
	SCharInfo* pChars=new SCharInfo[iFileSize];
	int nLines=0;
	int iCurrLinePos=0;
	unsigned short wCurrColor=0xf;
	
	for(iChar=0; iChar<iFileSize; iChar++)
	{
		//treat some special characters..
		switch(pcFile[iChar])
		{
		case '$':
			if(pcFile[iChar+1]=='$')//the escape character itself
			{
				//make that escape char disappear
				pChars[iChar].cLetter=' ';
				pChars[iChar].iLine=nLines;
				pChars[iChar].wColor=wCurrColor;
				pChars[iChar].iWidth=0;//actually only this is important
				pChars[iChar].iOffset=iCurrLinePos;
				iChar++;
			}
			else
			{
				wCurrColor=(HexCharToNumber(pcFile[iChar+1])<<12) |
							(HexCharToNumber(pcFile[iChar+2])<<8) |
							(HexCharToNumber(pcFile[iChar+3])<<4) |
							HexCharToNumber(pcFile[iChar+4]);
				//make these 5 letters disappear
				for(int iLetter=0; iLetter<5; iLetter++)
				{
					pChars[iChar+iLetter].cLetter=' ';
					pChars[iChar+iLetter].iLine=nLines;
					pChars[iChar+iLetter].wColor=wCurrColor;
					pChars[iChar+iLetter].iWidth=0;//actually only this is important
					pChars[iChar+iLetter].iOffset=iCurrLinePos;
				}
				iChar+=4;
				continue;
			}
			break;
		case '\t':
		{
			int iTabWidth=4*(aaiCharWidths[1][' ']-aaiCharWidths[0][' ']);
			pChars[iChar].iOffset=iCurrLinePos;
			iCurrLinePos/=iTabWidth;
			iCurrLinePos=(iCurrLinePos+1)*iTabWidth;
			pChars[iChar].cLetter='\t';
			pChars[iChar].iLine=nLines;
			pChars[iChar].wColor=wCurrColor;
			pChars[iChar].iWidth=iCurrLinePos-pChars[iChar].iOffset;
		}
		case ' '://fall through from '\t'
		{
			//make nice word wraps ;)
			if(!iCurrLinePos)
				break;//we already are in a new line, no good idea to think about wrapping here

			int iWordLength=(pcFile[iChar]==' ') ? aaiCharWidths[1][' ']-aaiCharWidths[0][' '] : 0;
			for(int iSearchPos=iChar+1; iSearchPos<iFileSize &&
				pcFile[iSearchPos]!=' ' && pcFile[iSearchPos]!='\n' && pcFile[iSearchPos]!='\t'; iSearchPos++)
				iWordLength+=aaiCharWidths[1][pcFile[iSearchPos]]-aaiCharWidths[0][pcFile[iSearchPos]];
			//now we got the size of the next word, test if it fits in the current line
			if(iCurrLinePos+iWordLength>=nWidth)
			//doesn't fit.. wrap around. do this by replacing the ' ' or '\t' by '\n'
				pcFile[iChar]='\n';
			else
			{
				if(pcFile[iChar]==' ')
					break;
				else
					continue;
			}
		}
			//fall through if we replaced the letter
		case '\n':
			iCurrLinePos=0;
			pChars[iChar].iOffset=0;
			pChars[iChar].iWidth=0;
			pChars[iChar].iLine=0;
			nLines++;
			continue;
		}
		if(iChar>=iFileSize)//we jumped over the ending of the text..
			break;
		int iLetterWidth=aaiCharWidths[1][pcFile[iChar]]-
			aaiCharWidths[0][pcFile[iChar]];

		//break the line if the letter does not fit
		if((iCurrLinePos+iLetterWidth)>nWidth)
		{
			nLines++;
			iCurrLinePos=0;
		}
		pChars[iChar].cLetter=pcFile[iChar];
		pChars[iChar].iLine=nLines;
		pChars[iChar].wColor=wCurrColor;
		pChars[iChar].iWidth=iLetterWidth;
		pChars[iChar].iOffset=iCurrLinePos;

		iCurrLinePos+=iLetterWidth;
	}
	if(iCurrLinePos)
		nLines++;//make it the number of lines instead of an index

	//now draw the texture into mem

	unsigned int* pdwPixels=new unsigned int[nLines*nWidth];
	//pixel format:
	//ccoorgba
	//c: char, o: offset in char, rgba: color
	//ATTENTION: we can't use UINTs directly because of endian issues (or at least not properly)

	//init the whole thing to the first unit of blank
	for(int iPixel=0; iPixel<nLines*nWidth; iPixel++)
		pdwPixels[iPixel]=0;

	int dbgLine=0;
	for(iChar=0; iChar<iFileSize; iChar++)
	{
		int iBaseIndex=pChars[iChar].iLine*nWidth+pChars[iChar].iOffset;

		if(dbgLine<pChars[iChar].iLine)
		{
			for(int iLine=0; iLine<pChars[iChar].iLine-dbgLine; iLine++)
				std::cout<<"\n";
			dbgLine=pChars[iChar].iLine;
		}
		if(pChars[iChar].cLetter=='\t')
		{
			//put some ' ' in there
			for(int iUnit=0; iUnit<pChars[iChar].iWidth; iUnit++)
			{
				int iSpaceWidth=aaiCharWidths[1][' ']-aaiCharWidths[0][' '];
				if((iUnit+iSpaceWidth-pChars[iChar].iOffset)%iSpaceWidth==0)
					std::cout<<" ";
				pdwPixels[iBaseIndex+iUnit]=' ' |
											(((iUnit+iSpaceWidth-pChars[iChar].iOffset)%iSpaceWidth+aaiCharWidths[0][' '])<<8) |
											((pChars[iChar].wColor&0xff)<<24) |
											((pChars[iChar].wColor&0xff00)<<8);
			}
			continue;
		}
		//not '\t'
		if(pChars[iChar].iWidth)
			std::cout<<pChars[iChar].cLetter;
		for(int iUnit=0; iUnit<pChars[iChar].iWidth; iUnit++)
		{
			pdwPixels[iBaseIndex+iUnit]=pChars[iChar].cLetter |
										((iUnit+aaiCharWidths[0][pChars[iChar].cLetter])<<8) |
										((pChars[iChar].wColor&0xff)<<24) |
										((pChars[iChar].wColor&0xff00)<<8);
		}
	}
	corona::Image *image = corona::CreateImage(nWidth,
		nLines,
		corona::PF_R8G8B8A8,
		pdwPixels);

	corona::SaveImage((std::string(argv[4]) + std::string(".png")).c_str(), corona::FF_PNG, image);
	delete pdwPixels;
/*
	*/
}